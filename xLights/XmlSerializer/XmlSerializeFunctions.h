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
#include "../models/Model.h"
#include <pugixml.hpp>
#include <optional>
#include <map>
#include <vector>
#include <string>

namespace XmlSerialize {

std::vector<std::vector<std::vector<int>>> ParseCustomModel(const std::string& customModel);
std::vector<std::vector<std::vector<int>>> ParseCompressed(const std::string& compressed);
std::vector<std::vector<std::vector<int>>> ParseCustomModelDataFromXml(pugi::xml_node node);
void DeserializeFaceInfo(pugi::xml_node f, FaceStateData & faceInfo);
void DeserializeStateInfo(pugi::xml_node f, FaceStateData & stateInfo);

void DeserializeModelScreenLocationAttributes(BaseObject* object, pugi::xml_node node, bool importing);
void DeserializeTwoPointScreenLocationAttributes(BaseObject* object, pugi::xml_node node);
void DeserializeThreePointScreenLocationAttributes(BaseObject* object, pugi::xml_node node);
void DeserializePolyPointScreenLocationAttributes(BaseObject* object, pugi::xml_node node);

void SerializeModelGroupsForModel(const Model* object, pugi::xml_node docNode);
void AddDimensions(pugi::xml_node node, const Model* m);

// String-based variants (no DOM; write directly into a StringSerializingVisitor)
// Declared here for convenience; StringSerializingVisitor is defined globally (not in this namespace).
void SerializeModelGroupsForModelToString(const Model* model, class StringSerializingVisitor& visitor);
void AddDimensionsToString(const Model* m, class StringSerializingVisitor& visitor);

// Data structure for importing custom models
struct CustomModelImportData {
    std::string name;
    int width;
    int height;
    int depth;
    std::vector<std::vector<std::vector<int>>> modelData;

    // Submodel data
    struct SubModelData {
        std::string name;
        bool vertical;
        bool isRanges;
        std::vector<std::string> strands;
        std::string subBuffer;
        std::string bufferStyle;
    };
    std::vector<SubModelData> subModels;

    // Face data
    struct FaceData {
        std::string name;
        std::string type;
        std::map<std::string, std::string> attributes;
    };
    std::vector<FaceData> faces;

    // State data
    struct StateData {
        std::string name;
        std::string type;
        std::map<std::string, std::string> attributes;
    };
    std::vector<StateData> states;
};

// Simple SubModel data structure for standalone import
struct SubModelImportData {
    std::string name;
    bool vertical;
    bool isRanges;
    std::vector<std::string> strands;  // node ranges like "1-10,15,20-25"
    std::string subBuffer;              // subbuffer definition like "10x10x50x50"
    std::string bufferStyle;           // "Default", "Keep XY", "Stacked"
};

// Parse a single submodel XML node into the data structure
[[nodiscard]] std::optional<SubModelImportData> ParseSubModelNode(pugi::xml_node node);

// Load custom model import data from XML file
[[nodiscard]] std::optional<CustomModelImportData> LoadCustomModelFromFile(const std::string& filename);

// Load custom model import data from XML node (for internal use)
[[nodiscard]] std::optional<CustomModelImportData> LoadCustomModelFromXml(pugi::xml_node node);

// Load submodels from a model's XML node
[[nodiscard]] std::vector<SubModelImportData> LoadSubModelsFromXml(pugi::xml_node modelNode);

// Convert model XML attributes to JSON string
std::string GetModelAttributesAsJSON(pugi::xml_node modelNode);

}
