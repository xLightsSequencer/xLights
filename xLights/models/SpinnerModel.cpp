//
// TODO
//
// Allow user to control how hollow the spinner is
// Get node numbering working correctly for center and bottom
// can I tag the first node with a green square


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
    
    AddStyleProperties(grid);

    wxPGProperty *p = grid->Append(new wxUIntProperty("Lights/Arm", "SpinnerArmNodeCount", parm2));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 200);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("# Strings", "SpinnerStringCount", parm1));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 640);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Arms/String", "FoldCount", parm3));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 250);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Hollow Pct", "Hollow", hollow));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 50);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Arc", "Arc", arc));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 360);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxEnumProperty("Starting Location", "SpinnerStart", TOP_BOT_LEFT_RIGHT, IsLtoR ? (isBotToTop ? 2 : 0) : (isBotToTop ? 3 : 1)));
}

void SpinnerModel::AddStyleProperties(wxPropertyGridInterface *grid) {
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
    } else if ("SpinnerStart" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("Dir");
        ModelXml->AddAttribute("Dir", event.GetValue().GetLong() == 0 || event.GetValue().GetLong() == 2 ? "L" : "R");
        ModelXml->DeleteAttribute("StartSide");
        ModelXml->AddAttribute("StartSide", event.GetValue().GetLong() == 0 || event.GetValue().GetLong() == 1 ? "T" : "B");
        SetFromXml(ModelXml, zeroBased);
        return 3;
    }
    
    return Model::OnPropertyGridChange(grid, event);
}

int SpinnerModel::GetNumStrands() const {
    if (SingleChannel) {
        return 1;
    }
    return parm1*parm3;
}

// parm1=NumStrings
// parm2=pixelsperstring
// parm3=StrandsPerString
void SpinnerModel::InitModel() {
    int stringcount = parm1;
    int nodesperarm = parm2;
    int armsperstring = parm3;
    int pixelsperstring = parm3*parm2;
    int armcount = parm3*parm1;
    int totalnodes = stringcount * armsperstring * nodesperarm;

    SetBufferSize(armcount, nodesperarm);
    SetNodeCount(stringcount, pixelsperstring, rgbOrder);
    screenLocation.SetRenderSize(2 * nodesperarm + 1 + (hollow * nodesperarm) / 100.0, 2 * nodesperarm + 1 + (hollow * nodesperarm) / 100.0);

    // create output mapping
    if (SingleNode) {
        size_t y = 0;
        for (size_t n = 0; n<Nodes.size(); n++) {
            Nodes[n]->ActChan = stringStartChan[n];
            size_t x = 0;
            size_t xincr = 1;
            for (size_t c = 0; c<pixelsperstring; c++) {
                Nodes[n]->Coords[c].bufX = x;
                Nodes[n]->Coords[c].bufY = isBotToTop ? y : armcount - y - 1;
                x += xincr;
                if (x < 0 || x >= nodesperarm) {
                    xincr = -xincr;
                    x += xincr;
                    y++;
                }
            }
        }
    }
    else {
        for (size_t y = 0; y < armcount; y++) {
            int stringnum = y / armsperstring;
            int segmentnum = y % armsperstring;
            for (size_t x = 0; x<nodesperarm; x++) {
                int idx = stringnum * pixelsperstring + segmentnum * nodesperarm + x;
                Nodes[idx]->ActChan = stringStartChan[stringnum] + segmentnum * nodesperarm * 3 + x * 3;
                Nodes[idx]->Coords[0].bufX = IsLtoR != (segmentnum % 2 == 0) ? nodesperarm - x - 1 : x;
                Nodes[idx]->Coords[0].bufY = isBotToTop ? y : armcount - y - 1;
                Nodes[idx]->StringNum = stringnum;
            }
        }
    }

    SetSpinnerCoord();

    DisplayAs = "Spinner";
}

