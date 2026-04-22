#ifndef HEXADRONE_STATE_MACHINE_HPP
#define HEXADRONE_STATE_MACHINE_HPP

namespace Hexadrone
{
    struct ControllerInput
    {
        float roll;
        float pitch;
        float throttle;
        float yaw;

        int armed_switch;
        int posture_switch;
        int gear_switch;
        bool oe_kill_button;

        float trim_coxa;
        float trim_femur;
        float trim_tibia;
        bool btn_prev_leg;
        bool btn_next_leg;
    };

    enum class DroneState
    {
        DRONE_DISARMED,
        DRONE_ARMED,
        DRONE_OE_KILLED,
        DRONE_ARMING,
        DRONE_DISARMING
    };

    enum class PostureState
    {
        POSTURE_PRONE,
        POSTURE_STANDARD,
        POSTURE_CROUCH,
        POSTURE_HIGH
    };

    enum class GearState
    {
        GEAR_NEUTRAL,
        GEAR_DRIVE,
        GEAR_REVERSE
    };
}

#endif
