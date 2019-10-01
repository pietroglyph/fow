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
	"errors"
	"fmt"
	"log"
	"math"
	"net/http"
	"sort"
	"sync"
	"time"

	geo "github.com/kellydunn/golang-geo"
	"github.com/pietroglyph/go-wsf"
	"github.com/skelterjohn/geom"
)

type ferryPath []geom.Coord
type processedFerryPath struct {
	originalPath     *ferryPath
	subdividedCoords []geom.Coord
	length           float64
}

type ferryDirection string

var seattleBainbridgePath = &ferryPath{
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
}

var (
	lastRequestTime    time.Time
	lastRequestTimeMux sync.Mutex
	currentPath        *processedFerryPath // Set depending on the route set by the user

	departing ferryDirection = "DEPARTING"
	arriving  ferryDirection = "ARRIVING"
)

func progressHandler(w http.ResponseWriter, r *http.Request) {
	if config.dumpUserAgents {
		log.Println("Progress request from", r.UserAgent())
	}

	data.updateMux.RLock()

	lastRequestTimeMux.Lock()
	lastRequestTime = time.Now()
	lastRequestTimeMux.Unlock()

	// Give dummy data until the data is no longer nil or stale
	if time.Now().Sub(data.lastUpdated).Seconds() > config.idleAfter || data.locations == nil {
		fmt.Fprint(w, formatOutput(0, 0, 0, departing), ":", formatOutput(1, 1, 0, arriving), ":-1")
		data.updateMux.RUnlock()
		return
	}

	// Add real boat progress info
	var ferriesFound int
	sort.Sort(byVesselID(*data.locations))
	for _, boat := range *data.locations {
		onRoute := false
		for _, routeName := range boat.OpRouteAbbrev {
			onRoute = onRoute || routeName == config.routeName
		}
		if !onRoute || !boat.InService {
			continue
		}

		ferriesFound++

		if boat.AtDock {
			dockedProgress := 0.0
			direction := departing
			if boat.ArrivingTerminalID == config.primaryTerminal || boat.DepartingTerminalID == config.secondaryTerminal {
				dockedProgress = 1.0
				direction = arriving
			}

			fmt.Fprint(w, formatOutput(dockedProgress, dockedProgress, 0.0, direction), ":")
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

			var direction ferryDirection
			if boat.DepartingTerminalID == config.primaryTerminal || boat.ArrivingTerminalID == config.secondaryTerminal {
				direction = departing
			} else {
				direction = arriving
			}

			fmt.Fprint(w,
				formatOutput(
					progressNow,
					progressLater,
					int64(time.Now().Sub(time.Time(boat.TimeStamp))/time.Millisecond),
					direction,
				), ":")
		}
	}
	data.updateMux.RUnlock()

	// Add extra dummy ferries if the route is understaffed
	for i := ferriesFound; i < config.minimumFerries; i++ {
		var progress float64
		var direction ferryDirection

		if math.Mod(float64(i), 2) == 0 {
			progress = 0.0
			direction = departing
		} else {
			progress = 1.0
			direction = arriving
		}
		fmt.Fprint(w, formatOutput(progress, progress, 0.0, direction), ":")
	}
	fmt.Fprint(w, config.updateFrequency*1000) // 1000 converts to milliseconds, we don't use time constants because they are arbitrary
}

func formatOutput(one interface{}, two interface{}, three interface{}, four ferryDirection) string {
	return fmt.Sprint(one, ",", two, ",", three, ",", string(four))
}

func (path *processedFerryPath) progress(vesselLoc *wsf.VesselLocation, durationAhead time.Duration) (float64, error) {
	var cumulativeDistanceTravelled float64
	var closestSegment int

	// Interpolate forward in time using the heading and the speed
	distanceAhead := (durationAhead.Hours() * vesselLoc.Speed) * 1.852001 // We use this magic number to convert to kM/h
	interpolatedCoordinate := convertGeoPoint(geo.NewPoint(vesselLoc.Latitude, vesselLoc.Longitude).PointAtDistanceAndBearing(distanceAhead, vesselLoc.Heading))

	// Find the our progress along the connected line segments of path
	closestSegment = -1
	smallestDistanceToSegment := -1.0
	for i, v := range path.subdividedCoords {
		distance := interpolatedCoordinate.DistanceFrom(v)
		if distance < smallestDistanceToSegment || smallestDistanceToSegment == -1 {
			closestSegment = i
			smallestDistanceToSegment = distance
		}
	}
	if closestSegment == -1 {
		return 0, errors.New("Reference path is invalid")
	}
	for i := 1; i < closestSegment; i++ {
		cumulativeDistanceTravelled += path.subdividedCoords[i].DistanceFrom(path.subdividedCoords[i-1])
	}

	return math.Min(cumulativeDistanceTravelled/path.length, 1), nil // Make sure that we don't return anything larger than 1
}

func (path *ferryPath) getProcessedPath() *processedFerryPath {
	var length float64
	processedPath := &processedFerryPath{
		originalPath: path,
	}
	for i, v := range *path {
		if i <= 0 {
			continue
		}

		rootCoord := (*path)[i-1]

		// Subdivide the path into a bunch of segments with a maximum length of config.subdividedSegmentMinSize
		processedPath.subdividedCoords = append(processedPath.subdividedCoords, rootCoord)
		segmentTotalLength := v.DistanceFrom(rootCoord)
		for s := 1; s < int(math.Ceil(segmentTotalLength/config.subdividedSegmentMaxSize)); s++ {
			subsegmentLengthPercentage := (config.subdividedSegmentMaxSize * float64(s)) / segmentTotalLength
			newCoord := v.Minus(rootCoord)
			newCoord.Scale(subsegmentLengthPercentage, subsegmentLengthPercentage)
			newCoord = newCoord.Plus(rootCoord)
			processedPath.subdividedCoords = append(processedPath.subdividedCoords, newCoord)
		}

		length += v.DistanceFrom(rootCoord)
	}
	processedPath.length = length
	return processedPath
}

func convertGeoPoint(pnt *geo.Point) geom.Coord {
	return geom.Coord{X: pnt.Lat(), Y: pnt.Lng()}
}

type byVesselID wsf.VesselLocations

func (a byVesselID) Len() int           { return len(a) }
func (a byVesselID) Swap(i, j int)      { a[i], a[j] = a[j], a[i] }
func (a byVesselID) Less(i, j int) bool { return a[i].VesselID < a[j].VesselID }
