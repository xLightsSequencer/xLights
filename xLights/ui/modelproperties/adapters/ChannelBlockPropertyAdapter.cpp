/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "ChannelBlockPropertyAdapter.h"
#include "../../../models/ChannelBlockModel.h"
#include "../../../models/OutputModelManager.h"
#include "../../wxUtilities.h"
#include "UtilFunctions.h"

#define MAX_CB_CHANNELS 128

ChannelBlockPropertyAdapter::ChannelBlockPropertyAdapter(Model& model)
    : ModelPropertyAdapter(model), _channelBlock(static_cast<ChannelBlockModel&>(model)) {}

void ChannelBlockPropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    wxPGProperty* p = grid->Append(new wxUIntProperty("# Channels", "ChannelBlockCount", _channelBlock.GetNumChannels()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", MAX_CB_CHANNELS);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxBoolProperty("Indiv Colors", "ChannelProperties", true));
    p->Enable(false);

    const auto& colors = _channelBlock.GetChannelColors();
    for (int x = 0; x < _channelBlock.GetNumChannels(); ++x) {
        wxString nm = "ChannelColor" + std::to_string(x + 1);
        grid->AppendIn(p, new wxColourProperty(nm, nm, wxColor(colors[x])));
    }
}

int ChannelBlockPropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if ("ChannelBlockCount" == event.GetPropertyName()) {
        _channelBlock.SetNumChannels(static_cast<int>(event.GetPropertyValue().GetLong()));
        _channelBlock.IncrementChangeCount();
        _channelBlock.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_RELOAD_PROPERTYGRID |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "ChannelBlockPropertyAdapter::OnPropertyGridChange::ChannelBlockCount");
        return 0;
    } else if (event.GetPropertyName().StartsWith("ChannelProperties.")) {
        wxColor c;
        c << event.GetProperty()->GetValue();
        xlColor xc = wxColourToXlColor(c);
        std::string text = event.GetPropertyName();
        int val = ExtractTrailingInt(text);
        if (val < 1) val = 1;
        if (val > _channelBlock.GetNumChannels()) val = _channelBlock.GetNumChannels();
        _channelBlock.SetChannelColor(val - 1, std::string(xc));
        _channelBlock.IncrementChangeCount();
        _channelBlock.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "ChannelBlockPropertyAdapter::OnPropertyGridChange::ChannelProperties");
        return 0;
    }

    return ModelPropertyAdapter::OnPropertyGridChange(grid, event);
}

void ChannelBlockPropertyAdapter::DisableUnusedProperties(wxPropertyGridInterface* grid) {
    wxPGProperty* p = grid->GetPropertyByName("ModelStringType");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("ModelStringColor");
    if (p != nullptr) {
        p->Enable(false);
    }
}
