package main

import (
	"fmt"
	"net/http"
	"strconv"
	"strings"
	"time"

	wsf "github.com/pietroglyph/go-wsf"
)

func debugHandler(w http.ResponseWriter, r *http.Request) {
	splitPath := strings.Split(r.URL.Path, "/")
	// There's an extra / at the beginning of the path
	if len(splitPath) != 4 {
		http.Error(w, "Invalid URL path.", 400)
		return
	}

	latLng := strings.Split(splitPath[3], ",")
	if len(latLng) != 2 {
		http.Error(w, "Invalid latitude and longitude argument (it should be two comma delineated numbers.)", 400)
		return
	}

	latitude, err := strconv.ParseFloat(latLng[0], 64)
	if err != nil {
		http.Error(w, "Couldn't parse latitude into a double.", 400)
		return
	}
	longitude, err := strconv.ParseFloat(latLng[1], 64)
	if err != nil {
		http.Error(w, "Couldn't parse longitude into a double.", 400)
		return
	}

	progress, err := seattleBainbridgePath.progress(&wsf.VesselLocation{Latitude: latitude, Longitude: longitude, Speed: 16.5, Heading: 180}, time.Duration(0)*time.Second)
	if err != nil {
		http.Error(w, err.Error(), 404)
		return
	}
	w.Header().Set("Access-Control-Allow-Origin", "*")
	fmt.Fprint(w, progress)
}
