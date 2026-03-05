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

#include "../Model.h"

class wxPropertyGridInterface;
class wxPropertyGridEvent;
class BaseObject;
class wxXmlNode;

class DmxShutterAbility
{
    public:
        DmxShutterAbility();
        virtual ~DmxShutterAbility();

        void AddShutterTypeProperties(wxPropertyGridInterface *grid);
        int OnShutterPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, BaseObject* base);

        bool IsShutterOpen(const std::vector<NodeBaseClassPtr> &Nodes) const;

        int GetShutterChannel() const { return shutter_channel; }
        int GetShutterThreshold() const { return shutter_threshold; }
        int GetShutterOnValue() const { return shutter_on_value; }

        void SetShutterChannel(int chan) { shutter_channel = chan; }
        void SetShutterThreshold(int thres) { shutter_threshold = thres; }
        void SetShutterOnValue(int val) { shutter_on_value = val; }

    protected:
    
    int shutter_channel {0};
    int shutter_threshold {1};
    int shutter_on_value {0};

    private:
};

