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

#include "../ViewObjectPropertyAdapter.h"

class TerrainObject;

class TerrainObjectPropertyAdapter : public ViewObjectPropertyAdapter {
public:
    explicit TerrainObjectPropertyAdapter(TerrainObject& obj);
    ~TerrainObjectPropertyAdapter() override = default;

    void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
    int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;

private:
    TerrainObject& _terrain;
};
