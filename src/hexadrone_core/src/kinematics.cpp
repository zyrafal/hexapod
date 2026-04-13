#include "hexadrone_core/kinematics.hpp"

namespace Hexadrone
{
    Kinematics::Kinematics() {}

    /**
     * @brief Applies hardware orientation logic to a raw joint value.
     * @param leg_index 0-5
     * @param joint_index 0-2 (Coxa, Femur, Tibia)
     * @param value Raw radian offset
     */
    float Kinematics::applySignLogic(int leg_index, int joint_index, float value)
    {
        return value * legSignMap[leg_index][joint_index];
    }

    /**
     * @brief Merges the three motion layers into final servo commands.
     * @param base The persistent, smoothed posture layer (already signed).
     * @param gait The rhythmic walking layer (raw offsets).
     * @param manual The individual leg trim layer (raw offsets).
     */

    std::vector<float> Kinematics::finalizeAngles(const std::vector<float> &base,
                                                  const std::vector<float> &gait,
                                                  const std::vector<float> &manual)
    {
        std::vector<float> final_output(18);

        for (int leg = 0; leg < 6; leg++)
        {
            for (int joint = 0; joint < 3; joint++)
            {
                int idx = (leg * 3) + joint;

                float semantic = base[idx] + gait[idx] + manual[idx] + urdfBias[joint];
                final_output[idx] = applySignLogic(leg, joint, semantic);
            }
        }

        return final_output;
    }

    std::vector<float> Kinematics::getBasePosture(PostureState posture)
    {
        auto pose = [](float c, float f, float t) -> std::vector<float> {
            return {c,f,t, c,f,t, c,f,t, c,f,t, c,f,t, c,f,t};
        };

        switch (posture)
        {
        case PostureState::POSTURE_PRONE:    return pose(  0,   0,   0);
        case PostureState::POSTURE_STANDARD: return pose(  0,  40,  26);
        case PostureState::POSTURE_CROUCH:   return pose(  0,  20,  46);
        case PostureState::POSTURE_HIGH:     return pose(  0,  55,  11);
        default:                             return pose(  0,  90,  90);
        }
    }
}
