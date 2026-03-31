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

#include "ControllerPropertyAdapter.h"

class ControllerSerial;

class ControllerSerialPropertyAdapter : public ControllerPropertyAdapter {
public:
    explicit ControllerSerialPropertyAdapter(Controller& controller);

    void AddProperties(wxPropertyGrid* propertyGrid, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) override;
    bool HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager) override;
    void ValidateProperties(OutputManager* om, wxPropertyGrid* propGrid) const override;

private:
    ControllerSerial& _serial;
};
