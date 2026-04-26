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
    bool isOTALockdown() const { return _otaLockdown; }

private:
    CommsState _state = CommsState::DISCONNECTED;
    Hexadrone::DroneState _lastDroneState = Hexadrone::DroneState::DRONE_ARMED;

    String _ssid;
    String _password;

    unsigned long _lastDotTime = 0;
    const unsigned long _dotInterval = 500;

    AsyncWebServer _server{80};
    bool _wifiEnabled = false;
    bool _serversStarted = false;
    bool _otaLockdown = false;
};
