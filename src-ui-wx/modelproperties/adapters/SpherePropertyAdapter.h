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

class SphereModel;

class SpherePropertyAdapter : public MatrixPropertyAdapter {
public:
    explicit SpherePropertyAdapter(Model& model);

    int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;

protected:
    void AddStyleProperties(wxPropertyGridInterface* grid) override;

private:
    SphereModel& _sphere;
};
