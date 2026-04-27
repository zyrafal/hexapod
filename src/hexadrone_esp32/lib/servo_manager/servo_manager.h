#pragma once
#include <Adafruit_PWMServoDriver.h>
#include <vector>
#include <config.h>
#include <logger.h>
#include <hexadrone_core/state_machine.hpp>

class ServoManager
{
public:
    void begin();
    void update(Hexadrone::DroneState currentState);

    void applyAngles(const std::vector<float> &angles_deg);
    void rapidKill();

    bool isDisarming() const { return _isDisarming; }
    void setPostureState(Hexadrone::PostureState posture);

private:
    Adafruit_PWMServoDriver _board1 = Adafruit_PWMServoDriver(ADDR_SERVO_1);
    Adafruit_PWMServoDriver _board2 = Adafruit_PWMServoDriver(ADDR_SERVO_2);

    bool _killed = false;
    bool _pcaPresent = false;

    // Sequential Arming Trackers
    int _currentActive = 0;
    int _targetActive = 0;
    unsigned long _lastArmTime = 0;

    // Soft Cuttoff Trackers
    Hexadrone::DroneState _lastDroneState = Hexadrone::DroneState::DRONE_DISARMED;
    unsigned long _disarmTriggerTime = 0;
    bool _isDisarming = false;

    Hexadrone::PostureState _currentPosture = Hexadrone::PostureState::POSTURE_STANDARD;

    void setRawPWM(int index, int pulse);
    int degreesToTicks(float degrees);
};
