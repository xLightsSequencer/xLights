
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "LorController.h"
#include "UtilFunctions.h"

#include <cstdlib>

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

LorController::LorController(pugi::xml_node node) {

    _unit_id = node.attribute("UnitId").as_int(1);
    _num_channels = node.attribute("NumChannels").as_int(16);
    _type = node.attribute("CntlrType").as_string("AC Controller");
    _mode = AddressMode(node.attribute("AddrMode").as_int(1));
    _description = node.attribute("CntlrDesc").as_string("LOR Controller");
    _expanded = std::string_view(node.attribute("Expanded").as_string("TRUE")) == "TRUE";
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

void LorController::Save(pugi::xml_node node) {

    node.append_attribute("UnitId") = _unit_id;
    node.append_attribute("NumChannels") = _num_channels;
    node.append_attribute("CntlrType") = _type;
    node.append_attribute("AddrMode") = (int)_mode;
    node.append_attribute("CntlrDesc") = _description;
    node.append_attribute("Expanded") = _expanded ? "TRUE" : "FALSE";
    _dirty = false;
}
#pragma endregion

#pragma region Getters and Setters
int LorController::GetTotalNumChannels() const {

    if (StartsWith(_type, "Pixie")) {
        return GetNumChannels() * static_cast<int>(std::strtol(_type.substr(5).c_str(), nullptr, 10));
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
