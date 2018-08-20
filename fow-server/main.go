/*
   Copyright (c) 2017, 2018 Declan Freeman-Gleason. All rights reserved.

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
	"fmt"
	"log"
	"net/http"
	"os"
	"strings"
	"sync"
	"time"

	flag "github.com/ogier/pflag"
	"github.com/pietroglyph/go-wsf"
)

type ferryData struct {
	locations   *wsf.VesselLocations
	lastUpdated time.Time
	updateMux   sync.Mutex
}

type configuration struct {
	accessCode       string
	bind             string
	terminal         int
	updateFrequency  float64
	idleAfter        float64
	routeWidthFactor float64
	testingMode      bool
	minimumFerries   int
}

var data *ferryData
var config configuration

func main() {
	config = configuration{}
	flag.StringVarP(&config.accessCode, "accesscode", "c", "", "WSDOT Traveller Information API key (provisioned at https://wsdot.wa.gov/traffic/api/)") // Required
	flag.StringVarP(&config.bind, "bind", "b", "localhost:8000", "Host IP and port for the webserver to run on.")
	flag.IntVarP(&config.terminal, "terminal", "t", 3, "Terminal to track ferries to and from.") // 3 is Bainbridge Island
	flag.Float64VarP(&config.updateFrequency, "update", "u", 15, "Frequency in seconds to update data from the REST API.")
	flag.Float64VarP(&config.idleAfter, "idle", "i", 60, "Time in seconds after an update to stop updating.")
	flag.Float64VarP(&config.routeWidthFactor, "width", "w", 300, "The 'width' factor of the route, this determines how far away the ferry can be to still be considered on route")
	flag.BoolVarP(&config.testingMode, "fake", "f", false, "Output a fake value so that you don't have to wait for the ferries while they're docked.")
	flag.IntVarP(&config.minimumFerries, "minimum-ferries", "m", 2, "The server will ensure that it returns values (default or otherwise) for at least this number of ferries.")
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

	log.Println("Flags parsed.")

	client := wsf.NewClient(nil)
	client.AccessCode = config.accessCode
	client.UserAgent = "fow-mini/100"

	data = &ferryData{}
	go data.keepUpdated(client)

	log.Println("Trying to bind to", config.bind+"...")

	http.HandleFunc("/progress", progressHandler)
	http.ListenAndServe(config.bind, nil)
}
