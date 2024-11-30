/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>
#include <wx/log.h>
#include <wx/filedlg.h>

#include "CircleModel.h"
#include "ModelScreenLocation.h"
#include "../OutputModelManager.h"
#include "xLightsVersion.h"
#include "../xLightsMain.h"
#include "../UtilFunctions.h"
#include "../ModelPreview.h"

CircleModel::CircleModel(wxXmlNode* node, const ModelManager& manager, bool zeroBased) : ModelWithScreenLocation(manager), insideOut(false)
{
    // convert old circle sizes to new Layer sizes setting - this also reverses the order
    if (node->GetAttribute("circleSizes", "") != "") {
        node->DeleteAttribute("LayerSizes");
        node->AddAttribute("LayerSizes", ReverseCSV(node->GetAttribute("circleSizes", "")));
        node->DeleteAttribute("circleSizes");
    }

    SetFromXml(node, zeroBased);
}

CircleModel::~CircleModel()
{
}

int CircleModel::GetStrandLength(int strand) const {
    return SingleNode ? 1 : GetLayerSize(GetLayerSizeCount() - strand - 1);
}

int CircleModel::MapToNodeIndex(int strand, int node) const {
    int idx = 0;
    for (int x = 0; x < strand; x++) {
        idx += GetStrandLength(x);
    }
    idx += node;
    return idx;
}

int CircleModel::GetNumStrands() const {
    return GetLayerSizeCount();
}

bool CircleModel::AllNodesAllocated() const
{
    int allocated = 0;
    for (auto it : GetLayerSizes()) {
        allocated += it;
    }

    return (allocated == GetNodeCount());
}

// parm3 is number of points
// top left=top ccw, top right=top cw, bottom left=bottom cw, bottom right=bottom ccw
void CircleModel::InitModel()
{
    if (!ModelXml->HasAttribute("StartSide")) {
        isBotToTop = false;
    }
    if (GetLayerSizeCount() == 0) {
        SetLayerSizeCount(1);
    }
    if (GetLayerSizeCount() == 1) {
        SetLayerSize(0, parm1 * parm2);
    }

    if (ModelXml->HasAttribute("InsideOut")) {
        insideOut = wxAtoi(ModelXml->GetAttribute("InsideOut"));
    }
    InitCircle();
    SetCircleCoord();
    screenLocation.RenderDp = 10.0f;  // give the bounding box a little depth
}

static const char* CIRCLE_START_LOCATION_VALUES[] = { 
        "Top Outer-CCW",
        "Top Outer-CW",
        "Top Inner-CCW",
        "Top Inner-CW",
        "Bottom Outer-CCW",
        "Bottom Outer-CW",
        "Bottom Inner-CCW",
        "Bottom Inner-CW"
};

static wxPGChoices CIRCLE_START_LOCATION(wxArrayString(8, CIRCLE_START_LOCATION_VALUES));

void CircleModel::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager)
{

    wxPGProperty *p = grid->Append(new wxUIntProperty("# Strings", "CircleStringCount", parm1));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("This is typically the number of connections from the prop to your controller.");

    if (SingleNode) {
        p = grid->Append(new wxUIntProperty("Lights/String", "CircleLightCount", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 2000);
        p->SetEditor("SpinCtrl");
    }
    else {
        p = grid->Append(new wxUIntProperty("Nodes/String", "CircleLightCount", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 2000);
        p->SetEditor("SpinCtrl");
        p->SetHelpString("This is typically the total number of pixels per #String.");
    }

    p = grid->Append(new wxUIntProperty("Center %", "CircleCenterPercent", parm3));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    AddLayerSizeProperty(grid);

    int start = IsLtoR ? 1 : 0;
    if (insideOut) {
        start += 2;
    }
    if (isBotToTop) {
        start += 4;
    }
    grid->Append(new wxEnumProperty("Starting Location", "CircleStart", CIRCLE_START_LOCATION, start));
}

int CircleModel::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event)
{
    if ("CircleStringCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm1");
        ModelXml->AddAttribute("parm1", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        //AdjustStringProperties(grid, parm1);
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CircleModel::OnPropertyGridChange::CircleStringCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "CircleModel::OnPropertyGridChange::CircleStringCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CircleModel::OnPropertyGridChange::CircleStringCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "CircleModel::OnPropertyGridChange::CircleStringCount");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "CircleModel::OnPropertyGridChange::CircleStringCount");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "CircleModel::OnPropertyGridChange::CircleStringCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "CircleModel::OnPropertyGridChange::CircleStringCount");
        return 0;
    }
    else if ("CircleLightCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CircleModel::OnPropertyGridChange::CircleLightCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "CircleModel::OnPropertyGridChange::CircleLightCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CircleModel::OnPropertyGridChange::CircleLightCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "CircleModel::OnPropertyGridChange::CircleLightCount");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "CircleModel::OnPropertyGridChange::CircleLightCount");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "CircleModel::OnPropertyGridChange::CircleLightCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "CircleModel::OnPropertyGridChange::CircleLightCount");
        return 0;
    }
    else if ("CircleCenterPercent" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm3");
        ModelXml->AddAttribute("parm3", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CircleModel::OnPropertyGridChange::CircleCenterPercent");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "CircleModel::OnPropertyGridChange::CircleCenterPercent");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CircleModel::OnPropertyGridChange::CircleCenterPercent");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "CircleModel::OnPropertyGridChange::CircleCenterPercent");
        return 0;
    }
    else if ("CircleStart" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("Dir");
        ModelXml->DeleteAttribute("StartSide");
        ModelXml->DeleteAttribute("InsideOut");

        int v = event.GetValue().GetLong();
        ModelXml->AddAttribute("Dir", v & 0x1 ? "L" : "R");
        ModelXml->AddAttribute("StartSide", v < 4 ? "T" : "B");
        ModelXml->AddAttribute("InsideOut", v & 0x2 ? "1" : "0");

        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CircleModel::OnPropertyGridChange::CircleStart");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "CircleModel::OnPropertyGridChange::CircleStart");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CircleModel::OnPropertyGridChange::CircleStart");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "CircleModel::OnPropertyGridChange::CircleLayerSizes");
        return 0;
    }

    return Model::OnPropertyGridChange(grid, event);
}

