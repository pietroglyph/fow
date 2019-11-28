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

#ifndef ConnectionManager_h
#define ConnectionManager_h

#include "SettingsManager.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266HTTPClient.h>
#include <FS.h>
#include <ESP8266httpUpdate.h>

#if !defined(VERSION) || !defined(BUILD_INFO) || !defined(HARDWARE_REVISION)
#define VERSION "unknown"
#define BUILD_INFO "Build info unknown"
#define HARDWARE_REVISION "unknown"
#endif

#ifndef UPDATE_CHANNEL
#define UPDATE_CHANNEL "production"
#endif

class ConnectionManager {
  public:
    ConnectionManager(String programName);

    bool ready();
    void update();
    String get();
    bool isConnectedToWiFi();
  private:
    const String baseURL = "http://bridge.ferries-over-winslow.org";
    const String progressPath = "/progress";
    const String updateFlashPath = "/update?type=flash";
    const String updateSPIFFSPath = "/update?type=spiffs";
    const String updateVersionHeader = VERSION + String("@") + UPDATE_CHANNEL + String(":") + HARDWARE_REVISION;

    // Below are all in milleseconds
    const unsigned long connectionTimeout = 20000;
    const unsigned long periodicReconnectDelay = 60000;
    const unsigned long updateCheckDelay = 60000 * 15;

    unsigned long lastUpdateAttempt = 0;
    unsigned long lastPeriodicReconnectAttempt = 0;

    ESP8266WebServer* server = new ESP8266WebServer(80);
    SettingsManager settingsManager;

    // The esp8266 library provides an mDNS server, but mDNS is flaky as hell so we just set up a captive portal
    DNSServer dnsServer;
    const unsigned int dnsPort = 53;

    WiFiClient wifiClient; // You need to have one of these for the HTTPClient to work (unless you use the deprecated API)
    HTTPClient http;

    String name;
    String chipIdStr;
    String ssid = "";
    String password = "";
    bool setupMode = true;
    bool connectionTimedOut = false;
    bool isConnecting = false;

    static String getContentType(String filename) {
      if (filename.endsWith(".htm")) return "text/html";
      else if (filename.endsWith(".html")) return "text/html";
      else if (filename.endsWith(".css")) return "text/css";
      else if (filename.endsWith(".js")) return "application/javascript";
      else if (filename.endsWith(".png")) return "image/png";
      else if (filename.endsWith(".gif")) return "image/gif";
      else if (filename.endsWith(".jpg")) return "image/jpeg";
      else if (filename.endsWith(".ico")) return "image/x-icon";
      return "text/plain";
    };

    void downloadAndFlashUpdates();
    bool handleRequestedFile(String path);
    void connectToWiFiNetwork(bool noTimeout = false);
};

#endif
