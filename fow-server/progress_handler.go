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
	length: 0,
}

var lastRequested time.Time

func progressHandler(w http.ResponseWriter, r *http.Request) {
	if config.testingMode {
		now := time.Now().UnixNano() / (int64(time.Millisecond) / int64(time.Nanosecond))
		rotatingProgressNow := (math.Sin(float64(time.Now().Unix())/30) * 0.5) + 0.5
		rotatingProgressFuture := (math.Sin((float64(time.Now().Unix())+config.updateFrequency)/30) * 0.5) + 0.5
		fmt.Fprint(w, formatOutput(rotatingProgressNow, rotatingProgressFuture, now), ":", formatOutput(rotatingProgressNow, rotatingProgressFuture, now), ":", config.updateFrequency)
	}

	// Give dummy data until the data is no longer nil or stale
	if time.Now().Sub(data.lastUpdated).Seconds() > config.idleAfter || data.locations == nil {
		lastRequested = time.Now()

		seattleBainbridgePath.updateLength()
		fmt.Fprint(w, formatOutput(0, 0, 0), ":", formatOutput(1, 1, 0), ":-1")
		return
	}
	lastRequested = time.Now()

	var ferriesFound int

	data.updateMux.Lock()
	sort.Sort(byDepartingID(*data.locations))
	for _, v := range *data.locations {
		if v.DepartingTerminalID != config.terminal && v.ArrivingTerminalID != config.terminal {
			continue
		}

		ferriesFound++

		if v.AtDock || !v.InService {
			dockedProgress := 0
			if v.ArrivingTerminalID == config.terminal {
				dockedProgress = 1
			}
			fmt.Fprint(w, formatOutput(dockedProgress, dockedProgress, 0), ":")
		} else {
			fmt.Fprint(w,
				formatOutput(
					seattleBainbridgePath.progress(&v, time.Duration(0)*time.Second),
					seattleBainbridgePath.progress(&v, time.Duration(config.updateFrequency)*time.Second),
					int64(time.Now().Sub(time.Time(v.TimeStamp))/time.Millisecond),
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

func (p *ferryPath) progress(vesselLoc *wsf.VesselLocation, durationAhead time.Duration) float64 {
	var cumulativeDistanceTravelled float64
	var closestSegment int
	var subClosestSegmentProgress float64 // The progress of the ferry along the closest segment

	// Interpolate forward in time using the heading and the speed
	distanceAhead := (durationAhead.Hours() * vesselLoc.Speed) * 1.852001 // We use this magic number to convert to kM/h
	interpolatedCoordinate := convertGeoPoint(geo.NewPoint(vesselLoc.Latitude, vesselLoc.Longitude).PointAtDistanceAndBearing(distanceAhead, vesselLoc.Heading))

	// Find the closest point
	closestSegment = -1
	smallestDistanceToSegment := -1.0
	for i, v := range seattleBainbridgePath.coords {
		if i <= 0 {
			continue
		}
		var slope geom.Coord
		// Get the negative reciprocal of the slope of the segment we're testing against
		// so that the tester segment is perpendicular if it intersects
		slope.X = (seattleBainbridgePath.coords[i-1].Minus(v).Y * config.routeWidthFactor) * -1
		slope.Y = (seattleBainbridgePath.coords[i-1].Minus(v).X * config.routeWidthFactor) * -1
		intersectionTestSegment := geom.Segment{A: interpolatedCoordinate.Plus(slope), B: interpolatedCoordinate.Minus(slope)}
		p, ok := intersectionTestSegment.Intersection(&geom.Segment{A: seattleBainbridgePath.coords[i-1], B: v})
		if ok {
			distanceToSegment := p.DistanceFrom(interpolatedCoordinate)
			if distanceToSegment < smallestDistanceToSegment || smallestDistanceToSegment == -1.0 {
				smallestDistanceToSegment = distanceToSegment
				closestSegment = i
				subClosestSegmentProgress = p.DistanceFrom(seattleBainbridgePath.coords[i-1])
			}
		}
	}

	if closestSegment == -1 {
		log.Println("Ferry is not on path, consider increasing the width flag's value")
		return 0
	}

	for i := 1; i < closestSegment; i++ {
		cumulativeDistanceTravelled += seattleBainbridgePath.coords[i].DistanceFrom(seattleBainbridgePath.coords[i-1])
	}
	cumulativeDistanceTravelled += subClosestSegmentProgress

	return math.Min(cumulativeDistanceTravelled/seattleBainbridgePath.length, 1) // Make sure that we don't return anything larger than 1
}

func (p *ferryPath) updateLength() {
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

// We sort by the unique vessel ID, because almost everything else might switch at some time
func (a byDepartingID) Len() int           { return len(a) }
func (a byDepartingID) Swap(i, j int)      { a[i], a[j] = a[j], a[i] }
func (a byDepartingID) Less(i, j int) bool { return a[i].VesselID < a[j].VesselID }
