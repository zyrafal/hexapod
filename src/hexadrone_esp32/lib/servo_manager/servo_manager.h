
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

    // Physical wiring order on PCA9685 boards → logical leg index (LF=0,LM=1,LB=2,RF=3,RM=4,RB=5)
    const int legRemap[6] = {1, 4, 3, 0, 5, 2}; // LM, RM, RF, LF, RB, LB

    // Sign map indexed by physical slot (LM,RM,RF,LF,RB,LB). Order per leg: Coxa,Femur,Tibia.
    const float signMap[6][3] = {
        { 1.0f,  1.0f,  1.0f}, // LM (Positive Down)
        { 1.0f, -1.0f, -1.0f}, // RM (Negative Down)
        { 1.0f, -1.0f, -1.0f}, // RF
        { 1.0f,  1.0f,  1.0f}, // LF
        { 1.0f, -1.0f, -1.0f}, // RB
        { 1.0f,  1.0f,  1.0f}, // LB
    };

    // Prone position in radians, applied before sign map. Physical order: LM,RM,RF,LF,RB,LB.
    const float proneOffset[6][3] = {
        { 0.00f, -0.70f,  0.45f}, // LM
        { 0.00f,  0.70f, -0.45f}, // RM
        { 0.00f, -0.70f,  0.45f}, // RF
        { 0.00f,  0.70f, -0.45f}, // LF
        { 0.00f, -0.70f,  0.45f}, // RB
        { 0.00f,  0.70f, -0.45f}, // LB
    };

    void setRawPWM(int index, int pulse);
    int degreesToTicks(float degrees);
};
