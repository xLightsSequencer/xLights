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

class DmxDimmerAbility
{
    public:
        DmxDimmerAbility();
        virtual ~DmxDimmerAbility();

        void AddDimmerTypeProperties(wxPropertyGridInterface *grid);
        int OnDimmerPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, BaseObject* base);

        int GetDimmerChannel() const { return dimmer_channel; }
        void SetDimmerChannel( int chan ) { dimmer_channel = chan; }

    protected:
    
        int dimmer_channel;
};


