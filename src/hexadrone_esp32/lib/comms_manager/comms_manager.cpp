#include "comms_manager.h"

void CommsManager::begin(const char *ssid, const char *password)
{
    _ssid = ssid;
    _password = password;

    _server.on("/log", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        if (LittleFS.exists("/log.txt")) {
            request->send(LittleFS, "/log.txt", "text/plain", true); 
        } else {
            request->send(404, "text/plain", "---Log file not found.---");
        } });

    ArduinoOTA.onStart([]()
                       { Serial.println("\n[OTA] Update Starting."); });
    ArduinoOTA.onEnd([]()
                     { Serial.println("\n[OTA] Update Complete."); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                          { Serial.printf("[OTA] Progress: %u%%\r", (progress / (total / 100))); });
    ArduinoOTA.onError([](ota_error_t error)
                       { Serial.printf("[OTA] Error[%u]\n", error); });

    enableWiFi(_ssid, _password);
}

void CommsManager::update()
{
    if (!_wifiEnabled)
        return;

    if (_state == CommsState::CONNECTING)
    {
        if (millis() - _lastDotTime >= _dotInterval)
        {
            Serial.print(".");
            _lastDotTime = millis();
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            _state = CommsState::CONNECTED;

            _server.begin();
            ArduinoOTA.begin();

            Serial.println("\n[COMMS] WiFi Connected Successfully.");
            Serial.print("[COMMS] ESP32 IP Address: ");
            Serial.println(WiFi.localIP());

            if (MDNS.begin("hexadrone"))
            {
                Serial.println("[COMMS] mDNS started at: http://hexadrone.local");
            }
            else
            {
                Serial.println("[COMMS] Error starting mDNS.");
            }
        }
    }
    else if (_state == CommsState::CONNECTED)
    {
        ArduinoOTA.handle();

        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("\n[COMMS] WiFi Signal lost. Attempting to reconnect...");
            _state = CommsState::CONNECTING;
            WiFi.disconnect();
            WiFi.reconnect();
        }
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
        Serial.println("[COMMS] WiFi Disabled (Radio Safe Mode).");
    }
}

void CommsManager::enableWiFi(const char *ssid, const char *password)
{
    if (!_wifiEnabled)
    {
        Serial.printf("[COMMS] Connecting to %s.", ssid);

        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);

        _wifiEnabled = true;
        _state = CommsState::CONNECTING;
        _lastDotTime = millis();
    }
}
