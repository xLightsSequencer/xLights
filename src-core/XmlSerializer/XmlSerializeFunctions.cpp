/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "StringSerializingVisitor.h"
#include "XmlSerializeFunctions.h"
#include "XmlNodeKeys.h"
#include "UtilFunctions.h"
#include "FileUtils.h"
#include "../models/ModelGroup.h"
#include "../models/ModelManager.h"
#include "../render/UICallbacks.h"

#include <algorithm>
#include <string_view>
#include <spdlog/fmt/fmt.h>

namespace XmlSerialize {

void DeserializeModelScreenLocationAttributes(BaseObject* object, pugi::xml_node node, bool importing) {
    glm::vec3 loc;
    loc.x = node.attribute(XmlNodeKeys::WorldPosXAttribute).as_float(0);
    loc.y = node.attribute(XmlNodeKeys::WorldPosYAttribute).as_float(0);
    loc.z = node.attribute(XmlNodeKeys::WorldPosZAttribute).as_float(0);
    object->GetBaseObjectScreenLocation().SetWorldPosition(loc);
    glm::vec3 scale(1.0f, 1.0f, 1.0f);
    scale.x = node.attribute(XmlNodeKeys::ScaleXAttribute).as_float(1.0f);
    scale.y = node.attribute(XmlNodeKeys::ScaleYAttribute).as_float(1.0f);
    scale.z = node.attribute(XmlNodeKeys::ScaleZAttribute).as_float(1.0f);
    object->GetBaseObjectScreenLocation().SetScaleMatrix(scale);
    glm::vec3 rotate(0.0f, 0.0f, 0.0f);
    rotate.x = node.attribute(XmlNodeKeys::RotateXAttribute).as_float(0);
    rotate.y = node.attribute(XmlNodeKeys::RotateYAttribute).as_float(0);
    rotate.z = node.attribute(XmlNodeKeys::RotateZAttribute).as_float(0);
    object->GetBaseObjectScreenLocation().SetRotation(rotate);
    if( !importing ) {
         bool locked = node.attribute(XmlNodeKeys::LockedAttribute).as_int(0) > 0;
        object->GetBaseObjectScreenLocation().Lock(locked);
    }
}

void DeserializeTwoPointScreenLocationAttributes(BaseObject* object, pugi::xml_node node) {
    float x2 = node.attribute(XmlNodeKeys::X2Attribute).as_float(0);
    float y2 = node.attribute(XmlNodeKeys::Y2Attribute).as_float(0);
    float z2 = node.attribute(XmlNodeKeys::Z2Attribute).as_float(0);
    TwoPointScreenLocation& screenLoc = dynamic_cast<TwoPointScreenLocation&>(object->GetBaseObjectScreenLocation());
    screenLoc.SetX2(x2);
    screenLoc.SetY2(y2);
    screenLoc.SetZ2(z2);
}

void DeserializeThreePointScreenLocationAttributes(BaseObject* object, pugi::xml_node node) {
    DeserializeTwoPointScreenLocationAttributes(object, node);
    int angle = node.attribute(XmlNodeKeys::AngleAttribute).as_int(0);
    float height = node.attribute(XmlNodeKeys::HeightAttribute).as_float(1.0f);
    float shear = node.attribute(XmlNodeKeys::ShearAttribute).as_float(0.0f);
    ThreePointScreenLocation& screenLoc = dynamic_cast<ThreePointScreenLocation&>(object->GetBaseObjectScreenLocation());
    screenLoc.SetAngle(angle);
    screenLoc.SetMHeight(height);
    screenLoc.SetYShear(shear);
}

void DeserializePolyPointScreenLocationAttributes(BaseObject* object, pugi::xml_node node) {
    int num_points = node.attribute(XmlNodeKeys::NumPointsAttribute).as_int(2);
    PolyPointScreenLocation& screenLoc = dynamic_cast<PolyPointScreenLocation&>(object->GetBaseObjectScreenLocation());
    screenLoc.SetNumPoints(num_points);
    screenLoc.SetDataFromString(node.attribute(XmlNodeKeys::PointDataAttribute).as_string("0.0, 0.0, 0.0, 0.0, 0.0, 0.0"));
    screenLoc.SetCurveDataFromString(node.attribute(XmlNodeKeys::cPointDataAttribute).as_string());
}


std::vector<std::vector<std::vector<int>>> ParseCustomModel(const std::string& customModel)
{
    // layers - rows - cols
    std::vector<std::vector<std::vector<int>>> locations;

    uint32_t width = 1;
    uint32_t height = 1;

    std::vector<std::string> layers;
    std::vector<std::string> rows;
    std::vector<std::string> cols;
    layers.reserve(20);
    rows.reserve(100);
    cols.reserve(100);

    Split(customModel, '|', layers);
    int layer = 0;

    for (auto lv : layers) {
        locations.emplace_back(std::vector<std::vector<int>>());

        rows.clear();
        Split(lv, ';', rows);
        height = rows.size();
        locations.back().resize(height);

        int row = 0;
        for (auto rv : rows) {
            cols.clear();
            Split(rv, ',', cols);
            if (cols.size() > width)
                width = cols.size();
            int col = 0;
            locations.back()[row].resize(width, -1);
            for (auto value : cols) {
                while (value.length() > 0 && value[0] == ' ') {
                    value = value.substr(1);
                }
                if (!value.empty()) {
                    locations[layer][row][col] = (int)std::strtol(value.c_str(), nullptr, 10);
                }
                col++;
            }
            row++;
        }
        layer++;
    }

    for (auto& lyr : locations) {
        lyr.resize(height);
        for (auto& rw : lyr) {
            rw.resize(width, -1);
        }
    }

    return locations;
}

std::vector<std::vector<std::vector<int>>> ParseCompressed(const std::string& compressed) {
    // node, row, col, [layer];
    std::vector<std::vector<std::vector<int>>> locations;

    // parse all the strings
    std::vector<std::tuple<int,int,int,int>> nodes;
    nodes.reserve(4000);
    std::vector<std::string> nodeStrings;
    nodeStrings.reserve(4000);
    Split(compressed, ';', nodeStrings);
    for (const auto& n : nodeStrings) {
        std::vector<std::string> nodeData;
        Split(n, ',', nodeData);
        if (nodeData.size() == 3) {
            nodes.emplace_back(std::make_tuple((int)std::strtol(nodeData[0].c_str(), nullptr, 10), (int)std::strtol(nodeData[1].c_str(), nullptr, 10), (int)std::strtol(nodeData[2].c_str(), nullptr, 10), 0));
        } else if (nodeData.size() == 4) {
            nodes.emplace_back(std::make_tuple((int)std::strtol(nodeData[0].c_str(), nullptr, 10), (int)std::strtol(nodeData[1].c_str(), nullptr, 10), (int)std::strtol(nodeData[2].c_str(), nullptr, 10), (int)std::strtol(nodeData[3].c_str(), nullptr, 10)));
        }
    }

    // work out the required dimensions
    int layers = 0;
    int rows = 0;
    int cols = 0;
    for (const auto& n : nodes) {
        layers = std::max(layers, std::get<3>(n));
        rows = std::max(rows, std::get<1>(n));
        cols = std::max(cols, std::get<2>(n));
    }

    // create enough space
    locations.reserve(layers + 1);
    for (int l = 0; l <= layers; l++) {
        locations.emplace_back(std::vector<std::vector<int>>());
        locations.back().reserve(rows + 1);
        for (int r = 0; r <= rows; r++) {
            locations.back().emplace_back(std::vector<int>());
            locations.back().back().reserve(cols + 1);
            for (int c = 0; c <= cols; c++) {
                locations.back().back().emplace_back(-1);
            }
        }
    }

    // fill in data
    for (const auto& n : nodes)
    {
        locations[std::get<3>(n)][std::get<1>(n)][std::get<2>(n)] = std::get<0>(n);
    }

    return locations;
}

std::vector<std::vector<std::vector<int>>> ParseCustomModelDataFromXml(pugi::xml_node node)
{
    std::string compressed = node.attribute(XmlNodeKeys::CustomModelCmpAttribute).as_string();
    if (!compressed.empty()) {
        return XmlSerialize::ParseCompressed(compressed);
    } else {
        std::string customModel = node.attribute(XmlNodeKeys::CustomModelAttribute).as_string();
        return XmlSerialize::ParseCustomModel(customModel);
    }
}

void DeserializeFaceInfo(pugi::xml_node f, FaceStateData & faceInfo) {
    std::string name = f.attribute(XmlNodeKeys::StateNameAttribute).as_string("SingleNode");
    std::string type = f.attribute(XmlNodeKeys::StateTypeAttribute).as_string("SingleNode");
    if (name.empty()) {
        name = type;
    }
    if (!(type == "SingleNode" || type == "NodeRange" || type == "Matrix")) {
        if (type == "Coro") {
            type = "SingleNode";
        } else {
            type = "Matrix";
        }
    }
    for (pugi::xml_attribute att = f.first_attribute(); att; att = att.next_attribute()) {
        std::string_view attName = att.name();
        if (attName != XmlNodeKeys::StateNameAttribute) {
            if (attName.substr(0, 5) == "Mouth" || attName.substr(0, 4) == "Eyes") {
                if (type == XmlNodeKeys::MatrixType) {
                    faceInfo[name][std::string(attName)] = FileUtils::FixFile(std::string(""), std::string(att.value()));
                    if (std::string(att.value()) != faceInfo[name][std::string(attName)])
                        att.set_value(faceInfo[name][std::string(attName)]);
                } else {
                    faceInfo[name][std::string(attName)] = att.value();
                }
            } else {
                faceInfo[name][std::string(attName)] = att.value();
            }
        }
    }
}

void DeserializeStateInfo(pugi::xml_node f, FaceStateData & stateInfo) {
    std::string name = f.attribute(XmlNodeKeys::StateNameAttribute).as_string("SingleNode");
    std::string type = f.attribute(XmlNodeKeys::StateTypeAttribute).as_string("SingleNode");
    if (name.empty()) {
        name = type;
    }
    if (!(type == "SingleNode" || type == "NodeRange")) {
        if (type == "Coro") {
            type = "SingleNode";
        }
    }
    for (pugi::xml_attribute att = f.first_attribute(); att; att = att.next_attribute()) {
        std::string_view attName = att.name();
        if (attName != "Name") {
            std::string_view attValue = att.value();
            if (!attValue.empty()) { // we only save non default values to keep xml file small
                std::string key(attName);
                std::string value(attValue);
                std::string storedKey = key;
                if (key.find('s') == 0) { // Handle all keys starting with 's'
                    size_t sPos = key.find('s');
                    size_t dashPos = key.find('-');
                    size_t endPos = (dashPos != std::string::npos) ? dashPos : key.length();
                    if (sPos == 0 && sPos + 1 < endPos) {
                        std::string numStr = key.substr(sPos + 1, endPos - sPos - 1);
                        int num = (int)std::strtol(numStr.c_str(), nullptr, 10);
                        storedKey = fmt::format("s{:03d}{}", num, (dashPos != std::string::npos ? key.substr(dashPos) : ""));
                    }
                }
                stateInfo[name][storedKey] = value;
            }
        }
    }
}

std::optional<CustomModelImportData> LoadCustomModelFromFile(const std::string& filename) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(filename.c_str());
    if (!result) {
        return std::nullopt;
    }

