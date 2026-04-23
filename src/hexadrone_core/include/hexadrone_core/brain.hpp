#ifndef HEXADRONE_BRAIN_HPP
#define HEXADRONE_BRAIN_HPP

#include <vector>
#include "hexadrone_core/state_machine.hpp"
#include "hexadrone_core/kinematics.hpp"
#include "hexadrone_core/gait_engine.hpp"

namespace Hexadrone
{
    class Brain
    {
    public:
        Brain();

        // The single entry point for ROS2 and ESP32
        std::vector<float> update(float dt, const ControllerInput &input);

        // Getter for the ESP32 hardware layer
        DroneState getState() const { return droneState; }

    private:
        // Internal State Tracking
        DroneState droneState;
        PostureState postureState;
        GearState gearState;

        // Stateful selection tracking
        GearState last_gear_state;
        int sequence_ptr = 0;
        bool last_btn_prev = false;
        bool last_btn_next = false;

        // Spatial CW Order: LF(0)->RF(3)->RM(1)->RB(5)->LB(2)->LM(4)
        static constexpr int CW_MAP[6] = {0, 3, 1, 5, 2, 4};

        // Physical State Tracking
        std::vector<float> base_angles;    // Smoothed posture (pre-gait)
        std::vector<float> manual_offsets; // Trim buttons adjustments
        std::vector<float> gait_offsets;   // Walking wiggles
        std::vector<float> current_angles; // Final output

        float oe_kill_timer = 0.0f; // Stopwatch for the button

        // Specialist Workers (Internal Pipeline)
        void handleBuffers();
        void checkSafety(const ControllerInput &input, float dt);
        void syncStates(const ControllerInput &input);
        void updateManual(const ControllerInput &input, float dt);
        void calculateSmoothing(float dt);

        // Sub-modules
        Kinematics kins;
        GaitEngine gait;

        // Brain Tuning Constants
        static constexpr float STAND_UP_SPEED = 0.75f;   // LERP smoothing factor
        static constexpr float TRIM_SENSITIVITY = 30.0f; // Deg/sec for manual control
        static constexpr float OE_KILL_TIMEOUT = 1.0f;   // Hold duration for SE (s)
    };
}

#endif
