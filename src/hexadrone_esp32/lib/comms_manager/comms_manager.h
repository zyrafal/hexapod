#pragma once

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoOTA.h>
#include <LittleFS.h>
#include <ESPmDNS.h>

class CommsManager
{
public:
    void begin(const char* ssid, const char* password);
    void update();
    void disableWiFi();
    void enableWiFi(const char *ssid, const char *password);

private:
    AsyncWebServer _server{80};
    bool _wifiEnabled = false;
};
