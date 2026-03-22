/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "DmxMotor.h"
#include "../BaseObject.h"
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

DmxMotor::DmxMotor(const std::string& _name)
    : base_name(_name) {
}

DmxMotor::~DmxMotor()
{
}

void DmxMotor::Init()
{
    if (reverse) {
        rev = -1;
    } else {
        rev = 1;
    }
}
 
int DmxMotor::ConvertPostoCmd(float position)
{
    float limited_pos = position;
    if (limited_pos > max_limit) {
        limited_pos = max_limit;
    } else if (limited_pos < min_limit) {
        limited_pos = min_limit;
    }

    if (upside_down) {
        limited_pos = -1.0f * limited_pos;
    }

    float goto_home = (float)max_value * (float)orient_home / range_of_motion;
    float amount_to_move = (float)max_value * limited_pos / range_of_motion * rev;
    float cmd = goto_home + amount_to_move;
    float full_spin = (float)max_value * 360.0 / range_of_motion;

    if (cmd < 0) {
        if (cmd + full_spin < max_value) {
            cmd += full_spin;
        } else {
            cmd = 0; // tbd....figure out which limit is closer to desired target
        }
    } else if (cmd > max_value) {
        if (cmd - full_spin >= 0.0f) {
            cmd -= full_spin;
        } else {
            cmd = max_value; // tbd....figure out which limit is closer to desired target
        }
    }
    return cmd;
}

float DmxMotor::GetPosition(int channel_value)
{
    return ((1.0 - ((channel_value - min_value) / (float)(max_value - min_value))) * (rev * range_of_motion) - (rev * range_of_motion));
}


