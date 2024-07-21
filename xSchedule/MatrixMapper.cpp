/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "MatrixMapper.h"
#include "RGBEffects.h"
#include "../xLights/UtilFunctions.h"

#include "../xLights/outputs/OutputManager.h"
#include <log4cpp/Category.hh>
#include <wx/string.h>
#include <wx/wx.h>
#include <wx/xml/xml.h>

MatrixMapper::MatrixMapper(OutputManager* outputManager, int strings, int strandsPerString, int stringLength, MMORIENTATION orientation, MMSTARTLOCATION startLocation, const std::string& startChannel, const std::string& name, const std::string& fromModel) {
    _outputManager = outputManager;
    _lastSavedChangeCount = 0;
    _changeCount = 1;
    _name = name;
    _strings = strings;
    _strandsPerString = strandsPerString;
    _stringLength = stringLength;
    _orientation = orientation;
    _startLocation = startLocation;
    _startChannel = startChannel;
    _fromModel = fromModel;

    if (_fromModel != "") {
        LoadModel();
    }

    if (_strandsPerString < 1)
        _strandsPerString = 1;
}

MatrixMapper::MatrixMapper(OutputManager* outputManager, int strings, int strandsPerString, int stringLength, const std::string& orientation, const std::string& startLocation, const std::string& startChannel, const std::string& name, const std::string& fromModel) {
    _outputManager = outputManager;
    _lastSavedChangeCount = 0;
    _changeCount = 1;
    _name = name;
    _strings = strings;
    _strandsPerString = strandsPerString;
    _stringLength = stringLength;
    _orientation = MatrixMapper::EncodeOrientation(orientation);
    _startLocation = MatrixMapper::EncodeStartLocation(startLocation);
    _startChannel = startChannel;
    _fromModel = fromModel;

    if (_fromModel != "") {
        LoadModel();
    }

    if (_strandsPerString < 1)
        _strandsPerString = 1;
}

MatrixMapper::MatrixMapper(OutputManager* outputManager, wxXmlNode* n) {
    _outputManager = outputManager;
    _lastSavedChangeCount = 0;
    _changeCount = 0;
    _name = n->GetAttribute("Name", "");
    _strings = wxAtoi(n->GetAttribute("Strings", "1"));
    _strandsPerString = wxAtoi(n->GetAttribute("StrandsPerString", "1"));
    _stringLength = wxAtoi(n->GetAttribute("StringLength", "50"));
    _orientation = (MMORIENTATION)wxAtoi(n->GetAttribute("Orientation", "0"));
    _startLocation = (MMSTARTLOCATION)wxAtoi(n->GetAttribute("StartLocation", "0"));
    _startChannel = n->GetAttribute("StartChannel", "1").ToStdString();
    _fromModel = n->GetAttribute("FromModel", "");

    if (_fromModel != "") {
        _valid = LoadModel();
    }

    if (_strandsPerString < 1)
        _strandsPerString = 1;
}

wxXmlNode* MatrixMapper::Save() {
    wxXmlNode* res = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "Matrix");

    res->AddAttribute("Name", _name);
    res->AddAttribute("Strings", wxString::Format(wxT("%i"), _strings));
    res->AddAttribute("StrandsPerString", wxString::Format(wxT("%i"), _strandsPerString));
    res->AddAttribute("StringLength", wxString::Format(wxT("%i"), _stringLength));
    res->AddAttribute("Orientation", wxString::Format(wxT("%i"), (int)_orientation));
    res->AddAttribute("StartLocation", wxString::Format(wxT("%i"), (int)_startLocation));
    res->AddAttribute("StartChannel", _startChannel);
    res->AddAttribute("FromModel", _fromModel);

    return res;
}

