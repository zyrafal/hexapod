#include "servo_manager.h"
#include <Arduino.h>
#include <math.h>
#include <algorithm>
#include <hexadrone_core/state_machine.hpp>

void ServoManager::begin()
{
    pinMode(OE_CUSTOM, OUTPUT);
    digitalWrite(OE_CUSTOM, LOW);

    // Initialize the logic regardless
    _board1.begin();
    _board1.setPWMFreq(PWM_FREQ);
    _board2.begin();
    _board2.setPWMFreq(PWM_FREQ);

    // Ping the specific hardware addresses
    Wire.beginTransmission(ADDR_SERVO_1);
    byte err1 = Wire.endTransmission();

    Wire.beginTransmission(ADDR_SERVO_2);
    byte err2 = Wire.endTransmission();

    // Only log success if BOTH boards reply
    if (err1 == 0 && err2 == 0)
    {
        _pcaPresent = true;
        Blackbox.logSystem("[SERVOS] 2x PCA9685 Initialized Successfully.");
    }
    else
    {
        _pcaPresent = false;
        Blackbox.logSystem("[SERVOS] ERROR: PCA9685 Hardware Missing! (Err1:%d, Err2:%d)", err1, err2);
    }
}

void ServoManager::setPostureState(Hexadrone::PostureState posture) {
    _currentPosture = posture;
}

void ServoManager::applyAngles(const std::vector<float> &angles)
{
    if (angles.size() != 18 || !_pcaPresent)
        return;

    for (int logical_leg = 0; logical_leg < 6; ++logical_leg)
    {
        int phys_block = LOGICAL_TO_PHYSICAL[logical_leg];
        bool is_board_2 = (phys_block >= 4);
        int base_pin = (phys_block % 4) * 4;

        for (int joint = 0; joint < 3; ++joint)
        {
            int logical_idx = (logical_leg * 3) + joint;

            // 1. POWER GATE: Prevent fighting during disarm sequence
            if (logical_idx >= _currentActive)
                continue;

            float corrected_angle;
            if (_currentPosture == Hexadrone::PostureState::POSTURE_PRONE) {
                corrected_angle = angles[logical_idx] * -SERVO_SIGNS[logical_idx];
            } else {
                corrected_angle = angles[logical_idx] * SERVO_SIGNS[logical_idx];
            }

            uint16_t pwm_value = degreesToTicks(corrected_angle);

            if (is_board_2)
            {
                _board2.setPWM(base_pin + joint, 0, pwm_value);
            }
            else
            {
                _board1.setPWM(base_pin + joint, 0, pwm_value);
            }
        }
    }
}

void ServoManager::update(Hexadrone::DroneState currentState)
{
    if (_killed || !_pcaPresent)
        return;

    // --- Arming/Disarming powering sequence ---
    // 1. Edge Detection: Detect when the switch is flipped
    if (currentState != _lastDroneState)
    {
        if (currentState == Hexadrone::DroneState::DRONE_ARMED)
        {
            Blackbox.logSystem("[SERVOS] System ARMED: Beginning sequential power up.");
            _targetActive = 18;
            _isDisarming = false;
        }
        else if (currentState == Hexadrone::DroneState::DRONE_DISARMED)
        {
            Blackbox.logSystem("[SERVOS] System DISARMED: Lowering to prone posture...");
            _isDisarming = true;
            _disarmTriggerTime = millis();
        }
        _lastDroneState = currentState;
    }

    // 2. Soft Cutoff Delay
    if (_isDisarming && (millis() - _disarmTriggerTime >= DISARM_TRIGGER_INTERVAL))
    {
        _targetActive = 0;
        _isDisarming = false;

        Blackbox.logSystem("[SERVOS] Prone posture complete. De-energizing...");

        // Save all data now that the drone is stationary and safe
        Blackbox.flushSystem();
        Blackbox.flushPower();
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
    if (_killed) return;
    
    _killed = true;
    digitalWrite(OE_CUSTOM, HIGH); // Primary Hardware Kill

    // SECONDARY SOFTWARE KILL: Broadcast Full-Off (4096) to all channels
    // just in case the physical OE wire is disconnected.
    if (_pcaPresent) {
        for(int i = 0; i < 16; i++) {
            _board1.setPWM(i, 0, 4096);
            _board2.setPWM(i, 0, 4096);
        }
    }

    Blackbox.logSystem("[SERVOS] OE-Kill-switch condition met. Hardware reset required.");
    Blackbox.flushSystem();
    Blackbox.flushPower();
}

void ServoManager::setRawPWM(int global_servo_idx, int pulse)
{
    int logical_leg = global_servo_idx / 3;
    int joint = global_servo_idx % 3;

    int phys_block = LOGICAL_TO_PHYSICAL[logical_leg];
    bool is_board_2 = (phys_block >= 4);
    int target_pin = ((phys_block % 4) * 4) + joint;

    // PCA9685 Magic Number: 4096 triggers the "Full OFF" register bit
    int off_value = (pulse == 0) ? 4096 : pulse;

    if (is_board_2) {
        _board2.setPWM(target_pin, 0, off_value);
    } else {
        _board1.setPWM(target_pin, 0, off_value);
    }
}

int ServoManager::degreesToTicks(float degrees)
{
    // Physical servo range: -90° to +90°
    // The Core already clamps to +/- 90.0f, but we keep this as a final hardware failsafe.
    float clamped = std::max(-90.0f, std::min(90.0f, degrees));
    return (int)(SERVOMIN + (SERVOMAX - SERVOMIN) * (clamped + 90.0f) / 180.0f);
}
