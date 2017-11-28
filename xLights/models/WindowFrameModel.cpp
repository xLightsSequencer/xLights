#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>
#include "WindowFrameModel.h"
#include "ModelScreenLocation.h"

WindowFrameModel::WindowFrameModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased) : ModelWithScreenLocation(manager)
{
    SetFromXml(node, zeroBased);
}

WindowFrameModel::~WindowFrameModel()
{
    //dtor
}
 
void WindowFrameModel::InitModel() {
    InitFrame();
    //CopyBufCoord2ScreenCoord();
}

int WindowFrameModel::NodesPerString()
{
    return GetChanCount() / GetChanCountPerNode();
}

// initialize buffer coordinates
// parm1=Nodes on Top
// parm2=Nodes left and right
// parm3=Nodes on Bottom
void WindowFrameModel::InitFrame() {
    int x,y,newx,newy;
    SetNodeCount(1,parm1+2*parm2+parm3,rgbOrder);
    int FrameWidth=std::max(parm1,parm3)+2;
    SetBufferSize(parm2,FrameWidth);   // treat as outside of matrix
    //SetBufferSize(1,Nodes.size());   // treat as single string
    screenLocation.SetRenderSize(FrameWidth, parm2);
    int chan=stringStartChan[0];
    int ChanIncr=SingleChannel ?  1 : 3;

    float top_incr = (float)(FrameWidth-1)/(float)(parm1+1);
    float bot_incr = -1*(float)(FrameWidth-1)/(float)(parm3+1);

    float screenxincr[4]= {0.0f,top_incr,0.0f,bot_incr}; // indexed by side
    int xincr[4]= {0,1,0,-1}; // indexed by side
    int yincr[4]= {1,0,-1,0};
    x=IsLtoR ? 0 : FrameWidth-1;
    y=isBotToTop ? 0 : parm2-1;
    int dir=1;            // 1=clockwise
    int side=x>0 ? 2 : 0; // 0=left, 1=top, 2=right, 3=bottom
    int SideIncr=1;       // 1=clockwise
    if ((parm1 > parm3 && x>0) || (parm3 > parm1 && x==0)) {
        // counter-clockwise
        dir=-1;
        SideIncr=3;
    }

    // determine starting position
    if (parm1 > parm3) {
        // more nodes on top, must start at bottom
        y=0;
    } else if (parm3 > parm1) {
        // more nodes on bottom, must start at top
        y=parm2-1;
    } else {
        // equal top and bottom, can start in any corner
        // assume clockwise numbering
        if (x>0 && y==0) {
            // starting in lower right
            side=3;
        } else if (x==0 && y>0) {
            // starting in upper left
            side=1;
        }
    }

    int xoffset=BufferWi/2;
    int yoffset=BufferHt/2;

    float screenx = x - xoffset;

    size_t NodeCount=GetNodeCount();
    for(size_t n=0; n<NodeCount; n++) {
        Nodes[n]->ActChan=chan;
        chan+=ChanIncr;
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++) {
            Nodes[n]->Coords[c].bufX=x;
            Nodes[n]->Coords[c].bufY=y;
            Nodes[n]->Coords[c].screenX=screenx;
            Nodes[n]->Coords[c].screenY=y-yoffset;
            float new_screenx=screenx+(screenxincr[side]*(float)dir);
            newx=x+xincr[side]*dir;
            newy=y+yincr[side]*dir;
            if (newx < 0 || newx >= FrameWidth || newy < 0 || newy >= parm2) {
                // move to the next side
                side=(side+SideIncr) % 4;
                newx=x+xincr[side]*dir;
                newy=y+yincr[side]*dir;
                new_screenx=screenx+(screenxincr[side]*(float)dir);
            }
            x=newx;
            y=newy;
            screenx = new_screenx;
        }
    }
}


static wxPGChoices TOP_BOT_LEFT_RIGHT;

void WindowFrameModel::AddTypeProperties(wxPropertyGridInterface *grid) {
    if (TOP_BOT_LEFT_RIGHT.GetCount() == 0) {
        TOP_BOT_LEFT_RIGHT.Add("Top Left");
        TOP_BOT_LEFT_RIGHT.Add("Top Right");
        TOP_BOT_LEFT_RIGHT.Add("Bottom Left");
        TOP_BOT_LEFT_RIGHT.Add("Bottom Right");
    }
    wxPGProperty *p = grid->Append(new wxUIntProperty("# Lights Top", "WFTopCount", parm1));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 1000);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("# Lights Left/Right", "WFLeftRightCount", parm2));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 1000);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("# Lights Bottom", "WFBottomCount", parm3));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 1000);
    p->SetEditor("SpinCtrl");

    grid->Append(new wxEnumProperty("Starting Location", "WFStartLocation", TOP_BOT_LEFT_RIGHT, IsLtoR ? (isBotToTop ? 2 : 0) : (isBotToTop ? 3 : 1)));
}
int WindowFrameModel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("WFTopCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm1");
        ModelXml->AddAttribute("parm1", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3 | 0x0008;
    } else if ("WFLeftRightCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3 | 0x0008;
    } else if ("WFBottomCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm3");
        ModelXml->AddAttribute("parm3", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3 | 0x0008;
    } else if ("WFStartLocation" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("Dir");
        ModelXml->AddAttribute("Dir", event.GetValue().GetLong() == 0 || event.GetValue().GetLong() == 2 ? "L" : "R");
        ModelXml->DeleteAttribute("StartSide");
        ModelXml->AddAttribute("StartSide", event.GetValue().GetLong() == 0 || event.GetValue().GetLong() == 1 ? "T" : "B");
        SetFromXml(ModelXml, zeroBased);
        return 3;
    }

    return Model::OnPropertyGridChange(grid, event);
}

