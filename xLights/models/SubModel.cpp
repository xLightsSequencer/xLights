/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SubModel.h"
#include "ModelManager.h"
#include "ModelGroup.h"

#include <wx/xml/xml.h>
#include <wx/tokenzr.h>
#include <wx/propgrid/propgrid.h>

#include <log4cpp/Category.hh>

static const std::string DEFAULT("Default");
static const std::string STACKED_STRANDS("Stacked Strands");

const std::vector<std::string> SubModel::BUFFER_STYLES{ DEFAULT, KEEP_XY, STACKED_STRANDS };

SubModel::SubModel(Model *p, const std::string _name, bool vertical, bool ranges, const std::string bufferStyle) :
    Model(p->GetModelManager()),
    parent(p),
    _vert(vertical),
    _isRanges(ranges),
    _layout(vertical ? "vertical" : "horizontal"),
    _type(ranges ? "ranges" : "subbuffer"),
    _bufferStyle(bufferStyle)
 {
    // copy change count from owning model ... otherwise we lose track of changes when the model is recreated
    changeCount = p->changeCount;

    _nodesAllValid = true;
    StrobeRate = 0;
    Nodes.clear();
    DisplayAs = "SubModel";

    name = _name;
    parm1 = 1;
    parm2 = 1;
    parm3 = 1;
    
    StringType = p->StringType;

     // inherit pixel properties from parent model
     _pixelStyle = p->_pixelStyle;
     transparency = p->transparency;
     blackTransparency = p->blackTransparency;
     pixelSize = p->pixelSize;
}

SubModel::SubModel(Model *newParent, const SubModel* source) :
    Model(newParent->GetModelManager()),
    parent(newParent),
    _vert(source->_vert),
    _isRanges(source->_isRanges),
    _layout(source->_layout),
    _type(source->_type),
    _bufferStyle(source->_bufferStyle)
{
    // Copy change count from owning model
    changeCount = newParent->changeCount;

    // Copy SubModel properties
    _nodesAllValid = source->_nodesAllValid;
    StrobeRate = source->StrobeRate;
    Nodes.clear();
    DisplayAs = "SubModel";

    name = source->name;
    parm1 = source->parm1;
    parm2 = source->parm2;
    parm3 = source->parm3;
    
    StringType = newParent->StringType;

    // Inherit pixel properties from new parent model
    _pixelStyle = newParent->_pixelStyle;
    transparency = newParent->transparency;
    blackTransparency = newParent->blackTransparency;
    pixelSize = newParent->pixelSize;

    // Copy SubModel-specific data
    _propertyGridDisplay = source->_propertyGridDisplay;
    _sameLineDuplicates = source->_sameLineDuplicates;
    _crossLineDuplicates = source->_crossLineDuplicates;
    _nodeIndexes = source->_nodeIndexes;
    _nodeIdx = source->_nodeIdx;
    _startChannel = source->_startChannel;
    
    // Copy buffer-specific data
    _row = source->_row;
    _col = source->_col;
    _maxRow = source->_maxRow;
    _maxCol = source->_maxCol;
    _nodeIndexMap = source->_nodeIndexMap;
    _ranges = source->_ranges;
    
    // Copy aliases from source SubModel
    aliases = source->GetAliases();
}

bool SubModel::IsXYBufferStyle() { return _bufferStyle == KEEP_XY; }

void SubModel::AddProperties(wxPropertyGridInterface* grid, OutputManager* outputManager)
{
    wxPGProperty* p = grid->Append(new wxStringProperty("SubModel Type", "SMT", _type));
    p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    p->ChangeFlag(wxPGFlags::ReadOnly, true);

    p = grid->Append(new wxStringProperty("SubModel Layout", "SML", _layout));
    p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    p->ChangeFlag(wxPGFlags::ReadOnly, true);

    p = grid->Append(new wxStringProperty("SubModel Buffer Style", "SMBS", _bufferStyle));
    p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    p->ChangeFlag(wxPGFlags::ReadOnly, true);

    p = grid->Append(new wxStringProperty("SubModel", "SMN", _propertyGridDisplay));
    p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    p->ChangeFlag(wxPGFlags::ReadOnly, true);

    auto modelGroups = parent->GetModelManager().GetGroupsContainingModel(this);
    if (modelGroups.size() > 0) {
        std::string mgs;
        std::string mgscr;
        for (const auto& it : modelGroups) {
            if (mgs != "") {
                mgs += ", ";
                mgscr += "\n";
            }
            mgs += it;
            mgscr += it;
        }
        p = grid->Append(new wxStringProperty("In Model Groups", "MGS", mgs));
        p->SetHelpString(mgscr);
        p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
        p->ChangeFlag(wxPGFlags::ReadOnly, true);
    }
    auto smaliases = parent->GetSubModel(this->GetName())->GetAliases();
    if (smaliases.size() > 0) {
        std::string sma;
        std::string smacr;
        for (const auto& it : smaliases) {
            if (sma != "") {
                sma += ", ";
                smacr += "\n";
            }
            sma += it;
            smacr += it;
        }
        p = grid->Append(new wxStringProperty("SubModel Aliases", "SMA", sma));
        p->SetHelpString(smacr);
        p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
        p->ChangeFlag(wxPGFlags::ReadOnly, true);
    }
  
}

