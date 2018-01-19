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

#include "ConnectionManager.h"

const static char configPage[] PROGMEM = R"(<!DOCTYPE html>
<html>
<!doctype html>
<html>
<head>
<meta charset="UTF-8">
<title>(Mini) Ferries Over Winslow Configuration</title>
</head>
<body>

<style>
  html {
    height: 100%;}
  body {
    font-family: Gotham, "Helvetica Neue", Helvetica, Arial, "sans-serif";
    color: white;
      background-size: cover;
      background-repeat:no-repeat;
    background: #44A08D;  /* fallback for old browsers */
    background: -webkit-linear-gradient(#093637, #44A08D);
    background: -o-linear-gradient(#093637, #44A08D);
    background: radial-gradient(#44A08D, #093637);  /* Chrome 10-25, Safari 5.1-6 */ /* W3C, IE 10+/ Edge, Firefox 16+, Chrome 26+, Opera 12+, Safari 7+ */ }
  h1 {
    font-size: 200%;
    text-align: center;}
  h2 {
    font-size: 150%;
    text-align: left;}
  p  {
    font-size: 100%;
    text-align: center;
    opacity: .5;}
  form  {
    font-size: 100%;
    text-align: left;}
  hr {width:80%;}
  iframe {
    border: none;
    height: 5em;
    }
</style>
  <h1>FerryClock Setup</h1>
  <p>Get FerryClock motoring within your local wireless network</p>
  <hr>
    <h2>Status</h2>
    <iframe src="/status" width="100%"></iframe>
    <h2>Network Configuration</h2>
    <form method="POST" action="/">
    Network Name: <input type="text" name="ssid">
    <br>
    Password: <input type="password" name="password">
    <br>
    <input type="submit" value="Apply">
    </form>
</body>
</html>
  )";

ConnectionManager::ConnectionManager(const String programName) : name(programName) {
  Serial.begin(115200);
  
  Serial.println("\nStarting the ferry connection configuration WiFi AP...");

  // Setup in soft access point mode
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(name.c_str());

  // Tell the http client to allow reuse if the server supports it (we make lots of requests to the same server, this should decrease overhead)
  client.setReuse(true);

  // Start a mDNS responder so that users can connect easily
  Serial.printf("MDNS responder initalization has %s.\n", MDNS.begin(name.c_str()) ? "been successful" : "failed");

  // In case the MDNS responder can't start
  Serial.printf("Server local IP is %s.\n", WiFi.softAPIP().toString().c_str());

  // Handle requests to the base path by showing a simple config page
  server.on("/", [&]() {
    server.send(200, "text/html", configPage);

    if (server.hasArg("ssid") || server.hasArg("password")) {
      ssid = server.arg("ssid");
      password = server.arg("password");

      WiFi.disconnect();
      WiFi.begin(ssid.c_str(), password.c_str());

      unsigned long startTime = millis();
      while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime > timeout) {
          Serial.print("\nWiFi connection attempt timed out.\n");
          return;
        }
        delay(500);
        Serial.print(".");
      }
      Serial.printf("\nConnected to WiFi with a private IP of %s.\n", WiFi.localIP().toString().c_str());
    }
  });

  server.on("/status", [&]() {
    String connStatus;
    switch (WiFi.status()) {
      case WL_CONNECTED :
        if (ssid == "" || password == "") {
          connStatus = "Disconnected"; // We get WL_CONNECTED only with an AP connection, so this kind of fixes that
          break;
        }
        connStatus = "Connected";
        break;
      case WL_CONNECT_FAILED :
        connStatus = "Connection attempt failed";
        break;
      case WL_CONNECTION_LOST :
        connStatus = "Connection lost";
        break;
      case WL_DISCONNECTED :
        connStatus = "Disconnected";
        break;
      default :
        connStatus = "Other";
        break;
    }
    server.send(200, "text/html",
                String("<html><body style='color: white; font-size: 14px; font-family: monospace;'>Network Name: ") + ssid +
                "<br>Password: " + password +
                "<br>Connection Status: " + connStatus +
                "</body></html>");
  });

  server.on("/rawresponse", [&]() {
    server.send(200, "text/plain", this->get());
  });

  // Handle requests to the /config path by changing configuration
  server.begin();

  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", port);

  Serial.println("HTTP server has been started.");
}

boolean ConnectionManager::ready() {
  if (ssid == "" && password == "")
    return false;
  else if (WiFi.status() == WL_CONNECTED)
    return true;
}

void ConnectionManager::update() {
  server.handleClient();
}

String ConnectionManager::get() {
  // If we're probably not connected, don't do anything (We have to check for ssid beacuse WL_CONNECTED isn't always reliable)
  if (ssid == "" || WiFi.status() != WL_CONNECTED) {
    Serial.println("GET aborted, ssid is blank and/or WiFi isn't connected.");
    return "";
  }

  client.begin(host, port, path);

  int statusCode = client.GET();
  if (statusCode != HTTP_CODE_OK) {
    Serial.printf("Remote server returned a non-OK status code of %i.\n", statusCode);
    return "";
  }

  return client.getString(); // This only returns the response body.
}
