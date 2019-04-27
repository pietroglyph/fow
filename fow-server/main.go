/*
   Copyright (c) 2017-2019 Declan Freeman-Gleason.

   This file is part of Ferries Over Winslow.

   Ferries Over Winslow is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Ferries Over Winslow is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this Ferries Over Winslow.  If not, see <http://www.gnu.org/licenses/>.
*/

package main

import (
	"bufio"
	"crypto/md5"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"path/filepath"
	"strings"
	"sync"
	"time"

	flag "github.com/ogier/pflag"
	"github.com/pietroglyph/go-wsf"
)

type ferryData struct {
	locations   *wsf.VesselLocations
	lastUpdated time.Time
	updateMux   sync.RWMutex
}

type configuration struct {
	accessCode               string
	bind                     string
	terminal                 int
	updateFrequency          float64
	idleAfter                float64
	subdividedSegmentMaxSize float64
	minimumFerries           int
	debugMode                bool
	debugPagePath            string
	maxDataStaleness         float64
	updatesDirectory         string
}

var (
	data        *ferryData
	config      configuration
	updateFiles = make(map[string]updateInfo)
)

func main() {
	config = configuration{}
	flag.StringVarP(&config.accessCode, "accesscode", "c", "", "WSDOT Traveller Information API key (provisioned at https://wsdot.wa.gov/traffic/api/)") // Required
	flag.StringVarP(&config.bind, "bind", "b", "localhost:8000", "Host IP and port for the webserver to run on.")
	flag.IntVarP(&config.terminal, "terminal", "t", 3, "Terminal to track ferries to and from.") // 3 is Bainbridge Island
	flag.Float64VarP(&config.updateFrequency, "update", "u", 15, "Frequency in seconds to update data from the REST API.")
	flag.Float64VarP(&config.idleAfter, "idle", "i", 60, "Time in seconds after an update to stop updating.")
	flag.Float64VarP(&config.subdividedSegmentMaxSize, "segment-size", "s", 10e-6, "The minimum size of a segment on the subdivided reference ferry path. The smaller the smoother and more accurate the estimates.")
	flag.IntVarP(&config.minimumFerries, "minimum-ferries", "m", 2, "The server will ensure that it returns values (default or otherwise) for at least this number of ferries.")
	flag.BoolVar(&config.debugMode, "debug", false, "Serve a debugging page on /debug.")
	flag.StringVar(&config.debugPagePath, "debug-path", "./debug.html", "Path to the debug.html file.")
	flag.Float64VarP(&config.maxDataStaleness, "max-staleness", "S", 18, "Maximum staleness of WSF data, in seconds (setting this to nonnegative values will enable staleness compensation, which may increase update frequency)")
	flag.StringVarP(&config.updatesDirectory, "updates-dir", "U", "", "Specify a directory where updates are kept. Their names should be in the format \"<VERSION>@<UPDATE CHANNEL>:<HARDWARE REVISION>:<TYPE>.bin\"")
	flag.Parse()

	// accesscode flag is required
	if config.accessCode == "" {
		log.Fatal("Please specify an access code using the -c flag")
	}

	// We only have the ferryPathPoints for the Seattle-Bainbridge route
	if config.terminal != 3 {
		fmt.Print("Processing location data is only implemented for Terminal ID 3, continue (y/N)? ")
		stdin := bufio.NewScanner(os.Stdin)
		stdin.Scan()
		if strings.ToLower(stdin.Text()) != "y" {
			return
		}
	}
	currentPath = seattleBainbridgePath.getProcessedPath() // This is the only available path, but we could switch based on a flag if we wanted to later

	log.Println("Flags parsed")

	client := wsf.NewClient(nil)
	client.AccessCode = config.accessCode
	client.UserAgent = "fow-server (https://github.com/pietroglyph/fow)"

	data = &ferryData{}
	go data.keepUpdated(client)

	log.Println("Trying to bind to", config.bind+"...")

	if config.debugMode {
		log.Println("Serving debug information under /debug")
		http.HandleFunc("/debug", func(w http.ResponseWriter, r *http.Request) {
			http.ServeFile(w, r, config.debugPagePath)
		})
		http.HandleFunc("/debug/get/", debugHandler)
		http.HandleFunc("/debug/path/coords", pathCoordInfoHandler)
	}
	if config.updatesDirectory != "" {
		paths, err := filepath.Glob(filepath.Clean(config.updatesDirectory) + "/*.bin")
		if err != nil {
			log.Fatal(err)
		}
		for _, path := range paths {
			filename := filepath.Base(path)
			filename = strings.TrimSuffix(filename, filepath.Ext(filename))

			var split []string
			var info updateInfo
			var key string

			split = strings.Split(filename, "@")
			if len(split) < 2 {
				log.Println("Update name \"" + filename + "\" is invalid")
				continue
			}
			info.version = split[0]
			key = split[1]

			if _, keyAlreadySet := updateFiles[key]; keyAlreadySet {
				log.Fatal("Duplicate update files for key \"" + key + "\". Cannot continue!")
			}

			split = strings.Split(split[1], ":")
			if len(split) < 2 {
				log.Println("Update channel and hardware revision part \"" + split[1] + "\" is invalid")
				continue
			}

			info.channel = split[0]
			info.hardwareRevision = split[1]
			info.file, err = os.Open(path)
			if err != nil {
				log.Println("Couldn't open update file at", path+":", err)
				continue
			}

			fileContents, err := ioutil.ReadAll(info.file)
			if err != nil {
				log.Println("Couldn't read update file ("+path+") to compute md5 sum:", err)
				continue
			}
			info.md5String = fmt.Sprintf("%x", md5.Sum(fileContents))
			_, err = info.file.Seek(0, 0)
			if err != nil {
				log.Println("Couldn't seek to beginning of file at", path)
			}

			updateFiles[key] = info
		}
		http.HandleFunc("/update", updateHandler)
	}
	http.HandleFunc("/progress", progressHandler)
	log.Panicln(http.ListenAndServe(config.bind, nil))
}
