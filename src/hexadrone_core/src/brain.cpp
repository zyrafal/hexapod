#include "hexadrone_core/brain.hpp"
#include <algorithm>

namespace Hexadrone
{

    Brain::Brain() : droneState(DroneState::DRONE_DISARMED),
                     postureState(PostureState::POSTURE_PRONE),
                     gearState(GearState::GEAR_NEUTRAL),
                     base_angles(18, 0.0f),
                     manual_offsets(18, 0.0f),
                     gait_offsets(18, 0.0f),
                     current_angles(18, 0.0f),
                     oe_kill_timer(0.0f)
    {
    }

    /**
     * @brief Main logic loop for the Hexadrone.
     * * @param dt Delta time (seconds) since the last cycle.
     * @param input Constant reference to the ControllerInput struct mapped from RadioMaster Pocket axes:
     * - CH1 (roll)        : Body Roll additive lean (Right Stick X)
     * - CH2 (pitch)       : Body Pitch additive lean (Right Stick Y)
     * - CH3 (velocity)    : Forward/Backward velocity throttle [0, 1] (Left Stick Y, remapped by BridgeOps)
     * - CH4 (yaw)         : Z-axis rotation/turning (Left Stick X)
     * - CH5 (armed_switch): Toggle SA (1: ARMED | -1: DISARMED)
     * - CH7 (posture_switch): 3-Pos SB (1: High | 0: Standard | -1: Crouch)
     * - CH8 (gear_switch)   : 3-Pos SC (1: Reverse | 0: Neutral | -1: Drive)
     * - CH9 (oe_kill_button): Momentary SE (High > 0.5f: Trigger OE-Kill Timer)
     * - CH11 (trim_coxa)    : Manual swing for selected leg
     * - CH12 (trim_femur)   : Manual lift for selected leg
     * - CH13 (trim_tibia)   : Manual reach for selected leg
     * - CH14 (leg_selector) : Axis-stepped selector (1 to 6) for manual control
     * * @return std::vector<float> 18-element array of final servo angles (degrees).
     */
    std::vector<float> Brain::update(float dt, const ControllerInput &input)
    {
        // 1. The Watchdog: Check for OE-Kill trigger
        checkSafety(input, dt);

        // 1.1 If OE-Killed, constantly return (hardware reset required)
        if (droneState == DroneState::DRONE_OE_KILLED)
        {
            return current_angles;
        }

        // 2. Buffer: Reset frame-specific dynamic layers to zero
        resetBuffers();

        // 3. Sync: Update internal state enums
        syncStates(input);

        // 4. Manual: Update trim offsets
        if (gearState == GearState::GEAR_NEUTRAL)
        {
            updateManual(input); // TODO: Currently doesn't accumulate manual adjustments
        }

        // 5. Stage: Calculate smooth posture
        calculateSmoothing(dt);

        // 6. Gait: Calculate walking offsets
        if (droneState == DroneState::DRONE_ARMED)
        {
            gait_offsets = gait.update(dt, input.velocity, input.yaw, gearState);
        }

        // 7. Distribution: Update global state for next frame and return
        current_angles = kins.finalizeAngles(base_angles, gait_offsets, manual_offsets);
        return current_angles;
    }

    // --- Private Worker Implementations ---

    void Brain::resetBuffers()
    {
        // Clear dynamic layers so they don't affect the next frame
        std::fill(manual_offsets.begin(), manual_offsets.end(), 0.0f);
        std::fill(gait_offsets.begin(), gait_offsets.end(), 0.0f);
    }

    void Brain::checkSafety(const ControllerInput &input, float dt)
    {
        // A. Handle hold for OE-Kill Switch
        if (input.oe_kill_button)
        {
            oe_kill_timer += dt;
            if (oe_kill_timer >= oe_kill_timeout)
                droneState = DroneState::DRONE_OE_KILLED;
        }
        else
        {
            oe_kill_timer = 0.0f;
        }

        // B. If OE-Killed, enforce safety
        if (droneState == DroneState::DRONE_OE_KILLED)
        {
            // Snap "ghost" posture to Prone
            current_angles = kins.getBasePosture(postureState);
        }
    }

    void Brain::syncStates(const ControllerInput &input)
    {
        droneState = (input.armed_switch == 1) ? DroneState::DRONE_ARMED : DroneState::DRONE_DISARMED;

        if (input.posture_switch == 1)
            postureState = PostureState::POSTURE_HIGH;
        else if (input.posture_switch == -1)
            postureState = PostureState::POSTURE_CROUCH;
        else
            postureState = PostureState::POSTURE_STANDARD;

        if (droneState == DroneState::DRONE_DISARMED)
        {
            postureState = PostureState::POSTURE_PRONE;
        }

        if (input.gear_switch == 1)
            gearState = GearState::GEAR_REVERSE;
        else if (input.gear_switch == -1)
            gearState = GearState::GEAR_DRIVE;
        else
            gearState = GearState::GEAR_NEUTRAL;
    }

    void Brain::updateManual(const ControllerInput &input)
    {
        // Clamp selector 1-6 to index 0-5
        int leg_idx = std::max(0, std::min(input.leg_selector - 1, 5));

        // Fill the specific indices for the selected leg
        manual_offsets[leg_idx * 3 + 0] = input.trim_coxa;
        manual_offsets[leg_idx * 3 + 1] = input.trim_femur;
        manual_offsets[leg_idx * 3 + 2] = input.trim_tibia;
    }

    void Brain::calculateSmoothing(float dt)
    {
        // Get the target base posture from Kinematics based on current state
        std::vector<float> target_pose = kins.getBasePosture(postureState);

        // Linear Interpolation (LERP) for all 18 joints
        for (int i = 0; i < 18; i++)
        {
            base_angles[i] += (target_pose[i] - base_angles[i]) * stand_up_speed * dt;
        }
    }
}
