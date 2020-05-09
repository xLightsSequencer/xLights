/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

//
// TODO
//
// Need to get RGB Dumb working ... 1 arm per string, multi arms per string and all one dumb rgb or single colour


#include "SpinnerModel.h"
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>
#include "ModelScreenLocation.h"

SpinnerModel::SpinnerModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased) : ModelWithScreenLocation(manager)
{
    SetFromXml(node, zeroBased);
}

SpinnerModel::SpinnerModel(const ModelManager &manager) : ModelWithScreenLocation(manager)
{
    //ctor
}

SpinnerModel::~SpinnerModel()
{
    //dtor
}

// Top left = Centre + CCW
// Top Right = Centre + CW
// BottomLeft = End + CCW
// Bottom Right = End + CW

static const char* TOP_BOT_LEFT_RIGHT_VALUES[] = { 
    "Center Counter Clockwise",
    "Center Clockwise",
    "End Counter Clockwise",
    "End Clockwise",
    "Center Alternate Counter Clockwise",
    "Center Alternate Clockwise"
};
static wxPGChoices TOP_BOT_LEFT_RIGHT(wxArrayString(6, TOP_BOT_LEFT_RIGHT_VALUES));

wxString DecodeStart(long start)
{
    return TOP_BOT_LEFT_RIGHT.GetLabel(start);
}

int EncodeStarts(const wxString& start)
{
    int i = 0;
    for (auto it = TOP_BOT_LEFT_RIGHT.GetLabels().begin(); it != TOP_BOT_LEFT_RIGHT.GetLabels().end(); ++it)
    {
        if (*it == start)
        {
            return i;
        }
        i++;
    }
    return -1;
}

void SpinnerModel::DecodeStartLocation(int sl)
{
    switch(sl)
    {
    case 0:
        alternate = false;
        isBotToTop = false;
        IsLtoR = true;
        break;
    case 1:
        alternate = false;
        isBotToTop = false;
        IsLtoR = false;
        break;
    case 2:
        alternate = false;
        isBotToTop = true;
        IsLtoR = true;
        break;
    case 3:
        alternate = false;
        isBotToTop = true;
        IsLtoR = false;
        break;
    case 4:
        isBotToTop = false;
        alternate = true;
        IsLtoR = true;
        zigzag = false;
        break;
    case 5:
        isBotToTop = false;
        alternate = true;
        IsLtoR = false;
        zigzag = false;
        break;
    default:
        wxASSERT(false);
        break;
    }
}

int SpinnerModel::EncodeStartLocation()
{
    if (alternate)
    {
        if (IsLtoR)
        {
            // Alternate CCW
            return 4;
        }
        else
        {
            // Alternate CW
            return 5;
        }
    }
    else
    {
        if (isBotToTop)
        {
            if (IsLtoR)
            {
                return 2; // Outsde CCW
            }
            else
            {
                return 3; // Outside CW
            }
        }
        else
        {
            if (IsLtoR)
            {
                return 0; // Center CCW
            }
            else
            {
                return 1; // Center CW
            }
        }
    }
    wxASSERT(false);
    return 0;
}

void SpinnerModel::AddTypeProperties(wxPropertyGridInterface *grid) {
    
    wxPGProperty *p = grid->Append(new wxUIntProperty("# Strings", "SpinnerStringCount", parm1));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 640);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Arms/String", "FoldCount", parm3));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 250);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Lights/Arm", "SpinnerArmNodeCount", parm2));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 200);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Hollow %", "Hollow", hollow));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 80);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Arc", "Arc", arc));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 360);
    p->SetEditor("SpinCtrl");

    grid->Append(new wxEnumProperty("Starting Location", "MatrixStart", TOP_BOT_LEFT_RIGHT, EncodeStartLocation()));

    p = grid->Append(new wxBoolProperty("Zig-Zag Start", "ZigZag", zigzag));
    p->SetEditor("CheckBox");
    p->Enable(alternate == false);
}

void SpinnerModel::UpdateTypeProperties(wxPropertyGridInterface* grid) {
    grid->GetPropertyByName("ZigZag")->Enable(alternate == false);
}

