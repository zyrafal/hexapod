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
                float final_deg = base[idx] + (semantic_offset * legSignMap[leg][joint]);

                // 3. Safety clamp to prevent servo/chassis damage
                final_output[idx] = std::max(-90.0f, std::min(90.0f, final_deg));
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

    std::vector<float> Kinematics::getBodyOffsets(float pitch, float roll)
    {
        std::vector<float> offsets(18, 0.0f);

        // 1. Deadzone: Prevent jitter from the spring-centered gimbal
        float active_pitch = (std::abs(pitch) < 0.05f) ? 0.0f : pitch;
        float active_roll = (std::abs(roll) < 0.05f) ? 0.0f : roll;

        // 2. Map stick input [-1.0, 1.0] to max deflection degrees
        float pitch_deg = active_pitch * MAX_BODY_LEAN;
        float roll_deg = active_roll * MAX_BODY_LEAN;

        // 3. Multiplier Maps based on physical leg layout
        // Indices: 0:LF, 1:RM, 2:LB, 3:RF, 4:LM, 5:RB

        // Pitch: Forward stick (+1.0) lowers the front legs and raises the back legs
        const float pitch_mult[6] = { -1.0f,  0.0f, 1.0f,  -1.0f,  0.0f, 1.0f};

        // Roll: Left stick (+1.0 inverted) lowers the left side and raises the right side
        const float roll_mult[6] = {1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f};

        // 4. Distribute the lean
        for (int leg = 0; leg < 6; ++leg)
        {
            // Apply primary body lean to the Femur (Joint 1)
            float femur_lean = (pitch_deg * pitch_mult[leg]) + (roll_deg * roll_mult[leg]);

            offsets[leg * 3 + 1] = femur_lean;

            // Compensate with the Tibia (Joint 2) to prevent the feet from dragging inward
            offsets[leg * 3 + 2] = -femur_lean * TIBIA_COMPENSATION;
        }

        return offsets;
    }
}
