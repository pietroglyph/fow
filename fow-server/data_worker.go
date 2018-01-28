/*
   Copyright (c) 2017 Declan Freeman-Gleason. All rights reserved.

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

func (d *ferryData) keepUpdated(wsfClient *wsf.Client) {
	for {
		time.Sleep(time.Duration(config.updateFrequency) * time.Second)
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
