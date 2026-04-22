#ifndef HEXADRONE_KINEMATICS_HPP
#define HEXADRONE_KINEMATICS_HPP

#include "hexadrone_core/lib.hpp"
#include "hexadrone_core/state_machine.hpp"
#include <vector>

namespace Hexadrone
{
    class Kinematics
    {
    public:
        Kinematics();

        std::vector<float> finalizeAngles(const std::vector<float> &base,
                                          const std::vector<float> &gait,
                                          const std::vector<float> &manual);
        
        std::vector<float> getBasePosture(PostureState posture);
        
        std::vector<float> getBodyOffsets(float pitch, float roll);

    private:
        // Lean Tuning Constants
        static constexpr float MAX_BODY_LEAN = 45.0f;     // Max tilt in degrees
        static constexpr float TIBIA_COMPENSATION = 0.5f; // Ratio to keep feet planted

        /**
         * @brief Multipliers to handle physical motor orientation.
         * Indices must strictly follow the interleaved power-group sequence:
         * 0:LF, 1:RM, 2:LB (Group A) | 3:RF, 4:LM, 5:RB (Group B)
         */
        const float legSignMap[6][3] = {
            {-1.0f, -1.0f,  1.0f}, // 0: LF (Group A)
            { 1.0f, -1.0f,  1.0f}, // 1: RM (Group A)
            {-1.0f, -1.0f,  1.0f}, // 2: LB (Group A)
            { 1.0f,  1.0f, -1.0f}, // 3: RF (Group B)
            {-1.0f,  1.0f, -1.0f}, // 4: LM (Group B)
            { 1.0f,  1.0f, -1.0f}  // 5: RB (Group B)
        };
    };
}

#endif
