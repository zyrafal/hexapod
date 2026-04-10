#ifndef HEXADRONE_KINEMATICS_HPP
#define HEXADRONE_KINEMATICS_HPP

#include "hexadrone_core/state_machine.hpp"
#include <vector>

namespace Hexadrone
{
    class Kinematics
    {
    public:
        Kinematics();

        float applySignLogic(int leg_index, int joint_index, float value);
        std::vector<float> finalizeAngles(const std::vector<float> &base,
                                          const std::vector<float> &gait,
                                          const std::vector<float> &manual);
        std::vector<float> getBasePosture(PostureState posture);

    private:
        // 1.0 = Normal, -1.0 = Inverted
        // Order: Coxa, Femur, Tibia
        const float legSignMap[6][3] = {
            {1.0, 1.0, 1.0},   // LM (Left Middle - Positive Down)
            {1.0, -1.0, -1.0}, // RM (Right Middle - Negative Down)
            {1.0, -1.0, -1.0}, // RF (Right Front)
            {1.0, 1.0, 1.0},   // LF (Left Front)
            {1.0, -1.0, -1.0}, // RB (Right Back)
            {1.0, 1.0, 1.0}    // LB (Left Back)
        };
    };
}

#endif
