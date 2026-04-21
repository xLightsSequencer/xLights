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

#include <wx/bmpbndl.h>
#include <map>
#include <utility>

class RenderableEffect;

class EffectIconCache {
public:
    static const wxBitmapBundle& GetEffectIcon(const RenderableEffect* effect, int defSize = 16);
    static void Clear();

private:
    static void BuildBundles(const RenderableEffect* effect);
    static std::map<std::pair<int, int>, wxBitmapBundle> cache;
};
