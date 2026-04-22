#include "hexadrone_core/brain.hpp"
#include <algorithm>

namespace Hexadrone
{

    Brain::Brain() : droneState(DroneState::DRONE_DISARMED),
                     postureState(PostureState::POSTURE_PRONE),
                     gearState(GearState::GEAR_NEUTRAL),
                     last_gear_state(GearState::GEAR_NEUTRAL),
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
     * @param input Constant reference to the ControllerInput struct containing raw signals:
     * - CH1 (roll)          : Body Roll additive lean (Right Stick X)
     * - CH2 (pitch)         : Body Pitch additive lean (Right Stick Y)
     * - CH3 (throttle)      : Velocity/Gas - Magnitude of walking gait (Left Stick Y)
     * - CH4 (yaw)           : Z-axis rotation/turning (Left Stick X)
     * - CH5 (armed_switch)  : ARM / Safety (Switch SA | 1: ARMED, -1: DISARMED)
     * - CH6 (posture_switch): Standing Height (Switch SB | -1: Crouch, 0: Std, 1: High)
     * - CH7 (gear_switch)   : Transmission (Switch SC | -1: Forward, 0: Neutral, 1: Backward)
     * - CH9 (oe_kill_button): Kill-Switch (Button SE | Triggers PRONE snap)
     * - CH11 (trim_coxa)    : Manual Coxa swing for the spatially selected leg
     * - CH12 (trim_femur)   : Manual Femur lift for the spatially selected leg
     * - CH13 (trim_tibia)   : Manual Tibia extension for the spatially selected leg
     * - CH14 (btns_prev/next): Raw buttons (T4 X) triggering internal CW/CCW selection
     * * @return std::vector<float> 18-element array of final servo angles in degrees.
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

        // 2. Sync: Update internal state enums
        syncStates(input);

        // 3. Buffer: Reset frame-specific dynamic layers to zero
        handleBuffers();

        // 4. Manual: Update trim offsets ONLY when ARMED and in NEUTRAL
        if (droneState == DroneState::DRONE_ARMED && gearState == GearState::GEAR_NEUTRAL)
        {
            updateManual(input, dt);
        }

        // 5. Stage: Calculate smooth posture
        calculateSmoothing(dt);

        // 6. Gait: Calculate walking offsets
        if (droneState == DroneState::DRONE_ARMED)
        {
            gait_offsets = gait.update(dt, input.throttle, input.yaw, gearState);
        }

        // 6.5 Dynamic Body Lean
        // Fetched instantly, bypassing smoothing for zero-latency stick response
        std::vector<float> lean_offsets(18, 0.0f);
        if (droneState == DroneState::DRONE_ARMED)
        {
            lean_offsets = kins.getBodyOffsets(input.pitch, input.roll);
        }

        // Combine the Walking Gait and the Body Lean into one dynamic layer
        std::vector<float> combined_dynamics(18, 0.0f);
        for (int i = 0; i < 18; ++i)
        {
            combined_dynamics[i] = gait_offsets[i] + lean_offsets[i];
        }

        // Mask Trims when Disarmed
        // Remembers exact manual adjustments for when it re-arms.
        std::vector<float> active_manual_offsets = (droneState == DroneState::DRONE_ARMED) ? manual_offsets : std::vector<float>(18, 0.0f);

        // 7. Distribution: Update global state for next frame and return
        current_angles = kins.finalizeAngles(base_angles, combined_dynamics, active_manual_offsets);
        return current_angles;
    }

    // --- Private Worker Implementations ---

    void Brain::handleBuffers()
    {
        // Reset trims if the gear has changed
        if (gearState != last_gear_state)
        {
            std::fill(manual_offsets.begin(), manual_offsets.end(), 0.0f);
            last_gear_state = gearState; // Update tracker
        }

        // Clear dynamic layers so they don't affect the next frame
        std::fill(gait_offsets.begin(), gait_offsets.end(), 0.0f);
    }

    void Brain::checkSafety(const ControllerInput &input, float dt)
    {
        // A. Handle hold for OE-Kill Switch
        if (input.oe_kill_button)
        {
            oe_kill_timer += dt;
            if (oe_kill_timer >= OE_KILL_TIMEOUT)
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
            current_angles = kins.getBasePosture(PostureState::POSTURE_PRONE);
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

    void Brain::updateManual(const ControllerInput &input, float dt)
    {
        // 1. Handle Stateful Leg Selection (Clockwise/Counter-Clockwise)
        if (input.btn_next_leg && !last_btn_next)
        {
            sequence_ptr = (sequence_ptr + 1) % 6;
        }
        else if (input.btn_prev_leg && !last_btn_prev)
        {
            sequence_ptr = (sequence_ptr == 0) ? 6 - 1 : sequence_ptr - 1;
        }
        last_btn_next = input.btn_next_leg;
        last_btn_prev = input.btn_prev_leg;

        // 2. Map sequence to internal leg index
        int current_leg_idx = CW_MAP[sequence_ptr];
        int base_idx = current_leg_idx * 3;

        // 3. Apply Trim Deltas
        float d_coxa = input.trim_coxa * TRIM_SENSITIVITY * dt;
        float d_femur = input.trim_femur * TRIM_SENSITIVITY * dt;
        float d_tibia = input.trim_tibia * TRIM_SENSITIVITY * dt;

        // 4. Accumulate and Clamp (Anti-Windup at +/- 90 degrees)
        manual_offsets[base_idx + 0] = std::clamp(manual_offsets[base_idx + 0] + d_coxa, -90.0f, 90.0f);
        manual_offsets[base_idx + 1] = std::clamp(manual_offsets[base_idx + 1] + d_femur, -90.0f, 90.0f);
        manual_offsets[base_idx + 2] = std::clamp(manual_offsets[base_idx + 2] + d_tibia, -90.0f, 90.0f);
    }

    void Brain::calculateSmoothing(float dt)
    {
        // Get the target base posture from Kinematics based on current state
        std::vector<float> target_pose = kins.getBasePosture(postureState);

        // Linear Interpolation (LERP) for all 18 joints
        for (int i = 0; i < 18; i++)
        {
            base_angles[i] += (target_pose[i] - base_angles[i]) * STAND_UP_SPEED * dt;
        }
    }
}