    pugi::xml_node root = doc.document_element();
    if (!root || std::string_view(root.name()) != "custommodel") {
        return std::nullopt;
    }

    return LoadCustomModelFromXml(root);
}

std::optional<CustomModelImportData> LoadCustomModelFromXml(pugi::xml_node node) {
    if (!node || std::string_view(node.name()) != "custommodel") {
        return std::nullopt;
    }

    CustomModelImportData data;

    // Load basic attributes
    data.name = node.attribute("name").as_string();
    // Read new attribute names first, fall back to old parm names
    data.width = !node.attribute("CustomWidth").empty() ?
        node.attribute("CustomWidth").as_int(1) : node.attribute("parm1").as_int(1);
    data.height = !node.attribute("CustomHeight").empty() ?
        node.attribute("CustomHeight").as_int(1) : node.attribute("parm2").as_int(1);
    data.depth = node.attribute("Depth").as_int(1);

    // Check if it's a 1-depth model (or empty for pre-3D models)
    std::string_view depthAttr = node.attribute("Depth").as_string();
    if (!depthAttr.empty() && data.depth != 1) {
        return std::nullopt; // Not a 2D model
    }

    // Load model data
    data.modelData = ParseCustomModelDataFromXml(node);

    // Load submodels
    for (pugi::xml_node child = node.first_child(); child; child = child.next_sibling()) {
        std::string_view childName = child.name();
        if (childName == "subModel") {
            if (auto smData = ParseSubModelNode(child)) {
                // Only import range-based submodels
                if (smData->isRanges) {
                    CustomModelImportData::SubModelData sm;
                    sm.name = smData->name;
                    sm.vertical = smData->vertical;
                    sm.isRanges = smData->isRanges;
                    sm.subBuffer = smData->subBuffer;
                    sm.bufferStyle = smData->bufferStyle;
                    sm.strands = smData->strands;
                    data.subModels.push_back(std::move(sm));
                }
            }
        }
        else if (childName == "faceInfo") {
            CustomModelImportData::FaceData face;
            face.name = child.attribute("Name").as_string();
            face.type = child.attribute("Type").as_string();

            if (face.type == "NodeRange") {
                // Store all attributes
                for (pugi::xml_attribute attr = child.first_attribute(); attr; attr = attr.next_attribute()) {
                    face.attributes[attr.name()] = attr.value();
                }
                data.faces.push_back(std::move(face));
            }
        }
        else if (childName == "stateInfo") {
            CustomModelImportData::StateData state;
            state.name = child.attribute("Name").as_string();
            state.type = child.attribute("Type").as_string();

            if (state.type == "NodeRange") {
                // Store all attributes
                for (pugi::xml_attribute attr = child.first_attribute(); attr; attr = attr.next_attribute()) {
                    state.attributes[attr.name()] = attr.value();
                }
                data.states.push_back(std::move(state));
            }
        }
    }

    return data;
}

