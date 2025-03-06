/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/
#pragma once

#include <vector>
#include "../../PixelBuffer.h"

class ISPCComputeUtilitiesData;

class ISPCComputeUtilities {
public:
    ISPCComputeUtilities();
    ~ISPCComputeUtilities();
    
    bool blendLayers(PixelBufferClass *pixelBuffer, int effectPeriod, const std::vector<bool>& validLayers, int saveLayer, bool saveToPixels);

    
    static ISPCComputeUtilities INSTANCE;
private:
    void blendLayers(PixelBufferClass *pixelBuffer, int effectPeriod, const std::vector<bool>& validLayers, int saveLayer, int start, int end);

    ISPCComputeUtilitiesData *data;
};