int SpinnerModel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("SpinnerStringCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm1");
        ModelXml->AddAttribute("parm1", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        //AdjustStringProperties(grid, parm1);
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "SpinnerModel::OnPropertyGridChange::SpinnerStringCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "SpinnerModel::OnPropertyGridChange::SpinnerStringCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "SpinnerModel::OnPropertyGridChange::SpinnerStringCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "SpinnerModel::OnPropertyGridChange::SpinnerStringCount");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "SpinnerModel::OnPropertyGridChange::SpinnerStringCount");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "SpinnerModel::OnPropertyGridChange::SpinnerStringCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "SpinnerModel::OnPropertyGridChange::SpinnerStringCount");
        return 0;
    } else if ("SpinnerArmNodeCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "SpinnerModel::OnPropertyGridChange::SpinnerArmNodeCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "SpinnerModel::OnPropertyGridChange::SpinnerArmNodeCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "SpinnerModel::OnPropertyGridChange::SpinnerArmNodeCount");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "SpinnerModel::OnPropertyGridChange::SpinnerArmNodeCount");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "SpinnerModel::OnPropertyGridChange::SpinnerArmNodeCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "SpinnerModel::OnPropertyGridChange::SpinnerArmNodeCount");
        return 0;
    } else if ("FoldCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm3");
        ModelXml->AddAttribute("parm3", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "SpinnerModel::OnPropertyGridChange::FoldCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "SpinnerModel::OnPropertyGridChange::FoldCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "SpinnerModel::OnPropertyGridChange::FoldCount");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "SpinnerModel::OnPropertyGridChange::FoldCount");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "SpinnerModel::OnPropertyGridChange::FoldCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "SpinnerModel::OnPropertyGridChange::FoldCount");
        return 0;
    } else if ("Hollow" == event.GetPropertyName()) {
         ModelXml->DeleteAttribute("Hollow");
         ModelXml->AddAttribute("Hollow", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
         AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "SpinnerModel::OnPropertyGridChange::Hollow");
         AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "SpinnerModel::OnPropertyGridChange::Hollow");
         AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "SpinnerModel::OnPropertyGridChange::Hollow");
         AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "SpinnerModel::OnPropertyGridChange::Hollow");
         return 0;
    } else if ("Arc" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("Arc");
        ModelXml->AddAttribute("Arc", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "SpinnerModel::OnPropertyGridChange::Arc");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "SpinnerModel::OnPropertyGridChange::Arc");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "SpinnerModel::OnPropertyGridChange::Arc");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "SpinnerModel::OnPropertyGridChange::Arc");
        return 0;
    } else if ("ZigZag" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("ZigZag");
        ModelXml->AddAttribute("ZigZag", event.GetPropertyValue().GetBool() ? "true" : "false");
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "SpinnerModel::OnPropertyGridChange::ZigZag");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "SpinnerModel::OnPropertyGridChange::ZigZag");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "SpinnerModel::OnPropertyGridChange::ZigZag");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "SpinnerModel::OnPropertyGridChange::ZigZag");
        return 0;
    } else if ("MatrixStart" == event.GetPropertyName()) {
        DecodeStartLocation(event.GetValue().GetLong());
        ModelXml->DeleteAttribute("Dir");
        ModelXml->AddAttribute("Dir", IsLtoR ? "L" : "R");
        ModelXml->DeleteAttribute("StartSide");
        ModelXml->AddAttribute("StartSide", isBotToTop ? "B" : "T");
        ModelXml->DeleteAttribute("Alternate");
        ModelXml->AddAttribute("Alternate", alternate ? "true" : "false");
        ModelXml->DeleteAttribute("ZigZag");
        ModelXml->AddAttribute("ZigZag",zigzag ? "true" : "false");
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "SpinnerModel::OnPropertyGridChange::MatrixStart");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "SpinnerModel::OnPropertyGridChange::MatrixStart");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "SpinnerModel::OnPropertyGridChange::MatrixStart");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "SpinnerModel::OnPropertyGridChange::MatrixStart");
        return 0;
    }
    
    return Model::OnPropertyGridChange(grid, event);
}

void SpinnerModel::GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform,
    int &BufferWi, int &BufferHi) const
{
    if (type == "Single Line") {
        BufferHi = 1;
        BufferWi = this->BufferWi * this->BufferHt;
        AdjustForTransform(transform, BufferWi, BufferHi);
    }
    else {
        Model::GetBufferSize(type, camera, transform, BufferWi, BufferHi);
    }
}

int SpinnerModel::GetNumStrands() const {
    if (SingleChannel) {
        return parm1;
    }
    return parm1*parm3;
}

