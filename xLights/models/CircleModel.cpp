

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>
#include "CircleModel.h"
#include "ModelScreenLocation.h"

CircleModel::CircleModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased) : ModelWithScreenLocation(manager), insideOut(false)
{
    SetFromXml(node, zeroBased);
}

CircleModel::~CircleModel()
{
}


int CircleModel::GetStrandLength(int strand) const {
    return SingleNode ? 1 : circleSizes[strand];
}

bool CircleModel::AllNodesAllocated() const
{
    int allocated = 0;
    for (auto it = circleSizes.begin(); it != circleSizes.end(); ++it)
    {
        allocated += *it;
    }

    return (allocated == GetNodeCount());
}

int CircleModel::MapToNodeIndex(int strand, int node) const {
    int idx = 0;
    for (int x = 0; x < strand; x++) {
        idx += circleSizes[x];
    }
    idx += node;
    return idx;
}
int CircleModel::GetNumStrands() const {
    return circleSizes.size();
}


// parm3 is number of points
// top left=top ccw, top right=top cw, bottom left=bottom cw, bottom right=bottom ccw

void CircleModel::InitModel() {
    if (!ModelXml->HasAttribute("StartSide")) {
        isBotToTop = false;
    }
    if( ModelXml->HasAttribute("circleSizes") )
    {
        wxString tempstr=ModelXml->GetAttribute("circleSizes");
        circleSizes.resize(0);
        while (tempstr.size() > 0) {
            wxString t2 = tempstr;
            if (tempstr.Contains(",")) {
                t2 = tempstr.SubString(0, tempstr.Find(","));
                tempstr = tempstr.SubString(tempstr.Find(",") + 1, tempstr.length());
            } else {
                tempstr = "";
            }
            long i2 = 0;
            t2.ToLong(&i2);
            if ( i2 > 0) {
                circleSizes.resize(circleSizes.size() + 1);
                circleSizes[circleSizes.size() - 1] = i2;
            }
        }
    }
    if (ModelXml->HasAttribute("InsideOut")) {
        insideOut = wxAtoi(ModelXml->GetAttribute("InsideOut"));
    }
    InitCircle();
    SetCircleCoord();
}

int CircleModel::maxSize() {
    int maxLights = 0;
    for (int x = 0; x < circleSizes.size(); x++) {
        if (circleSizes[x] > maxLights) {
            maxLights = circleSizes[x];
        }
    }
    return maxLights;
}


void CircleModel::InitCircle() {
    int maxLights = 0;
    int numLights = parm1 * parm2;
    int cnt = 0;

    if (circleSizes.size() == 0) {
        circleSizes.resize(1);
        circleSizes[0] = numLights;
    }
    for (int x = 0; x < circleSizes.size(); x++) {
        if ((cnt + circleSizes[x]) > numLights) {
            circleSizes[x] = numLights - cnt;
        }
        cnt += circleSizes[x];
        if (circleSizes[x] > maxLights) {
            maxLights = circleSizes[x];
        }
    }

    SetNodeCount(parm1,parm2,rgbOrder);
    SetBufferSize(circleSizes.size(),maxLights);
    int LastStringNum=-1;
    int chan = 0,idx;
    int ChanIncr=SingleChannel ?  1 : 3;
    size_t NodeCount=GetNodeCount();

    size_t node = 0;
    int nodesToMap = NodeCount;
    for (int circle = 0; circle < circleSizes.size(); circle++) {
        idx = 0;
        int loop_count = std::min(nodesToMap, circleSizes[circle]);
        for(size_t n=0; n<loop_count; n++) {
            if (Nodes[node]->StringNum != LastStringNum) {
                LastStringNum=Nodes[node]->StringNum;
                chan=stringStartChan[LastStringNum];
            }
            Nodes[node]->ActChan=chan;
            chan+=ChanIncr;
            double pct = (loop_count == 1) ? (double)n : (double)n / (double)(loop_count-1);
            size_t CoordCount=GetCoordCount(node);
            for(size_t c=0; c < CoordCount; c++) {
                int x_pos = (circleSizes[circle] == maxLights) ? idx : std::round(pct*(double)(maxLights-1));
                Nodes[node]->Coords[c].bufX = x_pos;
                Nodes[node]->Coords[c].bufY = circle;
                idx++;
            }
            node++;
        }
        nodesToMap -= loop_count;
    }
}