static const std::string VERT_PER_STRAND("Vertical Per Strand");
static const std::string HORIZ_PER_STRAND("Horizontal Per Strand");
static const std::string LEGACY_SINGLE_LINE("** Single Line Legacy");

// When in a ModelGroup, it may get these and we can optimize these as well
static const std::string HORIZ_PER_MODELSTRAND("Horizontal Per Model/Strand");
static const std::string VERT_PER_MODELSTRAND("Vertical Per Model/Strand");
static const std::string PERMODEL_VERT_PER_STRAND("Per Model Vertical Per Strand");
static const std::string PERMODEL_HORIZ_PER_STRAND("Per Model Horizontal Per Strand");

std::vector<std::string> SubModel::SUBMODEL_BUFFER_STYLES;
const std::vector<std::string>& SubModel::GetBufferStyles() const {
    struct Initializer {
        Initializer() {
            SUBMODEL_BUFFER_STYLES = Model::DEFAULT_BUFFER_STYLES;
            SUBMODEL_BUFFER_STYLES.push_back(VERT_PER_STRAND);
            SUBMODEL_BUFFER_STYLES.push_back(HORIZ_PER_STRAND);
        }
    };
    static Initializer ListInitializationGuard;
    return SUBMODEL_BUFFER_STYLES;
}
const std::string SubModel::AdjustBufferStyle(const std::string &style) const {
    if (style == LEGACY_SINGLE_LINE) {
        return style;
    }
    return Model::AdjustBufferStyle(style);
}

