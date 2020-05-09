/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>

#include "WindowFrameModel.h"
#include "ModelScreenLocation.h"

WindowFrameModel::WindowFrameModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased) : ModelWithScreenLocation(manager)
{
    rotation = (node->GetAttribute("Rotation", "CW") == "Clockwise" || node->GetAttribute("Rotation", "CW") == "CW") ? 0 : 1;
    SetFromXml(node, zeroBased);
}

WindowFrameModel::~WindowFrameModel()
{
    //dtor
}
 
void WindowFrameModel::InitModel() {
    rotation = (ModelXml->GetAttribute("Rotation", "CW") == "Clockwise" || ModelXml->GetAttribute("Rotation", "CW") == "CW") ? 0 : 1;
    InitFrame();
    screenLocation.RenderDp = 10.0f;  // give the bounding box a little depth
}

int WindowFrameModel::NodesPerString() const
{
    return GetChanCount() / std::max(GetChanCountPerNode(),1);
}

// initialize buffer coordinates
// parm1=Nodes on Top
// parm2=Nodes left and right
// parm3=Nodes on Bottom
void WindowFrameModel::InitFrame() 
{
    SetNodeCount(1, parm1 + 2 * parm2 + parm3, rgbOrder);

    int left = parm2;
    int top = parm1;
    int bottom = parm3;

    int width = std::max(top, bottom) + 2;
    int height = parm2;

    SetBufferSize(height, width);   // treat as outside of matrix
    screenLocation.SetRenderSize(width, height);
    int chan = stringStartChan[0];
    int ChanIncr = GetNodeChannelCount(StringType);

    float top_incr = (float)(width - 1) / (float)(top + 1);
    float bot_incr = -1 * (float)(width - 1) / (float)(bottom + 1);

    float screenxincr[4]= {0.0f, top_incr, 0.0f, bot_incr}; // indexed by side
    int xincr[4] = { 0, 1, 0, -1}; // indexed by side
    int yincr[4] = { 1, 0, -1, 0};

    int x;
    int y;
    int start;
    if (IsLtoR)
    {
        x = 0;
        if (isBotToTop)
        {
            y = 0;
            if (bottom == 0)
            {
                start = 0;
            }
            else
            {
                start = 3; // bottom
            }
        }
        else
        {
            y = height - 1;
            start = 0; // left
        }
    }
    else
    {
        x = width - 1;
        if (isBotToTop)
        {
            y = 0;
            start = 2; // right
        }
        else
        {
            y = height - 1;
            if (top == 0)
            {
                start = 2;
            }
            else
            {
                start = 1; // top
            }
        }
    }

    float dir = (ModelXml->GetAttribute("Rotation", "CW") == "Clockwise" || ModelXml->GetAttribute("Rotation", "CW") == "CW") ? 1.0 : -1.0;

    int xoffset = BufferWi/2;
    int yoffset = BufferHt/2;

    float screenx = x - xoffset;

    size_t NodeCount = GetNodeCount();
    for(size_t n = 0; n < NodeCount; n++) {
        Nodes[n]->ActChan = chan;
        chan += ChanIncr;
        size_t CoordCount = GetCoordCount(n);
        for (size_t c = 0; c < CoordCount; c++) {
            Nodes[n]->Coords[c].bufX = x;
            Nodes[n]->Coords[c].bufY = y;
            Nodes[n]->Coords[c].screenX = screenx;
            Nodes[n]->Coords[c].screenY = y - yoffset;
            float new_screenx = screenx + (screenxincr[start] * dir);
            int newx = x + xincr[start] * dir;
            int newy = y + yincr[start] * dir;
            if (newx < 0 || newx >= width || newy < 0 || newy >= height) {
                // move to the next side
                start = (int)(4 + start + dir) % 4;
                if ((start == 1 && top == 0) ||
                    (start == 3 && bottom == 0) ||
                    (left == 0 && (start == 0 || start == 2)))
                {
                    // skip over zero pixel sides
                    start = (int)(4 + start + dir) % 4;
                    switch(start)
                    {
                    case 0: // left
                        newx = 0;
                        if (dir == 1)
                        {
                            newy = 0;
                            new_screenx = screenx + screenxincr[3];
                        }
                        else
                        {
                            newy = height - 1;
                            new_screenx = screenx - screenxincr[1];
                        }
                        break;
                    case 1: // top
                        newy = height - 1;
                        if (dir == 1)
                        {
                            newx = 0;
                        }
                        else
                        {
                            newx = width - 1;
                        }
                        break;
                    case 2: // right
                        newx = width - 1;
                        if (dir == 1)
                        {
                            newy = height - 1;
                            new_screenx = screenx + screenxincr[1];
                        }
                        else
                        {
                            newy = 0;
                            new_screenx = screenx - screenxincr[3];
                        }
                        break;
                    case 3: // bottom
                        newy = 0;
                        if (dir == 1)
                        {
                            newx = width - 1;
                        }
                        else
                        {
                            newx = 0;
                        }
                        break;
                    default:
                        break;
                    }
                }
                else
                {
                    newx = x + xincr[start] * dir;
                    newy = y + yincr[start] * dir;
                    new_screenx = screenx + (screenxincr[start] * dir);
                }
            }
            x = newx;
            y = newy;
            screenx = new_screenx;
        }
    }
}

