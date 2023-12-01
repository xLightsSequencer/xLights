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
#include <wx/log.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>

#include "MatrixModel.h"
#include "ModelScreenLocation.h"
#include "../xLightsVersion.h"
#include "../xLightsMain.h"
#include "UtilFunctions.h"
#include "../ModelPreview.h"

#include <log4cpp/Category.hh>

MatrixModel::MatrixModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased) : ModelWithScreenLocation(manager)
{
    SetFromXml(node, zeroBased);
}

MatrixModel::MatrixModel(const ModelManager &manager) : ModelWithScreenLocation(manager)
{
    //ctor
}

MatrixModel::~MatrixModel()
{
    //dtor
}

static const char* TOP_BOT_LEFT_RIGHT_VALUES[] = { 
    "Top Left",
    "Top Right",
    "Bottom Left",
    "Bottom Right"
};
static wxPGChoices TOP_BOT_LEFT_RIGHT(wxArrayString(4, TOP_BOT_LEFT_RIGHT_VALUES));

static const char* MATRIX_STYLES_VALUES[] = {
    "Horizontal",
    "Vertical"
};
static wxPGChoices MATRIX_STYLES(wxArrayString(2, MATRIX_STYLES_VALUES));

void MatrixModel::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager)
{

    AddStyleProperties(grid);

    wxPGProperty *p = grid->Append(new wxUIntProperty("# Strings", "MatrixStringCount", parm1));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 10000);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("This is typically the number of connections from the prop to your controller. *This would also be the 'Height' of a Virtual Matrix.");

    if (SingleNode) {
        p = grid->Append(new wxUIntProperty("Lights/String", "MatrixLightCount", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");
    } else {
        p = grid->Append(new wxUIntProperty("Nodes/String", "MatrixLightCount", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");
        p->SetHelpString("This is typically the total number of pixels per #String. \n *This would also be the 'Width' of a Virtual Matrix.");
    }

    p = grid->Append(new wxUIntProperty("Strands/String", "MatrixStrandCount", parm3));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 2500);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("This is typically how many times the #String ZigZags.");
    if (parm2 % parm3 != 0) {
        p->SetBackgroundColour(*wxRED);
        p->SetHelpString("Strands/String must divide into Nodes/String evenly.");
    }
    else {
        p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
    }

    grid->Append(new wxEnumProperty("Starting Location", "MatrixStart", TOP_BOT_LEFT_RIGHT, IsLtoR ? (isBotToTop ? 2 : 0) : (isBotToTop ? 3 : 1)));
}

void MatrixModel::AddStyleProperties(wxPropertyGridInterface *grid) {
    grid->Append(new wxEnumProperty("Direction", "MatrixStyle", MATRIX_STYLES, vMatrix ? 1 : 0));
    wxPGProperty *p = grid->Append(new wxBoolProperty("Alternate Nodes", "AlternateNodes", _alternateNodes));
    p->SetEditor("CheckBox");
    p->Enable(_noZig == false);

    p = grid->Append(new wxBoolProperty("Don't Zig Zag", "NoZig", _noZig));
    p->SetEditor("CheckBox");
    p->Enable(_alternateNodes == false);
}

int MatrixModel::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event)
{
    if ("MatrixStyle" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DisplayAs");
        ModelXml->AddAttribute("DisplayAs", event.GetPropertyValue().GetLong() ? "Vert Matrix" : "Horiz Matrix");
        // AdjustStringProperties(grid, parm1);
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "MatrixModel::OnPropertyGridChange::MatrixStyle");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "MatrixModel::OnPropertyGridChange::MatrixStyle");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "MatrixModel::OnPropertyGridChange::MatrixStyle");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "MatrixModel::OnPropertyGridChange::MatrixStyle");
        return 0;
    } else if ("MatrixStringCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm1");
        ModelXml->AddAttribute("parm1", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        // AdjustStringProperties(grid, parm1);
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "MatrixModel::OnPropertyGridChange::MatrixStringCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "MatrixModel::OnPropertyGridChange::MatrixStringCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "MatrixModel::OnPropertyGridChange::MatrixStringCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "MatrixModel::OnPropertyGridChange::MatrixStringCount");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "MatrixModel::OnPropertyGridChange::MatrixStringCount");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "MatrixModel::OnPropertyGridChange::MatrixStringCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "MatrixModel::OnPropertyGridChange::MatrixStringCount");
        return 0;
    } else if ("MatrixLightCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "MatrixModel::OnPropertyGridChange::MatrixLightCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "MatrixModel::OnPropertyGridChange::MatrixLightCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "MatrixModel::OnPropertyGridChange::MatrixLightCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "MatrixModel::OnPropertyGridChange::MatrixLightCount");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "MatrixModel::OnPropertyGridChange::MatrixLightCount");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "MatrixModel::OnPropertyGridChange::MatrixLightCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "MatrixModel::OnPropertyGridChange::MatrixLightCount");
        return 0;
    } else if ("MatrixStrandCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm3");
        ModelXml->AddAttribute("parm3", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "MatrixModel::OnPropertyGridChange::MatrixStrandCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "MatrixModel::OnPropertyGridChange::MatrixStrandCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "MatrixModel::OnPropertyGridChange::MatrixStrandCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "MatrixModel::OnPropertyGridChange::MatrixStrandCount");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "MatrixModel::OnPropertyGridChange::MatrixStrandCount");
        return 0;
    } else if ("MatrixStart" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("Dir");
        ModelXml->AddAttribute("Dir", event.GetValue().GetLong() == 0 || event.GetValue().GetLong() == 2 ? "L" : "R");
        ModelXml->DeleteAttribute("StartSide");
        ModelXml->AddAttribute("StartSide", event.GetValue().GetLong() == 0 || event.GetValue().GetLong() == 1 ? "T" : "B");
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "MatrixModel::OnPropertyGridChange::MatrixStart");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "MatrixModel::OnPropertyGridChange::MatrixStart");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "MatrixModel::OnPropertyGridChange::MatrixStart");
        return 0;
    } else if (event.GetPropertyName() == "AlternateNodes") {
        ModelXml->DeleteAttribute("AlternateNodes");
        ModelXml->AddAttribute("AlternateNodes", event.GetPropertyValue().GetBool() ? "true" : "false");
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TreeModel::OnPropertyGridChange::AlternateNodes");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "TreeModel::OnPropertyGridChange::AlternateNodes");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "TreeModel::OnPropertyGridChange::AlternateNodes");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TreeModel::OnPropertyGridChange::AlternateNodes");
        grid->GetPropertyByName("NoZig")->Enable(event.GetPropertyValue().GetBool() == false);
        return 0;
    } else if (event.GetPropertyName() == "NoZig") {
        ModelXml->DeleteAttribute("NoZig");
        ModelXml->AddAttribute("NoZig", event.GetPropertyValue().GetBool() ? "true" : "false");
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TreeModel::OnPropertyGridChange::NoZig");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "TreeModel::OnPropertyGridChange::NoZig");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "TreeModel::OnPropertyGridChange::NoZig");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TreeModel::OnPropertyGridChange::NoZig");
        grid->GetPropertyByName("AlternateNodes")->Enable(event.GetPropertyValue().GetBool() == false);
        return 0;
    }

    return Model::OnPropertyGridChange(grid, event);
}

