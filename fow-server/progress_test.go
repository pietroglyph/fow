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
	"math"
	"sort"
	"testing"
	"time"

	geo "github.com/kellydunn/golang-geo"
	wsf "github.com/pietroglyph/go-wsf"
)

const unsortedErrorMsg = "vesselLocations were improperly sorted."

var processedPath *processedFerryPath

func TestConvertGeoPoint(t *testing.T) {
	lat := 4915.0
	lng := 9374.0
	coord := convertGeoPoint(geo.NewPoint(lat, lng))
	if coord.X != lat || coord.Y != lng {
		t.Error("Latitude and longitude conversion to geo.Point failed.")
	}
}

func TestSortByVesselId(t *testing.T) {
	vesselLocations := byVesselID([]wsf.VesselLocation{
		wsf.VesselLocation{VesselID: 4915},
		wsf.VesselLocation{VesselID: 0},
		wsf.VesselLocation{VesselID: 4915},
		wsf.VesselLocation{VesselID: 3},
	})

	sort.Sort(vesselLocations)
	for i, v := range vesselLocations {
		if (i == 0 && v.VesselID != 0) ||
			(i == 1 && v.VesselID != 3) ||
			((i == 2 || i == 3) && v.VesselID != 4915) {
			t.Error(unsortedErrorMsg)
		}
	}
}

func TestProgressAtBounds(t *testing.T) {
	if processedPath == nil {
		processedPath = seattleBainbridgePath.getProcessedPath()
	}

	zeroCoords := (*processedPath.originalPath)[0]
	oneCoords := (*processedPath.originalPath)[len(*processedPath.originalPath)-1]
	zeroProgress, err := processedPath.progress(&wsf.VesselLocation{Latitude: zeroCoords.X, Longitude: zeroCoords.Y}, (time.Duration)(0*time.Second))
	if err != nil {
		t.Error(err.Error())
	}
	oneProgress, err := processedPath.progress(&wsf.VesselLocation{Latitude: oneCoords.X, Longitude: oneCoords.Y}, (time.Duration)(0*time.Second))
	if err != nil {
		t.Error(err.Error())
	}

	// XXX: Path end values are off by ~3%... Is this because of floating point error?
	epsilonEqualsOrError(oneProgress, 1, 1E-1, "Incorrect value at end of path.", t)
	epsilonEqualsOrError(zeroProgress, 0, 1E-9, "Incorrect value at beginning of path.", t)
}

func epsilonEquals(a, b, epsilon float64) bool {
	return math.Abs(a-b) < epsilon
}

func epsilonEqualsOrError(a, b, epsilon float64, message string, t *testing.T) {
	if !epsilonEquals(a, b, epsilon) {
		t.Errorf("%s %f was not epsilon equal by %e to %f.", message, a, epsilon, b)
	}
}