void SpinnerModel::InitModel() {
    int stringcount = parm1;
    int nodesperarm = parm2;
    int armsperstring = parm3;
    int pixelsperstring = parm3*parm2;
    int armcount = parm3*parm1;
    hollow = wxAtoi(ModelXml->GetAttribute("Hollow", "20"));
    arc = wxAtoi(ModelXml->GetAttribute("Arc", "360"));
    zigzag = (ModelXml->GetAttribute("ZigZag", "false") == "true");
    alternate = (ModelXml->GetAttribute("Alternate", "false") == "true");
    SetNodeCount(stringcount, pixelsperstring, rgbOrder);
    screenLocation.SetRenderSize(2 * nodesperarm + 3 + (hollow * 2.0 * nodesperarm) / 100.0, 2 * nodesperarm + 3 + (hollow * 2.0 * nodesperarm) / 100.0);

    // create output mapping
    if (SingleNode) {
        SetBufferSize(1, armcount);
        for (size_t x = 0; x<Nodes.size(); x++) {
            Nodes[x]->ActChan = stringStartChan[x];
            Nodes[x]->StringNum = x;
            for (size_t c = 0; c<GetCoordCount(x); c++) {
                Nodes[x]->Coords[c].bufX = IsLtoR ? Nodes.size() - x - 1 : x;
                Nodes[x]->Coords[c].bufY = 0;
            }
        }
    }
    else {
        int chanPerNode = GetNodeChannelCount(StringType);
        SetBufferSize(nodesperarm, armcount);
        for (size_t x = 0; x < armcount; x++) {
            int stringnum = x / armsperstring;
            int segmentnum = x % armsperstring;
            for (size_t y = 0; y < nodesperarm; y++)
            {
                size_t idx = x * nodesperarm + y;
                Nodes[idx]->ActChan = stringStartChan[stringnum] + segmentnum * nodesperarm * chanPerNode + y * chanPerNode;
                Nodes[idx]->Coords[0].bufX = IsLtoR ? x : armcount - x - 1;
                if (alternate)
                {
                    if (y + 1 <= (nodesperarm+1) / 2)
                    {
                        Nodes[idx]->Coords[0].bufY = 2 * y;
                    }
                    else
                    {
                        Nodes[idx]->Coords[0].bufY = (nodesperarm - (y+1)) * 2 + 1;
                    }
                    Nodes[idx]->Coords[0].bufY = nodesperarm - Nodes[idx]->Coords[0].bufY - 1;
                }
                else
                {
                    if (!zigzag)
                    {
                        Nodes[idx]->Coords[0].bufY = isBotToTop ? y : nodesperarm - y - 1;
                    }
                    else
                    {
                        if (x % 2 == 0)
                        {
                            Nodes[idx]->Coords[0].bufY = isBotToTop ? y : nodesperarm - y - 1;
                        }
                        else
                        {
                            Nodes[idx]->Coords[0].bufY = isBotToTop ? nodesperarm - y - 1 : y;
                        }
                    }
                }
            }
        }
    }

    SetSpinnerCoord();
    screenLocation.RenderDp = 10.0f;  // give the bounding box a little depth
    DisplayAs = "Spinner";
}

void SpinnerModel::InitRenderBufferNodes(const std::string &type, const std::string &camera, const std::string &transform,
    std::vector<NodeBaseClassPtr> &newNodes, int &BufferWi, int &BufferHi) const {
    if (type == "Single Line") {
        BufferHi = 1;
        BufferWi = GetNodeCount();

        int stringcount = parm1;
        int nodesperarm = parm2;
        int armsperstring = parm3;
        int cur = 0;
        if (SingleNode)
        {
            for (int i = 0; i < stringcount; i++)
            {
                newNodes.push_back(NodeBaseClassPtr(Nodes[i]->clone()));
                for (size_t c = 0; c < newNodes[cur]->Coords.size(); c++) {
                    newNodes[cur]->Coords[c].bufX = cur;
                    newNodes[cur]->Coords[c].bufY = 0;
                }
                cur++;
            }
        }
        else
        {
            for (size_t y = 0; y < stringcount*armsperstring; y++) {
                for (int x = 0; x < nodesperarm; x++) {
                    int idx = y * nodesperarm + x;
                    newNodes.push_back(NodeBaseClassPtr(Nodes[idx]->clone()));
                    for (size_t c = 0; c < newNodes[cur]->Coords.size(); c++) {
                        newNodes[cur]->Coords[c].bufX = cur;
                        newNodes[cur]->Coords[c].bufY = 0;
                    }
                    cur++;
                }
            }
        }
        ApplyTransform(transform, newNodes, BufferWi, BufferHi);
    }
    else {
        Model::InitRenderBufferNodes(type, camera, transform, newNodes, BufferWi, BufferHi);
    }
}