//void SpinnerModel::InitModel() {
//    int stringcount = parm1;
//    int nodesperarm = parm2;
//    int armsperstring = parm3;
//    int pixelsperstring = parm3*parm2;
//    int armcount = parm3*parm1;
//    int totalnodes = stringcount * armsperstring * nodesperarm;
//
//    SetBufferSize(nodesperarm, armcount);
//    SetNodeCount(parm1, pixelsperstring, rgbOrder);
//    screenLocation.SetRenderSize(2*nodesperarm + 1, 2 * nodesperarm + 1);
//
//    // create output mapping
//    if (SingleNode) {
//        size_t x = 0;
//        for (size_t n = 0; n<Nodes.size(); n++) {
//            Nodes[n]->ActChan = stringStartChan[n];
//            size_t y = 0;
//            size_t yincr = 1;
//            for (size_t c = 0; c<pixelsperstring; c++) {
//                Nodes[n]->Coords[c].bufX = IsLtoR ? x : armcount - x - 1;
//                Nodes[n]->Coords[c].bufY = y;
//                y += yincr;
//                if (y < 0 || y >= nodesperarm) {
//                    yincr = -yincr;
//                    y += yincr;
//                    x++;
//                }
//            }
//        }
//    }
//    else {
//        std::vector<int> strandStartChan;
//        strandStartChan.clear();
//        strandStartChan.resize(armcount);
//        for (int x = 0; x < armcount; x++) {
//            int stringnum = x / armsperstring;
//            int segmentnum = x % armsperstring;
//            strandStartChan[x] = stringStartChan[stringnum] + segmentnum * nodesperarm * 3;
//        }
//
//        for (size_t x = 0; x < armcount; x++) {
//            int stringnum = x / armsperstring;
//            int segmentnum = x % armsperstring;
//            for (size_t y = 0; y < nodesperarm; y++) {
//                int idx = stringnum * pixelsperstring + segmentnum * nodesperarm + y;
//                Nodes[idx]->ActChan = strandStartChan[x] + y * 3;
//                Nodes[idx]->Coords[0].bufX = IsLtoR ? x : armcount - x - 1;
//                Nodes[idx]->Coords[0].bufY = isBotToTop == (segmentnum % 2 == 0) ? y : nodesperarm - y - 1;
//                Nodes[idx]->StringNum = stringnum;
//            }
//        }
//    }
//
//    SetSpinnerCoord();
//
//    DisplayAs = "Spinner";
//}

void SpinnerModel::GetBufferSize(const std::string &type, const std::string &transform, int &BufferWi, int &BufferHi) const {
    if (type == "Single Line") {
        BufferHi = 1;
        BufferWi = this->BufferWi * this->BufferHt;
        AdjustForTransform(transform, BufferWi, BufferHi);
    }
    else {
        Model::GetBufferSize(type, transform, BufferWi, BufferHi);
    }
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
        for (int y = 0; y < nodesperarm; y++) {
            for (int x = 0; x<stringcount*armsperstring; x++) {
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

    float angle = (M_PI * 2.0 * 270.0) / 360.0;
    float angleincrement = (M_PI * 2.0f * (float)arc) / ((float)stringcount * (float)armsperstring * 360.0);

    for (size_t a = 0; a < stringcount * armsperstring; a++)
    {
        // do each arm
        for (size_t n = 0; n < nodesperarm; n++)
        {
            // process each node on each arm
            size_t CoordCount = GetCoordCount(n);
            for (size_t c = 0; c < CoordCount; c++) {
                Nodes[n + a * nodesperarm]->Coords[c].screenX = (0.5f + (float)n + ((float)hollow * (float)nodesperarm) / 100.0) * cos(angle);
                Nodes[n + a * nodesperarm]->Coords[c].screenY = (0.5f + (float)n) * sin(angle);
            }
        }

        angle += angleincrement;
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
    return strand * parm2 + node;
}
int SpinnerModel::CalcCannelsPerString() {
    if (SingleNode) {
        return GetNodeChannelCount(StringType);
    }
    return GetNodeChannelCount(StringType) * parm2;
}
