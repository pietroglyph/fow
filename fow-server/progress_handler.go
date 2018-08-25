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
	"errors"
	"fmt"
	"log"
	"math"
	"net/http"
	"sort"
	"time"

	geo "github.com/kellydunn/golang-geo"
	"github.com/pietroglyph/go-wsf"
	"github.com/skelterjohn/geom"
)

type ferryPath struct {
	coords []geom.Coord
	length float64
}

var seattleBainbridgePath = &ferryPath{
	coords: []geom.Coord{
		{X: 47.622453, Y: -122.509274},
		{X: 47.620197, Y: -122.498288},
		{X: 47.620009, Y: -122.497602},
		{X: 47.619546, Y: -122.496700},
		{X: 47.619170, Y: -122.496078},
		{X: 47.618331, Y: -122.495220},
		{X: 47.617825, Y: -122.494855},
		{X: 47.617116, Y: -122.494469},
		{X: 47.608176, Y: -122.491014},
		{X: 47.607757, Y: -122.490735},
		{X: 47.607163, Y: -122.490134},
		{X: 47.606643, Y: -122.489362},
		{X: 47.606353, Y: -122.488804},
		{X: 47.605934, Y: -122.487774},
		{X: 47.605688, Y: -122.486615},
		{X: 47.605471, Y: -122.484770},
		{X: 47.605326, Y: -122.482388},
		{X: 47.604169, Y: -122.352440},
		{X: 47.603069, Y: -122.343750},
		{X: 47.602869, Y: -122.342291},
		{X: 47.602824, Y: -122.339544},
	},
	length: -1, // calculateLength must be called to set this to the right value
}

var (
	lastRequested time.Time
	currentPath   *ferryPath // Set depending on the route set by the user
)

func progressHandler(w http.ResponseWriter, r *http.Request) {
	// Give dummy data until the data is no longer nil or stale
	if time.Now().Sub(data.lastUpdated).Seconds() > config.idleAfter || data.locations == nil {
		lastRequested = time.Now()

		fmt.Fprint(w, formatOutput(0, 0, 0), ":", formatOutput(1, 1, 0), ":-1")
		return
	}
	lastRequested = time.Now()

	var ferriesFound int

	data.updateMux.Lock()
	sort.Sort(byDepartingID(*data.locations))
	for _, boat := range *data.locations {
		if boat.DepartingTerminalID != config.terminal && boat.ArrivingTerminalID != config.terminal {
			continue
		}

		ferriesFound++

		if boat.AtDock || !boat.InService {
			dockedProgress := 0
			if boat.ArrivingTerminalID == config.terminal {
				dockedProgress = 1
			}
			fmt.Fprint(w, formatOutput(dockedProgress, dockedProgress, 0), ":")
		} else {
			progressNow, err := currentPath.progress(&boat, time.Duration(0)*time.Second)
			if err != nil {
				// We just print the error and use the progress value of 0 that ferryPath.progress returns
				log.Println(err.Error())
			}
			progressLater, err := currentPath.progress(&boat, time.Duration(config.updateFrequency)*time.Second)
			if err != nil {
				log.Println(err.Error())
			}
			fmt.Fprint(w,
				formatOutput(
					progressNow,
					progressLater,
					int64(time.Now().Sub(time.Time(boat.TimeStamp))/time.Millisecond),
				), ":")
		}
	}
	for i := ferriesFound; i < config.minimumFerries; i++ {
		var progress int
		if math.Mod(float64(i), 2) == 0 {
			progress = 0
		} else {
			progress = 1
		}
		fmt.Fprint(w, formatOutput(progress, progress, 0), ":")
	}
	fmt.Fprint(w, config.updateFrequency*1000) // 1000 converts to milliseconds, we don't use time constants because they are arbitrary
	data.updateMux.Unlock()
}

func formatOutput(one interface{}, two interface{}, three interface{}) string {
	return fmt.Sprint(one, ",", two, ",", three)
}

func (p *ferryPath) progress(vesselLoc *wsf.VesselLocation, durationAhead time.Duration) (float64, error) {
	var cumulativeDistanceTravelled float64
	var closestSegment int
	var subClosestSegmentProgress float64 // The progress of the ferry along the closest segment

	// Interpolate forward in time using the heading and the speed
	distanceAhead := (durationAhead.Hours() * vesselLoc.Speed) * 1.852001 // We use this magic number to convert to kM/h
	interpolatedCoordinate := convertGeoPoint(geo.NewPoint(vesselLoc.Latitude, vesselLoc.Longitude).PointAtDistanceAndBearing(distanceAhead, vesselLoc.Heading))

	// Find the our progress along the connected line segments of currentPath
	closestSegment = -1
	smallestDistanceToSegment := -1.0
	for i, v := range currentPath.coords {
		if i <= 0 {
			continue
		}
		var slope geom.Coord
		// Get the negative reciprocal of the slope of the segment we're testing against
		// so that the tester segment is perpendicular if it intersects
		slope.X = (currentPath.coords[i-1].Minus(v).Y * config.routeWidthFactor) * -1
		slope.Y = (currentPath.coords[i-1].Minus(v).X * config.routeWidthFactor) * -1
		intersectionTestSegment := geom.Segment{A: interpolatedCoordinate.Plus(slope), B: interpolatedCoordinate.Minus(slope)}
		intersectionPoint, ok := intersectionTestSegment.Intersection(&geom.Segment{A: currentPath.coords[i-1], B: v})
		if ok {
			distanceToIntersection := intersectionPoint.DistanceFrom(interpolatedCoordinate)
			if distanceToIntersection < smallestDistanceToSegment || smallestDistanceToSegment == -1.0 {
				smallestDistanceToSegment = distanceToIntersection
				closestSegment = i
				subClosestSegmentProgress = intersectionPoint.DistanceFrom(currentPath.coords[i-1])
			}
		}
		distanceToSegmentStart := v.DistanceFrom(interpolatedCoordinate)
		if distanceToSegmentStart < smallestDistanceToSegment || smallestDistanceToSegment == -1.0 {
			smallestDistanceToSegment = distanceToSegmentStart
			closestSegment = i
			subClosestSegmentProgress = 0
		}
	}

	if closestSegment == -1 {
		return 0, errors.New("Ferry is not on path, consider increasing the width flag's value")
	}

	for i := 1; i < closestSegment; i++ {
		cumulativeDistanceTravelled += currentPath.coords[i].DistanceFrom(currentPath.coords[i-1])
	}
	cumulativeDistanceTravelled += subClosestSegmentProgress

	return math.Min(cumulativeDistanceTravelled/currentPath.length, 1), nil // Make sure that we don't return anything larger than 1
}

func (p *ferryPath) calculateLength() {
	var length float64
	for i, v := range p.coords {
		if i <= 0 {
			continue
		}
		length += math.Sqrt(math.Pow(p.coords[i-1].X-v.X, 2) + math.Pow(p.coords[i-1].Y-v.Y, 2))
	}
	p.length = length
}

func convertGeoPoint(pnt *geo.Point) geom.Coord {
	return geom.Coord{X: pnt.Lat(), Y: pnt.Lng()}
}

type byDepartingID wsf.VesselLocations

func (a byDepartingID) Len() int           { return len(a) }
func (a byDepartingID) Swap(i, j int)      { a[i], a[j] = a[j], a[i] }
func (a byDepartingID) Less(i, j int) bool { return a[i].DepartingTerminalID < a[j].DepartingTerminalID }
