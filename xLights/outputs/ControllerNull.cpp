#include <wx/xml/xml.h>

#include "ControllerNull.h"
#include "OutputManager.h"
#include "Output.h"
#include "../UtilFunctions.h"
#include "../SpecialOptions.h"
#include "../OutputModelManager.h"
#include "NullOutput.h"

#pragma region Constructors and Destructors
ControllerNull::ControllerNull(OutputManager* om, wxXmlNode* node, const std::string& showDir) : Controller(om, node, showDir) {
    _dirty = false;
    wxASSERT(_outputs.size() == 1);
}

ControllerNull::ControllerNull(OutputManager* om) : Controller(om) {
    _name = om->UniqueName("Null_");
    _outputs.push_back(new NullOutput());
}

wxXmlNode* ControllerNull::Save() {
    wxXmlNode* um = Controller::Save();

    return um;
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

void ControllerNull::Convert(wxXmlNode* node, std::string showDir) {

    Controller::Convert(node, showDir);

    _outputs.push_back(Output::Create(node, showDir));
    if (_name == "" || StartsWith(_name, "Null_")) {
        _id = dynamic_cast<NullOutput*>(_outputs.front())->GetId();
        if (_outputs.back()->GetDescription() != "") {
            _name = _outputManager->UniqueName(_outputs.back()->GetDescription());
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
    return wxString::Format("Channel %ld maps to ...\nTYPE: NULL\nName: %s\nChannel: %ld\n%s", 
        ch, 
        GetName(), 
        ch - GetStartChannel() + 1, 
        (IsActive() ? _("") : _("INACTIVE\n")));
}

std::string ControllerNull::GetExport() const {
    return wxString::Format("%s,%ld,%ld,%s,NULL,,,,\"%s\",%d,%ld,%s,,%s",
        GetName(),
        GetStartChannel(),
        GetEndChannel(),
        GetVMF(),
        GetDescription(),
        GetId(),
        GetChannels(),
        (IsActive() ? _("") : _("DISABLED")),
        (IsAutoSize() ? _("AutoSize") : _(""))
    );
}
#pragma endregion

#pragma region UI
#ifndef EXCLUDENETWORKUI
void ControllerNull::AddProperties(wxPropertyGrid* propertyGrid) {

    Controller::AddProperties(propertyGrid);

    if (_outputs.size() > 0) _outputs.front()->AddProperties(propertyGrid, true);
}

bool ControllerNull::HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager) {

    if (Controller::HandlePropertyEvent(event, outputModelManager)) return true;

    //wxString const name = event.GetPropertyName();
    //wxPropertyGrid* grid = dynamic_cast<wxPropertyGrid*>(event.GetEventObject());

    if (_outputs.size() > 0) {
        if (_outputs.front()->HandlePropertyEvent(event, outputModelManager)) return true;
    }

    return false;
}
#endif
#pragma endregion
