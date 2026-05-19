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

#include <list>
#include <string>

// Abstract node in the import-mapping tree, exposing only the surface that
// AutoMapper needs. The desktop's xLightsImportModelNode (a wxDataView tree
// store node) implements this interface; the iPad will provide its own
// concrete type backed by SwiftUI/ObjC++ state. AutoMapper itself is wx-free
// and works against this interface.
//
// Three node levels are represented in the same class via the
// (model, strand, node) triple. A bare model has empty strand and node; a
// strand or submodel has a non-empty strand and empty node; a node-level
// item has all three populated.
class ImportMappingNode {
public:
    virtual ~ImportMappingNode() = default;

    virtual const std::string& GetCoreModel() const = 0;
    virtual const std::string& GetCoreStrand() const = 0;
    virtual const std::string& GetCoreNode() const = 0;

    // Source name currently mapped onto this destination node. Empty if
    // unmapped.
    virtual const std::string& GetMapping() const = 0;

    virtual std::list<std::string> GetAliases() const = 0;

    virtual bool IsGroup() const = 0;

    // Composite "model[/strand[/node]]" name. Already exists on
    // xLightsImportModelNode.
    virtual std::string GetModelName() const = 0;

    // Record a mapping. `mappingModelType` is one of the model-type strings
    // (e.g. "Strand", "Node", "SubModel", "Unknown") that downstream UI may
    // surface in its column.
    virtual void Map(const std::string& mapTo, const std::string& mappingModelType) = 0;

    virtual unsigned int GetChildCount() const = 0;
    virtual ImportMappingNode* GetNthChild(unsigned int n) = 0;
};

// A flat source candidate from the incoming sequence. The dialog (or iPad
// equivalent) builds the available list before calling AutoMapper, lowering
// + trimming the canonical form once so the matcher loop doesn't repeat
// work. `displayName` preserves the caller's preferred casing — that's the
// string written into ImportMappingNode::Map. `modelType` is the resolved
// model-type tag (e.g. "Model", "ModelGroup", "SubModel") used when this
// source maps onto a destination model — caller resolves it from the
// layout once instead of having AutoMapper call back through wx code.
struct AvailableSource {
    std::string displayName;
    std::string canonicalName;
    std::string modelType;
    bool selected{ false };
};
