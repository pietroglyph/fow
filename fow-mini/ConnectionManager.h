#ifndef ConnectionManager_h
#define ConnectionManager_h

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

class ConnectionManager {
public:
  ConnectionManager(char* programName);

  void update();
  String get();
private:
  const char* host = "fow.naclad.tk";
  const char* path = "/progress";
  const int port = 80;
  const unsigned long timeout = 8000; // In milleseconds
  const char* configPage = R"(<!DOCTYPE html>
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

  ESP8266WebServer server;
  WiFiClient client;

  char* name;
  String ssid;
  String password;
};

#endif
