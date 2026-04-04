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

#include <memory>

class ViewObject;
class ViewObjectPropertyAdapter;

class ViewObjectPropertyManager {
public:
    static std::unique_ptr<ViewObjectPropertyAdapter> CreateAdapter(ViewObject& obj);
};