int CircleModel::maxSize() {
    int maxLights = 0;
    for (const auto x : GetLayerSizes()) {
        if (x > maxLights) {
            maxLights = x;
        }
    }
    return maxLights;
}

void CircleModel::InitCircle()
{
    int maxLights = 0;
    int numLights = parm1 * parm2;
    int cnt = 0;

    if (GetLayerSizeCount() == 0) {
        SetLayerSizeCount(1);
        SetLayerSize(0, numLights);
    }

    for (int x = 0; x < GetLayerSizeCount(); x++) {
        if ((cnt + GetLayerSize(x)) > numLights) {
            if (cnt > numLights) {
                SetLayerSize(x, 0);
            }
            else {
                SetLayerSize(x, numLights - cnt);
            }
        }
        cnt += GetLayerSize(x);
        if (GetLayerSize(x) > maxLights) {
            maxLights = GetLayerSize(x);
        }
    }

    SetNodeCount(parm1, parm2, rgbOrder);
    SetBufferSize(GetLayerSizeCount(), maxLights);
    int LastStringNum = -1;
    int chan = 0;
    int ChanIncr = GetNodeChannelCount(StringType);
    size_t NodeCount = GetNodeCount();

    size_t node = 0;
    int nodesToMap = NodeCount;
    for (int circle = 0; circle < GetLayerSizeCount(); circle++) {
        int idx = 0;
        auto strandLen = GetStrandLength(circle);
        int loop_count = std::min(nodesToMap, strandLen);
        // if the number of nodes in this layer is exactly divisible into the maximum loops then we dont fudge ... but if it isnt then we want to fudge things slightly 
        // so that the largest layer goes to the end but any layer not divisible does not use the last x value
        double fudge = -1 * maxLights / strandLen + 1;
        for (size_t n = 0; n < loop_count; n++) {
            if (Nodes[node]->StringNum != LastStringNum) {
                LastStringNum = Nodes[node]->StringNum;
                chan = stringStartChan[LastStringNum];
            }
            Nodes[node]->ActChan = chan;
            chan += ChanIncr;
            double pct = (loop_count == 1) ? (double)n : (double)n / (double)(loop_count - 1);
            size_t CoordCount = GetCoordCount(node);
            for (size_t c = 0; c < CoordCount; c++) {
                if (loop_count == 1) {
                    Nodes[node]->Coords[c].bufX = idx;
                    Nodes[node]->Coords[c].bufY = insideOut ? GetLayerSizeCount() - circle - 1 : circle;
                }
                else {
                    int x_pos = (GetStrandLength(circle) == maxLights) ? idx : std::floor(pct * ((double)maxLights - 1.0 + fudge));
                    Nodes[node]->Coords[c].bufX = x_pos;
                    Nodes[node]->Coords[c].bufY = insideOut? GetLayerSizeCount() - circle - 1 : circle;
                    idx++;
                }
            }
            node++;
        }
        nodesToMap -= loop_count;
    }
}

