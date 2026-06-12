/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ModelSet.h"

#include <algorithm>
#include <sstream>

bool ModelSet::HasMember(const std::string& modelName) const
{
    return std::find(_members.begin(), _members.end(), modelName) != _members.end();
}

void ModelSet::AddMember(const std::string& modelName)
{
    if (!HasMember(modelName)) {
        _members.push_back(modelName);
    }
}

void ModelSet::RemoveMember(const std::string& modelName)
{
    _members.erase(std::remove(_members.begin(), _members.end(), modelName),
                   _members.end());
}

void ModelSet::RenameMember(const std::string& oldName, const std::string& newName)
{
    for (auto& m : _members) {
        if (m == oldName) {
            m = newName;
        }
    }
}

void ModelSet::Load(pugi::xml_node node)
{
    _name = node.attribute("name").as_string();
    _members.clear();
    std::string modelsAttr = node.attribute("models").as_string();
    std::stringstream ss(modelsAttr);
    std::string token;
    while (std::getline(ss, token, ',')) {
        if (!token.empty()) {
            _members.push_back(token);
        }
    }
}

void ModelSet::Save(pugi::xml_node node) const
{
    node.append_attribute("name") = _name.c_str();
    std::string list;
    for (size_t i = 0; i < _members.size(); ++i) {
        if (i > 0) list += ",";
        list += _members[i];
    }
    node.append_attribute("models") = list.c_str();
}
