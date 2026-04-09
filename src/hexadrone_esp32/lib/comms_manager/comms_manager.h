#pragma once

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoOTA.h>
#include <LittleFS.h>
#include <ESPmDNS.h>

enum class CommsState
{
    DISCONNECTED,
    CONNECTING,
    CONNECTED
};

class CommsManager
{
public:
    void begin(const char *ssid, const char *password);
    void update();

    void disableWiFi();
    void enableWiFi(const char *ssid, const char *password);

private:
    CommsState _state = CommsState::DISCONNECTED;

    unsigned long _lastDotTime = 0;
    const unsigned long _dotInterval = 500;

    AsyncWebServer _server{80};
    bool _wifiEnabled = false;

    const char *_ssid;
    const char *_password;
};