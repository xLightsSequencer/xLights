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

#include "../ModelPropertyAdapter.h"

class SubModel;

class SubModelPropertyAdapter : public ModelPropertyAdapter {
public:
    explicit SubModelPropertyAdapter(SubModel& subModel);

    void AddProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
    void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
    int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;
    void DisableUnusedProperties(wxPropertyGridInterface* grid) override {}

private:
    SubModel& _subModel;
};
