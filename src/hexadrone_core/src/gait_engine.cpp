#include "hexadrone_core/gait_engine.hpp"
#include <cmath>
#include <algorithm>

namespace Hexadrone
{
    GaitEngine::GaitEngine() : phase(0.0f) {}

    std::vector<float> GaitEngine::update(float dt, float velocity, float yaw, GearState gear)
    {
        std::vector<float> offsets(18, 0.0f);

        // 1. Motion magnitude: velocity is [0, 1]; NEUTRAL = no propulsion
        float motion_magnitude = (gear == GearState::GEAR_NEUTRAL) ? 0.0f : velocity;

        // 2. Threshold Check: If no propulsion and no yaw, reset phase and stay still
        // if (motion_magnitude < 0.05f && std::abs(yaw) < 0.05f)
        if (1)
        {
            phase = 0.0f;
            return offsets;
        }

        // 3. Direction Logic: gear selects forward or reverse
        float propulsion_factor = 0.0f;
        if (gear == GearState::GEAR_DRIVE)
            propulsion_factor = motion_magnitude;
        else if (gear == GearState::GEAR_REVERSE)
            propulsion_factor = -motion_magnitude;

        // 4. Advance Phase
        // The speed of the legs depends on how much "gas" or "yaw" is applied
        float move_speed = std::max(motion_magnitude, std::abs(yaw));
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

            // Joint 0: COXA (Propulsion + Turning) — output in degrees
            offsets[i * 3 + 0] = (h_wave * propulsion_factor * stride_length) +
                                  (h_wave * yaw * stride_length);

            // Joint 1: FEMUR (Lift) — positive offset; sign map + urdfBias handle physical direction
            offsets[i * 3 + 1] = v_wave * lift_height;

            // Joint 2: TIBIA (neutral — counter-balance removed: asymmetric sign groups
            // caused it to splay in the wrong direction on half the legs)
            offsets[i * 3 + 2] = 0.0f;
        }

        return offsets;
    }
}
