/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "DmxShutterAbility.h"
#include "../../Color.h"

DmxShutterAbility::DmxShutterAbility()
{
}

DmxShutterAbility::~DmxShutterAbility()
{
}

bool DmxShutterAbility::IsShutterOpen(const std::vector<NodeBaseClassPtr> &Nodes) const {
    // determine if shutter is open for floods that support it
    bool shutter_open = true;
    if (shutter_channel > 0 && shutter_channel <= (int)Nodes.size()) {
        xlColor proxy;
        Nodes[shutter_channel - 1]->GetColor(proxy);
        int shutter_value = proxy.red;
        if (shutter_value >= 0) {
            shutter_open = shutter_value >= shutter_threshold;
        } else {
            shutter_open = shutter_value <= std::abs(shutter_threshold);
        }
    }
    return shutter_open;
}
