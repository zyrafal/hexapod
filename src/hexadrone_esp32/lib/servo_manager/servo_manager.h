
#pragma once
#include <Adafruit_PWMServoDriver.h>
#include <config.h>

enum class ServoState
{
    ARMING,    // Sequentially moving to 0 degrees
    DISARMING, // Sequentially moving to 4096 (relaxed)
    ARMED,     // All active
    DISARMED,  // All at 4096 (relaxed)
    KILLED     // Hardware OE pulled HIGH (Kill-switch)
};

class ServoManager
{
public:
    void begin();
    void update(bool armSwitch, bool killTriggered, bool isConnected); // TODO: Will be rewritten using Core IK

    void startArming();     // TODO: Will be rewritten using Core IK
    void startSoftDisarm(); // TODO: Will be rewritten using Core IK
    void rapidKill();

private:
    Adafruit_PWMServoDriver _board1 = Adafruit_PWMServoDriver(ADDR_SERVO_1);
    Adafruit_PWMServoDriver _board2 = Adafruit_PWMServoDriver(ADDR_SERVO_2);

    ServoState _state = ServoState::DISARMED;
    int _seqIndex = 0;
    unsigned long _lastStepTime = 0;

    void setRawPWM(int index, int pulse);
    int calculatePulse(float degrees);
};
