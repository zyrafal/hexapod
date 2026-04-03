#include <Arduino.h>
#include <Wire.h>
#include <comms_manager.h>
#include <radio_manager.h>
#include <servo_manager.h>
#include <power_manager.h>
#include <hexadrone_core/lib.hpp>
#include <config.h>

CommsManager comms;
RadioManager radio;
ServoManager servo;
PowerManager power;

void setup()
{
    Serial.begin(DEBUG_BAUD);
    Wire.begin(SDA_CUSTOM, SCL_CUSTOM);

    comms.begin(WIFI_SSID, WIFI_PASS);
    radio.begin();
    servo.begin();
    power.begin();

    Serial.println("Hexadrone Initialized.");
}

void loop()
{
    comms.update();
    radio.update();
    servo.update(radio.isArmed(), radio.isKillTriggered(), radio.isConnected());
    power.update(radio.getRSSI(), radio.getLQ());

    // Handle Debug Commands
    if (Serial.available())
    {
        char c = Serial.read();
        if (c == 'd')
            power.dumpLog();
        if (c == 'w')
            power.wipeLog();
    }
}
