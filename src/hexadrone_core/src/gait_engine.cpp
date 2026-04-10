#include "hexadrone_core/gait_engine.hpp"
#include <cmath>
#include <algorithm>

namespace Hexadrone
{
    GaitEngine::GaitEngine() : phase(0.0f) {}

    std::vector<float> GaitEngine::update(float dt, float velocity, float yaw, GearState gear)
    {
        std::vector<float> offsets(18, 0.0f);

        // 1. Remap throttle: RadioMaster -1.0 (bottom) to 1.0 (top) -> 0.0 to 1.0
        // Formula: (input + 1) / 2
        float throttle = (velocity + 1.0f) * 0.5f;

        // 2. Threshold Check: If throttle is low and no yaw, reset phase and stay still
        if (throttle < 0.05f && std::abs(yaw) < 0.05f)
        {
            phase = 0.0f;
            return offsets;
        }

        // 3. Direction Logic: Determine propulsion sign based on Gear
        float propulsion_factor = 0.0f;
        if (gear == GearState::GEAR_DRIVE)
            propulsion_factor = throttle;
        else if (gear == GearState::GEAR_REVERSE)
            propulsion_factor = -throttle;
        else
            return offsets; // Neutral = No movement

        // 4. Advance Phase
        // The speed of the legs depends on how much "gas" or "yaw" is applied
        float move_speed = std::max(throttle, std::abs(yaw));
        phase += dt * move_speed * step_frequency;
        
        if (phase > 1.0f) phase -= 1.0f;

        // 5. Calculate Tripod Waves (Using PI_F from lib.hpp)
        float angle_A = phase * 2.0f * hexadrone::core::PI_F;
        float angle_B = (phase + 0.5f) * 2.0f * hexadrone::core::PI_F;

        // Horizontal: cos provides the forward/back swing
        // Vertical: sin provides the lift (clamped to positive only)
        float h_wave_A = std::cos(angle_A);
        float v_wave_A = std::max(0.0f, std::sin(angle_A));

        float h_wave_B = std::cos(angle_B);
        float v_wave_B = std::max(0.0f, std::sin(angle_B));

        // 6. Distribute to 18 joints
        for (int i = 0; i < 6; i++)
        {
            bool is_group_A = (i % 2 == 0);
            float h_wave = is_group_A ? h_wave_A : h_wave_B;
            float v_wave = is_group_A ? v_wave_A : v_wave_B;

            // Joint 0: COXA (Propulsion + Turning)
            // We multiply h_wave by propulsion_factor so the swing direction 
            // respects the Gear (Drive vs Reverse).
            offsets[i * 3 + 0] = (h_wave * propulsion_factor * stride_length) + 
                                 (h_wave * yaw * stride_length);

            // Joint 1: FEMUR (Lift)
            offsets[i * 3 + 1] = v_wave * lift_height;

            // Joint 2: TIBIA (Counter-balance)
            // Moves slightly inward when femur lifts to keep foot vertical
            offsets[i * 3 + 2] = -v_wave * (lift_height * 0.5f);
        }

        return offsets;
    }
}
