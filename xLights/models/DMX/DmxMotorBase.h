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

class DmxMotorBase
{
public:
    DmxMotorBase() = default;

    virtual ~DmxMotorBase(){};

    virtual int GetChannelCoarse() const = 0;
    virtual int GetChannelFine() const = 0;

    virtual int ConvertPostoCmd(float position) = 0;
    virtual float GetPosition(int channel_value) = 0;
};