// Set screen coordinates for circles
void CircleModel::SetCircleCoord() {
    double x,y;
    size_t NodeCount=GetNodeCount();
    int maxLights = maxSize();
    screenLocation.SetRenderSize(maxLights, maxLights);
    int nodesToMap = NodeCount;
    int node = 0;
    double maxRadius = circleSizes[0] / 2.0;
    double minRadius = (double)parm3/100.0 * maxRadius;
    for (int c2 = 0; c2 < circleSizes.size(); c2++) {
        int circle = c2;
        int loop_count = std::min(nodesToMap, circleSizes[circle]);
        if (insideOut) {
            circle = circleSizes.size() - circle - 1;
        }
        double radius = (circleSizes.size() == 1) ? maxRadius : (double)minRadius + (maxRadius-minRadius)*(1.0-(double)circle/(double)(circleSizes.size()-1));
        for(size_t n=0; n<loop_count; n++) {
            size_t CoordCount=GetCoordCount(node);
            for(size_t c=0; c < CoordCount; c++) {
                double angle= (isBotToTop ? -M_PI : 0) + M_PI * ((loop_count==1) ? 1 : (double)n / (double)loop_count) * 2.0;
                if (!IsLtoR) {
                    angle *= -1;
                }
                x=sin(angle)*radius;
                y=cos(angle)*radius;
                Nodes[node]->Coords[c].screenX=x;
                Nodes[node]->Coords[c].screenY=y;
            }
            node++;
        }
        nodesToMap -= loop_count;
    }
}

static wxPGChoices CIRCLE_START_LOCATION;

void CircleModel::AddTypeProperties(wxPropertyGridInterface *grid) {
    if (CIRCLE_START_LOCATION.GetCount() == 0) {
        CIRCLE_START_LOCATION.Add("Top Outer-CCW");
        CIRCLE_START_LOCATION.Add("Top Outer-CW");
        CIRCLE_START_LOCATION.Add("Top Inner-CCW");
        CIRCLE_START_LOCATION.Add("Top Inner-CW");
        
        CIRCLE_START_LOCATION.Add("Bottom Outer-CCW");
        CIRCLE_START_LOCATION.Add("Bottom Outer-CW");
        CIRCLE_START_LOCATION.Add("Bottom Inner-CCW");
        CIRCLE_START_LOCATION.Add("Bottom Inner-CW");
    }
    
    wxPGProperty *p = grid->Append(new wxUIntProperty("# Strings", "CircleStringCount", parm1));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    if (SingleNode) {
        p = grid->Append(new wxUIntProperty("Lights/String", "CircleLightCount", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 1000);
        p->SetEditor("SpinCtrl");
    } else {
        p = grid->Append(new wxUIntProperty("Nodes/String", "CircleLightCount", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 1000);
        p->SetEditor("SpinCtrl");
    }

    p = grid->Append(new wxUIntProperty("Center %", "CircleCenterPercent", parm3));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    grid->Append(new wxStringProperty("Layer Sizes", "CircleLayerSizes", ModelXml->GetAttribute("circleSizes")));
    
    int start = IsLtoR ? 1 : 0;
    if (insideOut) {
        start += 2;
    }
    if (isBotToTop) {
        start += 4;
    }
    grid->Append(new wxEnumProperty("Starting Location", "CircleStart", CIRCLE_START_LOCATION, start));
}
int CircleModel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("CircleStringCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm1");
        ModelXml->AddAttribute("parm1", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        AdjustStringProperties(grid, parm1);
        return 3 | 0x0008;
    } else if ("CircleLightCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3 | 0x0008;
    } else if ("CircleCenterPercent" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm3");
        ModelXml->AddAttribute("parm3", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("CircleLayerSizes" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("circleSizes");
        ModelXml->AddAttribute("circleSizes", event.GetValue().GetString());
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if ("CircleStart" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("Dir");
        ModelXml->DeleteAttribute("StartSide");
        ModelXml->DeleteAttribute("InsideOut");
        
        int v = event.GetValue().GetLong();
        ModelXml->AddAttribute("Dir", v & 0x1 ? "L" : "R");
        ModelXml->AddAttribute("StartSide", v < 4 ? "T" : "B");
        ModelXml->AddAttribute("InsideOut", v & 0x2 ? "1" : "0");

        SetFromXml(ModelXml, zeroBased);
        return 3;
    }

    return Model::OnPropertyGridChange(grid, event);
}
