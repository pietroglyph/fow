package main

import (
	"io"
	"log"
	"net/http"
	"os"
	"path/filepath"
	"strconv"
	"strings"
)

const (
	versionHeader = "X-ESP8266-Version"
	typeQueryKey  = "type"
)

type updateInfo struct {
	version          string
	channel          string
	hardwareRevision string
	contentType      string // Usually "spiffs" or "flash"
	file             *os.File
	md5String        string // Hex encoded
}

func updateHandler(w http.ResponseWriter, r *http.Request) {
	if config.dumpUserAgents {
		log.Println("Got request on /updates: ", r.Header)
	}

	if r.URL.Query().Get(typeQueryKey) == "" {
		http.Error(w, "Invalid type query parameter", http.StatusBadRequest)
		return
	}

	updateInfo := strings.Split(r.Header.Get(versionHeader), "@")
	if len(updateInfo) < 2 {
		http.Error(w, versionHeader+" header is invalid", http.StatusBadRequest)
		return
	}

	selectedUpdateInfo, ok := updateFiles[updateInfo[1]+":"+r.URL.Query().Get(typeQueryKey)]
	if !ok {
		http.Error(w, "Update of that channel, hardware revision, or type not found", http.StatusBadRequest)
		return
	}

	if config.dumpUserAgents {
		log.Println("Got valid update check from", r.UserAgent(), "(Full version header is "+r.Header.Get(versionHeader)+")")
	}

	if selectedUpdateInfo.version == updateInfo[0] {
		http.Error(w, "No new updates available", http.StatusNotModified)
		return
	}

	_, err := selectedUpdateInfo.file.Seek(0, 0)
	if err != nil {
		http.Error(w, "Couldn't seek to the beginning of selected update file", http.StatusInternalServerError)
		return
	}

	fileInfo, err := selectedUpdateInfo.file.Stat()
	if err != nil {
		http.Error(w, "Coudln't get file status for selected update file", http.StatusInternalServerError)
		return
	}

	w.Header().Set("Content-Type", "application/octet-stream")
	w.Header().Set("Content-Disposition", "attachment; filename="+filepath.Base(selectedUpdateInfo.file.Name()))
	w.Header().Set("Content-Length", strconv.FormatInt(fileInfo.Size(), 10))
	w.Header().Set("X-MD5", selectedUpdateInfo.md5String)
	io.Copy(w, selectedUpdateInfo.file)
}
