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

#include "../ModelPropertyAdapter.h"

class CandyCaneModel;

class CandyCanePropertyAdapter : public ModelPropertyAdapter {
public:
    explicit CandyCanePropertyAdapter(Model& model);

    void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
    void UpdateTypeProperties(wxPropertyGridInterface* grid) override;
    void AddDimensionProperties(wxPropertyGridInterface* grid) override;
    int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;

private:
    CandyCaneModel& _candyCane;
};
