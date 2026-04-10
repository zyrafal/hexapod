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
        const float stride_length = 0.4f;  // How far the legs wiggle
        const float lift_height = 0.3f;    // How high they lift
        const float step_frequency = 2.0f; // Multiplier for phase speed
    };
}

#endif