std::list<std::string> MatrixModel::CheckModelSettings()
{
   std::list<std::string> res;

   if (!StartsWith(StringType, "Single Color") && parm2 % parm3 != 0)
   {
       res.push_back(wxString::Format("    ERR: Model %s strands are not equally sized %d does not divide into string length %d evenly. As a result only %d of %d nodes are initialised.", GetName(), parm3, parm2, (int)GetNodeCount(), parm1 * parm2));
   }

   res.splice(res.end(), Model::CheckModelSettings());
   return res;
}

int MatrixModel::GetNumStrands() const {
    if (SingleChannel || SingleNode) {
        return parm1;
    }
    return parm1*parm3;
}

bool MatrixModel::ChangeStringCount(long count, std::string & message)
{
    if (count == parm1) {
        return true;
    }
    auto oldTotalPix = parm1 * parm2;
    auto oldTotalStands = parm1 * parm3;
    if (oldTotalPix % count != 0) {
        message = "Pixel Count (" + std::to_string(oldTotalPix) + ") is not divisible by " + std::to_string(count);
        return false;
    }
    if (oldTotalStands % count != 0) {
        message = "Stand Count (" + std::to_string(oldTotalStands) + ") is not divisible by " + std::to_string(count);
        return false;
    }

    auto nparm2 = oldTotalPix / count;
    auto nparm3 = oldTotalStands / count;

    ModelXml->DeleteAttribute("parm1");
    ModelXml->AddAttribute("parm1", wxString::Format("%d", (int)count));
    ModelXml->DeleteAttribute("parm2");
    ModelXml->AddAttribute("parm2", wxString::Format("%d", (int)nparm2));
    ModelXml->DeleteAttribute("parm3");
    ModelXml->AddAttribute("parm3", wxString::Format("%d", (int)nparm3));
    AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "MatrixModel::ChangeStringCount::MatrixStringCount");
    AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "MatrixModel::ChangeStringCount::MatrixStringCount");
    AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "MatrixModel::ChangeStringCount::MatrixStringCount");
    AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "MatrixModel::ChangeStringCount::MatrixStringCount");
    AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "MatrixModel::ChangeStringCount::MatrixStringCount");
    AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "MatrixModel::ChangeStringCount::MatrixStringCount");
    AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "MatrixModel::ChangeStringCount::MatrixStringCount");
    return true;
}

