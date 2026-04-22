#ifndef HEXADRONE_BRIDGE_OPS_HPP
#define HEXADRONE_BRIDGE_OPS_HPP

#include <vector>

namespace Hexadrone
{
    class BridgeOps
    {
    public:
        static ControllerInput translate(
            const std::vector<float> &axes,
            const std::vector<int> &buttons)
        {
            Hexadrone::ControllerInput ci;

            // 1. STICKS (Signs inverted per /joy workaround)
            ci.roll     = -axes[0]; // CH1: Right X (Ail)
            ci.pitch    = -axes[1]; // CH2: Right Y (Ele)
            ci.throttle = -axes[2]; // CH3: Left  Y (Thr)
            ci.yaw      = -axes[3]; // CH4: Left  X (Rud)

            // 2. SWITCHES & BUTTONS
            // CH5: SA (State): Button 0
            ci.armed_switch = (buttons[0] == 1) ? 1 : -1;

            // CH6: SB (Posture): Axis 4 (Inverted 3-Pos)
            float sb_raw = -axes[4]; 
            ci.posture_switch = (sb_raw > 0.5f) ? 1 : (sb_raw < -0.5f ? -1 : 0);

            // CH7: SC (Gear): Button Pair 11 & 12
            ci.gear_switch = (buttons[11] == 1) ? -1 : (buttons[12] == 1 ? 1 : 0);

            // CH9: SE (OE-Kill Switch): Button 1
            ci.oe_kill_button = buttons[1] == 1;

            // 3. TRIMS (Button Pair Logic)
            // Manual Coxa (T1/CH11): Buttons 3 & 4
            ci.trim_coxa = (buttons[3] == 1) ? -1.0f : (buttons[4] == 1 ? 1.0f : 0.0f);

            // Manual Femur (T2/CH12): Buttons 5 & 6
            ci.trim_femur = (buttons[5] == 1) ? -1.0f : (buttons[6] == 1 ? 1.0f : 0.0f);

            // Manual Tibia (T3/CH13): Buttons 7 & 8
            ci.trim_tibia = (buttons[7] == 1) ? -1.0f : (buttons[8] == 1 ? 1.0f : 0.0f);

            // 4. LEG SELECTOR (Raw buttons passed to Brain)
            ci.btn_prev_leg = (buttons[9] == 1);
            ci.btn_next_leg = (buttons[10] == 1);

            return ci;
        }
    };
}
#endif
