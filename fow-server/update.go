package main

import (
	"io"
	"net/http"
	"path/filepath"
	"strings"
)

const (
	versionHeader = "HTTP_X_ESP8266_VERSION"
	espUserAgent  = "ESP8266-http-Update"
	typeQueryKey  = "type"
)

func updateHandler(w http.ResponseWriter, r *http.Request) {
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
		http.Error(w, "Update of that channel, hardware revision, and type not found", http.StatusBadRequest)
		return
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

	w.Header().Set("Content-Type", "application/octet-stream")
	w.Header().Set("Content-Disposition", "attachment; filename="+filepath.Base(selectedUpdateInfo.file.Name()))
	w.Header().Set("X-MD5", selectedUpdateInfo.md5String)
	io.Copy(w, selectedUpdateInfo.file)
}