// We do this separately as derived models such as Tree assume BufX/Y is set as if it was not single channel
// This goes back and sets them to more appropriate values.
void MatrixModel::InitSingleChannelModel()
{
    // rework bufX/bufY for singleChannel
    if (SingleNode)
    {
        int NumStrands = parm1 * parm3;
        int PixelsPerStrand = parm2 / parm3;
        if (vMatrix)
        {
            SetBufferSize(SingleNode ? 1 : PixelsPerStrand, SingleNode ? parm1 : NumStrands);
            int x = 0;
            for (size_t n = 0; n < Nodes.size(); n++) {
                for (auto& c : Nodes[n]->Coords)
                {
                    c.bufX = IsLtoR ? x : NumStrands - x - 1;
                    c.bufY = 0;
                }
                x++;
            }
        }
        else
        {
            SetBufferSize(SingleNode ? parm1 : NumStrands, SingleNode ? 1 : PixelsPerStrand);
            int y = 0;
            for (size_t n = 0; n < Nodes.size(); n++) {
                for (auto& c : Nodes[n]->Coords)
                {
                    c.bufY = IsLtoR ? y : NumStrands - y - 1;
                    c.bufX = 0;
                }
                y++;
            }
        }
    }
}

void MatrixModel::InitModel() {
    _alternateNodes = (ModelXml->GetAttribute("AlternateNodes", "false") == "true");
    _noZig = (ModelXml->GetAttribute("NoZig", "false") == "true");
    if (DisplayAs == "Vert Matrix") {
        InitVMatrix();
    } else if (DisplayAs == "Horiz Matrix") {
        InitHMatrix();
    }
    InitSingleChannelModel();
    DisplayAs = "Matrix";
    screenLocation.RenderDp = 10.0f;  // give the bounding box a little depth
}

