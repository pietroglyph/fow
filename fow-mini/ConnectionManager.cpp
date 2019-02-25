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

#include "ConnectionManager.h"

const static char configPage[] PROGMEM = R"(
<!DOCTYPE html>
<html>
  <head>
  <meta charset="UTF-8">
  <title>(Mini) Ferries Over Winslow Configuration</title>
  </head>
  <body>
    <style>
      html {
        height: 100%;
      }
      body {
        font-family: Gotham, "Helvetica Neue", "Helvetica", "Arial", sans-serif;
        color: white;
        background-size: cover;
        background-repeat:no-repeat;
        background: #44A08D;
        background: -webkit-linear-gradient(#093637, #44A08D);
        background: -o-linear-gradient(#093637, #44A08D);
        background: radial-gradient(#44A08D, #093637);
        overflow: hidden;
      }
      h1 {
        font-size: 200%;
        text-align: center;
      }
      h2 {
        font-size: 150%;
        text-align: left;
      }
      p {
        font-size: 100%;
        text-align: center;
        opacity: .5;
      }
      form {
        font-size: 100%;
        text-align: left;
        display: flex;
        flex-flow: column wrap;
        width: 35%;
      }
      hr {
        width:80%;
      }
      iframe {
        border: none;
        height: 0px; /* Will be set by JavaScript */
        width: 100%;
        position: relative;
      }
      input {
        margin-bottom: 10px;
      }
      .apply-button {
        width: 50%;
        margin-top: 10px;
        padding: 2%;
      }
      .noscript {
        position: fixed;
        top: 0;
        left: 0;
        z-index: 1000;
        width: 100%;
        height: 100vh;
        text-align: center;
        background-color: red;
        font-weight: bold;
        font-size: 120%;
        opacity: 1;
        padding-top: 45vh;
        margin: 0;
      }
      .loading {
        filter: blur(10px);
      }
      .loading-container {
          display: flex;
          justify-content: center;
          align-items: center;
          flex-flow: column;
          position: absolute;
          top: 0;
          left: 0;
          right: 0;
          bottom: 0;
      }
      .loading-indicator {
        width: 10px;
        height: auto;
        font-size: 200%;
        text-align: center;
        -webkit-animation-name: spin;
        -webkit-animation-duration: 2000ms;
        -webkit-animation-iteration-count: infinite;
        -webkit-animation-timing-function: linear;
        -moz-animation-name: spin;
        -moz-animation-duration: 2000ms;
        -moz-animation-iteration-count: infinite;
        -moz-animation-timing-function: linear;
        -ms-animation-name: spin;
        -ms-animation-duration: 2000ms;
        -ms-animation-iteration-count: infinite;
        -ms-animation-timing-function: linear;
        animation-name: spin;
        animation-duration: 2000ms;
        animation-iteration-count: infinite;
        animation-timing-function: linear;
      }
      @-moz-keyframes spin {
          from { -moz-transform: rotate(0deg); }
          to { -moz-transform: rotate(360deg); }
      }
      @-webkit-keyframes spin {
          from { -webkit-transform: rotate(0deg); }
          to { -webkit-transform: rotate(360deg); }
      }
      @keyframes spin {
          from {transform:rotate(0deg);}
          to {transform:rotate(360deg);}
      }

    </style>
    <noscript><p class="noscript">The Ferries Over Winslow configuration page will not work without JavaScript enabled.</p></noscript>
    <div class="loading">
      <h1>FerryClock Setup</h1>
      <p>Get FerryClock motoring within your local wireless network</p>
      <hr>
      <h2>Status</h2>
      <!-- Single quotes are needed because double quotes would end this text block -->
      <iframe src="/status" onload='frameLoaded(this)'></iframe>
      <h2>Network Configuration</h2>
      <form method="POST" action="/">
        Network Name: <input type="text" name="ssid">
        <br>
        Password: <input type="password" name="password">
        <br>
        <input class="apply-button" type="submit" value="Apply">
      </form>
    </div>
    <div class="loading-container">
      <h1>Loading...</h1>
      <div class="loading-indicator">.</div>
    </div>
    <script>
      const CONFIRM_TEXT = "Connection successful! Would you like to exit setup mode?\n\nIf you do, the setup WiFi network will dissapear, and you will not be able to change any settings without doing a full reset (pressing the reset button 3 times)."
      window.onload = function(e) {
        fetch("/promptforexitsetup").then((response) => {
          if (response.ok && confirm(CONFIRM_TEXT)) {
            fetch("/exitsetup").then(() => alert("Successfully exited setup mode."));
          }
        });
      }

      function frameLoaded(frame) {
        frame.style.height = frame.contentWindow.document.body.scrollHeight + 'px';
        document.querySelector(".loading-container").style.display = "none";
        document.querySelector(".loading").classList.remove("loading");
        document.body.style.overflow = "auto";
      }
    </script>
  </body>
</html>
  )";

ConnectionManager::ConnectionManager(const String programName) : name(programName) {
  WiFi.disconnect();

  // Tell the http client to allow reuse if the server supports it (we make lots of requests to the same server, this should decrease overhead)
  client.setReuse(true);

  Serial.println("Checking EEPROM for saved WiFi credentials...");
  settingsManager.updateFullResetTimer();

  if (!settingsManager.isInSetupMode()) {
    setupMode = false;

    ssid = settingsManager.getSetting(SettingsManager::Setting::SSID);
    password = settingsManager.getSetting(SettingsManager::Setting::PASSWORD);

    Serial.printf("Saved credentials found. SSID: %s, Password: %s.\n", ssid.c_str(), password.c_str());

    WiFi.softAPdisconnect(true); // Make sure that we don't broadcast
    connectToWiFiNetwork();

    return;
  }
  setupMode = true;

  Serial.println("No saved credentials found. Starting the ferry connection configuration WiFi AP...");

  char apSSID[sizeof(name.c_str()) + sizeof(ESP.getChipId()) + 1] = {0};
  sprintf(apSSID, (name + "-%06X").c_str(), ESP.getChipId());

  // Setup in soft access point mode
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(apSSID);

  // Start a mDNS responder so that users can connect easily
  Serial.printf("MDNS responder initalization has %s.\n", MDNS.begin(name.c_str()) ? "been successful" : "failed");

  // In case the MDNS responder can't start
  Serial.printf("Server local IP is %s.\n", WiFi.softAPIP().toString().c_str());

  // Handle requests to the base path by showing a simple config page
  server->on("/", [&]() {
    server->send_P(HTTP_CODE_OK, "text/html", configPage);

    if (server->hasArg("ssid") || server->hasArg("password")) {
      ssid = server->arg("ssid");
      password = server->arg("password");
      ssid.remove(settingsManager.maximumSettingLength - 1);
      password.remove(settingsManager.maximumSettingLength - 1);

      connectToWiFiNetwork(server->hasArg("notimeout"));
    }
  });

  server->on("/status", [&]() {
    String connStatus;
    switch (WiFi.status()) {
      case WL_CONNECTED :
        if (ssid == "" || password == "") {
          connStatus = "Disconnected"; // We get WL_CONNECTED only with an AP connection, so this kind of deals with that
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
    server->send(HTTP_CODE_OK, "text/html",
                 String("<html><body style='color: white; font-size: 14px; font-family: monospace;'>Network Name: ") + ssid +
                 "<br>Password: " + password +
                 "<br>Connection Status: " + connStatus +
                 "</body></html>");
  });

  server->on("/promptforexitsetup", [&]() {
    bool shouldPrompt = isConnectedToWiFi();
    server->send(shouldPrompt ? HTTP_CODE_OK : HTTP_CODE_INTERNAL_SERVER_ERROR, "text/plain", shouldPrompt ? "true" : "false");
  });

  server->on("/exitsetup", [&]() {
    if (!isConnectedToWiFi()) return;

    server->send(HTTP_CODE_OK, "text/plain", "Exiting setup...");

    setupMode = false;

    WiFi.softAPdisconnect(true);

    settingsManager.setSetting(SettingsManager::Setting::SSID, ssid);
    settingsManager.setSetting(SettingsManager::Setting::PASSWORD, password);
    settingsManager.exitSetupMode();
  });

  // Handle requests to the /config path by changing configuration
  server->begin();

  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", port);
  Serial.println("HTTP server has been started.");
}

bool ConnectionManager::ready() {
  return isConnectedToWiFi();
}

void ConnectionManager::update() {
  settingsManager.updateFullResetTimer();

  if (setupMode) server->handleClient();
  // Periodically attempt to reconnect if we're not in setup mode, and still disconnected.
  else if (!isConnectedToWiFi() && lastPeriodicReconnectAttempt - millis() >= periodicReconnectDelay) connectToWiFiNetwork();
}

String ConnectionManager::get() {
  // If we're probably not connected, don't do anything (We have to check for ssid beacuse WL_CONNECTED isn't always reliable)
  if (!isConnectedToWiFi()) {
    Serial.println("GET aborted, ssid is blank and/or WiFi isn't connected.");
    return "";
  }

  int statusCode = client.GET();
  if (statusCode != HTTP_CODE_OK) {
    Serial.printf("Remote server returned a non-OK status code of %i.\n", statusCode);
    return "";
  }

  return client.getString(); // This only returns the response body.
}

// noTimeout == true will result in an infinite connect loop if the network really
// doesn't exist. We just provide it so the user has an out if their network takes
// forever to connect. The user should reset the microcontroller if they wish to
// escape the loop.
void ConnectionManager::connectToWiFiNetwork(bool noTimeout /*= false, see header*/) {
  if (noTimeout)
    Serial.println("We will connect with no timeout. Reset the microcontroller to escape the infinte loop");

  lastPeriodicReconnectAttempt = millis();

  client.end();
  WiFi.disconnect();
  WiFi.begin(ssid.c_str(), password.c_str());
  connectionTimedOut = false;

  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startTime > timeout && !noTimeout) {
      Serial.println("\nWiFi connection attempt timed out.");
      connectionTimedOut = true;
      return;
    }
    for (int i = 0; i < 500; i++) {
      delay(1);
      // This is a bit of a janky way to delay 500ms, but we really need to update the reset flag bits on time
      settingsManager.updateFullResetTimer();
    }
    Serial.print(".");
  }
  Serial.printf("\nConnected to WiFi with a private IP of %s.\n", WiFi.localIP().toString().c_str());

  // Make a connection to the remote server
  client.begin(host, port, path);
}

bool ConnectionManager::isConnectedToWiFi() {
  return !(ssid == "" || WiFi.status() != WL_CONNECTED || connectionTimedOut);
}
