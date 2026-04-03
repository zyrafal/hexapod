#include "comms_manager.h"

void CommsManager::begin(const char *ssid, const char *password)
{
    Serial.printf("[COMMS] Connecting to %s", ssid);

    enableWiFi(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\n[COMMS] WiFi Connected Successfully.");
    Serial.print("[COMMS] ESP32 IP Address: ");
    Serial.println(WiFi.localIP());

    if (MDNS.begin("hexadrone"))
    {
        Serial.println("[COMMS] mDNS started at:");
        Serial.println("[COMMS] http://hexadrone.local");
    }
    else
    {
        Serial.println("[COMMS] Error starting mDNS.");
    }

    _server.on("/log", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        if (LittleFS.exists("/log.txt")) {
            request->send(LittleFS, "/log.txt", "text/plain", true); 
        } else {
            request->send(404, "text/plain", "Log file not found. Run a test first!");
        } });

    _server.begin();

    ArduinoOTA.onStart([]()
                       { Serial.println("\n[OTA] Update Starting..."); });
    ArduinoOTA.onEnd([]()
                     { Serial.println("\n[OTA] Update Complete!"); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                          { Serial.printf("[OTA] Progress: %u%%\r", (progress / (total / 100))); });
    ArduinoOTA.onError([](ota_error_t error)
                       { Serial.printf("[OTA] Error[%u]\n", error); });

    ArduinoOTA.begin();
}

void CommsManager::update()
{
    if (_wifiEnabled)
    {
        ArduinoOTA.handle();
    }
}

void CommsManager::disableWiFi()
{
    if (_wifiEnabled)
    {
        WiFi.mode(WIFI_OFF);
        _wifiEnabled = false;
        Serial.println("WiFi Disabled (Radio Safe Mode).");
    }
}

void CommsManager::enableWiFi(const char *ssid, const char *password)
{
    if (!_wifiEnabled)
    {
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
        Serial.print("Connecting to WiFi");

        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20)
        {
            delay(500);
            Serial.print(".");
            attempts++;
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("\nWiFi Connected!");
            Serial.print("Robot IP Address: ");
            Serial.println(WiFi.localIP());
            _wifiEnabled = true;
        }
        else
        {
            Serial.println("\nWiFi Failed to connect.");
        }
    }
}
