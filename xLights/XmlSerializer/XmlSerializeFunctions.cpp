#pragma once
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

#include "XmlSerializeFunctions.h"
#include "XmlNodeKeys.h"

#include <wx/xml/xml.h>

namespace XmlSerialize {

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
} // end namespace XmlSerialize
