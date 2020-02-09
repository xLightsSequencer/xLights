
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "NullOutput.h"
#include "../OutputModelManager.h"

#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#pragma region Constructors and Destructors
NullOutput::NullOutput(wxXmlNode* node) : Output(node) {

    SetId(wxAtoi(node->GetAttribute("Id", "64001")));
}

wxXmlNode* NullOutput::Save() {

    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "network");
    Output::Save(node);

    return node;
}
#pragma endregion

#pragma region Getters and Setters
std::string NullOutput::GetLongDescription() const {

    std::string res = "";

    if (!_enabled) res += "INACTIVE ";
    res += "NULL ";
    res += "(" + std::string(wxString::Format(wxT("%d"), _startChannel)) + "-" + std::string(wxString::Format(wxT("%d"), GetEndChannel())) + ")";

    return res;
}

std::string NullOutput::GetSortName() const {
    return wxString::Format("NULL%02d", _nullNumber).ToStdString();
}
#pragma endregion

#pragma region UI
#ifndef EXCLUDENETWORKUI
void NullOutput::AddProperties(wxPropertyGrid* propertyGrid, bool allSameSize) {

    wxPGProperty* p = propertyGrid->Append(new wxUIntProperty("Channels", "Channels", GetChannels()));
    p->SetEditor("SpinCtrl");
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 10000000);
}

bool NullOutput::HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager) {

    wxString name = event.GetPropertyName();
    wxPropertyGrid* grid = dynamic_cast<wxPropertyGrid*>(event.GetEventObject());

    if (name == "Channels")
    {
        SetChannels(event.GetValue().GetLong());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "NullOutput::HandlePropertyEvent::Channels");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "NullOutput::HandlePropertyEvent::Channels", nullptr);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "NullOutput::HandlePropertyEvent::Channels", nullptr);
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "NullOutput::HandlePropertyEvent::Channels", nullptr);
        return true;
    }

    if (Output::HandlePropertyEvent(event, outputModelManager)) return true;

    return false;
}
#endif
#pragma endregion
