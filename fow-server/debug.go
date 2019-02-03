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
	"encoding/json"
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

	progress, err := currentPath.progress(&wsf.VesselLocation{Latitude: latitude, Longitude: longitude, Speed: 16.5, Heading: 180}, time.Duration(0)*time.Second)
	if err != nil {
		http.Error(w, err.Error(), 404)
		return
	}
	w.Header().Set("Access-Control-Allow-Origin", "*")
	fmt.Fprint(w, progress)
}

func pathCoordInfoHandler(w http.ResponseWriter, r *http.Request) {
	jsonBytes, err := json.Marshal(currentPath.originalPath)
	if err != nil {
		http.Error(w, "Couldn't marshal path coords into JSON.", 500)
		return
	}

	w.Header().Set("Content-Type", "application/json")
	w.Header().Set("Access-Control-Allow-Origin", "*")
	w.Write(jsonBytes)
}
