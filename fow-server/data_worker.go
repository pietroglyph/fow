/*
   Copyright (c) 2017, 2018 Declan Freeman-Gleason.

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
	"log"
	"time"

	"github.com/pietroglyph/go-wsf"
)

// The below code may wait a maximum of config.updateFrequency seconds before actually making an initial request!
func (d *ferryData) keepUpdated(wsfClient *wsf.Client) {

	vesselLocations, err := wsfClient.Vessels.VesselLocations()

	if err != nil {
		log.Println(err)
	} else {
		for _, v := range *vesselLocations {
			if v.DepartingTerminalID != config.terminal && v.ArrivingTerminalID != config.terminal {
				continue
			}

			// We try to sychronize ourselves with the update rate of the WSP API server
			// We could try to sync exactly, but that has a roughly 50% chance of us
			// landing on the wrong side of things (e.g. we request just before the
			// serverside update). So we go in the middle.
			maxSyncWaitTime := time.Duration(config.updateFrequency/2) * time.Second
			time.Sleep((maxSyncWaitTime - (time.Now().Sub(time.Time(v.TimeStamp)) % maxSyncWaitTime)))
			break
		}
	}

	// We do a ticker instead of time.Sleep because a ticker will always be on time,
	// whereas sleep would drift by the duration of the remainder of the for loop,
	// which would be mean drift by however long it takes to make a request to the
	// WSF API.
	ticker := time.NewTicker(time.Duration(config.updateFrequency) * time.Second)
	for {
		lastRequestTimeMux.Lock()
		if time.Now().Sub(lastRequestTime).Seconds() >= float64(config.idleAfter) {
			lastRequestTimeMux.Unlock()
			continue
		}
		lastRequestTimeMux.Unlock()

		vesselLocations, err := wsfClient.Vessels.VesselLocations()
		updatedTime := time.Now()
		if err != nil {
			log.Println(err)
			continue
		}

		d.updateMux.Lock()
		d.lastUpdated = updatedTime
		d.locations = vesselLocations
		d.updateMux.Unlock()

		// We do this instead of the for range ticker.C pattern because we want to wait at the _end_
		<-ticker.C
	}
}