void SubModel::GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform, int &BufferWi, int &BufferHi, int stagger) const {
    std::string ntype = type;
    if (_isRanges && (type == VERT_PER_STRAND || type == HORIZ_PER_STRAND)) {
        if (_vert && (type == HORIZ_PER_STRAND)) {
            Model::GetBufferSize("Default", camera, "Rotate CW 90", BufferWi, BufferHi, stagger);
            AdjustForTransform(transform, BufferWi, BufferHi);
        } else if (!_vert && (type == VERT_PER_STRAND)) {
            Model::GetBufferSize("Default", camera, "Rotate CC 90", BufferWi, BufferHi, stagger);
            AdjustForTransform(transform, BufferWi, BufferHi);
        } else {
            Model::GetBufferSize(type, camera, transform, BufferWi, BufferHi, stagger);
        }
    } else if (_isRanges && (type == HORIZ_PER_MODELSTRAND || type == VERT_PER_MODELSTRAND || type == PERMODEL_HORIZ_PER_STRAND || type == PERMODEL_VERT_PER_STRAND)) {
        if (!_vert && (type == HORIZ_PER_MODELSTRAND || type == PERMODEL_HORIZ_PER_STRAND)) {
            Model::GetBufferSize("Default", camera, "Rotate CW 90", BufferWi, BufferHi, stagger);
            AdjustForTransform(transform, BufferWi, BufferHi);
        } else if (_vert && (type == VERT_PER_MODELSTRAND || type == PERMODEL_VERT_PER_STRAND)) {
            Model::GetBufferSize("Default", camera, "Rotate CC 90", BufferWi, BufferHi, stagger);
            AdjustForTransform(transform, BufferWi, BufferHi);
        } else {
            Model::GetBufferSize(type, camera, transform, BufferWi, BufferHi, stagger);
        }
    } else if (type == LEGACY_SINGLE_LINE) {
        std::vector<int> vsizes;
        std::vector<int> hsizes;
        for (auto &n : Nodes) {
            for (auto &c : n->Coords) {
                while (c.bufX >= hsizes.size()) {
                    hsizes.push_back(-1);
                }
                while (c.bufY >= vsizes.size()) {
                    vsizes.push_back(-1);
                }
                hsizes[c.bufX] = std::max(hsizes[c.bufX], c.bufY);
                vsizes[c.bufY] = std::max(vsizes[c.bufY], c.bufX);
            }
        }
        int total = 0;
        if (_vert) {
            for (auto s : vsizes) {
                total += s + 1;
            }
        } else {
            for (auto s : hsizes) {
                total += s + 1;
            }
        }
        BufferHi = 1;
        BufferWi = total;
        AdjustForTransform(transform, BufferWi, BufferHi);
    } else {
        Model::GetBufferSize(type, camera, transform, BufferWi, BufferHi, stagger);
    }
}
void SubModel::InitRenderBufferNodes(const std::string &type, const std::string &camera, const std::string &transform,
                                     std::vector<NodeBaseClassPtr> &newNodes, int &BufferWi, int &BufferHi, int stagger, bool deep) const {
    std::string ntype = type;
    int firstNode = newNodes.size();
    if (_isRanges && (type == VERT_PER_STRAND || type == HORIZ_PER_STRAND)) {
        // these can be optimized as the default for "isRanges" is per strand.  We can use "default" or a simple rotate
        // to avoid re-calculating everything
        if (_vert && (type == HORIZ_PER_STRAND)) {
            Model::InitRenderBufferNodes("Dafault", camera, "None", newNodes, BufferWi, BufferHi, stagger, deep);
            ApplyTransform("Rotate CW 90", newNodes, BufferWi, BufferHi, firstNode);
            ApplyTransform(transform, newNodes, BufferWi, BufferHi);
        } else if (!_vert && (type == VERT_PER_STRAND)) {
            Model::InitRenderBufferNodes("Dafault", camera, "None", newNodes, BufferWi, BufferHi, stagger, deep);
            ApplyTransform("Rotate CC 90", newNodes, BufferWi, BufferHi, firstNode);
            ApplyTransform(transform, newNodes, BufferWi, BufferHi);
        } else {
            Model::InitRenderBufferNodes("Default", camera, transform, newNodes, BufferWi, BufferHi, stagger, deep);
        }
    } else if (_isRanges && (type == HORIZ_PER_MODELSTRAND || type == VERT_PER_MODELSTRAND || type == PERMODEL_HORIZ_PER_STRAND || type == PERMODEL_VERT_PER_STRAND)) {

        if (!_vert && (type == HORIZ_PER_MODELSTRAND || type == PERMODEL_HORIZ_PER_STRAND)) {
            Model::InitRenderBufferNodes("Dafault", camera, "None", newNodes, BufferWi, BufferHi, stagger, deep);
            ApplyTransform("Rotate CW 90", newNodes, BufferWi, BufferHi, firstNode);
            ApplyTransform(transform, newNodes, BufferWi, BufferHi);
        } else if (_vert && (type == VERT_PER_MODELSTRAND || type == PERMODEL_VERT_PER_STRAND)) {
            Model::InitRenderBufferNodes("Dafault", camera, "None", newNodes, BufferWi, BufferHi, stagger, deep);
            ApplyTransform("Rotate CC 90", newNodes, BufferWi, BufferHi, firstNode);
            ApplyTransform(transform, newNodes, BufferWi, BufferHi);
        } else {
            Model::InitRenderBufferNodes("Default", camera, transform, newNodes, BufferWi, BufferHi, stagger, deep);
        }
    } else if (type == LEGACY_SINGLE_LINE) {
        Model::InitRenderBufferNodes("Default", camera, "None", newNodes, BufferWi, BufferHi, stagger, deep);
        std::vector<int> vsizes;
        std::vector<int> hsizes;
        for (auto &n : Nodes) {
            for (auto &c : n->Coords) {
                while (c.bufX >= hsizes.size()) {
                    hsizes.push_back(-1);
                }
                while (c.bufY >= vsizes.size()) {
                    vsizes.push_back(-1);
                }
                hsizes[c.bufX] = std::max(hsizes[c.bufX], c.bufY);
                vsizes[c.bufY] = std::max(vsizes[c.bufY], c.bufX);
            }
        }
        int total = 0;
        if (_vert) {
            for (auto s : vsizes) {
                total += s + 1;
            }
        } else {
            for (auto s : hsizes) {
                total += s + 1;
            }
        }
        BufferHi = 1;
        BufferWi = total;
        
        for (int n = firstNode; n < newNodes.size(); n++) {
            for (auto &c : newNodes[n]->Coords) {
                int curX = c.bufX;
                int curY = c.bufY;
                int pos = 0;
                if (_vert) {
                    while (curX > 0 && curY > vsizes[curX]) {
                        pos += vsizes[curX] + 1;
                        curY -= (vsizes[curX] + 1);
                        --curX;
                    }
                    pos += curY;
                } else {
                    while (curY > 0 && curX > hsizes[curY]) {
                        pos += hsizes[curY] + 1;
                        curX -= (hsizes[curY] + 1);
                        --curY;
                    }
                    pos += curX;
                }
                c.bufX = pos;
                c.bufY = 0;
            }
        }
        ApplyTransform(transform, newNodes, BufferWi, BufferHi);
    } else {
        Model::InitRenderBufferNodes(type, camera, transform, newNodes, BufferWi, BufferHi, stagger, deep);
    }
}

