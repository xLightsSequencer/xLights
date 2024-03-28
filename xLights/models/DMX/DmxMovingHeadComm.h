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

#include "DmxModel.h"

class DmxMotorBase;

class DmxMovingHeadComm : public DmxModel
{
    public:
        DmxMovingHeadComm(wxXmlNode* node, const ModelManager& manager, bool zeroBased = false) :
            DmxModel(node,manager,zeroBased)
        {
        }
        virtual ~DmxMovingHeadComm(){};

        virtual DmxMotorBase* GetPanMotor() = 0;
        virtual DmxMotorBase* GetTiltMotor() = 0;
        virtual int GetFixtureVal() const { return 1; };
};
