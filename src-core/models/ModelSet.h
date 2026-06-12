#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// A Model Set is a persistent, name-based association between models on the
// Layout tab that constrains translation only: when any member is dragged,
// every member translates by the same delta. Geometry, properties, effects,
// and sequencer Model Group membership are untouched. Distinct from
// <modelGroup> (effect routing).
//
// A model belongs to at most one Set. Persistence: <modelSet> elements
// inside a top-level <modelSets> node in xlights_rgbeffects.xml.

#include <pugixml.hpp>
#include <string>
#include <vector>

class ModelSet {
public:
    ModelSet() = default;
    explicit ModelSet(const std::string& name) : _name(name) {}

    const std::string& GetName() const { return _name; }
    void SetName(const std::string& n) { _name = n; }

    const std::vector<std::string>& GetMembers() const { return _members; }
    bool HasMember(const std::string& modelName) const;
    void AddMember(const std::string& modelName);
    void RemoveMember(const std::string& modelName);
    void RenameMember(const std::string& oldName, const std::string& newName);

    // XML round-trip.
    void Load(pugi::xml_node node);
    void Save(pugi::xml_node node) const;

private:
    std::string _name;
    std::vector<std::string> _members;
};
