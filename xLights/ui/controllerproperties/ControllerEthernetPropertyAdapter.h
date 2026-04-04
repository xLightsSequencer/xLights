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

class ControllerEthernet;

class ControllerEthernetPropertyAdapter : public ControllerPropertyAdapter {
public:
    explicit ControllerEthernetPropertyAdapter(Controller& controller);

    void AddProperties(wxPropertyGrid* propertyGrid, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) override;
    void UpdateProperties(wxPropertyGrid* propertyGrid, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties, OutputModelManager* outputModelManager) override;
    bool HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager) override;
    void ValidateProperties(OutputManager* om, wxPropertyGrid* propGrid) const override;
    void HandleExpanded(wxPropertyGridEvent& event, bool expanded) override;

    // Moved from ControllerEthernet
    bool SetChannelSize(int32_t channels, std::list<class Model*> models = {}, uint32_t universeSize = 510);
    bool SupportsUniversePerString() const;

private:
    void SetProtocolAndRebuildProperties(const std::string& protocol, wxPropertyGrid* grid, OutputModelManager* outputModelManager);

    ControllerEthernet& _ethernet;
};
