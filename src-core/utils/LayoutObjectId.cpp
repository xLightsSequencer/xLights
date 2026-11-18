/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "LayoutObjectId.h"

#include "string_utils.h"

#include <map>

namespace LayoutObjectId {

std::string Generate()
{
    return GenerateGuid();
}

bool Claim(std::string& id, std::unordered_set<std::string>& used)
{
    bool assigned = false;
    if (id.empty() || used.count(id) != 0) {
        do {
            id = Generate();
        } while (used.count(id) != 0);
        assigned = true;
    }
    used.insert(id);
    return assigned;
}

std::string Read(pugi::xml_node node)
{
    return node.attribute(ATTRIBUTE).as_string("");
}

void Write(pugi::xml_node node, const std::string& id)
{
    if (!node) return;
    if (id.empty()) {
        node.remove_attribute(ATTRIBUTE);
        return;
    }
    pugi::xml_attribute a = node.attribute(ATTRIBUTE);
    if (!a) {
        a = node.append_attribute(ATTRIBUTE);
    }
    a.set_value(id.c_str());
}

void StripFromXml(pugi::xml_node node)
{
    if (!node || node.type() != pugi::node_element) return;
    node.remove_attribute(ATTRIBUTE);
    for (pugi::xml_node c = node.first_child(); c; c = c.next_sibling()) {
        StripFromXml(c);
    }
}

void CopyLocalIds(pugi::xml_node local, pugi::xml_node replacement,
                  char const* subRecordName, char const* nameAttribute)
{
    if (!replacement) return;
    Write(replacement, Read(local));

    std::map<std::string, std::string> localSubIds;
    for (pugi::xml_node c = local.child(subRecordName); c; c = c.next_sibling(subRecordName)) {
        localSubIds.emplace(c.attribute(nameAttribute).as_string(""), Read(c));
    }
    for (pugi::xml_node c = replacement.child(subRecordName); c; c = c.next_sibling(subRecordName)) {
        auto it = localSubIds.find(c.attribute(nameAttribute).as_string(""));
        Write(c, it == localSubIds.end() ? std::string() : it->second);
    }
}

void StampChildren(pugi::xml_node parent, char const* childName,
                   std::function<std::string(std::string const&)> const& idForName,
                   std::unordered_set<std::string>& used,
                   char const* nameAttribute)
{
    for (pugi::xml_node c = parent.child(childName); c; c = c.next_sibling(childName)) {
        std::string id;
        if (idForName) {
            id = idForName(c.attribute(nameAttribute).as_string(""));
        }
        if (id.empty()) {
            id = Read(c);
        }
        Claim(id, used);
        Write(c, id);
    }
}

} // namespace LayoutObjectId
