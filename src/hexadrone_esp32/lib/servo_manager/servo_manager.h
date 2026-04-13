
#pragma once
#include <Adafruit_PWMServoDriver.h>
#include <vector>
#include <config.h>

class ServoManager
{
public:
    void begin();

    // Apply 18 joint angles (degrees) from Brain output to physical servos.
    // No-op if the hardware kill has been triggered.
    void applyAngles(const std::vector<float> &angles_deg);

    // Pull OE HIGH: physically disconnects all PWM. Requires hardware reset to recover.
    void rapidKill();

private:
    Adafruit_PWMServoDriver _board1 = Adafruit_PWMServoDriver(ADDR_SERVO_1);
    Adafruit_PWMServoDriver _board2 = Adafruit_PWMServoDriver(ADDR_SERVO_2);

    bool _killed = false;

    void setRawPWM(int index, int pulse);
    int degreesToTicks(float degrees);
};
