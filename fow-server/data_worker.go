package main

import (
	"log"
	"time"

	"github.com/pietroglyph/go-wsf"
)

func (d *ferryData) keepUpdated(wsfClient *wsf.Client) {
	for {
		if time.Now().Sub(d.lastUpdated).Seconds() < float64(config.updateFrequency) {
			continue
		} else if time.Now().Sub(lastRequested).Seconds() >= float64(config.idleAfter) {
			continue
		}
		d.lastUpdated = time.Now()
		vesselLocations, err := wsfClient.Vessels.VesselLocations()
		if err != nil {
			log.Println(err.Error())
			continue
		}

		d.updateMux.Lock()
		d.locations = vesselLocations
		d.updateMux.Unlock()
		log.Println("Location data updated.")
	}
}