std::optional<SubModelImportData> ParseSubModelNode(pugi::xml_node node) {
    if (!node || std::string_view(node.name()) != "subModel") {
        return std::nullopt;
    }

    SubModelImportData sm;
    sm.name = node.attribute(XmlNodeKeys::NameAttribute).as_string();
    sm.isRanges = std::string_view(node.attribute(XmlNodeKeys::SMTypeAttribute).as_string("ranges")) == "ranges";
    sm.vertical = std::string_view(node.attribute(XmlNodeKeys::LayoutAttribute).as_string("vertical")) == "vertical";
    sm.subBuffer = node.attribute(XmlNodeKeys::SubBufferAttribute).as_string();
    sm.bufferStyle = node.attribute(XmlNodeKeys::BufferStyleAttribute).as_string("Default");

    // Load strands/lines
    int lineNum = 0;
    while (true) {
        std::string lineAttr = fmt::format("line{}", lineNum);
        pugi::xml_attribute attr = node.attribute(lineAttr);
        if (attr.empty()) break;
        sm.strands.push_back(attr.as_string());
        lineNum++;
    }

    return sm;
}

std::vector<SubModelImportData> LoadSubModelsFromXml(pugi::xml_node modelNode) {
    std::vector<SubModelImportData> subModels;

    if (!modelNode) {
        return subModels;
    }

    // Iterate through child nodes looking for subModel elements
    for (pugi::xml_node child = modelNode.first_child(); child; child = child.next_sibling()) {
        if (auto sm = ParseSubModelNode(child)) {
            subModels.push_back(std::move(*sm));
        }
    }

    return subModels;
}

