package main

import (
	"testing"
	"time"

	geo "github.com/kellydunn/golang-geo"
	wsf "github.com/pietroglyph/go-wsf"
)

var processedPath *processedFerryPath

func TestMain(_ *testing.M) {
	processedPath = seattleBainbridgePath.getProcessedPath()
}

func TestConvertGeoPoint(t *testing.T) {
	lat := 4915.0
	lng := 9374.0
	coord := convertGeoPoint(geo.NewPoint(lat, lng))
	if coord.X != lat || coord.Y != lng {
		t.Error("Latitude and longitude conversion to geo.Point failed.")
	}
}

func TestProgressAtBounds(t *testing.T) {
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

	if oneProgress != 1 || zeroProgress != 0 {
		t.Error("Incorrect progress values at path extants.")
	}
}