// initialize buffer coordinates
// parm1=NumStrings
// parm2=PixelsPerString
// parm3=StrandsPerString
void MatrixModel::InitVMatrix(int firstExportStrand)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    vMatrix = true;
    int stringnum, segmentnum;
    if (parm3 > parm2) {
        parm3 = parm2;
    }
    int NumStrands = parm1 * parm3;
    int PixelsPerStrand = parm2 / parm3;
    int PixelsPerString = PixelsPerStrand * parm3;
    SetBufferSize(ApplyLowDefinition(PixelsPerStrand), ApplyLowDefinition(NumStrands));
    SetNodeCount(parm1, PixelsPerString, rgbOrder);
    screenLocation.SetRenderSize(NumStrands, PixelsPerStrand, 2.0f);
    int chanPerNode = GetNodeChannelCount(StringType);

    // create output mapping
    if (SingleNode) {
        int x = 0;
        float sx = 0;
        for (size_t n = 0; n < Nodes.size(); n++) {
            Nodes[n]->ActChan = stringStartChan[n];
            float sy = 0;
            for (auto& c : Nodes[n]->Coords) {
                c.screenX = isBotToTop ? sx : (NumStrands * parm3) - sx - 1;
                c.screenX -= ((float)NumStrands - 1.0) / 2.0;
                c.screenY = sy - ((float)PixelsPerStrand - 1.0) / 2.0;
                c.screenZ = 0;
                sy++;
                if (sy >= PixelsPerStrand) {
                    sy = 0;
                    sx++;
                }
            }

            int y = 0;
            int yincr = 1;
            for (size_t c = 0; c < PixelsPerString; c++) {
                Nodes[n]->Coords[c].bufX = IsLtoR ? x : NumStrands - x - 1;
                Nodes[n]->Coords[c].bufY = y;
                y += yincr;
                if (y < 0 || y >= PixelsPerStrand) {
                    yincr = -yincr;
                    y += yincr;
                    x++;
                }
            }
        }
        GetModelScreenLocation().SetRenderSize(NumStrands, PixelsPerStrand, GetModelScreenLocation().GetRenderDp());

    } else {
        std::vector<int> strandStartChan;
        strandStartChan.clear();
        strandStartChan.resize(NumStrands);
        for (int x2 = 0; x2 < NumStrands; x2++) {
            stringnum = x2 / parm3;
            segmentnum = x2 % parm3;
            strandStartChan[x2] = stringStartChan[stringnum] + segmentnum * PixelsPerStrand * chanPerNode;
        }
        if (firstExportStrand > 0 && firstExportStrand < NumStrands) {
            int offset = strandStartChan[firstExportStrand];
            for (int x2 = 0; x2 < NumStrands; x2++) {
                strandStartChan[x2] = strandStartChan[x2] - offset;
                if (strandStartChan[x2] < 0) {
                    strandStartChan[x2] += (PixelsPerStrand * NumStrands * chanPerNode);
                }
            }
        }

        if (_lowDefFactor == 100 || !SupportsLowDefinitionRender() || !GetModelManager().GetXLightsFrame()->IsLowDefinitionRender()) {
            for (int x = 0; x < NumStrands; x++) {
                stringnum = x / parm3;
                segmentnum = x % parm3;
                for (int y = 0; y < PixelsPerStrand; y++) {
                    int idx = stringnum * PixelsPerString + segmentnum * PixelsPerStrand + y;
                    Nodes[idx]->ActChan = strandStartChan[x] + y * chanPerNode;
                    Nodes[idx]->Coords[0].bufX = IsLtoR ? x : NumStrands - x - 1;
                    Nodes[idx]->StringNum = stringnum;
                    if (_alternateNodes) {
                        if (isBotToTop) {
                            if (y + 1 <= (PixelsPerStrand + 1) / 2) {
                                Nodes[idx]->Coords[0].bufY = y * 2;
                            } else {
                                Nodes[idx]->Coords[0].bufY = ((PixelsPerStrand - (y + 1)) * 2 + 1);
                            }
                        } else {
                            if (y + 1 <= (PixelsPerStrand + 1) / 2) {
                                Nodes[idx]->Coords[0].bufY = (PixelsPerStrand - 1) - (y * 2);
                            } else {
                                Nodes[idx]->Coords[0].bufY = (PixelsPerStrand - 1) - ((PixelsPerStrand - (y + 1)) * 2 + 1);
                            }
                        }
                    } else {
                        if (_noZig)
                        {
                            Nodes[idx]->Coords[0].bufY = isBotToTop == true ? y : PixelsPerStrand - y - 1;
                        } else {
                            Nodes[idx]->Coords[0].bufY = isBotToTop == (segmentnum % 2 == 0) ? y : PixelsPerStrand - y - 1;
                        }
                    }
                }
            }
            CopyBufCoord2ScreenCoord();
        } else {
            logger_base.debug("Building low definition buffer at %d%%", _lowDefFactor);

            int xoffset = NumStrands / 2;
            int yoffset = PixelsPerStrand / 2;

            for (int x = 0; x < NumStrands; x++) {
                stringnum = x / parm3;
                segmentnum = x % parm3;
                for (int y = 0; y < PixelsPerStrand; y++) {
                    int idx = stringnum * PixelsPerString + segmentnum * PixelsPerStrand + y;
                    Nodes[idx]->ActChan = strandStartChan[x] + y * chanPerNode;
                    Nodes[idx]->Coords[0].bufX = IsLtoR ? x : NumStrands - x - 1;
                    Nodes[idx]->StringNum = stringnum;
                    if (_alternateNodes) {
                        if (isBotToTop) {
                            if (y + 1 <= (PixelsPerStrand + 1) / 2) {
                                Nodes[idx]->Coords[0].bufY = y * 2;
                            } else {
                                Nodes[idx]->Coords[0].bufY = ((PixelsPerStrand - (y + 1)) * 2 + 1);
                            }
                        } else {
                            if (y + 1 <= (PixelsPerStrand + 1) / 2) {
                                Nodes[idx]->Coords[0].bufY = (PixelsPerStrand - 1) - (y * 2);
                            } else {
                                Nodes[idx]->Coords[0].bufY = (PixelsPerStrand - 1) - ((PixelsPerStrand - (y + 1)) * 2 + 1);
                            }
                        }
                    } else {
                        if (_noZig)
                        {
                            Nodes[idx]->Coords[0].bufY = isBotToTop == true ? y : PixelsPerStrand - y - 1;
                        } else {
                            Nodes[idx]->Coords[0].bufY = isBotToTop == (segmentnum % 2 == 0) ? y : PixelsPerStrand - y - 1;
                        }
                    }
                    // before we adjust the buffer capture the screen coordinates
                    for (size_t c = 0; c < GetCoordCount(idx); c++) {
                        Nodes[idx]->Coords[c].screenX = Nodes[idx]->Coords[0].bufX - xoffset;
                        Nodes[idx]->Coords[c].screenY = Nodes[idx]->Coords[0].bufY - yoffset;
                    }
                    // now we need to adjust the buffer coords
                    Nodes[idx]->Coords[0].bufX = ApplyLowDefinition(Nodes[idx]->Coords[0].bufX);
                    Nodes[idx]->Coords[0].bufY = ApplyLowDefinition(Nodes[idx]->Coords[0].bufY);
                }
            }
        }
    }
}

