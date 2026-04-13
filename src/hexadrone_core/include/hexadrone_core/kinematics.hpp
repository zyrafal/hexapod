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

        float applySignLogic(int leg_index, int joint_index, float value);
        std::vector<float> finalizeAngles(const std::vector<float> &base,
                                          const std::vector<float> &gait,
                                          const std::vector<float> &manual);
        std::vector<float> getBasePosture(PostureState posture);

    private:
        // Coxa:        Left legs = +1, Right legs = -1
        // Femur/Tibia: URDF axis group A (LM, RF, RB) = +1
        //              URDF axis group B (LF, LB, RM) = -1
        // Order: Coxa, Femur, Tibia
        const float legSignMap[6][3] = {
            {-1.0, -1.0,  1.0}, // LF (left coxa,  Group B femur/tibia)
            {-1.0,  1.0, -1.0}, // LM (left coxa,  Group A femur/tibia)
            {-1.0, -1.0,  1.0}, // LB (left coxa,  Group B femur/tibia)
            { 1.0,  1.0, -1.0}, // RF (right coxa, Group A femur/tibia)
            { 1.0, -1.0,  1.0}, // RM (right coxa, Group B femur/tibia)
            { 1.0,  1.0, -1.0}  // RB (right coxa, Group A femur/tibia)
        };

        // Hardware bias: URDF joint zeros correspond to the old "standard" stance.
        // Added before the sign map to translate PRONE-relative degrees into the
        // URDF's physical coordinate system. Derived exactly from PRONE_*_RAD.
        // THE PRONE-RELATIVE COORDINATE SYSTEM SHOULD NEVER BE CHANGED
        const float urdfBias[3] = {
            0.0,
            -0.7 * hexadrone::core::RAD_TO_DEG_F,
            -0.45 * hexadrone::core::RAD_TO_DEG_F
        };
    };
}

#endif
