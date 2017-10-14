package main

import (
	"bufio"
	"fmt"
	"log"
	"net/http"
	"os"
	"strings"
	"sync"

	flag "github.com/ogier/pflag"
	"github.com/pietroglyph/go-wsf"
)

type ferryData struct {
	locations *wsf.VesselLocations
	updateMux sync.Mutex
}

type configuration struct {
	accessCode       string
	bind             string
	terminal         int
	updateFrequency  int
	idleAfter        int
	routeWidthFactor float64
}

var data *ferryData
var config configuration

func main() {
	config = configuration{}
	flag.StringVarP(&config.accessCode, "accesscode", "c", "", "WSDOT Traveller Information API key (provisioned at https://wsdot.wa.gov/traffic/api/)") // Required
	flag.StringVarP(&config.bind, "bind", "b", "localhost:8000", "Host IP and port for the webserver to run on.")
	flag.IntVarP(&config.terminal, "terminal", "t", 3, "Terminal to track ferries to and from.") // 3 is Bainbridge Island
	flag.IntVarP(&config.updateFrequency, "update", "u", 15, "Frequency in seconds to update data from the REST API.")
	flag.IntVarP(&config.idleAfter, "idle", "i", 60, "Time in seconds after an update to stop updating.")
	flag.Float64VarP(&config.routeWidthFactor, "width", "w", 300, "The 'width' factor of the route, this determines how far away the ferry can be to still be considered on route")
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
