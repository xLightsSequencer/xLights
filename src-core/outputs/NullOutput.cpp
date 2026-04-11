
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "NullOutput.h"
#include "../models/OutputModelManager.h"

#include <spdlog/fmt/fmt.h>


#pragma region Constructors and Destructors
NullOutput::NullOutput(pugi::xml_node node) : Output(node) {

    SetId(node.attribute("Id").as_int(64001));
}

NullOutput::NullOutput(const NullOutput& from) : Output(from)
{
}

pugi::xml_node NullOutput::Save(pugi::xml_node parent) {

    pugi::xml_node node = parent.append_child("network");
    Output::SaveAttr(node);

    return node;
}
#pragma endregion

#pragma region Getters and Setters
std::string NullOutput::GetLongDescription() const {

    std::string res = "";

    if (!_enabled) res += "INACTIVE ";
    res += "NULL ";
    res += "(" + std::to_string(_startChannel) + "-" + std::to_string(GetEndChannel()) + ")";

    return res;
}

std::string NullOutput::GetSortName() const {
    return fmt::format("NULL{:02d}", _nullNumber);
}
#pragma endregion
