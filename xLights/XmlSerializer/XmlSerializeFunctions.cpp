/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "XmlSerializeFunctions.h"
#include "XmlNodeKeys.h"
#include "../UtilFunctions.h"

#include <wx/xml/xml.h>

namespace XmlSerialize {

void DeserializeModelScreenLocationAttributes(BaseObject* object, wxXmlNode* node, bool importing) {
    glm::vec3 loc;
    loc.x = std::stof(node->GetAttribute(XmlNodeKeys::WorldPosXAttribute, "0").ToStdString());
    loc.y = std::stof(node->GetAttribute(XmlNodeKeys::WorldPosYAttribute, "0").ToStdString());
    loc.z = std::stof(node->GetAttribute(XmlNodeKeys::WorldPosZAttribute, "0").ToStdString());
    object->GetBaseObjectScreenLocation().SetWorldPosition(loc);
    glm::vec3 scale(1.0f, 1.0f, 1.0f);
    scale.x = std::stof(node->GetAttribute(XmlNodeKeys::ScaleXAttribute, "1.0").ToStdString());
    scale.y = std::stof(node->GetAttribute(XmlNodeKeys::ScaleYAttribute, "1.0").ToStdString());
    scale.z = std::stof(node->GetAttribute(XmlNodeKeys::ScaleZAttribute, "1.0").ToStdString());
    object->GetBaseObjectScreenLocation().SetScaleMatrix(scale);
    glm::vec3 rotate(0.0f, 0.0f, 0.0f);
    rotate.x = std::stof(node->GetAttribute(XmlNodeKeys::RotateXAttribute, "0").ToStdString());
    rotate.y = std::stof(node->GetAttribute(XmlNodeKeys::RotateYAttribute, "0").ToStdString());
    rotate.z = std::stof(node->GetAttribute(XmlNodeKeys::RotateZAttribute, "0").ToStdString());
    object->GetBaseObjectScreenLocation().SetRotation(rotate);
    if( !importing ) {
         bool locked = std::stoi(node->GetAttribute(XmlNodeKeys::LockedAttribute, "0").ToStdString()) > 0;
        object->GetBaseObjectScreenLocation().Lock(locked);
    }
}

void DeserializeTwoPointScreenLocationAttributes(BaseObject* object, wxXmlNode* node) {
    float x2 = std::stof(node->GetAttribute(XmlNodeKeys::X2Attribute, "0").ToStdString());
    float y2 = std::stof(node->GetAttribute(XmlNodeKeys::Y2Attribute, "0").ToStdString());
    float z2 = std::stof(node->GetAttribute(XmlNodeKeys::Z2Attribute, "0").ToStdString());
    TwoPointScreenLocation& screenLoc = dynamic_cast<TwoPointScreenLocation&>(object->GetBaseObjectScreenLocation());
    screenLoc.SetX2(x2);
    screenLoc.SetY2(y2);
    screenLoc.SetZ2(z2);
}

void DeserializeThreePointScreenLocationAttributes(BaseObject* object, wxXmlNode* node) {
    DeserializeTwoPointScreenLocationAttributes(object, node);
    int angle = std::stoi(node->GetAttribute(XmlNodeKeys::AngleAttribute, "0").ToStdString());
    float height = std::stof(node->GetAttribute("Height", "1.0").ToStdString());
    float shear = std::stof(node->GetAttribute("Shear", "0.0").ToStdString());
    ThreePointScreenLocation& screenLoc = dynamic_cast<ThreePointScreenLocation&>(object->GetBaseObjectScreenLocation());
    screenLoc.SetAngle(angle);
    screenLoc.SetMHeight(height);
    screenLoc.SetYShear(shear);
}

void DeserializePolyPointScreenLocationAttributes(BaseObject* object, wxXmlNode* node) {
    int num_points = std::stoi(node->GetAttribute(XmlNodeKeys::NumPointsAttribute, "2").ToStdString());
    PolyPointScreenLocation& screenLoc = dynamic_cast<PolyPointScreenLocation&>(object->GetBaseObjectScreenLocation());
    screenLoc.SetNumPoints(num_points);
    screenLoc.SetDataFromString(node->GetAttribute(XmlNodeKeys::PointDataAttribute, "0.0, 0.0, 0.0, 0.0, 0.0, 0.0").ToStdString());
    screenLoc.SetCurveDataFromString(node->GetAttribute(XmlNodeKeys::cPointDataAttribute, "").ToStdString());
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
                    try {
                        locations[layer][row][col] = std::stoi(value);
                    } catch (...) {
                        // not a number, treat as 0
                    }
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
            nodes.emplace_back(std::make_tuple(std::stoi(nodeData[0]), std::stoi(nodeData[1]), std::stoi(nodeData[2]), 0));
        } else if (nodeData.size() == 4) {
            nodes.emplace_back(std::make_tuple(std::stoi(nodeData[0]), std::stoi(nodeData[1]), std::stoi(nodeData[2]), std::stoi(nodeData[3])));
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

std::vector<std::vector<std::vector<int>>> ParseCustomModelDataFromXml(const wxXmlNode* node)
{
    std::string compressed = node->GetAttribute(XmlNodeKeys::CustomModelCmpAttribute).ToStdString();
    if (compressed != "") {
        return XmlSerialize::ParseCompressed(compressed);
    } else {
        std::string customModel = node->GetAttribute(XmlNodeKeys::CustomModelAttribute).ToStdString();
        return XmlSerialize::ParseCustomModel(customModel);
    }
}

void DeserializeFaceInfo(wxXmlNode* f, FaceStateData & faceInfo) {
    std::string name = f->GetAttribute(XmlNodeKeys::StateNameAttribute, "SingleNode").ToStdString();
    std::string type = f->GetAttribute(XmlNodeKeys::StateTypeAttribute, "SingleNode").ToStdString();
    if (name == xlEMPTY_STRING) {
        name = type;
    }
    if (!(type == "SingleNode" || type == "NodeRange" || type == "Matrix")) {
        if (type == "Coro") {
            type = "SingleNode";
        } else {
            type = "Matrix";
        }
    }
    wxXmlAttribute* att = f->GetAttributes();
    while (att != nullptr) {
        if (att->GetName() != XmlNodeKeys::StateNameAttribute) {
            if (att->GetName().Left(5) == "Mouth" || att->GetName().Left(4) == "Eyes") {
                if (type == XmlNodeKeys::MatrixType) {
                    faceInfo[name][att->GetName().ToStdString()] = FixFile("", att->GetValue());
                    if (att->GetValue() != faceInfo[name][att->GetName().ToStdString()])
                        att->SetValue(faceInfo[name][att->GetName().ToStdString()]);
                } else {
                    faceInfo[name][att->GetName().ToStdString()] = att->GetValue();
                }
            } else {
                faceInfo[name][att->GetName().ToStdString()] = att->GetValue();
            }
        }
        att = att->GetNext();
    }
}

void DeserializeStateInfo(wxXmlNode* f, FaceStateData & stateInfo) {
    std::string name = f->GetAttribute(XmlNodeKeys::StateNameAttribute, "SingleNode").ToStdString();
    std::string type = f->GetAttribute(XmlNodeKeys::StateTypeAttribute, "SingleNode").ToStdString();
    if (name == "") {
        name = type;
    }
    if (!(type == "SingleNode" || type == "NodeRange")) {
        if (type == "Coro") {
            type = "SingleNode";
        }
    }
    wxXmlAttribute* att = f->GetAttributes();
    while (att != nullptr) {
        if (att->GetName() != "Name") {
            if (att->GetValue() != "") { // we only save non default values to keep xml file small
                std::string key = att->GetName().ToStdString();
                std::string value = att->GetValue().ToStdString();
                std::string storedKey = key;
                if (key.find('s') == 0) { // Handle all keys starting with 's'
                    size_t sPos = key.find('s');
                    size_t dashPos = key.find('-');
                    size_t endPos = (dashPos != std::string::npos) ? dashPos : key.length();
                    if (sPos == 0 && sPos + 1 < endPos) {
                        std::string numStr = key.substr(sPos + 1, endPos - sPos - 1);
                        int num = std::stoi(numStr);
                        std::string paddedNum = wxString::Format("%03d", num).ToStdString();
                        storedKey = "s" + paddedNum + (dashPos != std::string::npos ? key.substr(dashPos) : "");
                    }
                }
                stateInfo[name][storedKey] = value;
            }
        }
        att = att->GetNext();
    }
}

std::optional<CustomModelImportData> LoadCustomModelFromFile(const std::string& filename) {
    wxXmlDocument doc(filename);
    if (!doc.IsOk()) {
        return std::nullopt;
    }
    
    wxXmlNode* root = doc.GetRoot();
    if (!root || root->GetName() != "custommodel") {
        return std::nullopt;
    }
    
    return LoadCustomModelFromXml(root);
}

std::optional<CustomModelImportData> LoadCustomModelFromXml(const wxXmlNode* node) {
    if (!node || node->GetName() != "custommodel") {
        return std::nullopt;
    }
    
    CustomModelImportData data;
    
    // Load basic attributes
    data.name = node->GetAttribute("name", "").ToStdString();
    data.width = wxAtoi(node->GetAttribute("parm1", "1"));
    data.height = wxAtoi(node->GetAttribute("parm2", "1"));
    data.depth = wxAtoi(node->GetAttribute("Depth", "1"));
    
    // Check if it's a 1-depth model (or empty for pre-3D models)
    wxString depthAttr = node->GetAttribute("Depth", "");
    if (!depthAttr.IsEmpty() && data.depth != 1) {
        return std::nullopt; // Not a 2D model
    }
    
    // Load model data
    data.modelData = ParseCustomModelDataFromXml(node);
    
    // Load submodels
    for (wxXmlNode* child = node->GetChildren(); child != nullptr; child = child->GetNext()) {
        if (child->GetName() == "subModel") {
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
        else if (child->GetName() == "faceInfo") {
            CustomModelImportData::FaceData face;
            face.name = child->GetAttribute("Name", "").ToStdString();
            face.type = child->GetAttribute("Type", "").ToStdString();
            
            if (face.type == "NodeRange") {
                // Store all attributes
                for (wxXmlAttribute* attr = child->GetAttributes(); attr != nullptr; attr = attr->GetNext()) {
                    face.attributes[attr->GetName().ToStdString()] = attr->GetValue().ToStdString();
                }
                data.faces.push_back(std::move(face));
            }
        }
        else if (child->GetName() == "stateInfo") {
            CustomModelImportData::StateData state;
            state.name = child->GetAttribute("Name", "").ToStdString();
            state.type = child->GetAttribute("Type", "").ToStdString();
            
            if (state.type == "NodeRange") {
                // Store all attributes
                for (wxXmlAttribute* attr = child->GetAttributes(); attr != nullptr; attr = attr->GetNext()) {
                    state.attributes[attr->GetName().ToStdString()] = attr->GetValue().ToStdString();
                }
                data.states.push_back(std::move(state));
            }
        }
    }
    
    return data;
}

std::optional<SubModelImportData> ParseSubModelNode(const wxXmlNode* node) {
    if (!node || node->GetName() != "subModel") {
        return std::nullopt;
    }
    
    SubModelImportData sm;
    sm.name = node->GetAttribute(XmlNodeKeys::NameAttribute, "").ToStdString();
    sm.isRanges = node->GetAttribute(XmlNodeKeys::SMTypeAttribute, "ranges") == "ranges";
    sm.vertical = node->GetAttribute(XmlNodeKeys::LayoutAttribute, "vertical") == "vertical";
    sm.subBuffer = node->GetAttribute(XmlNodeKeys::SubBufferAttribute, "").ToStdString();
    sm.bufferStyle = node->GetAttribute(XmlNodeKeys::BufferStyleAttribute, "Default").ToStdString();
    
    // Load strands/lines
    int lineNum = 0;
    while (node->HasAttribute(wxString::Format("line%d", lineNum))) {
        sm.strands.push_back(node->GetAttribute(wxString::Format("line%d", lineNum), "").ToStdString());
        lineNum++;
    }
    
    return sm;
}

std::vector<SubModelImportData> LoadSubModelsFromXml(const wxXmlNode* modelNode) {
    std::vector<SubModelImportData> subModels;
    
    if (!modelNode) {
        return subModels;
    }
    
    // Iterate through child nodes looking for subModel elements
    for (wxXmlNode* child = modelNode->GetChildren(); child != nullptr; child = child->GetNext()) {
        if (auto sm = ParseSubModelNode(child)) {
            subModels.push_back(std::move(*sm));
        }
    }
    
    return subModels;
}

std::string GetModelAttributesAsJSON(const wxXmlNode* modelNode) {
    if (!modelNode) {
        return "{}";
    }
    
    std::string json = "{";
    bool first = true;
    
    // Process model attributes
    for (wxXmlAttribute* attrp = modelNode->GetAttributes(); attrp; attrp = attrp->GetNext()) {
        wxString value = attrp->GetValue();
        if (!value.empty()) {
            if (!first) {
                json += ",";
            }
            json += "\"" + attrp->GetName().ToStdString() + "\":\"" + JSONSafe(value.ToStdString()) + "\"";
            first = false;
        }
    }
    
    // Process ControllerConnection child node
    wxXmlNode* cc = nullptr;
    for (wxXmlNode* child = modelNode->GetChildren(); child != nullptr; child = child->GetNext()) {
        if (child->GetName() == "ControllerConnection") {
            cc = child;
            break;
        }
    }
    
    json += ",\"ControllerConnection\":{";
    if (cc) {
        bool first2 = true;
        for (wxXmlAttribute* attrp = cc->GetAttributes(); attrp; attrp = attrp->GetNext()) {
            wxString value = attrp->GetValue();
            if (!value.empty()) {
                if (!first2) {
                    json += ",";
                }
                json += "\"" + attrp->GetName().ToStdString() + "\":\"" + JSONSafe(value.ToStdString()) + "\"";
                first2 = false;
            }
        }
    }
    json += "}}";
    
    return json;
}

} // end namespace XmlSerialize
