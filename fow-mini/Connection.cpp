#include "Connection.h"

Connection::Connection(char* programName) : name(programName) {
  server = ESP8266WebServer(80);
  Serial.begin(115200);

  Serial.println("\r\nStarting the ferry connection configuration WiFi AP...");

  // Setup in soft access point mode
  WiFi.softAP(name);

  // Start a mDNS responder so that users can connect easily
  Serial.printf("MDNS responder initalization has %s.\n", MDNS.begin(name) ? "been successful" : "failed");

  // In case the MDNS responder can't start
  Serial.printf("Server local IP is %s.\n", WiFi.softAPIP().toString().c_str());

  // Handle requests to the base path by showing a simple config page
  server.on("/", [&]() {
    server.send(200, "text/html", configPage);
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
    server.send(200, "text/plain",
                String("Status: \n\tNetwork Name: ") + ssid +
                "\n\tPassword: " + password +
                "\n\tConnection Status: " + connStatus);
  });

  server.on("/netconfig", [&]() {
    if (!server.hasArg("ssid") || !server.hasArg("password")) {
      return;
    }
    ssid = server.arg("ssid");
    password = server.arg("password");
    server.send(200, "text/plain", "Done.");
    WiFi.mode(WIFI_AP_STA);
    WiFi.disconnect();
    WiFi.begin(ssid.c_str(), password.c_str());
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
      if (millis() - startTime > timeout) {
          
      }
      delay(500);
      Serial.print(".");
    }
    Serial.print("\n");
    this->get();
  });
  // Handle requests to the /config path by changing configuration
  server.begin();

  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);

  Serial.println("HTTP server has been started.");
}

void Connection::update() {
  server.handleClient();
}

String Connection::get() {
  if (!client.connect(host, port)) {
    return "";
  }
  client.print(String("GET ") + path + " HTTP/1.1\n" +
  "Host: " + host + "\r\n" +
  "Connection: close\n\n");

  unsigned long startTime = millis();
  while (!client.available()) {
    if (millis() - startTime > timeout) {
      client.stop();
      return "";
    }
  }

  while (client.available()) {
    Serial.println(client.readStringUntil('\n'));
  }
  return "";
}
