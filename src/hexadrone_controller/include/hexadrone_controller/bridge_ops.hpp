#ifndef HEXADRONE_BRIDGE_OPS_HPP
#define HEXADRONE_BRIDGE_OPS_HPP

#include <vector>

namespace Hexadrone
{
    class BridgeOps
    {
    public:
        static ControllerInput translate(const std::vector<float> &axes)
        {
            Hexadrone::ControllerInput ci;

            // axes[n-1] = CH[n]
            // Sticks
            ci.roll = axes[0];     // CH1
            ci.pitch = axes[1];    // CH2
            ci.velocity = (axes[2] + 1.0f) * 0.5f; // CH3: remap [-1, 1] → [0, 1]
            ci.yaw = axes[3];      // CH4

            // Switches (Using: > 0.5 is High, < -0.5 is Low)
            ci.armed_switch = (axes[4] > 0.5f) ? 1 : -1;                           // CH5 (SA)
            ci.posture_switch = (axes[6] > 0.5f) ? 1 : (axes[6] < -0.5f ? -1 : 0); // CH7 (SB)
            ci.gear_switch = (axes[7] > 0.5f) ? 1 : (axes[7] < -0.5f ? -1 : 0);    // CH8 (SC)
            ci.oe_kill_button = (axes[8] > 0.5f);                                  // CH9 (SE)

            // Trims
            ci.trim_coxa = axes[10];                      // CH11
            ci.trim_femur = axes[11];                     // CH12
            ci.trim_tibia = axes[12];                     // CH13
            ci.leg_selector = static_cast<int>(axes[13]); // CH14

            return ci;
        }
    };
}
#endif
