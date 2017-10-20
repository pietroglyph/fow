package main

import (
	"fmt"
	"log"
	"math"
	"net/http"
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
	if lastRequested.IsZero() || data.locations == nil {
		lastRequested = time.Now()

		// We haven't updated in a while, so we just give a zero value and wait for
		// the next request.
		seattleBainbridgePath.updateLength()
		fmt.Fprint(w, "0,0;", time.Now().Unix())
		return
	}
	lastRequested = time.Now()

	data.updateMux.Lock()
	var locationData wsf.VesselLocation
	for _, v := range *data.locations {
		if v.DepartingTerminalID != config.terminal {
			continue
		}
		// XXX: Assumes that only one ferry is departing a terminal at one time
		locationData = v
	}
	data.updateMux.Unlock()

	if locationData.AtDock || !locationData.InService {
		fmt.Fprint(w, "0,0;", time.Now().Unix())
		return
	}

	fmt.Fprint(w,
		seattleBainbridgePath.progress(&locationData, time.Duration(0)*time.Second),
		",",
		seattleBainbridgePath.progress(&locationData, time.Duration(config.updateFrequency)*time.Second),
		";",
		time.Time(locationData.TimeStamp).Unix(),
	)
}

func (p *ferryPath) progress(vesselLoc *wsf.VesselLocation, durationAhead time.Duration) float64 {
	var cumulativeDistanceTravelled float64
	var closestSegment int
	var subClosestSegmentProgress float64 // The progress of the ferry along the closest segment

	// Interpolate forward in time using the heading and the speed
	distanceAhead := durationAhead.Hours() * 1.852 // A knot is 1.852 KM/h
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

	return cumulativeDistanceTravelled / seattleBainbridgePath.length
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