std::vector<std::vector<std::vector<int>>> MatrixMapper::ParseCustomModel(const std::string& customModel) {
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

std::vector<std::vector<std::vector<int>>> MatrixMapper::ParseCustomModelDataFromXml(const wxXmlNode* node) {
    std::string compressed = node->GetAttribute("CustomModelCompressed").ToStdString();
    if (compressed != "")
        return ParseCompressed(compressed);
    std::string customModel = node->GetAttribute("CustomModel").ToStdString();
    return ParseCustomModel(customModel);
}

std::vector<std::vector<std::vector<int>>> MatrixMapper::ParseCompressed(const std::string& compressed) {
    // node, row, col, [layer];

    // layers - rows - cols
    std::vector<std::vector<std::vector<int>>> locations;

    // parse all the strings
    std::vector<std::tuple<int, int, int, int>> nodes;
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
    for (const auto& n : nodes) {
        locations[std::get<3>(n)][std::get<1>(n)][std::get<2>(n)] = std::get<0>(n);
    }

    return locations;
}

bool MatrixMapper::IsSupportedDisplayAs(const std::string& displayAs) {
    return (displayAs == "Custom" || displayAs == "Horiz Matrix" || displayAs == "Vert Matrix" ||
            displayAs == "Tree 360" || displayAs == "Tree Flat" || displayAs == "Tree Ribbon" ||
            displayAs == "Sphere");
}

bool MatrixMapper::LoadModel() {
    RGBEffects effects;

    wxXmlNode* node = effects.GetModel(_fromModel);

    if (node == nullptr) {
        return false;
    }

    _displayAs = node->GetAttribute("DisplayAs", "");

    if (!IsSupportedDisplayAs(_displayAs)) {
        return false;
    }

    if (_displayAs == "Custom") {
        if (node->GetAttribute("Depth", "1") != "1") {
            return false;
        }
        _customModelData = ParseCustomModelDataFromXml(node);
        if (_customModelData.size() == 0) {
            return false;
        }
        _startChannel = node->GetAttribute("StartChannel", "1");
    } else {
        _strings = wxAtoi(node->GetAttribute("parm1", "0"));
        _strandsPerString = wxAtoi(node->GetAttribute("parm3", "1"));
        _stringLength = wxAtoi(node->GetAttribute("parm2", "0"));
        if (_displayAs == "Horiz Matrix") {
            _orientation = MMORIENTATION::HORIZONTAL;
        } else {
            _orientation = MMORIENTATION::VERTICAL;
        }
        std::string startSide = node->GetAttribute("StartSide", "B");
        std::string dir = node->GetAttribute("Dir", "L");
        if (startSide == "B" && dir == "L") {
            _startLocation = MMSTARTLOCATION::BOTTOM_LEFT;
        } else if (startSide == "B" && dir == "R") {
            _startLocation = MMSTARTLOCATION::BOTTOM_RIGHT;
        } else if (startSide == "T" && dir == "L") {
            _startLocation = MMSTARTLOCATION::TOP_LEFT;
        } else {
            _startLocation = MMSTARTLOCATION::TOP_RIGHT;
        }
        _startChannel = node->GetAttribute("StartChannel", "1");
    }

    return true;
}

// this maps to first channel if the position contains no node or the coordinates are out of range
size_t MatrixMapper::MapCustom(int x, int y) const {
    size_t res = GetStartChannelAsNumber();

    if (_customModelData[0].size() < y)
        return res;
    if (_customModelData[0][y].size() < x)
        return res;

    if (_customModelData[0][y][x] < 0)
        return res;

    // this only works for 3 channel pixels
    return res + _customModelData[0][y][x] * 3;
}

size_t MatrixMapper::Map(int x, int y) const {
    wxASSERT(x >= 0 && x < GetWidth());
    wxASSERT(y >= 0 && y < GetHeight());

    if (_fromModel != "" && _displayAs == "Custom") {
        return MapCustom(x, y);
    }

    long startChannel = GetStartChannelAsNumber();
    size_t loc = startChannel;

    if (_orientation == MMORIENTATION::VERTICAL) {
        switch (_startLocation) {
        case MMSTARTLOCATION::BOTTOM_LEFT:
            loc += x * GetHeight() * 3;
            if (_strandsPerString % 2 == 0) {
                if (x % 2 == 0) {
                    loc += y * 3;
                } else {
                    loc += (GetHeight() - y - 1) * 3;
                }
            } else {
                if (_strandsPerString == 1 || ((x - ((x / _strandsPerString) * _strandsPerString)) % 2 == 0)) {
                    loc += y * 3;
                } else {
                    loc += (GetHeight() - y - 1) * 3;
                }
            }
            break;
        case MMSTARTLOCATION::BOTTOM_RIGHT:
            loc += (GetWidth() - x - 1) * GetHeight() * 3;
            if (_strandsPerString % 2 == 0) {
                if (x % 2 == 0) {
                    loc += (GetHeight() - y - 1) * 3;
                } else {
                    loc += y * 3;
                }
            } else {
                if (_strandsPerString == 1 || (((GetWidth() - x - 1) - (((GetWidth() - x - 1) / _strandsPerString) * _strandsPerString)) % 2 == 0)) {
                    loc += y * 3;
                } else {
                    loc += (GetHeight() - y - 1) * 3;
                }
            }
            break;
        case MMSTARTLOCATION::TOP_LEFT:
            loc += x * GetHeight() * 3;
            if (_strandsPerString % 2 == 0) {
                if (x % 2 == 1) {
                    loc += y * 3;
                } else {
                    loc += (GetHeight() - y - 1) * 3;
                }
            } else {
                if (_strandsPerString != 1 && (x - ((x / _strandsPerString) * _strandsPerString)) % 2 == 1) {
                    loc += y * 3;
                } else {
                    loc += (GetHeight() - y - 1) * 3;
                }
            }
            break;
        case MMSTARTLOCATION::TOP_RIGHT:
            loc += (GetWidth() - x - 1) * GetHeight() * 3;
            if (_strandsPerString % 2 == 0) {
                if (x % 2 == 1) {
                    loc += (GetHeight() - y - 1) * 3;
                } else {
                    loc += y * 3;
                }
            } else {
                if (_strandsPerString != 1 && (((GetWidth() - x - 1) - (((GetWidth() - x - 1) / _strandsPerString) * _strandsPerString)) % 2 == 1)) {
                    loc += y * 3;
                } else {
                    loc += (GetHeight() - y - 1) * 3;
                }
            }
            break;
        }
    } else {
        switch (_startLocation) {
        case MMSTARTLOCATION::BOTTOM_LEFT:
            loc += y * GetWidth() * 3;
            if (_strandsPerString % 2 == 0) {
                if (y % 2 == 0) {
                    loc += x * 3;
                } else {
                    loc += (GetWidth() - x - 1) * 3;
                }
            } else {
                if (_strandsPerString == 1 || ((y - ((y / _strandsPerString) * _strandsPerString)) % 2 == 0)) {
                    loc += x * 3;
                } else {
                    loc += (GetWidth() - x - 1) * 3;
                }
            }
            break;
        case MMSTARTLOCATION::BOTTOM_RIGHT:
            loc += y * GetWidth() * 3;
            if (_strandsPerString % 2 == 0) {
                if (y % 2 == 1) {
                    loc += x * 3;
                } else {
                    loc += (GetWidth() - x - 1) * 3;
                }
            } else {
                if (_strandsPerString != 1 && ((y - ((y / _strandsPerString) * _strandsPerString)) % 2 == 1)) {
                    loc += x * 3;
                } else {
                    loc += (GetWidth() - x - 1) * 3;
                }
            }
            break;
        case MMSTARTLOCATION::TOP_LEFT:
            loc += (GetHeight() - y - 1) * GetWidth() * 3;
            if (_strandsPerString % 2 == 0) {
                if (y % 2 == 0) {
                    loc += x * 3;
                } else {
                    loc += (GetWidth() - x - 1) * 3;
                }
            } else {
                if (_strandsPerString == 1 || (((GetHeight() - y - 1) - (((GetHeight() - y - 1) / _strandsPerString) * _strandsPerString)) % 2 == 0)) {
                    loc += x * 3;
                } else {
                    loc += (GetWidth() - x - 1) * 3;
                }
            }
            break;
        case MMSTARTLOCATION::TOP_RIGHT:
            loc += (GetHeight() - y - 1) * GetWidth() * 3;
            if (_strandsPerString % 2 == 0) {
                if (y % 2 == 1) {
                    loc += x * 3;
                } else {
                    loc += (GetWidth() - x - 1) * 3;
                }
            } else {
                if (((GetHeight() - y - 1) - (((GetHeight() - y - 1) / _strandsPerString) * _strandsPerString)) % 2 == 1) {
                    loc += x * 3;
                } else {
                    loc += (GetWidth() - x - 1) * 3;
                }
            }
            break;
        }
    }

    // make sure the value is within the range expected ... until i know my code is right
    if (loc < startChannel || loc >= startChannel + GetChannels()) {
        // location out of range ... this can happen if the user tampers with the matrix while it is in use
        // force it to a valid value
        loc = startChannel;
        wxASSERT(false);
    }

    return loc;
}

size_t MatrixMapper::GetChannels() const {
    return _stringLength * _strings * 3;
}

int MatrixMapper::GetWidth() const {
    if (_orientation == MMORIENTATION::VERTICAL) {
        return _strings * _strandsPerString;
    } else {
        if (_strandsPerString == 0)
            return _stringLength;
        return _stringLength / _strandsPerString;
    }
}

int MatrixMapper::GetHeight() const {
    if (_orientation == MMORIENTATION::VERTICAL) {
        if (_strandsPerString == 0)
            return _stringLength;
        return _stringLength / _strandsPerString;
    } else {
        return _strings * _strandsPerString;
    }
}

long MatrixMapper::GetStartChannelAsNumber() const {
    return _outputManager->DecodeStartChannel(_startChannel);
}

MMORIENTATION MatrixMapper::EncodeOrientation(const std::string orientation) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxString o = wxString(orientation).Lower();

    if (o == "horizontal") {
        return MMORIENTATION::HORIZONTAL;
    } else if (o == "vertical") {
        return MMORIENTATION::VERTICAL;
    }

    logger_base.error("Unknown orientation %s", (const char*)o.c_str());
    return MMORIENTATION::VERTICAL;
}