std::string GetModelAttributesAsJSON(pugi::xml_node modelNode) {
    if (!modelNode) {
        return "{}";
    }

    std::string json = "{";
    bool first = true;

    // Process model attributes
    for (pugi::xml_attribute attrp = modelNode.first_attribute(); attrp; attrp = attrp.next_attribute()) {
        std::string_view value = attrp.value();
        if (!value.empty()) {
            if (!first) {
                json += ",";
            }
            json += "\"" + std::string(attrp.name()) + "\":\"" + JSONSafe(std::string(value)) + "\"";
            first = false;
        }
    }

    // Process ControllerConnection child node
    pugi::xml_node cc = modelNode.child("ControllerConnection");

    json += ",\"ControllerConnection\":{";
    if (cc) {
        bool first2 = true;
        for (pugi::xml_attribute attrp = cc.first_attribute(); attrp; attrp = attrp.next_attribute()) {
            std::string_view value = attrp.value();
            if (!value.empty()) {
                if (!first2) {
                    json += ",";
                }
                json += "\"" + std::string(attrp.name()) + "\":\"" + JSONSafe(std::string(value)) + "\"";
                first2 = false;
            }
        }
    }
    json += "}}";

    return json;
}

void SerializeModelGroupsForModel(const Model* model, pugi::xml_node docNode) {
    if (model == nullptr) return;

    // Find this model's node within docNode by matching the name attribute,
    // since docNode may contain multiple model children in a multi-model export.
    pugi::xml_node modelNode;
    for (pugi::xml_node child : docNode.children()) {
        if (std::string_view(child.attribute("name").as_string("")) == model->GetName()) {
            modelNode = child;
            break;
        }
    }
    if (!modelNode) return;

    const ModelManager& mgr = model->GetModelManager();

    std::vector<std::string> allGroups;
    std::vector<std::string> onlyGroups;
    for (const auto& it : mgr.GetModels()) {
        if (it.second->GetDisplayAs() == DisplayAsType::ModelGroup) {
            if (dynamic_cast<ModelGroup*>(it.second)->OnlyContainsModel(model->Name())) {
                onlyGroups.push_back(it.first);
                allGroups.push_back(it.first);
            } else if (dynamic_cast<ModelGroup*>(it.second)->ContainsModelOrSubmodel(model)) {
                allGroups.push_back(it.first);
            }
        }
    }

    if (allGroups.empty()) {
        return;
    }

    UICallbacks* uiCallbacks = mgr.GetUICallbacks();
    if (!uiCallbacks) {
        return;
    }
    std::vector<std::string> selected = uiCallbacks->ChooseFromList(
        "Select Groups to Export for '" + model->GetName() + "' - cancel to include no groups", allGroups, onlyGroups);
    if (selected.empty()) {
        return;
    }

    // Serialize selected model groups to the modelNode (not docNode)
    for (const auto& it : mgr.GetModels()) {
        if (std::find(selected.begin(), selected.end(), it.first) != selected.end()) {
            ModelGroup* mg = dynamic_cast<ModelGroup*>(it.second);
            if (mg != nullptr) {
                // Get the model names from the ModelGroup
                const std::vector<std::string>& model_names = mg->ModelNames();

                // Create comma-delimited string of model names
                std::string modelsStr;
                for (size_t i = 0; i < model_names.size(); ++i) {
                    if (i > 0) {
                        modelsStr += ",";
                    }
                    modelsStr += model_names[i];
                }

                pugi::xml_node groupNode = modelNode.append_child("modelGroup");
                groupNode.append_attribute(XmlNodeKeys::NameAttribute) = it.first;
                groupNode.append_attribute(XmlNodeKeys::mgSelectedAttribute) = std::to_string(mg->IsSelected());
                groupNode.append_attribute(XmlNodeKeys::mgModelsAttribute) = modelsStr;
                groupNode.append_attribute(XmlNodeKeys::LayoutGroupAttribute) = mg->GetLayoutGroup();
                groupNode.append_attribute(XmlNodeKeys::LayoutAttribute) = mg->GetLayout();
                groupNode.append_attribute(XmlNodeKeys::mgGridSizeAttribute) = mg->GetGridSize();
                groupNode.append_attribute(XmlNodeKeys::mgCentrexAttribute) = std::to_string(mg->GetCentreX());
                groupNode.append_attribute(XmlNodeKeys::mgCentreyAttribute) = std::to_string(mg->GetCentreY());
                groupNode.append_attribute(XmlNodeKeys::mgCentreDefinedAttribute) = std::to_string(mg->GetCentreDefined());
            }
        }
    }
}

void AddDimensions(pugi::xml_node node, const Model* m) {
    std::string rdu = m->GetRulerDim();
    if (!rdu.empty()) {
        // Find this model's node by name — node may contain multiple model children.
        pugi::xml_node modelNode;
        for (pugi::xml_node child : node.children()) {
            if (std::string_view(child.attribute("name").as_string("")) == m->GetName()) {
                modelNode = child;
                break;
            }
        }
        if (!modelNode) return;

        pugi::xml_node xmlNode = modelNode.append_child(XmlNodeKeys::DimNodeName);
        xmlNode.append_attribute(XmlNodeKeys::DimDepthAttribute) = std::to_string(m->GetModelScreenLocation().GetRealDepth());
        xmlNode.append_attribute(XmlNodeKeys::DimHeightAttribute) = std::to_string(m->GetModelScreenLocation().GetRealHeight());
        xmlNode.append_attribute(XmlNodeKeys::DimUnitsAttribute) = rdu;
        xmlNode.append_attribute(XmlNodeKeys::DimWidthAttribute) = std::to_string(m->GetModelScreenLocation().GetRealWidth());
    }
}


} // end namespace XmlSerialize