static const char* TOP_BOT_LEFT_RIGHT_VALUES[] = { 
        "Top Left",
        "Top Right",
        "Bottom Left",
        "Bottom Right"
};
static wxPGChoices TOP_BOT_LEFT_RIGHT(wxArrayString(4, TOP_BOT_LEFT_RIGHT_VALUES));

static const char* CLOCKWISE_ANTI_VALUES[] = {
        "Clockwise",
        "Counter Clockwise"
};
static wxPGChoices CLOCKWISE_ANTI(wxArrayString(2, CLOCKWISE_ANTI_VALUES));

void WindowFrameModel::AddTypeProperties(wxPropertyGridInterface *grid) {
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

    grid->Append(new wxEnumProperty("Direction", "WFDirection", CLOCKWISE_ANTI, rotation));
}

int WindowFrameModel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("WFTopCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm1");
        ModelXml->AddAttribute("parm1", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "WindowFrameModel::OnPropertyGridChange::WFTopCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "WindowFrameModel::OnPropertyGridChange::WFTopCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "WindowFrameModel::OnPropertyGridChange::WFTopCount");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "WindowFrameModel::OnPropertyGridChange::WFTopCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "WindowFrameModel::OnPropertyGridChange::WFTopCount");
        return 0;
    } else if ("WFLeftRightCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "WindowFrameModel::OnPropertyGridChange::WFLeftRightCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "WindowFrameModel::OnPropertyGridChange::WFLeftRightCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "WindowFrameModel::OnPropertyGridChange::WFLeftRightCount");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "WindowFrameModel::OnPropertyGridChange::WFLeftRightCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "WindowFrameModel::OnPropertyGridChange::WFLeftRightCount");
        return 0;
    } else if ("WFBottomCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm3");
        ModelXml->AddAttribute("parm3", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "WindowFrameModel::OnPropertyGridChange::WFBottomCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "WindowFrameModel::OnPropertyGridChange::WFBottomCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "WindowFrameModel::OnPropertyGridChange::WFBottomCount");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "WindowFrameModel::OnPropertyGridChange::WFBottomCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "WindowFrameModel::OnPropertyGridChange::WFBottomCount");
        return 0;
    } else if ("WFStartLocation" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("StartSide");
        ModelXml->DeleteAttribute("Dir");
        ModelXml->AddAttribute("Dir", (event.GetValue().GetLong() == 0 || event.GetValue().GetLong() == 2) ? "L" : "R");
        ModelXml->AddAttribute("StartSide", (event.GetValue().GetLong() == 0 || event.GetValue().GetLong() == 1) ? "T" : "B");
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "WindowFrameModel::OnPropertyGridChange::WFStartLocation");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "WindowFrameModel::OnPropertyGridChange::WFStartLocation");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "WindowFrameModel::OnPropertyGridChange::WFStartLocation");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "WindowFrameModel::OnPropertyGridChange::WFStartLocation");
        return 0;
    } else if ("WFDirection" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("Rotation");
        ModelXml->AddAttribute("Rotation", event.GetValue().GetLong() == 0 ? "Clockwise" : "Counter Clockwise");
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "WindowFrameModel::OnPropertyGridChange::WFDirection");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "WindowFrameModel::OnPropertyGridChange::WFDirection");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "WindowFrameModel::OnPropertyGridChange::WFDirection");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "WindowFrameModel::OnPropertyGridChange::WFDirection");
        return 0;
    }

    return Model::OnPropertyGridChange(grid, event);
}