std::string MatrixMapper::DecodeOrientation(MMORIENTATION orientation) {
    if (orientation == MMORIENTATION::HORIZONTAL) {
        return "Horizontal";
    } else {
        return "Vertical";
    }
}

MMSTARTLOCATION MatrixMapper::EncodeStartLocation(const std::string startLocation) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxString sl = wxString(startLocation).Lower();

    if (sl == "bottom left") {
        return MMSTARTLOCATION::BOTTOM_LEFT;
    } else if (sl == "bottom right") {
        return MMSTARTLOCATION::BOTTOM_RIGHT;
    } else if (sl == "top left") {
        return MMSTARTLOCATION::TOP_LEFT;
    } else if (sl == "top right") {
        return MMSTARTLOCATION::TOP_RIGHT;
    }

    logger_base.error("Unknown start location %s", (const char*)sl.c_str());

    return MMSTARTLOCATION::TOP_LEFT;
}

std::string MatrixMapper::DecodeStartLocation(MMSTARTLOCATION startLocation) {
    switch (startLocation) {
    case MMSTARTLOCATION::BOTTOM_LEFT:
        return "Bottom Left";
    case MMSTARTLOCATION::BOTTOM_RIGHT:
        return "Bottom Right";
    case MMSTARTLOCATION::TOP_LEFT:
        return "Top Left";
    default:
        break;
    }

    return "Top Right";
}

