
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ControllerNullPropertyAdapter.h"

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/settings.h>

#include "outputs/Controller.h"
#include "outputs/ControllerNull.h"
#include "models/OutputModelManager.h"
#include "models/ModelManager.h"

ControllerNullPropertyAdapter::ControllerNullPropertyAdapter(Controller& controller)
    : ControllerPropertyAdapter(controller) {
}

void ControllerNullPropertyAdapter::AddProperties(wxPropertyGrid* propertyGrid, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) {

    ControllerPropertyAdapter::AddProperties(propertyGrid, modelManager, expandProperties);

    auto p = propertyGrid->Append(new wxStringProperty("Models", "Models", modelManager->GetModelsOnChannels(_controller.GetStartChannel(), _controller.GetEndChannel(), -1)));
    p->ChangeFlag(wxPGFlags::ReadOnly, true);
    p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));

    p = propertyGrid->Append(new wxUIntProperty("Channels", "Channels", _controller.GetChannels()));
    p->SetEditor("SpinCtrl");
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100000000);
}

bool ControllerNullPropertyAdapter::HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager) {

    if (ControllerPropertyAdapter::HandlePropertyEvent(event, outputModelManager)) return true;

    wxString const name = event.GetPropertyName();

    if (name == "Channels") {
        if (_controller.GetOutputCount() > 0) {
            _controller.GetFirstOutput()->SetChannels(event.GetValue().GetLong());
        }
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "NullOutput::HandlePropertyEvent::Channels");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "NullOutput::HandlePropertyEvent::Channels", nullptr);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "NullOutput::HandlePropertyEvent::Channels", nullptr);
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "NullOutput::HandlePropertyEvent::Channels", nullptr);
        return true;
    }
    return false;
}
