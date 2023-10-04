
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "LorController.h"
#include "../UtilFunctions.h"

#include <wx/wxchar.h>

#pragma region Constructors and Destructors
LorController::LorController() {

    _unit_id = 1;
    _num_channels = 16;
    _type = "AC Controller";
    _mode = AddressMode::LOR_ADDR_MODE_NORMAL;
    _description = "";
    _dirty = true;
    _expanded = true;
}

LorController::LorController(wxXmlNode* node) {

    _unit_id = wxAtoi(node->GetAttribute("UnitId", "1"));
    _num_channels = wxAtoi(node->GetAttribute("NumChannels", "16"));
    _type = node->GetAttribute("CntlrType", "AC Controller").ToStdString();
    _mode = AddressMode(wxAtoi(node->GetAttribute("AddrMode", "1")));
    _description = node->GetAttribute("CntlrDesc", "LOR Controller").ToStdString();
    _expanded = node->GetAttribute("Expanded", "TRUE") == "TRUE";
    _dirty = false;
}

LorController::LorController(const LorController& from)
{
    _unit_id = from._unit_id;
    _num_channels = from._num_channels;
    _type = from._type;
    _mode = from._mode;
    _description = from._description;
    _expanded = from._expanded;
    _dirty = true;
}

void LorController::Save(wxXmlNode* node) {
    
    node->AddAttribute("UnitId", wxString::Format("%d", _unit_id));
    node->AddAttribute("NumChannels", wxString::Format("%d", _num_channels));
    node->AddAttribute("CntlrType", wxString::Format("%s", _type));
    node->AddAttribute("AddrMode", wxString::Format("%d", _mode));
    node->AddAttribute("CntlrDesc", wxString::Format("%s", _description));
    node->AddAttribute("Expanded", _expanded ? _("TRUE") : _("FALSE"));
    _dirty = false;
}
#pragma endregion

#pragma region Getters and Setters
int LorController::GetTotalNumChannels() const {

    if (StartsWith(_type, "Pixie")) {
        return GetNumChannels() * wxAtoi(_type.substr(5));
    }

    return GetNumChannels();
}

std::string LorController::GetModeString() const {

    std::string result = "";
    if( _mode == AddressMode::LOR_ADDR_MODE_NORMAL ) {
        result = "Normal";
    }
    else if( _mode == AddressMode::LOR_ADDR_MODE_LEGACY ) {
        result = "Legacy";
    }
    else if( _mode == AddressMode::LOR_ADDR_MODE_SPLIT ) {
        result = "Split";
    }
    return result;
}
#pragma endregion