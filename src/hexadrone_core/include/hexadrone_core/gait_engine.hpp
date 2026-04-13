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

        // Gait Tuning Constants (degrees)
        const float stride_length  = 11.5f; // Coxa sweep amplitude. Smaller = less lateral arc = less sliding.
        const float lift_height    = 26.0f; // Femur lift amplitude. Must clear ground during swing.
        const float step_frequency = 1.5f;  // Phase advance speed multiplier (dimensionless).
    };
}

#endif
