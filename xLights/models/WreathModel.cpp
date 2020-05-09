/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

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
    screenLocation.RenderDp = 10.0f;  // give the bounding box a little depth
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
    int ChanIncr = GetNodeChannelCount(StringType);
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

static const char* TOP_BOT_LEFT_RIGHT_VALUES[] = { "Top Ctr-CCW", "Top Ctr-CW", "Bottom Ctr-CW", "Bottom Ctr-CCW" };
static wxPGChoices TOP_BOT_LEFT_RIGHT(wxArrayString(4, TOP_BOT_LEFT_RIGHT_VALUES));

void WreathModel::AddTypeProperties(wxPropertyGridInterface *grid) {
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
        //AdjustStringProperties(grid, parm1);
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "WreathModel::OnPropertyGridChange::WreathStringCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "WreathModel::OnPropertyGridChange::WreathStringCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "WreathModel::OnPropertyGridChange::WreathStringCount");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "WreathModel::OnPropertyGridChange::WreathStringCount");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "WreathModel::OnPropertyGridChange::WreathStringCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "WreathModel::OnPropertyGridChange::WreathStringCount");
        return 0;
    } else if ("WreathLightCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "WreathModel::OnPropertyGridChange::WreathLightCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "WreathModel::OnPropertyGridChange::WreathLightCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "WreathModel::OnPropertyGridChange::WreathLightCount");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "WreathModel::OnPropertyGridChange::WreathLightCount");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "WreathModel::OnPropertyGridChange::WreathLightCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "WreathModel::OnPropertyGridChange::WreathLightCount");
        return 0;
    } else if ("WreathStart" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("Dir");
        ModelXml->AddAttribute("Dir", event.GetValue().GetLong() == 0 || event.GetValue().GetLong() == 2 ? "L" : "R");
        ModelXml->DeleteAttribute("StartSide");
        ModelXml->AddAttribute("StartSide", event.GetValue().GetLong() == 0 || event.GetValue().GetLong() == 1 ? "T" : "B");
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "WreathModel::OnPropertyGridChange::WreathStart");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "WreathModel::OnPropertyGridChange::WreathStart");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "WreathModel::OnPropertyGridChange::WreathStart");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "WreathModel::OnPropertyGridChange::WreathStart");
        return 0;
    }

    return Model::OnPropertyGridChange(grid, event);
}