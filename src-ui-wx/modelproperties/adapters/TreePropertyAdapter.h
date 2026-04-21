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

#include "MatrixPropertyAdapter.h"

class TreeModel;

class TreePropertyAdapter : public MatrixPropertyAdapter {
public:
    explicit TreePropertyAdapter(Model& model);

    int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;

protected:
    void AddStyleProperties(wxPropertyGridInterface* grid) override;

private:
    TreeModel& _tree;
};
