#include "servo_manager.h"
#include <Arduino.h>
#include <math.h>
#include <algorithm>

void ServoManager::begin()
{
    pinMode(OE_CUSTOM, OUTPUT);
    digitalWrite(OE_CUSTOM, LOW);

    _board1.begin();
    _board1.setPWMFreq(60);
    _board2.begin();
    _board2.setPWMFreq(60);
}

void ServoManager::applyAngles(const std::vector<float> &angles_deg)
{
    if (_killed) return;

    int count = std::min((int)angles_deg.size(), TOTAL_SERVOS);
    for (int i = 0; i < count; i++)
    {
        setRawPWM(i, degreesToTicks(angles_deg[i]));
    }
}

void ServoManager::rapidKill()
{
    _killed = true;
    digitalWrite(OE_CUSTOM, HIGH);
    Serial.println("[SERVOS] Kill-switch executed. Hardware reset required.");
}

void ServoManager::setRawPWM(int index, int pulse)
{
    if (index < 16)
        _board1.setPWM(index, 0, pulse);
    else
        _board2.setPWM(index - 16, 0, pulse);
}

int ServoManager::degreesToTicks(float degrees)
{
    // Physical servo range: -90° to +90°
    float clamped = std::max(-90.0f, std::min(90.0f, degrees));
    return (int)(SERVOMIN + (SERVOMAX - SERVOMIN) * (clamped + 90.0f) / 180.0f);
}
