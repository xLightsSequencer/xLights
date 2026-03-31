
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ControllerNull.h"
#include "OutputManager.h"
#include "Output.h"
#include "UtilFunctions.h"
#include "../SpecialOptions.h"
#include "../OutputModelManager.h"
#include "NullOutput.h"
#include "../models/ModelManager.h"

#pragma region Constructors and Destructors
ControllerNull::ControllerNull(OutputManager* om, pugi::xml_node node, const std::string& showDir) : Controller(om, node, showDir) {
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

pugi::xml_node ControllerNull::Save(pugi::xml_node parent) {
    pugi::xml_node um = Controller::Save(parent);

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
    res += "(" + std::to_string(GetStartChannel()) + "-" + std::to_string(GetEndChannel()) + ") ";
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

void ControllerNull::Convert(pugi::xml_node node, std::string showDir) {

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
    return std::format("Channel {} maps to ...\nTYPE: NULL\nName: {}\nChannel: {}\n{}",
        ch,
        GetName(),
        ch - GetStartChannel() + 1,
        (IsActive() ? "" : "INACTIVE\n"));
}

std::string ControllerNull::GetExport() const {
    return std::format("{},{},{},{},NULL,,,,\"{}\",{},{},{},,{},{},,,{}",
        GetName(),
        GetStartChannel(),
        GetEndChannel(),
        GetVMV(),
        GetDescription(),
        GetId(),
        GetChannels(),
        (IsActive() ? "" : "DISABLED"),
        (IsAutoSize() ? "AutoSize" : ""),
        (IsAutoLayout() ? "AutoLayout" : ""),
        GetFPPProxy()
    );
}
#pragma endregion

// UI property grid methods moved to ui/controllerproperties/ControllerNullPropertyAdapter
#pragma endregion
