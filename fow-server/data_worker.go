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
	"log"
	"time"

	"github.com/pietroglyph/go-wsf"
)

const likelyRequestDelayMs = 0.3 // Sync fudge factor

// The below code may wait a maximum of config.updateFrequency seconds before actually making an initial request!
func (d *ferryData) keepUpdated(wsfClient *wsf.Client) {
	// We do a ticker instead of time.Sleep because a ticker will always be on time,
	// whereas sleep would drift by the duration of the remainder of the for loop,
	// which would be mean drift by however long it takes to make a request to the
	// WSF API.
	ticker := makeNewTicker()
	for {
		lastRequestTimeMux.Lock()
		if time.Now().Sub(lastRequestTime).Seconds() >= float64(config.idleAfter) {
			lastRequestTimeMux.Unlock()
			<-ticker.C
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

		// Attempt to keep ourselves relatively synchronized with the WSF server
		// (they say they update every 15 seconds, which means wost case is 30
		// seconds off, which isn't really great and makes for a jerky user experience)
		// XXX: Assumes that the TimeStamp of vessel 0 is the same as that of all others
		if config.maxDataStaleness > 0 && len(*vesselLocations) >= 1 {
			requestDurationAgo := time.Now().Sub(time.Time((*vesselLocations)[0].TimeStamp))
			if requestDurationAgo > time.Duration(config.maxDataStaleness)*time.Second {
				log.Println("Data is stale by", requestDurationAgo, "seconds, catching up...")
				time.Sleep(time.Duration(requestDurationAgo.Seconds()+likelyRequestDelayMs) * time.Second)
				ticker = makeNewTicker()
				continue
			}
		}
		<-ticker.C
	}
}

func makeNewTicker() *time.Ticker {
	return time.NewTicker(time.Duration(config.updateFrequency) * time.Second)
}
