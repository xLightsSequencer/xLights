#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
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
        int OnShutterPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, wxXmlNode* ModelXml, BaseObject* base);

        int GetShutterChannel() { return shutter_channel; }
        int GetShutterThreshold() { return shutter_threshold; }
        int GetShutterOnValue() { return shutter_on_value; }

    protected:
        bool IsShutterOpen(const std::vector<NodeBaseClassPtr> &Nodes) const;
    
        int shutter_channel;
        int shutter_threshold;
        int shutter_on_value;

    private:
};

