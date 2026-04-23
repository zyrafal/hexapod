#pragma once

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoOTA.h>
#include <LittleFS.h>
#include <ESPmDNS.h>
#include <logger.h>
#include <hexadrone_core/state_machine.hpp>

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
    void update(Hexadrone::DroneState currentState);

    void enableWiFi(const char *ssid, const char *password);
    void disableWiFi();

private:
    CommsState _state = CommsState::DISCONNECTED;
    // Initialize to ARMED so the first boot forces a state transition check
    Hexadrone::DroneState _lastDroneState = Hexadrone::DroneState::DRONE_ARMED;

    const char *_ssid;
    const char *_password;

    unsigned long _lastDotTime = 0;
    const unsigned long _dotInterval = 500;

    AsyncWebServer _server{80};
    bool _wifiEnabled = false;
    bool _serversStarted = false;
};
