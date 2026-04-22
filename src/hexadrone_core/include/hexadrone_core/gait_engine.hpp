#ifndef HEXADRONE_GAIT_ENGINE_HPP
#define HEXADRONE_GAIT_ENGINE_HPP

#include "hexadrone_core/lib.hpp"
#include "hexadrone_core/state_machine.hpp"
#include <vector>

namespace Hexadrone
{
    class GaitEngine
    {
    public:
        GaitEngine();

        std::vector<float> update(float dt, float throttle, float yaw, GearState gear);

    private:
        float phase = 0.0f; // 0.0 to 1.0

        // Gait Tuning Constants
        static constexpr float STRIDE_LENGTH = 11.25f;  // Amplitude of Coxa swing
        static constexpr float LIFT_HEIGHT = 22.5f;    // Amplitude of Femur lift
        static constexpr float STEP_FREQUENCY = 2.5f; // Max speed of the gait cycle
        static constexpr float PHASE_OFFSET_B = 0.5f;  // Tripod timing (180 deg)
        static constexpr float INPUT_DEADZONE = 0.05f; // Stick drift protection
    };
}

#endif
