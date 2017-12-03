#include "ConnectionManager.h"

ConnectionManager::ConnectionManager(String programName) : name(programName) {
  Serial.begin(115200);
  delay(10);

  Serial.println("\nStarting the ferry connection configuration WiFi AP...");

  // Setup in soft access point mode
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(name.c_str());

  // Tell the http client to allow reuse if the server supports it (we make lots of requests, this should decrease overhead)
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
                String("<html><body style='background-color: white; font-size: 12px; font-family: monospace;'>Network Name: ") + ssid +
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

void ConnectionManager::update() {
  server.handleClient();
}

String ConnectionManager::get() {
  // If we're probably not connected, don't do anything (We have to check for ssid beacuse WL_CONNECTED isn't always reliable)
  if (ssid == "" || WiFi.status() != WL_CONNECTED) {
    Serial.println("GET aborted, ssid is blank or WiFi isn't connected.");
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
