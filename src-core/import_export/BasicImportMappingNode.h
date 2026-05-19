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

#include "ImportMappingNode.h"

#include <list>
#include <memory>
#include <string>
#include <vector>

// Concrete, wx-free implementation of ImportMappingNode. Used by the iPad
// import flow where there's no wxDataView tree to back the data; the
// desktop continues to use xLightsImportModelNode (which inherits from
// both wxDataViewTreeStoreNode and ImportMappingNode).
//
// Owns its children via unique_ptr — drop the root and the whole subtree
// goes with it. Field access is direct; the ImportMappingNode interface
// methods are one-line forwards to the matching field.
class BasicImportMappingNode : public ImportMappingNode {
public:
    BasicImportMappingNode() = default;
    BasicImportMappingNode(std::string model,
                           std::string strand,
                           std::string node,
                           bool isGroup,
                           std::list<std::string> aliases,
                           std::string modelType,
                           bool isSubmodel) :
        _model(std::move(model)),
        _strand(std::move(strand)),
        _node(std::move(node)),
        _group(isGroup),
        _aliases(std::move(aliases)),
        _modelType(std::move(modelType)),
        _isSubmodel(isSubmodel) {}

    // ImportMappingNode interface
    const std::string& GetCoreModel() const override { return _model; }
    const std::string& GetCoreStrand() const override { return _strand; }
    const std::string& GetCoreNode() const override { return _node; }
    const std::string& GetMapping() const override { return _mapping; }
    std::list<std::string> GetAliases() const override { return _aliases; }
    bool IsGroup() const override { return _group; }
    std::string GetModelName() const override {
        std::string name = _model;
        if (!_strand.empty()) name += "/" + _strand;
        if (!_node.empty()) name += "/" + _node;
        return name;
    }
    void Map(const std::string& mapTo, const std::string& mappingModelType) override {
        _mapping = mapTo;
        _mappingExists = true;
        _mappingModelType = mappingModelType;
    }
    unsigned int GetChildCount() const override { return static_cast<unsigned int>(_children.size()); }
    BasicImportMappingNode* GetNthChild(unsigned int n) override {
        return n < _children.size() ? _children[n].get() : nullptr;
    }

    // Owning-add. Returns the inserted child for the caller to populate
    // further (chaining strands beneath models, nodes beneath strands).
    BasicImportMappingNode* AddChild(std::unique_ptr<BasicImportMappingNode> child) {
        BasicImportMappingNode* raw = child.get();
        _children.push_back(std::move(child));
        return raw;
    }

    void ClearMapping() {
        _mappingExists = true;
        _mapping.clear();
        _mappingModelType.clear();
        for (auto& c : _children) c->ClearMapping();
    }

    // Recursive — true if this node or any descendant has a mapping.
    bool HasMapping() const {
        if (!_mapping.empty()) return true;
        for (const auto& c : _children) {
            if (c->HasMapping()) return true;
        }
        return false;
    }

    // Public — matches the public-field convention of xLightsImportModelNode
    // so call sites that touch _model / _mapping / etc. work uniformly.
    std::string _model;
    std::string _strand;
    std::string _node;
    std::string _mapping;
    bool _group{ false };
    bool _mappingExists{ false };
    std::list<std::string> _aliases;
    std::string _modelType;
    std::string _groupModels;
    bool _isSubmodel{ false };
    std::string _modelClass;
    int _nodeCount{ 0 };
    int _strandCount{ 0 };
    int _effectCount{ 0 };
    std::string _mappingModelType;

private:
    std::vector<std::unique_ptr<BasicImportMappingNode>> _children;
};
