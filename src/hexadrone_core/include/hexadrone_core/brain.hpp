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

    private:
        // Internal State Tracking
        DroneState droneState;
        PostureState postureState;
        GearState gearState;

        // Physical State Tracking
        std::vector<float> base_angles;    // Smoothed posture (pre-gait)
        std::vector<float> manual_offsets; // Trim buttons adjustments
        std::vector<float> gait_offsets;   // Walking wiggles
        std::vector<float> current_angles; // Final output

        float oe_kill_timer = 0.0f; // Stopwatch for the button

        // Specialist Workers (Internal Pipeline)
        void resetBuffers();
        void checkSafety(const ControllerInput &input, float dt);
        void syncStates(const ControllerInput &input);
        void updateManual(const ControllerInput &input);
        void calculateSmoothing(float dt);

        // Sub-modules
        Kinematics kins;
        GaitEngine gait;

        // Tuning Constants
        const float stand_up_speed = 5.0f;
        const float oe_kill_timeout = 1.0f;
    };
}

#endif
