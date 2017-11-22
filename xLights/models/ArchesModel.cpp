#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "ArchesModel.h"
#include "ModelScreenLocation.h"

ArchesModel::ArchesModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased) : ModelWithScreenLocation(manager), arc(180)
{
    screenLocation.SetModelHandleHeight(true);
    screenLocation.SetSupportsAngle(true);
    SetFromXml(node, zeroBased);
}

ArchesModel::~ArchesModel()
{
}


static wxPGChoices LEFT_RIGHT;

void ArchesModel::AddTypeProperties(wxPropertyGridInterface *grid) {
    if (LEFT_RIGHT.GetCount() == 0) {
        LEFT_RIGHT.Add("Green Square");
        LEFT_RIGHT.Add("Blue Square");
    }
    wxPGProperty *p = grid->Append(new wxUIntProperty("# Arches", "ArchesCount", parm1));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Nodes Per Arch", "ArchesNodes", parm2));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 1000);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Lights Per Node", "ArchesLights", parm3));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 250);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Arc Degrees", "ArchesArc", arc));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 180);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxIntProperty("Arch Tilt", "ArchesSkew", screenLocation.GetAngle()));
    p->SetAttribute("Min", -180 );
    p->SetAttribute("Max", 180);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxEnumProperty("Starting Location", "ArchesStart", LEFT_RIGHT, IsLtoR ? 0 : 1));
}
int ArchesModel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("ArchesCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm1");
        ModelXml->AddAttribute("parm1", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        AdjustStringProperties(grid, parm1);
        return 3 | 0x0008;
    } else if ("ArchesNodes" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3 | 0x0008;
    } else if ("ArchesLights" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm3");
        ModelXml->AddAttribute("parm3", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3 | 0x0008;
    } else if ("ArchesArc" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("arc");
        ModelXml->AddAttribute("arc", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("ArchesSkew" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("Angle");
        ModelXml->AddAttribute("Angle", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("ArchesStart" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("Dir");
        ModelXml->AddAttribute("Dir", event.GetValue().GetLong() == 0 ? "L" : "R");
        SetFromXml(ModelXml, zeroBased);
        return 3;
    }

    return Model::OnPropertyGridChange(grid, event);
}



void ArchesModel::GetBufferSize(const std::string &type, const std::string &transform, int &BufferWi, int &BufferHi) const {
    if (type == "Single Line") {
        BufferHi = 1;
        BufferWi = this->BufferWi * this->BufferHt;
        AdjustForTransform(transform, BufferWi, BufferHi);
    } else {
        Model::GetBufferSize(type, transform, BufferWi, BufferHi);
    }
}
void ArchesModel::InitRenderBufferNodes(const std::string &type,  const std::string &transform,
                                        std::vector<NodeBaseClassPtr> &newNodes, int &BufferWi, int &BufferHi) const {
    if (type == "Single Line") {
        BufferHi = 1;
        BufferWi = GetNodeCount();

        int NumArches=parm1;
        int SegmentsPerArch=parm2;
        int cur = 0;
        for (int y=0; y < NumArches; y++) {
            for(int x=0; x<SegmentsPerArch; x++) {
                int idx = y * SegmentsPerArch + x;
                newNodes.push_back(NodeBaseClassPtr(Nodes[idx]->clone()));
                for(size_t c=0; c < newNodes[cur]->Coords.size(); c++) {
                    newNodes[cur]->Coords[c].bufX=cur;
                    newNodes[cur]->Coords[c].bufY=0;
                }
                cur++;
            }
        }
        ApplyTransform(transform, newNodes, BufferWi, BufferHi);
    } else {
        Model::InitRenderBufferNodes(type, transform, newNodes, BufferWi, BufferHi);
    }
}

void ArchesModel::InitModel() {
    int NumArches=parm1;
    int SegmentsPerArch=parm2;
    arc = wxAtoi(ModelXml->GetAttribute("arc", "180"));

    if (ModelXml->HasAttribute("ArchesSkew")) {
        ModelXml->DeleteAttribute("ArchesSkew");
        int skew = wxAtoi(ModelXml->GetAttribute("ArchesSkew", "0"));
        screenLocation.SetAngle(skew);
    }

    SetBufferSize(NumArches,SegmentsPerArch);
    if (SingleNode) {
        SetNodeCount(NumArches * SegmentsPerArch, parm3,rgbOrder);
    } else {
        SetNodeCount(NumArches, SegmentsPerArch, rgbOrder);
        if (parm3 > 1) {
            for (size_t x = 0; x < Nodes.size(); x++) {
                Nodes[x]->Coords.resize(parm3);
            }
        }
    }
    screenLocation.SetRenderSize(SegmentsPerArch, NumArches);

    for (int y=0; y < NumArches; y++) {
        for(int x=0; x<SegmentsPerArch; x++) {
            int idx = y * SegmentsPerArch + x;
            Nodes[idx]->ActChan = stringStartChan[y] + x*GetNodeChannelCount(StringType);
            Nodes[idx]->StringNum=y;
            for(size_t c=0; c < GetCoordCount(idx); c++) {
                Nodes[idx]->Coords[c].bufX=IsLtoR ? x : SegmentsPerArch-x-1;
                Nodes[idx]->Coords[c].bufY=isBotToTop ? y : NumArches-y-1;
            }
        }
    }
    SetArchCoord();
}
int ArchesModel::MapToNodeIndex(int strand, int node) const {
    return strand * parm2 + node;
}
int ArchesModel::GetNumStrands() const {
     return parm1;
}
int ArchesModel::CalcCannelsPerString() {
    SingleChannel = false;
    return GetNodeChannelCount(StringType) * parm2;
}
static inline double toRadians(long degrees) {
    return 2.0*M_PI*double(degrees)/360.0;
}

static void rotate_point(float cx,float cy, float angle, float &x, float &y)
{
    float s = sin(angle);
    float c = cos(angle);

    // translate point back to origin:
    x -= cx;
    y -= cy;

    // rotate point
    float xnew = x * c - y * s;
    float ynew = x * s + y * c;

    // translate point back:
    x = xnew + cx;
    y = ynew + cy;
}

void ArchesModel::SetArchCoord() {
    double xoffset,x,y;
    size_t NodeCount=GetNodeCount();
    double midpt=parm2*parm3;
    midpt -= 1.0;
    midpt /= 2.0;
    double total = toRadians(arc);
    double start = (M_PI - total) / 2.0;
    float skew_angle = toRadians(screenLocation.GetAngle());

    double angle=-M_PI/2.0 + start;
    x=midpt*sin(angle)*2.0+parm2*parm3;
    double width = parm2*parm3*2 - x;

    double minY = 999999;
    for(size_t n=0; n<NodeCount; n++) {
        xoffset=Nodes[n]->StringNum*width;
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++) {
            double angle=-M_PI/2.0 + start + total * ((double)(Nodes[n]->Coords[c].bufX * parm3 + c))/midpt/2.0;
            x=xoffset + midpt*sin(angle)*2.0+parm2*parm3;
            y=(parm2*parm3)*cos(angle);
            Nodes[n]->Coords[c].screenX=x;
            Nodes[n]->Coords[c].screenY=y * screenLocation.GetHeight();
            rotate_point(x, 0, skew_angle,
                        Nodes[n]->Coords[c].screenX,
                        Nodes[n]->Coords[c].screenY);
            minY = std::min(minY, y);
        }
    }
    float renderHt = parm2*parm3;
    if (minY > 1) {
        renderHt -= minY;
        for (auto it = Nodes.begin(); it != Nodes.end(); it++) {
            for (auto coord = (*it)->Coords.begin(); coord != (*it)->Coords.end(); coord++) {
                coord->screenY -= minY;
            }
        }
    }
    screenLocation.SetRenderSize(width*parm1, renderHt);
}
