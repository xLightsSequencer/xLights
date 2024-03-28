#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/
#include "DmxMotorBase.h"

class DmxMotorPanTilt : public DmxMotorBase
{
    public:
    DmxMotorPanTilt() :
        DmxMotorBase()
    {}
    virtual ~DmxMotorPanTilt(){};

    int GetChannelCoarse() const override
    {
        return 0;
    }
    int GetChannelFine() const override
    {
        return channel;
    }
    int ConvertPostoCmd(float position) override//in degrees -180 to 180
    {
        float mid_point = deg_of_rot / 2.0F;

        float amount_to_move = (float)mid_point + position + orient;
        float cmd = (amount_to_move / deg_of_rot) * (float)max_value;
        //todo: full spin
        return cmd;
    }

    float GetPosition(int channel_value) override
    {
        auto pos = (channel_value / (max_value - min_value)) * deg_of_rot + orient;
        return pos;
    }

    int channel{ 0 };
    float deg_of_rot{ 180.0f };
    int orient{ 0 };
    float slew_limit{ 0.0F };
    int min_value{ 0 };
    int max_value{ 255 };
};

