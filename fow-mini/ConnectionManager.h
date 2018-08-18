/*
    Copyright (c) 2017 Declan Freeman-Gleason. All rights reserved.

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
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>

class ConnectionManager {
  public:
    ConnectionManager(String programName);

    bool ready();
    void update();
    String get();
  private:
    /*
        DNS doesn't work with WiFi client in this code. This bug took me two weeks to fix. I ended up using HTTPClient though
        because of its TCP connection reuse, and convinient request processing (you can manipulate headers and it separates
        the body for you).
    */

    // TODO: Make ip/host/port configurable so we don't brick the ferries when/if these change
    const String host = "fow.nalcad.tk";
    const String path = "/progress";
    const int port = 80;
    const unsigned long timeout = 8000; // In milleseconds

    ESP8266WebServer server = ESP8266WebServer(80);
    SettingsManager settingsManager = SettingsManager();
    HTTPClient client;

    String name;
    String ssid = "";
    String password = "";
    bool setupMode = true;

    void connectToWiFiNetwork();
};

#endif
