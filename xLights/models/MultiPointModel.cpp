/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "MultiPointModel.h"
#include "../xLightsMain.h"
#include "../xLightsVersion.h"

MultiPointModel::MultiPointModel(const ModelManager &manager) : ModelWithScreenLocation(manager)
{
    parm1 = parm2 = parm3 = 0;
}

MultiPointModel::~MultiPointModel()
{
}

bool MultiPointModel::IsNodeFirst(int n) const
{
    return (GetIsLtoR() && n == 0) || (!GetIsLtoR() && n == Nodes.size() - 1);
}

int MultiPointModel::MapToNodeIndex(int strand, int node) const {
    return strand * screenLocation.num_points + node;
}

void MultiPointModel::InitModel()
{
    parm1 = 1;
    
    InitLine();

    // calculate min/max for the model
    float minX = 100000.0f;
    float minY = 100000.0f;
    float minZ = 100000.0f;
    float maxX = 0.0f;
    float maxY = 0.0f;
    float maxZ = 0.0f;

    std::vector<PolyPointScreenLocation::xlPolyPoint> pPos(screenLocation.num_points);

    for (int i = 0; i < screenLocation.num_points; ++i) {
        if (screenLocation.mPos[i].x < minX) minX = screenLocation.mPos[i].x;
        if (screenLocation.mPos[i].y < minY) minY = screenLocation.mPos[i].y;
        if (screenLocation.mPos[i].z < minZ) minZ = screenLocation.mPos[i].z;
        if (screenLocation.mPos[i].x > maxX) maxX = screenLocation.mPos[i].x;
        if (screenLocation.mPos[i].y > maxY) maxY = screenLocation.mPos[i].y;
        if (screenLocation.mPos[i].z > maxZ) maxZ = screenLocation.mPos[i].z;
    }
    float deltax = maxX - minX;
    float deltay = maxY - minY;
    float deltaz = maxZ - minZ;

    // normalize all the point data
    for (int i = 0; i < screenLocation.num_points; ++i) {
        if (deltax == 0.0f) {
            pPos[i].x = 0.0f;
        } else {
            pPos[i].x = (screenLocation.mPos[i].x - minX) / deltax;
        }
        if (deltay == 0.0f) {
            pPos[i].y = 0.0f;
        } else {
            pPos[i].y = (screenLocation.mPos[i].y - minY) / deltay;
        }
        if (deltaz == 0.0f) {
            pPos[i].z = 0.0f;
        } else {
            pPos[i].z = (screenLocation.mPos[i].z - minZ) / deltaz;
        }
    }

    screenLocation.SetRenderSize(1.0, 1.0);

    double model_height = deltay;
    if (model_height < GetModelScreenLocation().GetRenderHt()) {
        model_height = GetModelScreenLocation().GetRenderHt();
    }

    // place the nodes/coords along each line segment
    int i = 0;
    if ( Nodes.size() > 0 && (BufferWi > 1 || Nodes.front()->Coords.size() > 1))
    {
        for (auto& n : Nodes)
        {
            for (auto& c : n->Coords)
            {
                c.screenX = pPos[i].x;
                c.screenY = pPos[i].y;
                c.screenZ = pPos[i].z;
            }
            i++;
        }
    }
    else if (Nodes.size() > 0)
    {
        // 1 node 1 light
        Nodes.front()->Coords.front().screenY = 0.0;
        Nodes.front()->Coords.front().screenX = 0.5;
    }
}

