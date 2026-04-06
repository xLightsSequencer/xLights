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

#include <string>

// Core-safe data struct for layout group serialization.
// Populated from LayoutGroup* at the UI boundary.
struct LayoutGroupData {
    std::string name;
    std::string backgroundImage;
    int backgroundBrightness = 100;
    int backgroundAlpha = 100;
    bool backgroundScaled = false;
    int posX = 0;
    int posY = 0;
    int paneWidth = 0;
    int paneHeight = 0;
};
