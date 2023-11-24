
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/xml/xml.h>

#include "ControllerNull.h"
#include "OutputManager.h"
#include "Output.h"
#include "../UtilFunctions.h"
#include "../SpecialOptions.h"
#include "../OutputModelManager.h"
#include "NullOutput.h"
#include "../models/ModelManager.h"

#pragma region Constructors and Destructors
ControllerNull::ControllerNull(OutputManager* om, wxXmlNode* node, const std::string& showDir) : Controller(om, node, showDir) {
    _dirty = false;
    wxASSERT(_outputs.size() == 1);
}

ControllerNull::ControllerNull(OutputManager* om) : Controller(om) {
    _name = om->UniqueName("Null_");
    _outputs.push_back(new NullOutput());
}

ControllerNull::ControllerNull(OutputManager* om, const ControllerNull& from) :
    Controller(om, from)
{
    // Nothing to actually copy
}

wxXmlNode* ControllerNull::Save() {
    wxXmlNode* um = Controller::Save();

    return um;
}
bool ControllerNull::UpdateFrom(Controller* from)
{
    bool changed = Controller::UpdateFrom(from);

    // Nothing to copy that wont be copied by the base
    //ControllerNull* fromNull = static_cast<ControllerNull*>(from);

    return changed;
}
Controller* ControllerNull::Copy(OutputManager* om)
{
    return new ControllerNull(om, *this);
}
#pragma endregion

#pragma region Virtual Functions
void ControllerNull::SetId(int id) {
    Controller::SetId(id);
    dynamic_cast<NullOutput*>(GetFirstOutput())->SetId(id);
}

std::string ControllerNull::GetLongDescription() const {

    std::string res = "";
    if (!IsActive()) res += "INACTIVE ";
    res += GetName() + "  NULL ";
    res += "(" + std::string(wxString::Format(wxT("%ld"), GetStartChannel())) + "-" + std::string(wxString::Format(wxT("%ld"), GetEndChannel())) + ") ";
    res += _description;

    return res;
}

std::string ControllerNull::GetShortDescription() const {

	std::string res = "";
	if (!IsActive()) res += "INACTIVE ";
	res += GetName() + "  NULL";
	if (!_description.empty()) {
		res += " ";
		res += _description;
	}
	return res;
}

void ControllerNull::Convert(wxXmlNode* node, std::string showDir) {

    Controller::Convert(node, showDir);

    _outputs.push_back(Output::Create(this, node, showDir));
    if (_name == "" || StartsWith(_name, "Null_")) {
        _id = dynamic_cast<NullOutput*>(_outputs.front())->GetId();
        if (_outputs.back()->GetDescription_CONVERT() != "") {
            _name = _outputManager->UniqueName(_outputs.back()->GetDescription_CONVERT());
        }
        else {
            _name = _outputManager->UniqueName("Unnamed");
        }
    }
    if (_outputs.back() == nullptr) {
        // this shouldnt happen unless we are loading a future file with an output type we dont recognise
        _outputs.pop_back();
    }
}

std::string ControllerNull::GetChannelMapping(int32_t ch) const {
    return wxString::Format("Channel %d maps to ...\nTYPE: NULL\nName: %s\nChannel: %d\n%s",
        ch, 
        GetName(), 
        ch - GetStartChannel() + 1, 
        (IsActive() ? _("") : _("INACTIVE\n")));
}

std::string ControllerNull::GetExport() const {
    return wxString::Format("%s,%d,%d,%s,NULL,,,,\"%s\",%d,%d,%s,,%s,%s,,,%s",
        GetName(),
        GetStartChannel(),
        GetEndChannel(),
        GetVMV(),
        GetDescription(),
        GetId(),
        GetChannels(),
        (IsActive() ? _("") : _("DISABLED")),
        (IsAutoSize() ? _("AutoSize") : _("")),
        (IsAutoLayout() ? _("AutoLayout") : _("")),
        GetFPPProxy()
    );
}
#pragma endregion

#pragma region UI
#ifndef EXCLUDENETWORKUI
void ControllerNull::AddProperties(wxPropertyGrid* propertyGrid, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) {

    Controller::AddProperties(propertyGrid, modelManager, expandProperties);

    auto p = propertyGrid->Append(new wxStringProperty("Models", "Models", modelManager->GetModelsOnChannels(GetStartChannel(), GetEndChannel(), -1)));
    p->ChangeFlag(wxPG_PROP_READONLY, true);
        p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));

    p = propertyGrid->Append(new wxUIntProperty("Channels", "Channels", GetChannels()));
    p->SetEditor("SpinCtrl");
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100000000);
}

bool ControllerNull::HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager) {

    if (Controller::HandlePropertyEvent(event, outputModelManager)) return true;

    wxString const name = event.GetPropertyName();
    //wxPropertyGrid* grid = dynamic_cast<wxPropertyGrid*>(event.GetEventObject());

    if (name == "Channels") {
        if (_outputs.size() > 0) {
            _outputs.front()->SetChannels(event.GetValue().GetLong());
        }
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "NullOutput::HandlePropertyEvent::Channels");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "NullOutput::HandlePropertyEvent::Channels", nullptr);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "NullOutput::HandlePropertyEvent::Channels", nullptr);
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "NullOutput::HandlePropertyEvent::Channels", nullptr);
        return true;
    }
    return false;
}
#endif
#pragma endregion
