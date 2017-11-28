#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include "WreathModel.h"

WreathModel::WreathModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased): ModelWithScreenLocation(manager)
{
    SetFromXml(node, zeroBased);
}

WreathModel::~WreathModel()
{
    //dtor
}


void WreathModel::InitModel() {
    InitWreath();
    CopyBufCoord2ScreenCoord();
}


// top left=top ccw, top right=top cw, bottom left=bottom cw, bottom right=bottom ccw
void WreathModel::InitWreath() {
    SetNodeCount(parm1,parm2,rgbOrder);
    int numlights=parm1*parm2;
    SetBufferSize(numlights+1,numlights+1);
    int LastStringNum=-1;
    int offset=numlights/2;
    double r=offset;
    int chan = 0,x,y;
    double pct=isBotToTop ? 0.5 : 0.0;          // % of circle, 0=top
    double pctIncr=1.0 / (double)numlights;     // this is cw
    if (IsLtoR != isBotToTop) pctIncr*=-1.0;    // adjust to ccw
    int ChanIncr=SingleChannel ?  1 : 3;
    size_t NodeCount=GetNodeCount();
    for(size_t n=0; n<NodeCount; n++) {
        if (Nodes[n]->StringNum != LastStringNum) {
            LastStringNum=Nodes[n]->StringNum;
            chan=stringStartChan[LastStringNum];
        }
        Nodes[n]->ActChan=chan;
        chan+=ChanIncr;
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++) {
            x=r*sin(pct*2.0*M_PI) + offset + 0.5;
            y=r*cos(pct*2.0*M_PI) + offset + 0.5;
            Nodes[n]->Coords[c].bufX=x;
            Nodes[n]->Coords[c].bufY=y;
            pct+=pctIncr;
            if (pct >= 1.0) pct-=1.0;
            if (pct < 0.0) pct+=1.0;
        }
    }
}



static wxPGChoices TOP_BOT_LEFT_RIGHT;

void WreathModel::AddTypeProperties(wxPropertyGridInterface *grid) {
    if (TOP_BOT_LEFT_RIGHT.GetCount() == 0) {
        TOP_BOT_LEFT_RIGHT.Add("Top Ctr-CCW");
        TOP_BOT_LEFT_RIGHT.Add("Top Ctr-CW");
        TOP_BOT_LEFT_RIGHT.Add("Bottom Ctr-CW");
        TOP_BOT_LEFT_RIGHT.Add("Bottom Ctr-CCW");
    }
    wxPGProperty *p = grid->Append(new wxUIntProperty("# Strings", "WreathStringCount", parm1));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 640);
    p->SetEditor("SpinCtrl");

    if (SingleNode) {
        p = grid->Append(new wxUIntProperty("Lights/String", "WreathLightCount", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 640);
        p->SetEditor("SpinCtrl");
    } else {
        p = grid->Append(new wxUIntProperty("Nodes/String", "WreathLightCount", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 640);
        p->SetEditor("SpinCtrl");
    }

    grid->Append(new wxEnumProperty("Starting Location", "WreathStart", TOP_BOT_LEFT_RIGHT, IsLtoR ? (isBotToTop ? 2 : 0) : (isBotToTop ? 3 : 1)));
}
int WreathModel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("WreathStringCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm1");
        ModelXml->AddAttribute("parm1", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        AdjustStringProperties(grid, parm1);
        return 3 | 0x0008;
    } else if ("WreathLightCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3 | 0x0008;
    } else if ("WreathStart" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("Dir");
        ModelXml->AddAttribute("Dir", event.GetValue().GetLong() == 0 || event.GetValue().GetLong() == 2 ? "L" : "R");
        ModelXml->DeleteAttribute("StartSide");
        ModelXml->AddAttribute("StartSide", event.GetValue().GetLong() == 0 || event.GetValue().GetLong() == 1 ? "T" : "B");
        SetFromXml(ModelXml, zeroBased);
        return 3;
    }

    return Model::OnPropertyGridChange(grid, event);
}


