#include "comms_manager.h"

void CommsManager::begin(const char *ssid, const char *password)
{
    _ssid = ssid;
    _password = password;

    _server.on("/blackbox", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        if (LittleFS.exists("/blackbox.txt")) {
            request->send(LittleFS, "/blackbox.txt", "text/plain", true); 
        } else {
            request->send(404, "text/plain", "---Blackbox file not found.---");
        } });

    ArduinoOTA.onStart([]()
                       { Serial.println("\n[OTA] Update Starting."); });
    ArduinoOTA.onEnd([]()
                     { Serial.println("\n[OTA] Update Complete."); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                          { Serial.printf("[OTA] Progress: %u%%\r", (progress / (total / 100))); });
    ArduinoOTA.onError([](ota_error_t error)
                       { Serial.printf("[OTA] Error[%u]\n", error); });
}

void CommsManager::update(Hexadrone::DroneState currentState)
{
    // 1. STATE MACHINE ALERTS (Run regardless of WiFi status)

    // Emergency Override!
    if (currentState == Hexadrone::DroneState::DRONE_OE_KILLED)
    {
        if (!_wifiEnabled)
        {
            Blackbox.println("[COMMS] OE-KILL detected: Enabling WiFi for emergency diagnostics.");
            enableWiFi(_ssid, _password);
        }
    }
    // Normal State Changes
    else if (currentState != _lastDroneState)
    {
        if (currentState == Hexadrone::DroneState::DRONE_ARMED)
        {
            Blackbox.println("[COMMS] System ARMED: Suppressing WiFi for Radio performance.");
            disableWiFi();
        }
        else if (currentState == Hexadrone::DroneState::DRONE_DISARMED)
        {
            Blackbox.println("[COMMS] System DISARMED: Booting WiFi for Diagnostics.");
            enableWiFi(_ssid, _password);
        }
        _lastDroneState = currentState;
    }

    // 2. NETWORK MANAGEMENT (Block this section if WiFi is supposed to be off)
    if (!_wifiEnabled)
        return;

    if (_state == CommsState::CONNECTING)
    {
        if (millis() - _lastDotTime >= _dotInterval)
        {
            _lastDotTime = millis();
            Blackbox.print(".");
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            _state = CommsState::CONNECTED;

            if (!_serversStarted)
            {
                _server.begin();
                ArduinoOTA.begin();
                _serversStarted = true;
            }

            Blackbox.println("\n[COMMS] WiFi Connected Successfully.");
            Blackbox.printf("[COMMS] ESP32 IP Address: %s\n", WiFi.localIP().toString().c_str());

            if (MDNS.begin("hexadrone"))
                Blackbox.println("[COMMS] mDNS started at: http://hexadrone.local");
            else
                Blackbox.println("[COMMS] Error starting mDNS.");
        }
    }
    else if (_state == CommsState::CONNECTED)
    {
        ArduinoOTA.handle();

        if (WiFi.status() != WL_CONNECTED)
        {
            _state = CommsState::CONNECTING;
            Blackbox.println("\n[COMMS] WiFi Signal lost. Attempting to reconnect...");
            WiFi.disconnect();
            WiFi.reconnect();
        }
    }
}

void CommsManager::enableWiFi(const char *ssid, const char *password)
{
    if (!_wifiEnabled)
    {
        Blackbox.printf("[COMMS] Connecting to %s.", ssid);

        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);

        _wifiEnabled = true;
        _state = CommsState::CONNECTING;
        _lastDotTime = millis();
    }
}

void CommsManager::disableWiFi()
{
    if (_wifiEnabled)
    {
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);

        _wifiEnabled = false;
        _state = CommsState::DISCONNECTED;

        Blackbox.println("[COMMS] WiFi Disabled (Radio Safe Mode).");
    }
}