// Set screen coordinates for circles
void CircleModel::SetCircleCoord()
{
    size_t NodeCount = GetNodeCount();
    int maxLights = maxSize();
    screenLocation.SetRenderSize(maxLights, maxLights);
    int nodesToMap = NodeCount;
    int node = 0;
    double maxRadius = maxLights / 2.0;
    double minRadius = (double)parm3 / 100.0 * maxRadius;
    for (int c2 = 0; c2 < GetLayerSizeCount(); c2++) {
        int circle = c2;
        int loop_count = std::min(nodesToMap, GetStrandLength(circle));
        double radius = (GetLayerSizeCount() == 1) ? maxRadius : insideOut ? (double)minRadius + (maxRadius - minRadius) * (1.0 - (double)(GetLayerSizeCount() - circle - 1) / (double)(GetLayerSizeCount() - 1)) : (double)minRadius + (maxRadius - minRadius) * (1.0 - (double)circle / (double)(GetLayerSizeCount() - 1));
        for (size_t n = 0; n < loop_count; n++) {
            size_t CoordCount = GetCoordCount(node);
            for (size_t c = 0; c < CoordCount; c++) {
                double angle = (isBotToTop ? -M_PI : 0) + M_PI * ((loop_count == 1) ? (double)c / (double)CoordCount : (double)n / (double)loop_count) * 2.0;
                if (!IsLtoR) {
                    angle *= -1;
                }
                double x = sin(angle) * radius;
                double y = cos(angle) * radius;
                Nodes[node]->Coords[c].screenX = x;
                Nodes[node]->Coords[c].screenY = y;
            }
            node++;
        }
        nodesToMap -= loop_count;
    }
}

void CircleModel::ExportXlightsModel()
{
    wxString name = ModelXml->GetAttribute("name");
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty()) return;
    wxFile f(filename);
    
    if (!f.Create(filename, true) || !f.IsOpened()) {
        DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());
        return;
    }

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
    wxString ls = ModelXml->GetAttribute("LayerSizes");
    wxString io = ModelXml->GetAttribute("InsideOut");
    wxString da = ModelXml->GetAttribute("DisplayAs");


    wxString v = xlights_version_string;
    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<circlemodel \n");
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
    f.Write(wxString::Format("LayerSizes=\"%s\" ", ls));
    f.Write(wxString::Format("InsideOut=\"%s\" ", io));
    f.Write(wxString::Format("SourceVersion=\"%s\" ", v));
    f.Write(ExportSuperStringColors());
    f.Write(" >\n");
    wxString aliases = SerialiseAliases();
    if (aliases != "") {
        f.Write(aliases);
    }
    wxString state = SerialiseState();
    if (state != "") {
        f.Write(state);
    }
    wxString submodel = SerialiseSubmodel();
    if (submodel != "") {
        f.Write(submodel);
    }
    wxString groups = SerialiseGroups();
    if (groups != "") {
        f.Write(groups);
    }
    ExportDimensions(f);
    f.Write("</circlemodel>");
    f.Close();
}

bool CircleModel::ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
    if (root->GetName() == "circlemodel") {
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
        wxString ls = root->GetAttribute("circleSizes");
        if (ls == "") {
            ls = root->GetAttribute("LayerSizes");
        } else {
            ls = ReverseCSV(ls);
        }
        wxString io = root->GetAttribute("InsideOut");
        //wxString v = root->GetAttribute("SourceVersion");
        wxString da = root->GetAttribute("DisplayAs");
        wxString pc = root->GetAttribute("PixelCount");
        wxString pt = root->GetAttribute("PixelType");
        wxString psp = root->GetAttribute("PixelSpacing");

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
        SetProperty("LayerSizes", ls);
        SetProperty("InsideOut", io);
        SetProperty("DisplayAs", da);
        SetProperty("PixelCount", pc);
        SetProperty("PixelType", pt);
        SetProperty("PixelSpacing", psp);

        wxString newname = xlights->AllModels.GenerateModelName(name.ToStdString());
        GetModelScreenLocation().Write(ModelXml);
        SetProperty("name", newname, true);

        ImportSuperStringColours(root);
        ImportModelChildren(root, xlights, newname, min_x, max_x, min_y, max_y);

        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CircleModel::ImportXlightsModel");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "CircleModel::ImportXlightsModel");

        return true;
    } else {
        DisplayError("Failure loading Circle model file.");
        return false;
    }
}

void CircleModel::OnLayerSizesChange(bool countChanged)
{
    // if string count is 1 then adjust nodes per string to match sum of nodes
    if (parm1 == 1 && GetLayerSizeCount() > 0) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%d", (int)GetLayerSizesTotalNodes()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CircleModel::OnLayerSizesChange");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "CircleModel::OnLayerSizesChange");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CircleModel::OnLayerSizesChange");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "CircleModel::OnLayerSizesChange");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "CircleModel::OnLayerSizesChange");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "CircleModel::OnLayerSizesChange");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "CircleModel::OnLayerSizesChange");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "CircleModel::OnLayerSizesChange");
    }
}