// initialize buffer coordinates
// parm1=NumStrings
// parm2=PixelsPerString
// parm3=StrandsPerString
void MatrixModel::InitHMatrix() {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    vMatrix = false;
    int idx,stringnum,segmentnum,xincr;
    if (parm3 > parm2) {
        parm3 = parm2;
    }
    int NumStrands=parm1*parm3;
    int PixelsPerStrand=parm2/parm3;
    int PixelsPerString=PixelsPerStrand*parm3;
    SetBufferSize(ApplyLowDefinition(NumStrands), ApplyLowDefinition(PixelsPerStrand));
    SetNodeCount(parm1,PixelsPerString,rgbOrder);
    screenLocation.SetRenderSize(PixelsPerStrand, NumStrands, 2.0f);
    
    int chanPerNode = GetNodeChannelCount(StringType);

    // create output mapping
    if (SingleNode) {
        int y=0;
        float sy = 0;
        for (size_t n=0; n<Nodes.size(); n++) {
            Nodes[n]->ActChan = stringStartChan[n];
            float sx = 0;
            for (auto& c : Nodes[n]->Coords)
            {
                c.screenY = isBotToTop ? sy : (NumStrands * parm3) - sy - 1;
                c.screenY -= ((float)NumStrands-1.0) / 2.0;
                c.screenX = sx - ((float)PixelsPerStrand-1.0) / 2.0;
                c.screenZ = 0;
                sx++;
                if (sx >= PixelsPerStrand)
                {
                    sx = 0;
                    sy++;
                }
            }

            int x = 0;
            xincr = 1;
            for (size_t c = 0; c < PixelsPerString; c++) {
                Nodes[n]->Coords[c].bufX = x;
                Nodes[n]->Coords[c].bufY = isBotToTop ? y : NumStrands - y - 1;
                x += xincr;
                if (x < 0 || x >= PixelsPerStrand) {
                    xincr = -xincr;
                    x += xincr;
                    y++;
                }
            }
        }
        GetModelScreenLocation().SetRenderSize(PixelsPerStrand, NumStrands, GetModelScreenLocation().GetRenderDp());

    } else {
        if (_lowDefFactor == 100 || !SupportsLowDefinitionRender() || !GetModelManager().GetXLightsFrame()->IsLowDefinitionRender()) {
            for (int y = 0; y < NumStrands; y++) {
                stringnum = y / parm3;
                segmentnum = y % parm3;
                for (int x = 0; x < PixelsPerStrand; x++) {
                    idx = stringnum * PixelsPerString + segmentnum * PixelsPerStrand + x;
                    Nodes[idx]->ActChan = stringStartChan[stringnum] + segmentnum * PixelsPerStrand * chanPerNode + x * chanPerNode;

                    Nodes[idx]->Coords[0].bufY = isBotToTop ? y : NumStrands - y - 1;
                    Nodes[idx]->StringNum = stringnum;

                    if (_alternateNodes) {
                        if (IsLtoR) {
                            if (x + 1 <= (PixelsPerStrand + 1) / 2) {
                                Nodes[idx]->Coords[0].bufX = x * 2;
                            } else {
                                Nodes[idx]->Coords[0].bufX = ((PixelsPerStrand - (x + 1)) * 2 + 1);
                            }
                        } else {
                            if (x + 1 <= (PixelsPerStrand + 1) / 2) {
                                Nodes[idx]->Coords[0].bufX = (PixelsPerStrand - 1) - (x * 2);
                            } else {
                                Nodes[idx]->Coords[0].bufX = (PixelsPerStrand - 1) - ((PixelsPerStrand - (x + 1)) * 2 + 1);
                            }
                        }
                    } else {
                        if (_noZig)
                        {
                            Nodes[idx]->Coords[0].bufX = IsLtoR != true ? PixelsPerStrand - x - 1 : x;
                        } else {
                            Nodes[idx]->Coords[0].bufX = IsLtoR != (segmentnum % 2 == 0) ? PixelsPerStrand - x - 1 : x;
                        }
                    }
                }
            }
            CopyBufCoord2ScreenCoord();
        } else {
            logger_base.debug("Building low definition buffer at %d%%", _lowDefFactor);

            int xoffset = PixelsPerStrand / 2;
            int yoffset = NumStrands / 2;

            for (int y = 0; y < NumStrands; y++) {
                stringnum = y / parm3;
                segmentnum = y % parm3;
                for (int x = 0; x < PixelsPerStrand; x++) {
                    idx = stringnum * PixelsPerString + segmentnum * PixelsPerStrand + x;
                    Nodes[idx]->ActChan = stringStartChan[stringnum] + segmentnum * PixelsPerStrand * chanPerNode + x * chanPerNode;

                    Nodes[idx]->Coords[0].bufY = isBotToTop ? y : NumStrands - y - 1;
                    Nodes[idx]->StringNum = stringnum;

                    if (_alternateNodes) {
                        if (IsLtoR) {
                            if (x + 1 <= (PixelsPerStrand + 1) / 2) {
                                Nodes[idx]->Coords[0].bufX = x * 2;
                            } else {
                                Nodes[idx]->Coords[0].bufX = ((PixelsPerStrand - (x + 1)) * 2 + 1);
                            }
                        } else {
                            if (x + 1 <= (PixelsPerStrand + 1) / 2) {
                                Nodes[idx]->Coords[0].bufX = (PixelsPerStrand - 1) - (x * 2);
                            } else {
                                Nodes[idx]->Coords[0].bufX = (PixelsPerStrand - 1) - ((PixelsPerStrand - (x + 1)) * 2 + 1);
                            }
                        }
                    } else {
                        if (_noZig)
                        {
                            Nodes[idx]->Coords[0].bufX = IsLtoR != true ? PixelsPerStrand - x - 1 : x;
                        } else {
                            Nodes[idx]->Coords[0].bufX = IsLtoR != (segmentnum % 2 == 0) ? PixelsPerStrand - x - 1 : x;
                        }
                    }
                    // before we adjust the buffer capture the screen coordinates
                    for (size_t c = 0; c < GetCoordCount(idx); c++) {
                        Nodes[idx]->Coords[c].screenX = Nodes[idx]->Coords[0].bufX - xoffset;
                        Nodes[idx]->Coords[c].screenY = Nodes[idx]->Coords[0].bufY - yoffset;
                    }
                    // now we need to adjust the buffer coords
                    Nodes[idx]->Coords[0].bufX = ApplyLowDefinition(Nodes[idx]->Coords[0].bufX);
                    Nodes[idx]->Coords[0].bufY = ApplyLowDefinition(Nodes[idx]->Coords[0].bufY);
                }
            }
        }
    }
}

