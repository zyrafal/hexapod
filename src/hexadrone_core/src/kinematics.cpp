#include "hexadrone_core/kinematics.hpp"

namespace Hexadrone
{
    Kinematics::Kinematics() {}

    /**
     * @brief Merges motion layers into final servo commands.
     * @param base The pre-signed posture layer (Hardware-ready).
     * @param gait The semantic gait offsets (Degrees).
     * @param manual The semantic trim offsets (Degrees).
     */
    std::vector<float> Kinematics::finalizeAngles(const std::vector<float> &base,
                                                  const std::vector<float> &gait,
                                                  const std::vector<float> &manual)
    {
        std::vector<float> final_output(18, 0.0f);

        for (int leg = 0; leg < 6; ++leg)
        {
            for (int joint = 0; joint < 3; ++joint)
            {
                int idx = (leg * 3) + joint;

                // 1. Combine dynamic semantic offsets (Gait + Trims)
                float semantic_offset = gait[idx] + manual[idx];

                // 2. Add signed base to signed dynamic offset
                final_output[idx] = base[idx] + (semantic_offset * legSignMap[leg][joint]);
            }
        }

        return final_output;
    }

    std::vector<float> Kinematics::getBasePosture(PostureState posture)
    {
        switch (posture)
        {
        case PostureState::POSTURE_PRONE:
            // Arming/Disarming: Belly on ground, legs spread out
            return {
                0.0, 40.0, -26.0, // LF (Group A)
                0.0, 40.0, -26.0, // RM (Group A)
                0.0, 40.0, -26.0, // LB (Group A)
                0.0, -40.0, 26.0, // RF (Group B)
                0.0, -40.0, 26.0, // LM (Group B)
                0.0, -40.0, 26.0  // RB (Group B)
            };

        case PostureState::POSTURE_STANDARD:
            // Standard height: True Zero (Kinematic Neutral)
            return std::vector<float>(18, 0.0f);

        case PostureState::POSTURE_CROUCH:
            // Max fold: Legs compressed at ~90 degrees
            return {
                0.0, 90.0, 90.0,   // LF
                0.0, 90.0, 90.0,   // RM
                0.0, 90.0, 90.0,   // LB
                0.0, -90.0, -90.0, // RF
                0.0, -90.0, -90.0, // LM
                0.0, -90.0, -90.0  // RB
            };

        case PostureState::POSTURE_HIGH:
            // Verified stance for max ground clearance
            return {
                0.0, -40.0, -34.4, // LF
                0.0, -40.0, -34.4, // RM
                0.0, -40.0, -34.4, // LB
                0.0, 40.0, 34.4,   // RF
                0.0, 40.0, 34.4,   // LM
                0.0, 40.0, 34.4    // RB
            };

        default:
            // Fallback: Safe Prone position to protect the chassis
            return this->getBasePosture(PostureState::POSTURE_PRONE);
        }
    }
}
