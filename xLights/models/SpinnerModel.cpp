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

static wxPGChoices TOP_BOT_LEFT_RIGHT;

wxString DecodeStart(long start)
{
    return TOP_BOT_LEFT_RIGHT.GetLabel(start);
}

int EncodeStarts(const wxString& start)
{
    int i = 0;
    for (auto it = TOP_BOT_LEFT_RIGHT.GetLabels().begin(); it != TOP_BOT_LEFT_RIGHT.GetLabels().end(); it++)
    {
        if (*it == start)
        {
            return i;
        }
        i++;
    }
    return -1;
}

void SpinnerModel::AddTypeProperties(wxPropertyGridInterface *grid) {
    if (TOP_BOT_LEFT_RIGHT.GetCount() == 0) {
        TOP_BOT_LEFT_RIGHT.Add("Center Counter Clockwise");
        TOP_BOT_LEFT_RIGHT.Add("Center Clockwise");
        TOP_BOT_LEFT_RIGHT.Add("End Counter Clockwise");
        TOP_BOT_LEFT_RIGHT.Add("End Clockwise");
    }
    
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
    p->SetAttribute("Max", 50);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Arc", "Arc", arc));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 360);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxEnumProperty("Starting Location", "MatrixStart", TOP_BOT_LEFT_RIGHT, IsLtoR ? (isBotToTop ? 2 : 0) : (isBotToTop ? 3 : 1)));

    p = grid->Append(new wxBoolProperty("Zig-Zag Start", "ZigZag", zigzag));
    p->SetEditor("CheckBox");
}

int SpinnerModel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("SpinnerStringCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm1");
        ModelXml->AddAttribute("parm1", wxString::Format("%d", event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        AdjustStringProperties(grid, parm1);
        return 3 | 0x0008;
    } else if ("SpinnerArmNodeCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%d", event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3 | 0x0008;
    } else if ("FoldCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm3");
        ModelXml->AddAttribute("parm3", wxString::Format("%d", event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3 | 0x0008;
    } else if ("Hollow" == event.GetPropertyName()) {
         ModelXml->DeleteAttribute("Hollow");
         ModelXml->AddAttribute("Hollow", wxString::Format("%d", event.GetPropertyValue().GetLong()));
         SetFromXml(ModelXml, zeroBased);
         return 3 | 0x0008;
    } else if ("Arc" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("Arc");
        ModelXml->AddAttribute("Arc", wxString::Format("%d", event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3 | 0x0008;
    } else if ("ZigZag" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("ZigZag");
        ModelXml->AddAttribute("ZigZag", event.GetPropertyValue().GetBool() ? "true" : "false");
        SetFromXml(ModelXml, zeroBased);
        return 3 | 0x0008;
    } else if ("MatrixStart" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("Dir");
        ModelXml->AddAttribute("Dir", event.GetValue().GetLong() == 0 || event.GetValue().GetLong() == 2 ? "L" : "R");
        ModelXml->DeleteAttribute("StartSide");
        ModelXml->AddAttribute("StartSide", event.GetValue().GetLong() == 0 || event.GetValue().GetLong() == 1 ? "T" : "B");
        SetFromXml(ModelXml, zeroBased);
        return 3;
    }
    
    return Model::OnPropertyGridChange(grid, event);
}

void SpinnerModel::GetBufferSize(const std::string &type, const std::string &transform,
    int &BufferWi, int &BufferHi) const
{
    if (type == "Single Line") {
        BufferHi = 1;
        BufferWi = this->BufferWi * this->BufferHt;
        AdjustForTransform(transform, BufferWi, BufferHi);
    }
    else {
        Model::GetBufferSize(type, transform, BufferWi, BufferHi);
    }
}

int SpinnerModel::GetNumStrands() const {
    if (SingleChannel) {
        return 1;
    }
    return parm1*parm3;
}

void SpinnerModel::InitModel() {
    int stringcount = parm1;
    int nodesperarm = parm2;
    int armsperstring = parm3;
    int pixelsperstring = parm3*parm2;
    int armcount = parm3*parm1;
    int totalnodes = stringcount * armsperstring * nodesperarm;
    hollow = wxAtoi(ModelXml->GetAttribute("Hollow", "20"));
    arc = wxAtoi(ModelXml->GetAttribute("Arc", "360"));
    zigzag = (ModelXml->GetAttribute("ZigZag", "false") == "true");

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
        SetBufferSize(nodesperarm, armcount);
        for (size_t x = 0; x < armcount; x++) {
            int stringnum = x / armsperstring;
            int segmentnum = x % armsperstring;
            for (size_t y = 0; y < nodesperarm; y++)
            {
                size_t idx = stringnum * pixelsperstring + segmentnum * nodesperarm + y;
                Nodes[idx]->ActChan = stringStartChan[stringnum] + segmentnum * nodesperarm * 3 + y * 3;
                Nodes[idx]->Coords[0].bufX = IsLtoR != (segmentnum % 2 == 0) ? x : armcount - x - 1;
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

    SetSpinnerCoord();

    DisplayAs = "Spinner";
}

void SpinnerModel::InitRenderBufferNodes(const std::string &type, const std::string &transform,
    std::vector<NodeBaseClassPtr> &newNodes, int &BufferWi, int &BufferHi) const {
    if (type == "Single Line") {
        BufferHi = 1;
        BufferWi = GetNodeCount();

        int stringcount = parm1;
        int nodesperarm = parm2;
        int armsperstring = parm3;
        int cur = 0;
        for (size_t y = 0; y < stringcount*armsperstring; y++) {
            for (int x = 0; x<nodesperarm; x++) {
                int idx = y * nodesperarm + x;
                newNodes.push_back(NodeBaseClassPtr(Nodes[idx]->clone()));
                for (size_t c = 0; c < newNodes[cur]->Coords.size(); c++) {
                    newNodes[cur]->Coords[c].bufX = cur;
                    newNodes[cur]->Coords[c].bufY = 0;
                }
                cur++;
            }
        }
        ApplyTransform(transform, newNodes, BufferWi, BufferHi);
    }
    else {
        Model::InitRenderBufferNodes(type, transform, newNodes, BufferWi, BufferHi);
    }
}

void SpinnerModel::SetSpinnerCoord() {
    int stringcount = parm1;
    int nodesperarm = parm2;
    int armsperstring = parm3;
    int armcount = parm3*parm1;

    float angle = (M_PI * 2.0 * 270.0) / 360.0;
    float angleincrement = (M_PI * 2.0f * (float)arc) / ((float)stringcount * (float)armsperstring * 360.0);
    if (arc < 360 & armsperstring * stringcount > 1)
    {
        angleincrement = (M_PI * 2.0f * (float)arc) / (((float)stringcount * (float)armsperstring - 1) * 360.0);
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
            size_t CoordCount = GetCoordCount(a1);
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
    for (auto it = Nodes.begin(); it != Nodes.end(); it++) {
        for (auto it2 = (*it)->Coords.begin(); it2 != (*it)->Coords.end(); it2++) {
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
    return GetNodeChannelCount(StringType) * parm2;
}
