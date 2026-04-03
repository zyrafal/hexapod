#include "servo_manager.h"

void ServoManager::begin()
{
    pinMode(OE_CUSTOM, OUTPUT);
    digitalWrite(OE_CUSTOM, LOW);

    _board1.begin();
    _board1.setPWMFreq(60);
    _board2.begin();
    _board2.setPWMFreq(60);
}

void ServoManager::update(bool armSwitch, bool killTriggered, bool isConnected)
{
    // 1. Priority: Hardware Kill
    if (killTriggered)
    {
        if (_state != ServoState::KILLED)
        {
            rapidKill();
        }
        return;
    }

    // 2. State Transitions based on ArmSwitch and Connection
    if ((!isConnected || !armSwitch) && (_state == ServoState::ARMED || _state == ServoState::ARMING))
    {
        startSoftDisarm();
    }
    else if (isConnected && armSwitch && (_state == ServoState::DISARMED || _state == ServoState::DISARMING))
    {
        startArming();
    }

    // 3. Sequence Timer Logic
    if (millis() - _lastStepTime >= SERVO_ARM_INTERVAL)
    {
        if (_state == ServoState::ARMING)
        {
            setRawPWM(_seqIndex, calculatePulse(0)); // Move to neutral
            _seqIndex++;
            if (_seqIndex >= TOTAL_SERVOS)
            {
                _state = ServoState::ARMED;
                Serial.println("[SERVOS] All servos armed.");
            }
            _lastStepTime = millis();
        }
        else if (_state == ServoState::DISARMING)
        {
            setRawPWM(_seqIndex, 4096); // Relax
            _seqIndex++;
            if (_seqIndex >= TOTAL_SERVOS)
            {
                _state = ServoState::DISARMED;
                Serial.println("[SERVOS] All servos disarmed.");
            }
            _lastStepTime = millis();
        }
    }
}

void ServoManager::startArming()
{
    _state = ServoState::ARMING;
    _seqIndex = 0;
    _lastStepTime = millis();
    Serial.println("[SERVOS] Starting sequential arming...");
}

void ServoManager::startSoftDisarm()
{
    _state = ServoState::DISARMING;
    _seqIndex = 0;
    _lastStepTime = millis();
    Serial.println("[SERVOS] Starting soft-disarming sequence...");
}

void ServoManager::rapidKill()
{
    _state = ServoState::KILLED;
    digitalWrite(OE_CUSTOM, HIGH); // Pull OE HIGH (Physically disconnects all PWM)
    Serial.println("[SERVOS] Kill-Switch executed. System reboot required.");
}

// Routes the 0-31 index to the correct physical board
void ServoManager::setRawPWM(int index, int pulse)
{
    if (index < 16)
    {
        _board1.setPWM(index, 0, pulse);
    }
    else
    {
        _board2.setPWM(index - 16, 0, pulse);
    }
}

int ServoManager::calculatePulse(float degrees)
{
    return map(degrees, -90, 90, SERVOMIN, SERVOMAX);
}
