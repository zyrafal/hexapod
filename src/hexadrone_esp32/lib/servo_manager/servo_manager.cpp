#include "servo_manager.h"
#include <Arduino.h>
#include <math.h>
#include <algorithm>

void ServoManager::begin()
{
    pinMode(OE_CUSTOM, OUTPUT);
    digitalWrite(OE_CUSTOM, LOW);

    _board1.begin();
    _board1.setPWMFreq(PWM_FREQ);
    _board2.begin();
    _board2.setPWMFreq(PWM_FREQ);
}

void ServoManager::applyAngles(const std::vector<float> &angles_deg)
{
    int count = std::min({(int)angles_deg.size(), 18, _currentActive});
    for (int i = 0; i < count; i++)
    {
        int physLeg = i / 3;
        int joint = i % 3;
        int logicLeg = LEG_REMAP[physLeg];

        float angle_deg = angles_deg[logicLeg * 3 + joint];
        setRawPWM(i, degreesToTicks(angle_deg));
    }
}

void ServoManager::update(Hexadrone::DroneState currentState)
{
    if (_killed)
        return;

    // --- Arming/Disarming powering sequence ---
    // 1. Edge Detection: Detect when the switch is flipped
    if (currentState != _lastDroneState)
    {
        if (currentState == Hexadrone::DroneState::DRONE_ARMED)
        {
            Blackbox.println("[SERVOS] System ARMED: Beginning sequential power up.");
            _targetActive = 18;
            _isDisarming = false;
        }
        else if (currentState == Hexadrone::DroneState::DRONE_DISARMED)
        {
            Blackbox.println("[SERVOS] System DISARMED: Lowering to prone posture...");
            _isDisarming = true;
            _disarmTriggerTime = millis();
        }
        _lastDroneState = currentState;
    }

    // 2. Soft Cutoff Delay
    if (_isDisarming && (millis() - _disarmTriggerTime >= DISARM_TRIGGER_INTERVAL))
    {
        Blackbox.println("[SERVOS] Prone posture complete. De-energizing...");
        _targetActive = 0;
        _isDisarming = false;
    }

    // 3. Sequential Power Logic
    if (_currentActive != _targetActive)
    {
        if (millis() - _lastArmTime >= SERVO_ARM_INTERVAL)
        {
            if (_currentActive < _targetActive)
            {
                _currentActive++;
            }
            else
            {
                _currentActive--;
                setRawPWM(_currentActive, 0); // Decrement first because count is 1-18, but indices are 0-17
            }
            _lastArmTime = millis();
        }
    }
}

void ServoManager::rapidKill()
{
    if (_killed)
        return;

    _killed = true;
    digitalWrite(OE_CUSTOM, HIGH);
    Blackbox.println("[SERVOS] OE-Kill-switch condition met. Hardware reset required.");
    Blackbox.flush();
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
    // The Core already clamps to +/- 90.0f, but we keep this as a final hardware failsafe.
    float clamped = std::max(-90.0f, std::min(90.0f, degrees));
    return (int)(SERVOMIN + (SERVOMAX - SERVOMIN) * (clamped + 90.0f) / 180.0f);
}