// initialize buffer coordinates
// parm1=Number of Strings/Arches/Canes
// parm2=Pixels Per String/Arch/Cane
void MultiPointModel::InitLine() {
    int numLights = parm1 * screenLocation.num_points;
    Nodes.clear();
    SetNodeCount(parm1,screenLocation.num_points,rgbOrder);
    SetBufferSize(1,SingleNode?parm1:numLights);
    int LastStringNum=-1;
    int chan = 0;
    int ChanIncr = GetNodeChannelCount(StringType);
    size_t NodeCount=GetNodeCount();
    if (!IsLtoR) {
        ChanIncr = -ChanIncr;
    }

    int idx = 0;
    for(size_t n=0; n<NodeCount; n++) {
        if (Nodes[n]->StringNum != LastStringNum) {
            LastStringNum=Nodes[n]->StringNum;
            chan=stringStartChan[LastStringNum];
            if (!IsLtoR) {
                chan += NodesPerString(LastStringNum) * GetNodeChannelCount(StringType);
                chan += ChanIncr;
            }
        }
        Nodes[n]->ActChan=chan;
        chan+=ChanIncr;
        Nodes[n]->Coords.resize(SingleNode?screenLocation.num_points:parm3);
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++) {
            Nodes[n]->Coords[c].bufX=idx;
            Nodes[n]->Coords[c].bufY=0;
        }
        idx++;
    }
}
void MultiPointModel::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager)
{
    wxPGProperty* p;
    if (SingleNode) {
        p = grid->Append(new wxUIntProperty("# Lights", "MultiPointNodes", screenLocation.num_points));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");
        p->Enable(false); // number of nodes is determined by number of points
    }
    else {
        p = grid->Append(new wxUIntProperty("# Nodes", "MultiPointNodes", screenLocation.num_points));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");
        p->Enable(false); // number of nodes is determined by number of points
    }

    p = grid->Append(new wxUIntProperty("Strings", "MultiPointStrings", _strings));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 48);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("This is typically the number of connections from the prop to your controller.");

    if (_strings == 1) {
        // cant set start node
    } else {
        wxString nm = StartNodeAttrName(0);

        p = grid->Append(new wxBoolProperty("Indiv Start Nodes", "ModelIndividualStartNodes", _hasIndivNodes));
        p->SetAttribute("UseCheckbox", true);

        wxPGProperty* psn = grid->AppendIn(p, new wxUIntProperty(nm, nm, _hasIndivNodes ? _indivStartNodes[0] : 1));
        psn->SetAttribute("Min", 1);
        psn->SetAttribute("Max", (int)GetNodeCount());
        psn->SetEditor("SpinCtrl");

        if (_hasIndivNodes) {
            int c = _strings;
            for (int x = 0; x < c; x++) {
                nm = StartNodeAttrName(x);
                int v = _indivStartNodes[x];
                if (v < 1) v = 1;
                if (v > NodesPerString()) v = NodesPerString();
                if (x == 0) {
                    psn->SetValue(v);
                } else {
                    grid->AppendIn(p, new wxUIntProperty(nm, nm, v));
                }
            }
        } else {
            psn->Enable(false);
        }
    }

    p = grid->Append(new wxFloatProperty("Height", "ModelHeight", _height));
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");
}

int MultiPointModel::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event)
{
    if ("MultiPointNodes" == event.GetPropertyName()) {
        screenLocation.num_points = (int)event.GetPropertyValue().GetLong();
        wxPGProperty* sp = grid->GetPropertyByLabel("# Nodes");
        if (sp == nullptr) {
            sp = grid->GetPropertyByLabel("# Lights");
        }
        sp->SetValueFromInt((int)event.GetPropertyValue().GetLong());
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "MultiPointModel::OnPropertyGridChange::MultiPointNodes");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "MultiPointModel::OnPropertyGridChange::MultiPointNodes");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "MultiPointModel::OnPropertyGridChange::MultiPointNodes");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "MultiPointModel::OnPropertyGridChange::MultiPointNodes");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "MultiPointModel::OnPropertyGridChange::MultiPointNodes");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "MultiPointModel::OnPropertyGridChange::MultiPointNodes");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "MultiPointModel::OnPropertyGridChange::MultiPointNodes");
        return 0;
    }
    else if ("MultiPointStrings" == event.GetPropertyName()) {
        int old_string_count = _strings;
        int new_string_count = event.GetValue().GetInteger();
        _strings = new_string_count;
        if (old_string_count != new_string_count) {
            if (_hasIndivNodes) {
                for (int x = 0; x < new_string_count; x++) {
                    _indivStartNodes[x] = ComputeStringStartNode(x);
                }
            }
        }
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "MultiPointModel::OnPropertyGridChange::MultiPointStrings");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "MultiPointModel::OnPropertyGridChange::MultiPointStrings");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "MultiPointModel::OnPropertyGridChange::MultiPointStrings");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "MultiPointModel::OnPropertyGridChange::MultiPointStrings");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "MultiPointModel::OnPropertyGridChange::MultiPointStrings");
        return 0;
    } else if (!GetModelScreenLocation().IsLocked() && !IsFromBase() && "ModelHeight" == event.GetPropertyName()) {
        _height = event.GetValue().GetDouble();
        if (std::abs(_height) < 0.01f) {
            if (_height < 0.0f) {
                _height = -0.01f;
            }
            else {
                _height = 0.01f;
            }
        }
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "MultiPointModel::OnPropertyGridChange::ModelHeight");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "MultiPointModel::OnPropertyGridChange::ModelHeight");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "MultiPointModel::OnPropertyGridChange::ModelHeight");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "MultiPointModel::OnPropertyGridChange::ModelHeight");
        return 0;
    } else if ((GetModelScreenLocation().IsLocked() || IsFromBase()) && "ModelHeight" == event.GetPropertyName()) {
        event.Veto();
        return 0;
    }
    return Model::OnPropertyGridChange(grid, event);
}

