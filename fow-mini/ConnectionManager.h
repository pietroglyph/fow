#ifndef ConnectionManager_h
#define ConnectionManager_h

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>

class ConnectionManager {
public:
  ConnectionManager(String programName);

  void update();
  String get();
private:
  /* 
   *  DNS doesn't work with WiFi client in this code. This bug took me two weeks to fix. I ended up using HTTPClient though
   *  because of its TCP connection reuse, and convinient request processing (you can manipulate headers and it separates
   *  the body).
  */
  // TODO: Make ip/host/port configurable so we don't brick the ferries when/if these change
  const String host = "fow.nalcad.tk";
  const String path = "/progress";
  const int port = 80;
  const unsigned long timeout = 8000; // In milleseconds
  const String configPage = R"(<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>Mini-FOW Configuration</title>
  <style>
    body {
      font-family: sans-serif;
      width: 100%;
    }
    iframe {
      border: none;
      background-image:url("data:image/svg+xml;utf8,<svg xmlns='http://www.w3.org/2000/svg' version='1.1' height='100%' width='100%'><text x='0' y='15' fill='black' font-size='20'>Loading status information...</text></svg>");
    }
  </style>
</head>
<body>
  <h1>Mini Ferries Over Winslow Configuration</h1>
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
  )";

  ESP8266WebServer server = ESP8266WebServer(80);
  HTTPClient client;

  String name;
  String ssid;
  String password;
};

#endif