void MatrixModel::ExportXlightsModel()
{
    wxString name = ModelXml->GetAttribute("name");
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty()) return;
    wxFile f(filename);
    //    bool isnew = !FileExists(filename);
    if (!f.Create(filename, true) || !f.IsOpened()) DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());
    wxString p1 = ModelXml->GetAttribute("parm1");
    wxString p2 = ModelXml->GetAttribute("parm2");
    wxString p3 = ModelXml->GetAttribute("parm3");
    wxString st = ModelXml->GetAttribute("StringType");
    wxString ps = ModelXml->GetAttribute("PixelSize");
    wxString t = ModelXml->GetAttribute("Transparency", "0");
    wxString mb = ModelXml->GetAttribute("ModelBrightness", "0");
    wxString a = ModelXml->GetAttribute("Antialias");
    wxString ss = ModelXml->GetAttribute("StartSide");
    wxString dir = ModelXml->GetAttribute("Dir");
    wxString sn = ModelXml->GetAttribute("StrandNames");
    wxString nn = ModelXml->GetAttribute("NodeNames");
    wxString da = ModelXml->GetAttribute("DisplayAs");
    wxString an = ModelXml->GetAttribute("AlternateNodes", "false");
    wxString nz = ModelXml->GetAttribute("NoZig", "false");
    wxString ld = ModelXml->GetAttribute("LowDefinition", "100");
    wxString v = xlights_version_string;
    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<matrixmodel \n");
    f.Write(wxString::Format("name=\"%s\" ", name));
    f.Write(wxString::Format("parm1=\"%s\" ", p1));
    f.Write(wxString::Format("parm2=\"%s\" ", p2));
    f.Write(wxString::Format("parm3=\"%s\" ", p3));
    f.Write(wxString::Format("DisplayAs=\"%s\" ", da));
    f.Write(wxString::Format("StringType=\"%s\" ", st));
    f.Write(wxString::Format("Transparency=\"%s\" ", t));
    f.Write(wxString::Format("PixelSize=\"%s\" ", ps));
    f.Write(wxString::Format("ModelBrightness=\"%s\" ", mb));
    f.Write(wxString::Format("Antialias=\"%s\" ", a));
    f.Write(wxString::Format("StartSide=\"%s\" ", ss));
    f.Write(wxString::Format("Dir=\"%s\" ", dir));
    f.Write(wxString::Format("StrandNames=\"%s\" ", sn));
    f.Write(wxString::Format("NodeNames=\"%s\" ", nn));
    f.Write(wxString::Format("AlternateNodes=\"%s\" ", an));
    f.Write(wxString::Format("NoZig=\"%s\" ", nz));
    f.Write(wxString::Format("SourceVersion=\"%s\" ", v));
    f.Write(wxString::Format("LowDefinition=\"%s\" ", ld));
    f.Write(ExportSuperStringColors());
    f.Write(" >\n");
    wxString state = SerialiseState();
    if (state != "")
    {
        f.Write(state);
    }
    wxString face = SerialiseFace();
    if (face != "")
    {
        f.Write(face);
    }
    wxString submodel = SerialiseSubmodel();
    if (submodel != "")
    {
        f.Write(submodel);
    }
    wxString groups = SerialiseGroups();
    if (groups != "") {
        f.Write(groups);
    }
    wxString connection = SerialiseConnection();
    if (connection != "") {
        f.Write(connection);
    }
    ExportDimensions(f);
    f.Write("</matrixmodel>");
    f.Close();
}