int MultiPointModel::ComputeStringStartNode(int x) const
{
    if (x == 0) return 1;

    int strings = GetNumPhysicalStrings();
    int nodes = GetNodeCount();
    float nodesPerString = (float)nodes / (float)strings;

    return (int)(x * nodesPerString + 1);
}

int MultiPointModel::NodesPerString() const
{
    return Model::NodesPerString();
}

 int MultiPointModel::NodesPerString(int string) const
{
     int num_nodes = 0;
     if (_strings == 1) {
        return NodesPerString();
     } else {
        if (SingleNode) {
            return 1;
        } else {
            int v1 = 0;
            int v2 = 0;
            if (_hasIndivNodes) {
                v1 = _indivStartNodes[string];
                if (string < _strings - 1) { // not last string
                    v2 = _indivStartNodes[string+1];
                }
            } else {
                v1 = ComputeStringStartNode(string);
                if (string < _strings - 1) { // not last string
                    v2 = ComputeStringStartNode(string + 1);
                }
            }
            if (string < _strings - 1) { // not last string
                num_nodes = v2 - v1;
            } else {
                num_nodes = GetNodeCount() - v1 + 1;
            }
        }
        int ts = GetSmartTs();
        if (ts <= 1) {
            return num_nodes;
        } else {
            return num_nodes * ts;
        }
     }
}

int MultiPointModel::OnPropertyGridSelection(wxPropertyGridInterface* grid, wxPropertyGridEvent& event)
{
    if (event.GetPropertyName().StartsWith("ModelIndividualSegments.")) {
        wxString str = event.GetPropertyName();
        str = str.SubString(str.Find(".") + 1, str.length());
        str = str.SubString(3, str.length());
        int segment = wxAtoi(str) - 1;
        return segment;
    }
    return -1;
}

int MultiPointModel::GetNumPhysicalStrings() const
{
    int ts = GetSmartTs();
    if (ts <= 1) {
        return _strings;
    } else {
        int strings = _strings / ts;
        if (strings == 0)
            strings = 1;
        return strings;
    }
}

void MultiPointModel::DeleteHandle(int handle_) {
    // handle is offset by 1 due to the center handle at 0
    int handle = handle_ - 1;
    GetModelScreenLocation().DeleteHandle(handle);
}

// Call this before exporting the points
void MultiPointModel::NormalizePointData()
{

}

//TODO:  Do we need this code to Normalize...was used for an export
/*std::string MultiPointModel::GetPointDataAsString() const
{
    // First normalize all the data
    float minX = 100000.0f;
    float minY = 100000.0f;
    float minZ = 100000.0f;
    float maxX = 0.0f;
    float maxY = 0.0f;
    float maxZ = 0.0f;

    for (int i = 0; i < screenLocation.num_points; ++i) {
        if (screenLocation.mPos[i].x < minX)
            minX = screenLocation.mPos[i].x;
        if (screenLocation.mPos[i].y < minY)
            minY = screenLocation.mPos[i].y;
        if (screenLocation.mPos[i].z < minZ)
            minZ = screenLocation.mPos[i].z;
        if (screenLocation.mPos[i].x > maxX)
            maxX = screenLocation.mPos[i].x;
        if (screenLocation.mPos[i].y > maxY)
            maxY = screenLocation.mPos[i].y;
        if (screenLocation.mPos[i].z > maxZ)
            maxZ = screenLocation.mPos[i].z;
    }
    float deltax = maxX - minX;
    float deltay = maxY - minY;
    float deltaz = maxZ - minZ;

    // normalize all the point data
    std::vector<xlMultiPoint> pPos;
    pPos.resize(screenLocation.num_points);
    for (int i = 0; i < screenLocation.num_points; ++i) {
        if (deltax == 0.0f) {
            pPos[i].x = 0.0f;
        } else {
            pPos[i].x = (screenLocation.mPos[i].x - minX) / deltax;
        }
        if (deltay == 0.0f) {
            pPos[i].y = 0.0f;
        } else {
            pPos[i].y = (screenLocation.mPos[i].y - minY) / deltay;
        }
        if (deltaz == 0.0f) {
            pPos[i].z = 0.0f;
        } else {
            pPos[i].z = (screenLocation.mPos[i].z - minZ) / deltaz;
        }
    }

    std::string point_data = "";
    for (int i = 0; i < screenLocation.num_points; ++i) {
        point_data += wxString::Format("%f,", screenLocation.mPos[i].x);
        point_data += wxString::Format("%f,", screenLocation.mPos[i].y);
        point_data += wxString::Format("%f", screenLocation.mPos[i].z);
        if (i != screenLocation.num_points - 1) {
            point_data += ",";
        }
    }
    return point_data;
}*/