void SpinnerModel::SetSpinnerCoord() {
    int stringcount = parm1;
    int nodesperarm = parm2;
    int armsperstring = parm3;
    int armcount = parm3*parm1;

    float angle = ((float)M_PI * 2.0f * 270.0f) / 360.0f;
    float angleincrement = (M_PI * 2.0f * (float)arc) / ((float)stringcount * (float)armsperstring * 360.0f);
    if (arc < 360 && armsperstring * stringcount > 1)
    {
        angleincrement = (M_PI * 2.0f * (float)arc) / (((float)stringcount * (float)armsperstring - 1) * 360.0f);
    }

    bool cw = !IsLtoR;
    bool fromcentre = !isBotToTop;

    // top = centre
    // bottom = end
    // left = ccw
    // right = cw

    for (size_t a = 0; a < armcount; a++)
    {
        if (SingleNode)
        {
            int a1 = a / armsperstring;
            int start = a * nodesperarm - a1 * armsperstring * nodesperarm;
            int end = start + nodesperarm;
            for (size_t c = start; c < end; c++) {
                int c2 = c - start;
                int c1 = 0;
                if (!fromcentre) {
                    c1 = nodesperarm - c2 - 1;
                }
                else {
                    c1 = c2;
                }

                if (zigzag && a % 2 > 0) {
                    if (!fromcentre) {
                        c1 = c2;
                    }
                    else {
                        c1 = nodesperarm - c2 - 1;
                    }
                }

                Nodes[a1]->Coords[c].screenX = (0.5f + (float)c1 + ((float)hollow * 2.0 * (float)nodesperarm) / 100.0) * cos(angle);
                Nodes[a1]->Coords[c].screenY = (0.5f + (float)c1 + ((float)hollow * 2.0 * (float)nodesperarm) / 100.0) * sin(angle);
            }
        }
        else
        {
            for (size_t n = 0; n < nodesperarm; n++)
            {
                int n1 = 0;
                if (alternate)
                {
                    if (n + 1 <= (nodesperarm + 1) / 2)
                    {
                        n1 = 2 * n;
                    }
                    else
                    {
                        n1 = (nodesperarm - (n + 1)) * 2 + 1;
                    }
                }
                else
                {
                    if (!fromcentre)
                    {
                        n1 = nodesperarm - n - 1;
                    }
                    else
                    {
                        n1 = n;
                    }

                    if (zigzag && a % 2 > 0)
                    {
                        if (!fromcentre)
                        {
                            n1 = n;
                        }
                        else
                        {
                            n1 = nodesperarm - n - 1;
                        }
                    }
                }

                // process each node on each arm
                size_t CoordCount = GetCoordCount(n);
                for (size_t c = 0; c < CoordCount; c++) {
                    Nodes[n + a * nodesperarm]->Coords[c].screenX = (0.5f + (float)n1 + ((float)hollow * 2.0 * (float)nodesperarm) / 100.0) * cos(angle);
                    Nodes[n + a * nodesperarm]->Coords[c].screenY = (0.5f + (float)n1 + ((float)hollow * 2.0 * (float)nodesperarm) / 100.0) * sin(angle);
                }
            }
        }
        if (cw)
        {
            angle -= angleincrement;
        }
        else
        {
            angle += angleincrement;
        }
    }
    
    float min = 99999;
    float max = -9999;
    for (auto it = Nodes.begin(); it != Nodes.end(); ++it) {
        for (auto it2 = (*it)->Coords.begin(); it2 != (*it)->Coords.end(); ++it2) {
            min = std::min(min, it2->screenY);
            max = std::max(max, it2->screenY);
        }
    }
}

int SpinnerModel::MapToNodeIndex(int strand, int node) const {
    if (SingleNode) {
        return strand;
    }

    return strand * parm2 + node;
}
int SpinnerModel::CalcCannelsPerString() {
    if (SingleNode) {
        return GetNodeChannelCount(StringType);
    }
    return GetNodeChannelCount(StringType) * parm2 * parm3;
}

int SpinnerModel::NodesPerString() const {
    return SingleNode ? 1 : parm2 * parm3;
}
