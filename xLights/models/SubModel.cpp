/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "SubModel.h"

#include <wx/xml/xml.h>
#include <wx/tokenzr.h>
#include <wx/propgrid/propgrid.h>

#include <log4cpp/Category.hh>

static const std::string DEFAULT("Default");
static const std::string KEEP_XY("Keep XY");
static const std::string STACKED_STRANDS("Stacked Strands");

const std::vector<std::string> SubModel::BUFFER_STYLES{ DEFAULT, KEEP_XY, STACKED_STRANDS };

// Check for duplicate nodes in a submodel
void SubModel::CheckDuplicates(const std::vector<int>& nodeIndexes)
{
    _duplicateNodes = "";

    auto it = begin(nodeIndexes);

    while (it != end(nodeIndexes)) {
        auto it2 = it;
        ++it2;
        while (it2 != end(nodeIndexes)) {
            if (*it == *it2) {
                if (_duplicateNodes != "") _duplicateNodes += ",";
                _duplicateNodes += wxString::Format("%d", *it + 1);
            }
            ++it2;
        }
        ++it;
    }
}

SubModel::SubModel(Model* p, wxXmlNode* n) :
    Model(p->GetModelManager()),
    parent(p),
    _layout(n->GetAttribute("layout")),
    _type(n->GetAttribute("type", "ranges")),
    _bufferStyle(n->GetAttribute("bufferstyle", DEFAULT))
{

    // copy change count from owning model ... otherwise we lose track of changes when the model is recreated
    changeCount = p->changeCount;

    _nodesAllValid = true;
    ModelXml = n;
    StrobeRate = 0;
    Nodes.clear();
    DisplayAs = "SubModel";

    name = n->GetAttribute("name").Trim(true).Trim(false).ToStdString();
    parm1 = 1;
    parm2 = 1;
    parm3 = 1;

    //static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    //logger_base.debug("Submodel init %s:%s", (const char*)p->GetFullName().c_str(), (const char*)name.c_str());

    bool vert = _layout == "vertical";
    bool isRanges = _type == "ranges";

    auto getRange = [](wxString const& a) {
        if (a.Contains("-")) {
            int idx = a.Index('-');
            return std::make_pair(wxAtoi(a.Left(idx)), wxAtoi(a.Right(a.size() - idx - 1)));
        }
        return std::make_pair(wxAtoi(a), wxAtoi(a));
    };

    unsigned int startChannel = UINT32_MAX;
    if (isRanges) {
        if (_bufferStyle == KEEP_XY) {
            int line = 0;
            std::vector<int> nodeIndexes;
            std::set<int> nodeIdx;
            while (n->HasAttribute(wxString::Format("line%d", line))) {
                wxString nodes = n->GetAttribute(wxString::Format("line%d", line));
                _properyGridDisplay = nodes + "," + _properyGridDisplay;
                wxStringTokenizer wtkz(nodes, ",");
                while (wtkz.HasMoreTokens()) {
                    wxString valstr = wtkz.GetNextToken();
                    auto [start, end] = getRange(valstr);
                    if (start != 0) {
                        if (start > end) {//order is always lowest to highest for grid
                            std::swap(start, end);
                        }
                        start--;
                        end--;
                        for (int i = start; i <= end; i++) {
                            nodeIdx.insert(i);
                            nodeIndexes.push_back(i);
                        }
                    }
                }
                line++;
            }

            CheckDuplicates(nodeIndexes);

            float minx = 10000;
            float maxx = -1;
            float miny = 10000;
            float maxy = -1;
            for (auto const& idx : nodeIdx) {
                // ignore nodes indexes if they are out of range
                if (idx < p->Nodes.size()) {
                    NodeBaseClass* node = p->Nodes[idx]->clone();
                    for (const auto& c : node->Coords) {
                        if (c.bufX < minx)
                            minx = c.bufX;
                        if (c.bufY < miny)
                            miny = c.bufY;
                        if (c.bufX > maxx)
                            maxx = c.bufX;
                        if (c.bufY > maxy)
                            maxy = c.bufY;
                    }
                    delete node;
                }
            }
            for (auto const& idx : nodeIdx) {
                // ignore nodes indexes if they are out of range
                if (idx < p->Nodes.size()) {
                    NodeBaseClass* node = p->Nodes[idx]->clone();
                    startChannel = (std::min)(startChannel, node->ActChan);
                    Nodes.push_back(NodeBaseClassPtr(node));
                    for (auto& c : node->Coords) {
                        c.bufX -= minx;
                        c.bufY -= miny;
                    }
                }
            }
            if (maxx < minx || maxy < miny || Nodes.size() == 0) {
                SetBufferSize(1, 1);
            }
            else {
                int x_size = int(std::ceil(maxx - minx)) + 1;
                int y_size = int(std::ceil(maxy - miny)) + 1;
                SetBufferSize(y_size, x_size);
            }
        } else { //default and stacked buffer styles
            int row = 0;
            int col = 0;
            int line = 0;
            int maxRow = 0;
            int maxCol = 0;
            std::vector<int> nodeIndexes;
            while (n->HasAttribute(wxString::Format("line%d", line))) {
                wxString nodes = n->GetAttribute(wxString::Format("line%d", line));
                //logger_base.debug("    Line %d: %s", line, (const char*)nodes.c_str());
                _properyGridDisplay = nodes + "," + _properyGridDisplay;
                wxStringTokenizer wtkz(nodes, ",");
                while (wtkz.HasMoreTokens()) {
                    wxString valstr = wtkz.GetNextToken();
                    auto [start, end] = getRange(valstr);
                    if (start == 0)
                    {
                        if (vert) {
                            row++;
                        }
                        else {
                            col++;
                        }
                    }
                    else
                    {
                        start--;
                        end--;
                        bool done = false;
                        wxInt32 nn = start;
                        while (!done) {
                            if ((uint32_t)nn < p->GetNodeCount()) {
                                nodeIndexes.push_back(nn);
                                NodeBaseClass* node = p->Nodes[nn]->clone();
                                startChannel = (std::min)(startChannel, node->ActChan);
                                Nodes.push_back(NodeBaseClassPtr(node));
                                for (auto& c : node->Coords) {
                                    c.bufX = col;
                                    c.bufY = row;
                                }
                                if (vert) {
                                    row++;
                                }
                                else {
                                    col++;
                                }
                            }
                            else {
                                _nodesAllValid = false;
                            }
                            if (start > end) {
                                nn--;
                                done = nn < end;
                            }
                            else {
                                nn++;
                                done = nn > end;
                            }
                        }
                    }
                }
                if (vert) {
                    row--;
                }
                else {
                    col--;
                }
                if (maxRow < row) {
                    maxRow = row;
                }
                if (maxCol < col) {
                    maxCol = col;
                }
                if (_bufferStyle == STACKED_STRANDS) {
                    row = 0;
                    col = 0;
                } else {
                    if (vert) {
                        row = 0;
                        col++;
                    }
                    else {
                        col = 0;
                        row++;
                    }
                }
                line++;
            }
            CheckDuplicates(nodeIndexes);
            SetBufferSize(maxRow + 1, maxCol + 1);
        }
    }
    else {
        // subbuffers cant generate duplicate nodes
        wxString range = n->GetAttribute("subBuffer");
        _properyGridDisplay = range;
        float x1 = 0;
        float x2 = 100;
        float y1 = 0;
        float y2 = 100;
        if (range != "") {
            wxArrayString v = wxSplit(range, 'x');
            x1 = v.size() > 0 ? wxAtof(v[0]) : 0.0;
            y1 = v.size() > 1 ? wxAtof(v[1]) : 0.0;
            x2 = v.size() > 2 ? wxAtof(v[2]) : 100.0;
            y2 = v.size() > 3 ? wxAtof(v[3]) : 100.0;
        }

        if (x1 > x2) std::swap(x1, x2);
        if (y1 > y2) std::swap(y1, y2);

        x1 *= (float)p->GetDefaultBufferWi();
        x2 *= (float)p->GetDefaultBufferWi();
        y1 *= (float)p->GetDefaultBufferHt();
        y2 *= (float)p->GetDefaultBufferHt();
        x1 /= 100.0;
        x2 /= 100.0;
        y1 /= 100.0;
        y2 /= 100.0;
        x1 = std::ceil(x1);
        y1 = std::ceil(y1);

        float minx = 10000;
        float maxx = -1;
        float miny = 10000;
        float maxy = -1;

        int nn = p->GetNodeCount();
        for (int m = 0; m < nn; m++) {
            if (p->IsNodeInBufferRange(m, x1, y1, x2, y2)) {
                NodeBaseClass* node = p->Nodes[m]->clone();
                for (const auto& c : node->Coords) {

                    if (c.bufX < minx) minx = c.bufX;
                    if (c.bufY < miny) miny = c.bufY;
                    if (c.bufX > maxx) maxx = c.bufX;
                    if (c.bufY > maxy) maxy = c.bufY;
                }
                delete node;
            }
        }

        for (int m = 0; m < nn; m++) {
            if (p->IsNodeInBufferRange(m, x1, y1, x2, y2)) {
                NodeBaseClass* node = p->Nodes[m]->clone();
                startChannel = (std::min)(startChannel, node->ActChan);
                Nodes.push_back(NodeBaseClassPtr(node));
                for (auto& c : node->Coords) {
                    c.bufX -= minx;
                    c.bufY -= miny;
                }
            }
        }

        if (maxx < minx || maxy < miny || Nodes.size() == 0) {
            // invalid buffer, set it to just a 1x1 as 0x0 can cause some render issues
            SetBufferSize(1, 1);
        }
        else {
            x2 = int(std::ceil(maxx - minx)) + 1;
            y2 = int(std::ceil(maxy - miny)) + 1;
            SetBufferSize(y2, x2);
        }
    }
    //ModelStartChannel is 1 based
    this->ModelStartChannel = wxString::Format("%u", (startChannel + 1));

    // inherit pixel properties from parent model
    _pixelStyle = p->_pixelStyle;
    transparency = p->transparency;
    blackTransparency = p->blackTransparency;
    pixelSize = p->pixelSize;
}

void SubModel::AddProperties(wxPropertyGridInterface* grid, OutputManager* outputManager)
{
    wxPGProperty* p = grid->Append(new wxStringProperty("SubModel Type", "SMT", _type));
    p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    p->ChangeFlag(wxPG_PROP_READONLY, true);

    p = grid->Append(new wxStringProperty("SubModel Layout", "SML", _layout));
    p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    p->ChangeFlag(wxPG_PROP_READONLY, true);

    p = grid->Append(new wxStringProperty("SubModel Buffer Style", "SMBS", _bufferStyle));
    p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    p->ChangeFlag(wxPG_PROP_READONLY, true);

    p = grid->Append(new wxStringProperty("SubModel", "SMN", _properyGridDisplay));
    p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    p->ChangeFlag(wxPG_PROP_READONLY, true);
}
