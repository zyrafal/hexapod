#include "hexadrone_core/gait_engine.hpp"
#include <cmath>
#include <algorithm>

namespace Hexadrone
{
    GaitEngine::GaitEngine() : phase(0.0f) {}

    std::vector<float> GaitEngine::update(float dt, float throttle, float yaw, GearState gear)
    {
        std::vector<float> offsets(18, 0.0f);

        if (gear == GearState::GEAR_NEUTRAL)
            return offsets;

        // 1. Map physical drone stick [-1.0, 1.0] to speed magnitude [0.0, 1.0]
        float mapped_throttle = (throttle + 1.0f) / 2.0f;

        // 2. Independent Deadzones (throttle at bottom, yaw at center)
        float active_throttle = (mapped_throttle < INPUT_DEADZONE) ? 0.0f : mapped_throttle;
        float active_yaw = (std::abs(yaw) < INPUT_DEADZONE) ? 0.0f : yaw;

        if (active_throttle == 0.0f && active_yaw == 0.0f)
        {
            phase = 0.0f;
            return offsets;
        }

        // 3. Advance Phase - The speed of the legs depends on how much "gas" or "yaw" is applied
        float propulsion_factor = (gear == GearState::GEAR_DRIVE) ? active_throttle : -active_throttle;
        float move_speed = std::max(active_throttle, std::abs(active_yaw));
        phase += dt * move_speed * STEP_FREQUENCY;

        if (phase > 1.0f)
            phase -= 1.0f;

        // 4. Calculate Tripod Waves (Using PI_F from lib.hpp)
        float angle_A = phase * 2.0f * hexadrone::core::PI_F;
        float angle_B = (phase + PHASE_OFFSET_B) * 2.0f * hexadrone::core::PI_F;

        // Horizontal: cos provides the forward/back swing
        // Vertical: sin provides the lift (clamped to positive only)
        float h_wave_A = std::cos(angle_A);
        float v_wave_A = std::max(0.0f, std::sin(angle_A));

        float h_wave_B = std::cos(angle_B);
        float v_wave_B = std::max(0.0f, std::sin(angle_B));

        // 5. Distribute to 18 joints
        for (int i = 0; i < 6; i++)
        {
            // Grouping: 0-2 are A, 3-5 are B
            bool is_group_A = (i < 3);
            float h_wave = is_group_A ? h_wave_A : h_wave_B;
            float v_wave = is_group_A ? v_wave_A : v_wave_B;

            // Side Identification: Evens (0,2,4) are Left | Odds (1,3,5) are Right
            bool is_left = (i % 2 == 0);

            // Turn Direction: To turn right (+yaw), Left pushes forward (+1), Right pushes back (-1)
            float turn_dir = is_left ? 1.0f : -1.0f;

            // Propulsion is universal. Yaw is differential.
            float mixed_signal = propulsion_factor + (active_yaw * turn_dir);

            // Clamp to prevent physical over-extension
            mixed_signal = std::clamp(mixed_signal, -1.0f, 1.0f);

            // Balance the 2-vs-1 tripod torque for straight walking
            float balance_multiplier = (i == 1 || i == 4) ? 2.0f : 1.0f;

            offsets[i * 3 + 0] = h_wave * mixed_signal * STRIDE_LENGTH * balance_multiplier;
            offsets[i * 3 + 1] = v_wave * LIFT_HEIGHT;
            offsets[i * 3 + 2] = 0.0f;
        }

        return offsets;
    }
}