// Check for duplicate nodes in a submodel
void SubModel::CheckDuplicates()
{
    _sameLineDuplicates = "";
    _crossLineDuplicates = "";
    std::set<int> lineNodes;
    std::set<int> seenNodes;
    std::set<int> sameLineDups;     // Track same-line duplicates
    std::vector<int> sameDupNodes;  // For sorting
    std::vector<int> crossDupNodes; // For sorting

    for (int node : _nodeIndexes) {
        if (node == -1) {
            lineNodes.clear();
            continue;
        }

        if (!lineNodes.insert(node).second) {
            sameLineDups.insert(node);
            sameDupNodes.push_back(node);
        }

        if (!seenNodes.insert(node).second && sameLineDups.find(node) == sameLineDups.end()) {
            crossDupNodes.push_back(node);
        }
    }

    // Sort the vectors
    std::sort(sameDupNodes.begin(), sameDupNodes.end());
    std::sort(crossDupNodes.begin(), crossDupNodes.end());

    // Remove duplicates from the sorted vectors
    sameDupNodes.erase(std::unique(sameDupNodes.begin(), sameDupNodes.end()), sameDupNodes.end());
    crossDupNodes.erase(std::unique(crossDupNodes.begin(), crossDupNodes.end()), crossDupNodes.end());

    for (int node : sameDupNodes) {
        if (_sameLineDuplicates != "")
            _sameLineDuplicates += ", ";
        _sameLineDuplicates += wxString::Format("%d", node + 1);
    }

    for (int node : crossDupNodes) {
        if (_crossLineDuplicates != "")
            _crossLineDuplicates += ", ";
        _crossLineDuplicates += wxString::Format("%d", node + 1);
    }
}