void MatrixModel::ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
    if (root->GetName() == "matrixmodel") {
        wxString name = root->GetAttribute("name");
        wxString p1 = root->GetAttribute("parm1");
        wxString p2 = root->GetAttribute("parm2");
        wxString p3 = root->GetAttribute("parm3");
        wxString st = root->GetAttribute("StringType");
        wxString ps = root->GetAttribute("PixelSize");
        wxString t = root->GetAttribute("Transparency", "0");
        wxString mb = root->GetAttribute("ModelBrightness", "0");
        wxString a = root->GetAttribute("Antialias");
        wxString ss = root->GetAttribute("StartSide");
        wxString dir = root->GetAttribute("Dir");
        wxString sn = root->GetAttribute("StrandNames");
        wxString nn = root->GetAttribute("NodeNames");
        wxString v = root->GetAttribute("SourceVersion");
        wxString da = root->GetAttribute("DisplayAs");
        wxString pc = root->GetAttribute("PixelCount");
        wxString pt = root->GetAttribute("PixelType");
        wxString psp = root->GetAttribute("PixelSpacing");
        wxString an = root->GetAttribute("AlternateNodes");
        wxString nz = root->GetAttribute("NoZig");
        wxString ld = root->GetAttribute("LowDefinition", "100");

        // generally xmodels dont have these ... but there are some cases where we do where it would point to a shadow model ... in those cases we want to bring it in
        wxString smf = root->GetAttribute("ShadowModelFor");
        wxString sc = root->GetAttribute("StartChannel");

        // Add any model version conversion logic here
        // Source version will be the program version that created the custom model

        SetProperty("parm1", p1);
        SetProperty("parm2", p2);
        SetProperty("parm3", p3);
        SetProperty("StringType", st);
        SetProperty("PixelSize", ps);
        SetProperty("Transparency", t);
        SetProperty("ModelBrightness", mb);
        SetProperty("Antialias", a);
        SetProperty("StartSide", ss);
        SetProperty("Dir", dir);
        SetProperty("StrandNames", sn);
        SetProperty("NodeNames", nn);
        SetProperty("DisplayAs", da);
        SetProperty("PixelCount", pc);
        SetProperty("PixelType", pt);
        SetProperty("PixelSpacing", psp);
        SetProperty("AlternateNodes", an);
        SetProperty("NoZig", nz);
        SetProperty("LowDefinition", ld);
        if (smf != "") {
            SetProperty("ShadowModelFor", smf);
        }
        if (sc != "") {
            SetControllerName("Use Start Channel");
            SetProperty("StartChannel", sc);
        }

        wxString newname = xlights->AllModels.GenerateModelName(name.ToStdString());
        GetModelScreenLocation().Write(ModelXml);
        SetProperty("name", newname, true);

        ImportSuperStringColours(root);
        ImportModelChildren(root, xlights, newname, min_x, max_x, min_y, max_y);

        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "MatrixModel::ImportXlightsModel");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "MatrixModel::ImportXlightsModel");
    } else {
        DisplayError("Failure loading Matrix model file.");
    }
}

