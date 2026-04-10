#include "hexadrone_core/kinematics.hpp"

namespace Hexadrone
{
    Kinematics::Kinematics() {}

    /**
     * @brief Applies hardware orientation logic to a raw joint value.
     * @param leg_index 0-5
     * @param joint_index 0-2 (Coxa, Femur, Tibia)
     * @param value Raw radian offset
     */
    float Kinematics::applySignLogic(int leg_index, int joint_index, float value)
    {
        return value * legSignMap[leg_index][joint_index];
    }

    /**
     * @brief Merges the three motion layers into final servo commands.
     * @param base The persistent, smoothed posture layer (already signed).
     * @param gait The rhythmic walking layer (raw offsets).
     * @param manual The individual leg trim layer (raw offsets).
     */

    std::vector<float> Kinematics::finalizeAngles(const std::vector<float> &base,
                                                  const std::vector<float> &gait,
                                                  const std::vector<float> &manual)
    {
        std::vector<float> final_output(18);

        for (int leg = 0; leg < 6; leg++)
        {
            for (int joint = 0; joint < 3; joint++)
            {
                int idx = (leg * 3) + joint;

                // 1. Combine dynamic offsets (Gait + Manual Trims)
                float dynamic_offset = gait[idx] + manual[idx];

                // 2. Apply sign logic only to the dynamic offsets.
                // The base is already oriented correctly from getBasePosture.
                final_output[idx] = base[idx] + applySignLogic(leg, joint, dynamic_offset);
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
                0.00, -0.70, 0.45, // LM
                0.00, 0.70, -0.45, // RM
                0.00, -0.70, 0.45, // RF
                0.00, 0.70, -0.45, // LF
                0.00, -0.70, 0.45, // RB
                0.00, 0.70, -0.45  // LB
            };

        case PostureState::POSTURE_STANDARD:
            return std::vector<float>(18, 0.0f);

        case PostureState::POSTURE_CROUCH:
            // Max fold: Legs compressed at ~90 degrees (1.57 rad)
            return {
                0.00, -1.57, -1.57, // LM
                0.00, 1.57, 1.57,   // RM
                0.00, -1.57, -1.57, // RF
                0.00, 1.57, 1.57,   // LF
                0.00, -1.57, -1.57, // RB
                0.00, 1.57, 1.57    // LB
            };

        case PostureState::POSTURE_HIGH:
            // Verified stance for max ground clearance
            return {
                0.00, 0.70, 0.60,   // LM
                0.00, -0.70, -0.60, // RM
                0.00, 0.70, 0.60,   // RF
                0.00, -0.70, -0.60, // LF
                0.00, 0.70, 0.60,   // RB
                0.00, -0.70, -0.60  // LB
            };

        default:
            // Fallback to standard to prevent erratic servo behavior
            return std::vector<float>(18, 0.0f);
        }
    }
}