void SubModel::CalcRangeXYBufferSize()
{
    float minx = 10000;
    float maxx = -1;
    float miny = 10000;
    float maxy = -1;
    for (auto const& idx : _nodeIdx) {
        // ignore nodes indexes if they are out of range
        if (idx < parent->Nodes.size()) {
            NodeBaseClass* node = parent->Nodes[idx]->clone();
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
    for (auto const& idx : _nodeIdx) {
        // ignore nodes indexes if they are out of range
        if (idx < parent->Nodes.size()) {
            NodeBaseClass* node = parent->Nodes[idx]->clone();
            _startChannel = (std::min)(_startChannel, node->ActChan);
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
}

auto getRange = [](wxString const& a) {
    if (a.Contains("-")) {
        int idx = a.Index('-');
        return std::make_pair(wxAtoi(a.Left(idx)), wxAtoi(a.Right(a.size() - idx - 1)));
    }
    int i = wxAtoi(a);
    return std::make_pair(i, i);
};

void SubModel::AddRangeXY( wxString const& nodes )
{
   _ranges.push_back(nodes);
   if (_propertyGridDisplay == "") {
        _propertyGridDisplay = nodes;
    } else {
        _propertyGridDisplay = _propertyGridDisplay + "," + nodes;
    }
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
                _nodeIdx.insert(i);
                _nodeIndexes.push_back(i);
            }
        }
    }
    _nodeIndexes.push_back(-1);
    
    //ModelStartChannel is 1 based
    this->ModelStartChannel = wxString::Format("%u", (_startChannel + 1));
}

void SubModel::AddDefaultBuffer( wxString const& nodes )
{
    _ranges.push_back(nodes);
    if (_propertyGridDisplay == "") {
        _propertyGridDisplay = nodes;
    } else {
        _propertyGridDisplay = _propertyGridDisplay + "," + nodes;
    }
    wxStringTokenizer wtkz(nodes, ",");
    while (wtkz.HasMoreTokens()) {
        wxString valstr = wtkz.GetNextToken();
        auto [start, end] = getRange(valstr);
        if (start == 0) {
            if (_vert) {
                _row++;
            } else {
                _col++;
            }
        } else {
            start--;
            end--;
            bool done = false;
            wxInt32 nn = start;
            while (!done) {
                if ((uint32_t)nn < parent->GetNodeCount()) {
                    _nodeIndexes.push_back(nn);
                    NodeBaseClass* node;
                    if (_nodeIndexMap.find(nn) == _nodeIndexMap.end()) {
                        node = parent->Nodes[nn]->clone();
                        _startChannel = (std::min)(_startChannel, node->ActChan);
                        _nodeIndexMap[nn] = Nodes.size();
                        Nodes.push_back(NodeBaseClassPtr(node));
                        if (node->Coords.size() > 1) {
                            node->Coords.resize(1);
                        }
                        for (auto& c : node->Coords) {
                            c.bufX = _col;
                            c.bufY = _row;
                        }
                    } else {
                        node = Nodes[_nodeIndexMap[nn]].get();
                        node->Coords.push_front(node->Coords[0]);
                        node->Coords[0].bufX = _col;
                        node->Coords[0].bufY = _row;
                    }
                    if (_vert) {
                        _row++;
                    } else {
                        _col++;
                    }
                } else {
                    _nodesAllValid = false;
                }
                if (start > end) {
                    nn--;
                    done = nn < end;
                } else {
                    nn++;
                    done = nn > end;
                }
            }
        }
    }
    if (_vert) {
        _row--;
    } else {
        _col--;
    }
    if (_maxRow < _row) {
        _maxRow = _row;
    }
    if (_maxCol < _col) {
        _maxCol = _col;
    }
    if (_bufferStyle == STACKED_STRANDS) {
        _row = 0;
        _col = 0;
    } else {
        if (_vert) {
            _row = 0;
            _col++;
        } else {
            _col = 0;
            _row++;
        }
    }
    _nodeIndexes.push_back(-1);
    SetBufferSize(_maxRow + 1, _maxCol + 1);

    //ModelStartChannel is 1 based
    this->ModelStartChannel = wxString::Format("%u", (_startChannel + 1));
}

void SubModel::AddSubbuffer(std::string const& range )
{
    // subbuffers cant generate duplicate nodes
    _propertyGridDisplay = range;
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
    
    x1 *= (float)parent->GetDefaultBufferWi();
    x2 *= (float)parent->GetDefaultBufferWi();
    y1 *= (float)parent->GetDefaultBufferHt();
    y2 *= (float)parent->GetDefaultBufferHt();
    x1 /= 100.0;
    x2 /= 100.0;
    y1 /= 100.0;
    y2 /= 100.0;
    x1 = std::round(x1);
    y1 = std::round(y1);
    
    float minx = 10000;
    float maxx = -1;
    float miny = 10000;
    float maxy = -1;
    
    int nn = parent->GetNodeCount();
    for (int m = 0; m < nn; m++) {
        if (parent->IsNodeInBufferRange(m, x1, y1, x2, y2)) {
            NodeBaseClass* node = parent->Nodes[m]->clone();
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
        if (parent->IsNodeInBufferRange(m, x1, y1, x2, y2)) {
            NodeBaseClass* node = parent->Nodes[m]->clone();
            _startChannel = (std::min)(_startChannel, node->ActChan);
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
    } else {
        x2 = int(std::ceil(maxx - minx)) + 1;
        y2 = int(std::ceil(maxy - miny)) + 1;
        SetBufferSize(y2, x2);
    }
    
    //ModelStartChannel is 1 based
    this->ModelStartChannel = wxString::Format("%u", (_startChannel + 1));
}