void MatrixMapper::Test(OutputManager* outputManager) {
    MatrixMapper h_bl_e(outputManager, 2, 4, 200, "Horizontal", "Bottom Left", "1", "Test", "");
    wxASSERT(h_bl_e.Map(0, 0) == 1);
    wxASSERT(h_bl_e.Map(49, 0) == 148);
    wxASSERT(h_bl_e.Map(0, 3) == 598);

    MatrixMapper h_br_e(outputManager, 2, 4, 200, "Horizontal", "Bottom Right", "1", "Test", "");
    wxASSERT(h_br_e.Map(0, 0) == 148);
    wxASSERT(h_br_e.Map(49, 0) == 1);
    wxASSERT(h_br_e.Map(0, 3) == 451);

    MatrixMapper h_bl_o(outputManager, 2, 3, 150, "Horizontal", "Bottom Left", "1", "Test", "");
    wxASSERT(h_bl_o.Map(0, 0) == 1);
    wxASSERT(h_bl_o.Map(49, 0) == 148);
    wxASSERT(h_bl_o.Map(0, 3) == 451);

    MatrixMapper h_br_o(outputManager, 2, 3, 150, "Horizontal", "Bottom Right", "1", "Test", "");
    wxASSERT(h_br_o.Map(0, 0) == 148);
    wxASSERT(h_br_o.Map(49, 0) == 1);
    wxASSERT(h_br_o.Map(0, 3) == 598);

    MatrixMapper h_tl_o(outputManager, 40, 1, 150, "Horizontal", "Top Left", "1", "Test", "");
    wxASSERT(h_tl_o.Map(0, 0) == 17551);
    wxASSERT(h_tl_o.Map(149, 0) == 17998);
    wxASSERT(h_tl_o.Map(0, 3) == 16201);

    MatrixMapper v_tl_e(outputManager, 4, 4, 600, "Vertical", "Top Left", "1", "Test", "");
    wxASSERT(v_tl_e.Map(0, 0) == 448);
    wxASSERT(v_tl_e.Map(15, 149) == 7198);
    wxASSERT(v_tl_e.Map(0, 3) == 439);

    MatrixMapper v_tr_o(outputManager, 4, 3, 450, "Vertical", "Top Right", "1", "Test", "");
    wxASSERT(v_tr_o.Map(0, 0) == 5398);
    wxASSERT(v_tr_o.Map(11, 149) == 1);
    wxASSERT(v_tr_o.Map(0, 3) == 5389);

    MatrixMapper v_tl_o(outputManager, 4, 3, 450, "Vertical", "Top Left", "1", "Test", "");
    wxASSERT(v_tl_o.Map(0, 0) == 448);
    wxASSERT(v_tl_o.Map(11, 149) == 4951);
    wxASSERT(v_tl_o.Map(0, 3) == 439);

    MatrixMapper v_tl_o2(outputManager, 1, 30, 210, "Vertical", "Top Left", "1", "Test", "");
    wxASSERT(v_tl_o2.Map(0, 0) == 19);
    wxASSERT(v_tl_o2.Map(29, 6) == 209 * 3 + 1);
    wxASSERT(v_tl_o2.Map(0, 3) == 10);
    wxASSERT(v_tl_o2.Map(10, 3) == 220);

    MatrixMapper v_tl_o3(outputManager, 30, 1, 7, "Vertical", "Top Left", "1", "Test", "");
    wxASSERT(v_tl_o3.Map(0, 0) == 19);
    wxASSERT(v_tl_o3.Map(29, 6) == 610);
    wxASSERT(v_tl_o3.Map(0, 3) == 10);
    wxASSERT(v_tl_o3.Map(10, 3) == 220);

    MatrixMapper v_tr_o2(outputManager, 1, 75, 525, "Vertical", "Top Right", "1", "Test", "");
    wxASSERT(v_tr_o2.Map(0, 0) == 1573);
    wxASSERT(v_tr_o2.Map(74, 6) == 1);
    wxASSERT(v_tr_o2.Map(0, 3) == 1564);
    wxASSERT(v_tr_o2.Map(65, 4) == 202);

    MatrixMapper v_tr_o3(outputManager, 75, 1, 7, "Vertical", "Top Right", "1", "Test", "");
    wxASSERT(v_tr_o3.Map(0, 0) == 1573);
    wxASSERT(v_tr_o3.Map(74, 6) == 1);
    wxASSERT(v_tr_o3.Map(0, 3) == 1564);
    wxASSERT(v_tr_o3.Map(65, 4) == 196);

    MatrixMapper v_bl_e(outputManager, 4, 4, 600, "Vertical", "Bottom Left", "1", "Test", "");
    wxASSERT(v_bl_e.Map(0, 0) == 1);
    wxASSERT(v_bl_e.Map(15, 149) == 6751);
    wxASSERT(v_bl_e.Map(0, 3) == 10);

    MatrixMapper v_bl_o(outputManager, 4, 3, 450, "Vertical", "Bottom Left", "1", "Test", "");
    wxASSERT(v_bl_o.Map(0, 0) == 1);
    wxASSERT(v_bl_o.Map(11, 149) == 5398);
    wxASSERT(v_bl_o.Map(0, 3) == 10);

    MatrixMapper v_br_e(outputManager, 4, 4, 600, "Vertical", "Bottom Right", "1", "Test", "");
    wxASSERT(v_br_e.Map(0, 0) == 7198);
    wxASSERT(v_br_e.Map(15, 149) == 448);
    wxASSERT(v_br_e.Map(0, 3) == 7189);

    MatrixMapper v_tr_e(outputManager, 4, 4, 600, "Vertical", "Top Right", "1", "Test", "");
    wxASSERT(v_tr_e.Map(0, 0) == 6751);
    wxASSERT(v_tr_e.Map(15, 149) == 1);
    wxASSERT(v_tr_e.Map(0, 3) == 6760);

    MatrixMapper v_br_o(outputManager, 4, 3, 450, "Vertical", "Bottom Right", "1", "Test", "");
    wxASSERT(v_br_o.Map(0, 0) == 4951);
    wxASSERT(v_br_o.Map(11, 149) == 448);
    wxASSERT(v_br_o.Map(0, 3) == 4960);

    MatrixMapper v_br_e2(outputManager, 10, 1, 10, "Vertical", "Bottom Right", "1", "Test", "");
    wxASSERT(v_br_e2.Map(0, 0) == 271);
    wxASSERT(v_br_e2.Map(9, 9) == 28);
    wxASSERT(v_br_e2.Map(0, 3) == 280);

    MatrixMapper v_bl_e2(outputManager, 10, 1, 10, "Vertical", "Bottom Left", "1", "Test", "");
    wxASSERT(v_bl_e2.Map(0, 0) == 1);
    wxASSERT(v_bl_e2.Map(9, 9) == 298);
    wxASSERT(v_bl_e2.Map(0, 3) == 10);

    MatrixMapper v_tr_e2(outputManager, 10, 1, 10, "Vertical", "Top Right", "1", "Test", "");
    wxASSERT(v_tr_e2.Map(0, 0) == 298);
    wxASSERT(v_tr_e2.Map(9, 9) == 1);
    wxASSERT(v_tr_e2.Map(0, 3) == 289);

    MatrixMapper v_tl_e2(outputManager, 10, 1, 10, "Vertical", "Top Left", "1", "Test", "");
    wxASSERT(v_tl_e2.Map(0, 0) == 28);
    wxASSERT(v_tl_e2.Map(9, 9) == 271);
    wxASSERT(v_tl_e2.Map(0, 3) == 19);

    MatrixMapper v_br_o2(outputManager, 9, 1, 10, "Vertical", "Bottom Right", "1", "Test", "");
    wxASSERT(v_br_o2.Map(0, 0) == 241);
    wxASSERT(v_br_o2.Map(8, 9) == 28);
    wxASSERT(v_br_o2.Map(0, 3) == 250);

    MatrixMapper v_bl_o2(outputManager, 9, 1, 10, "Vertical", "Bottom Left", "1", "Test", "");
    wxASSERT(v_bl_o2.Map(0, 0) == 1);
    wxASSERT(v_bl_o2.Map(8, 9) == 268);
    wxASSERT(v_bl_o2.Map(0, 3) == 10);

    MatrixMapper v_tr_o4(outputManager, 9, 1, 10, "Vertical", "Top Right", "1", "Test", "");
    wxASSERT(v_tr_o4.Map(0, 0) == 268);
    wxASSERT(v_tr_o4.Map(8, 9) == 1);
    wxASSERT(v_tr_o4.Map(0, 3) == 259);

    MatrixMapper v_tl_o4(outputManager, 9, 1, 10, "Vertical", "Top Left", "1", "Test", "");
    wxASSERT(v_tl_o4.Map(0, 0) == 28);
    wxASSERT(v_tl_o4.Map(8, 9) == 241);
    wxASSERT(v_tl_o4.Map(0, 3) == 19);
}

std::string MatrixMapper::GetConfigDescription() const {
    if (_fromModel != "") {
        return "From Model: " + _fromModel;
    }

    return wxString::Format("Orientation: %s, Start: %s, Strings: %d, Strands/String: %d, Nodes Per String: %d, StartChannel:%s:%ld.",
                            DecodeOrientation(_orientation),
                            DecodeStartLocation(_startLocation),
                            _strings,
                            _strandsPerString,
                            _stringLength,
                            _startChannel,
                            GetStartChannelAsNumber())
        .ToStdString();
}

MatrixMapper::MatrixMapper(OutputManager* outputManager) {
    _outputManager = outputManager;
    _lastSavedChangeCount = 0;
    _changeCount = 1;
    _name = "";
    _strings = 1;
    _strandsPerString = 1;
    _stringLength = 50;
    _orientation = MMORIENTATION::HORIZONTAL;
    _startLocation = MMSTARTLOCATION::BOTTOM_LEFT;
    _startChannel = 1;
    _fromModel = "";
}
