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
#include <wx/msgdlg.h>
#include <wx/log.h>
#include <wx/filedlg.h>

#include "ArchesModel.h"
#include "ModelScreenLocation.h"
#include "xLightsVersion.h"
#include "../xLightsMain.h"
#include "UtilFunctions.h"
#include "../ModelPreview.h"
#include "../ModelPreview.h"

ArchesModel::ArchesModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased) : ModelWithScreenLocation(manager), arc(180)
{
    screenLocation.SetModelHandleHeight(true);
    screenLocation.SetSupportsAngle(true);
    screenLocation.SetPreferredSelectionPlane(ModelScreenLocation::MSLPLANE::GROUND);
    SetFromXml(node, zeroBased);
}

ArchesModel::~ArchesModel()
{
}


static const char *LEFT_RIGHT_VALUES[] = {
    "Green Square", 
    "Blue Square"
};
static wxPGChoices LEFT_RIGHT(wxArrayString(2, LEFT_RIGHT_VALUES));

static const char* LEFT_RIGHT_INSIDE_OUTSIDE_VALUES[] = {
    "Green Square Inside",
    "Green Square Outside",
    "Blue Square Inside",
    "Blue Square Outside"
};
static wxPGChoices LEFT_RIGHT_INSIDE_OUTSIDE(wxArrayString(4, LEFT_RIGHT_INSIDE_OUTSIDE_VALUES));

void ArchesModel::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager)
{
    wxPGProperty* p = grid->Append(new wxBoolProperty("Layered Arches", "LayeredArches", GetLayerSizeCount() != 0));
    p->SetEditor("CheckBox");

    if (GetLayerSizeCount() == 0) {
        p = grid->Append(new wxUIntProperty("# Arches", "ArchesCount", parm1));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 100);
        p->SetEditor("SpinCtrl");

        p = grid->Append(new wxUIntProperty("Nodes Per Arch", "ArchesNodes", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 1000);
        p->SetEditor("SpinCtrl");
    } else {
        p = grid->Append(new wxUIntProperty("Nodes", "ArchesNodes", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");

        AddLayerSizeProperty(grid);

        p = grid->Append(new wxUIntProperty("Hollow %", "Hollow", _hollow));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 95);
        p->SetEditor("SpinCtrl");
    }

    p = grid->Append(new wxUIntProperty("Lights Per Node", "ArchesLights", parm3));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 250);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Arc Degrees", "ArchesArc", arc));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 180);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxIntProperty("Arch Tilt", "ArchesSkew", screenLocation.GetAngle()));
    p->SetAttribute("Min", -180);
    p->SetAttribute("Max", 180);
    p->SetEditor("SpinCtrl");

    if (GetLayerSizeCount() == 0) {
        p = grid->Append(new wxIntProperty("Gap Between Arches", "ArchesGap", _gap));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 500);
        p->SetEditor("SpinCtrl");
    }

    if (GetLayerSizeCount() != 0) {
        grid->Append(new wxEnumProperty("Starting Location", "ArchesStart", LEFT_RIGHT_INSIDE_OUTSIDE, (IsLtoR ? 0 : 2) + (isBotToTop ? 1 : 0)));
    } else {
        grid->Append(new wxEnumProperty("Starting Location", "ArchesStart", LEFT_RIGHT, IsLtoR ? 0 : 1));
    }
}

int ArchesModel::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event)
{
    if ("ArchesCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm1");
        ModelXml->AddAttribute("parm1", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        //AdjustStringProperties(grid, parm1);
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ArchesModel::OnPropertyGridChange::ArchesCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ArchesModel::OnPropertyGridChange::ArchesCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "ArchesModel::OnPropertyGridChange::ArchesCount");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ArchesModel::OnPropertyGridChange::ArchesCount");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ArchesModel::OnPropertyGridChange::ArchesCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "ArchesModel::OnPropertyGridChange::ArchesCount");
        AddASAPWork(OutputModelManager::WORK_UPDATE_PROPERTYGRID, "ArchesModel::OnPropertyGridChange::ArchesCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ArchesModel::OnPropertyGridChange::ArchesCount");
        return 0;
    } else if ("ArchesNodes" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ArchesModel::OnPropertyGridChange::ArchesNodes");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ArchesModel::OnPropertyGridChange::ArchesNodes");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ArchesModel::OnPropertyGridChange::ArchesNodes");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "ArchesModel::OnPropertyGridChange::ArchesNodes");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "ArchesModel::OnPropertyGridChange::ArchesNodes");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ArchesModel::OnPropertyGridChange::ArchesNodes");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "ArchesModel::OnPropertyGridChange::SingleLineNodes");
        return 0;
    } else if ("ArchesLights" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm3");
        ModelXml->AddAttribute("parm3", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ArchesModel::OnPropertyGridChange::ArchesLights");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ArchesModel::OnPropertyGridChange::ArchesLights");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "ArchesModel::OnPropertyGridChange::ArchesLights");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ArchesModel::OnPropertyGridChange::ArchesLights");
        return 0;
    } else if ("ArchesArc" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("arc");
        ModelXml->AddAttribute("arc", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ArchesModel::OnPropertyGridChange::ArchesArc");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ArchesModel::OnPropertyGridChange::ArchesArc");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "ArchesModel::OnPropertyGridChange::ArchesArc");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ArchesModel::OnPropertyGridChange::ArchesArc");
        return 0;
    } else if ("ArchesSkew" == event.GetPropertyName()) {
        screenLocation.SetAngle(event.GetPropertyValue().GetLong());
        ModelXml->DeleteAttribute("Angle");
        ModelXml->AddAttribute("Angle", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ArchesModel::OnPropertyGridChange::ArchesSkew");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ArchesModel::OnPropertyGridChange::ArchesSkew");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "ArchesModel::OnPropertyGridChange::ArchesSkew");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ArchesModel::OnPropertyGridChange::ArchesSkew");
        return 0;
    } else if ("LayeredArches" == event.GetPropertyName()) {
        if (event.GetPropertyValue().GetBool()) {
            parm1 = 1;
            SetLayerSizeCount(1);
            SetLayerSize(0, parm2);
        } else {
            SetLayerSizeCount(0);
        }

        ModelXml->DeleteAttribute("LayerSizes");
        ModelXml->AddAttribute("LayerSizes", SerialiseLayerSizes());
        OnLayerSizesChange(true);

        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ArchesModel::HandleLayerSizePropertyChange::Layers");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ArchesModel::HandleLayerSizePropertyChange::LayeredArches");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "ArchesModel::HandleLayerSizePropertyChange::LayeredArches");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ArchesModel::HandleLayerSizePropertyChange::LayeredArches");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ArchesModel::HandleLayerSizePropertyChange::LayeredArches");
        return 0;
    } else if ("Hollow" == event.GetPropertyName()) {
        _hollow = event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("Hollow");
        ModelXml->AddAttribute("Hollow", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ArchesModel::OnPropertyGridChange::ArchesGap");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ArchesModel::OnPropertyGridChange::ArchesGap");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "ArchesModel::OnPropertyGridChange::ArchesGap");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ArchesModel::OnPropertyGridChange::ArchesGap");
        return 0;
    } else if ("ArchesGap" == event.GetPropertyName()) {
        _gap = event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("Gap");
        ModelXml->AddAttribute("Gap", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ArchesModel::OnPropertyGridChange::ArchesGap");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ArchesModel::OnPropertyGridChange::ArchesGap");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "ArchesModel::OnPropertyGridChange::ArchesGap");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ArchesModel::OnPropertyGridChange::ArchesGap");
        return 0;
    } else if ("ArchesStart" == event.GetPropertyName()) {
        int value = event.GetValue().GetLong();
        if (GetLayerSizeCount() != 0) {
            ModelXml->DeleteAttribute("Dir");
            ModelXml->AddAttribute("Dir", (value == 0 || value == 1) ? "L" : "R");
            ModelXml->DeleteAttribute("StartSide");
            ModelXml->AddAttribute("StartSide", (value == 0 || value == 2) ? "T" : "B");
        } else {
            ModelXml->DeleteAttribute("Dir");
            ModelXml->AddAttribute("Dir", value == 0 ? "L" : "R");
        }
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ArchesModel::OnPropertyGridChange::ArchesStart");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ArchesModel::OnPropertyGridChange::ArchesStart");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "ArchesModel::OnPropertyGridChange::ArchesStart");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ArchesModel::OnPropertyGridChange::ArchesStart");
        return 0;
    }

    return Model::OnPropertyGridChange(grid, event);
}

void ArchesModel::GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform, int &BufferWi, int &BufferHi, int stagger) const {
    if (type == "Single Line") {
        BufferHi = 1;
        BufferWi = this->BufferWi * this->BufferHt;
        AdjustForTransform(transform, BufferWi, BufferHi);
    } else {
        Model::GetBufferSize(type, camera, transform, BufferWi, BufferHi, stagger);
    }
}
void ArchesModel::InitRenderBufferNodes(const std::string& type, const std::string& camera, const std::string& transform,
    std::vector<NodeBaseClassPtr>& newNodes, int& BufferWi, int& BufferHi, int stagger, bool deep) const
{
    if (type == "Single Line") {
        BufferHi = 1;
        BufferWi = GetNodeCount();

        int cur = 0;

        for (int x = 0; x < Nodes.size(); x++) {
            newNodes.push_back(NodeBaseClassPtr(Nodes[x]->clone()));
            for (size_t c = 0; c < newNodes[cur]->Coords.size(); c++) {
                newNodes[cur]->Coords[c].bufX = cur;
                newNodes[cur]->Coords[c].bufY = 0;
            }
            cur++;
        }
        ApplyTransform(transform, newNodes, BufferWi, BufferHi);
    } else {
        Model::InitRenderBufferNodes(type, camera, transform, newNodes, BufferWi, BufferHi, stagger);
    }
}

bool ArchesModel::IsNodeFirst(int n) const 
{
    if (GetLayerSizeCount() == 0) {
        return (GetIsLtoR() && n == 0) || (!GetIsLtoR() && n == Nodes.size() - 1);
    }
    else {
        return n == 0;
    }
}

void ArchesModel::InitModel()
{
    arc = wxAtoi(ModelXml->GetAttribute("arc", "180"));
    _hollow = wxAtoi(ModelXml->GetAttribute("Hollow", "70"));

    if (ModelXml->HasAttribute("ArchesSkew")) {
        ModelXml->DeleteAttribute("ArchesSkew");
        int skew = wxAtoi(ModelXml->GetAttribute("ArchesSkew", "0"));
        screenLocation.SetAngle(skew);
    }

    if (GetLayerSizeCount() == 0) {
        int NumArches = parm1;
        int SegmentsPerArch = parm2;
        _gap = wxAtoi(ModelXml->GetAttribute("Gap", "0"));

        SetBufferSize(NumArches, SegmentsPerArch);
        if (SingleNode) {
            SetNodeCount(NumArches * SegmentsPerArch, parm3, rgbOrder);
        } else {
            SetNodeCount(NumArches, SegmentsPerArch, rgbOrder);
            if (parm3 > 1) {
                for (size_t x = 0; x < Nodes.size(); x++) {
                    Nodes[x]->Coords.resize(parm3);
                }
            }
        }
        screenLocation.SetRenderSize(SegmentsPerArch, NumArches);

        for (int y = 0; y < NumArches; y++) {
            for (int x = 0; x < SegmentsPerArch; x++) {
                int idx = y * SegmentsPerArch + x;
                int startChan = stringStartChan[y] + x * GetNodeChannelCount(StringType);
                if (!IsLtoR) {
                    startChan = stringStartChan[NumArches - y - 1] + (SegmentsPerArch - x - 1) * GetNodeChannelCount(StringType);
                }

                Nodes[idx]->ActChan = startChan;
                Nodes[idx]->StringNum = y;

                for (size_t c = 0; c < GetCoordCount(idx); c++) {
                    Nodes[idx]->Coords[c].bufX = x;
                    Nodes[idx]->Coords[c].bufY = y;
                }
            }
        }
        SetArchCoord();
    } else {
        int maxLen = 0;
        int lcount = 0;
        int sumNodes = 0;
        for (const auto& it : GetLayerSizes()) {
            maxLen = std::max(maxLen, it);
            lcount++;
            sumNodes += it;
        }
        SetBufferSize(lcount, maxLen);
        if (SingleNode) {
            SetNodeCount(parm2, parm3, rgbOrder);
        } else {
            SetNodeCount(1, parm2, rgbOrder);
            if (parm3 > 1) {
                for (size_t x = 0; x < Nodes.size(); x++) {
                    Nodes[x]->Coords.resize(parm3);
                }
            }
        }
        screenLocation.SetRenderSize(maxLen, 1);

        int idx = 0;
        bool dir = IsLtoR;
        bool in_out = isBotToTop;
        float y = 0;
        for (int layer = 0; layer < GetLayerSizeCount(); layer++) {
            int yy = layer;
            if (in_out) yy = GetLayerSizeCount() - layer - 1;
            int it = GetLayerSizes()[yy];
            if (idx < Nodes.size()) {
                if (it == 1) {
                    int startChan = stringStartChan[0] + idx * GetNodeChannelCount(StringType);
                    Nodes[idx]->ActChan = startChan;
                    Nodes[idx]->StringNum = yy;
                    for (size_t c = 0; c < GetCoordCount(idx); c++) {
                        Nodes[idx]->Coords[c].bufX = maxLen / 2;
                        Nodes[idx]->Coords[c].bufY = yy;
                    }
                    idx++;
                } else {
                    float gap = (float)(maxLen - 1) / (float)(it - 1);
                    for (int x = 0; x < it; x++) {
                        if (idx < Nodes.size()) {
                            int xx = std::round((float)x * gap);
                            if (!dir) {
                                xx = maxLen - 1 - xx;
                            }
                            int startChan = stringStartChan[0] + idx * GetNodeChannelCount(StringType);

                            Nodes[idx]->ActChan = startChan;
                            Nodes[idx]->StringNum = 0;

                            for (size_t c = 0; c < GetCoordCount(idx); c++) {
                                Nodes[idx]->Coords[c].bufX = xx;
                                Nodes[idx]->Coords[c].bufY = yy;
                            }
                        }
                        idx++;
                    }
                }
                y = y + 1;
                dir = !dir;
            }
        }

        SetLayerdArchCoord(lcount, maxLen);
    }
}

int ArchesModel::MapToNodeIndex(int strand, int node) const {
    if (GetLayerSizeCount() != 0) {
        int idx = 0;
        for (int x = GetLayerSizeCount() - 1; x > strand; x--) {
            idx += GetStrandLength(x);
        }
        idx += node;
        return idx;
    }
    return strand * parm2 + node;
}
int ArchesModel::GetNumStrands() const {
    if (GetLayerSizeCount() != 0) {
        return GetLayerSizeCount();
    }
    return parm1;
}

int ArchesModel::GetStrandLength(int strand) const
{
    if (GetLayerSizeCount() == 0) {
        return Model::GetStrandLength(strand);
    } else {
        return GetLayerSize(strand);
    }
}

int ArchesModel::GetMappedStrand(int strand) const
{
    if (GetLayerSizeCount() != 0) {
        return GetLayerSizeCount() - strand - 1;
    }
    return strand;
}

int ArchesModel::CalcCannelsPerString()
{
    SingleChannel = false;
    return GetNodeChannelCount(StringType) * parm2;
}

static void rotate_point(float cx, float cy, float angle, float& x, float& y)
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

void ArchesModel::SetLayerdArchCoord(int arches, int maxLen)
{
    double x;
    size_t NodeCount = GetNodeCount();
    double midpt = maxLen * parm3;
    midpt -= 1.0;
    midpt /= 2.0;
    double total = toRadians(arc);
    double start = (M_PI - total) / 2.0;
    float skew_angle = toRadians(screenLocation.GetAngle());

    double angle = -M_PI / 2.0 + start;
    x = midpt * sin(angle) * 2.0 + maxLen * parm3;
    double width = maxLen * parm3 * 2 - x;

    double archgap = 0;
    if (arches > 1) {
        archgap = (double)(1.0 - _hollow / 100.0) / (double)(arches - 1);
    }

    double minY = 999999;
    for (size_t n = 0; n < NodeCount; n++) {
        size_t CoordCount = GetCoordCount(n);
        double adj = 1.0 - archgap * (arches - 1 - Nodes[n]->Coords[0].bufY);
        for (size_t c = 0; c < CoordCount; c++) {
            double angle2 = -M_PI / 2.0 + start + total * ((double)(Nodes[n]->Coords[c].bufX * parm3 + c)) / midpt / 2.0;
            x = midpt * sin(angle2) * 2.0 * adj + maxLen * parm3;
            double y = (maxLen * parm3) * cos(angle2);
            Nodes[n]->Coords[c].screenX = x;
            Nodes[n]->Coords[c].screenY = y * screenLocation.GetHeight() * adj;
            rotate_point(x, 0, skew_angle,
                Nodes[n]->Coords[c].screenX,
                Nodes[n]->Coords[c].screenY);
            minY = std::min(minY, y);
        }
    }
    float renderHt = parm2 * parm3;
    if (minY > 1) {
        renderHt -= minY;
        for (const auto& it : Nodes) {
            for (auto& coord : it->Coords) {
                coord.screenY -= minY;
            }
        }
    }
    screenLocation.SetRenderSize(width, renderHt);
}

void ArchesModel::SetArchCoord()
{
    double x;
    size_t NodeCount = GetNodeCount();
    double midpt = parm2 * parm3;
    midpt -= 1.0;
    midpt /= 2.0;
    double total = toRadians(arc);
    double start = (M_PI - total) / 2.0;
    float skew_angle = toRadians(screenLocation.GetAngle());

    double angle = -M_PI / 2.0 + start;
    x = midpt * sin(angle) * 2.0 + parm2 * parm3;
    double width = parm2 * parm3 * 2 - x;

    double minY = 999999;
    int gaps = 0;
    for (size_t n = 0; n < NodeCount; n++) {
        double xoffset = Nodes[n]->StringNum * width;
        size_t CoordCount = GetCoordCount(n);
        for (size_t c = 0; c < CoordCount; c++) {
            double angle2 = -M_PI / 2.0 + start + total * ((double)(Nodes[n]->Coords[c].bufX * parm3 + c)) / midpt / 2.0;
            x = xoffset + midpt * sin(angle2) * 2.0 + parm2 * parm3 + gaps * _gap;
            double y = (parm2 * parm3) * cos(angle2);
            Nodes[n]->Coords[c].screenX = x;
            Nodes[n]->Coords[c].screenY = y * screenLocation.GetHeight();
            rotate_point(x, 0, skew_angle,
                Nodes[n]->Coords[c].screenX,
                Nodes[n]->Coords[c].screenY);
            minY = std::min(minY, y);
        }
        if ((n + 1) % parm2 == 0) {
            gaps++;
        }
    }
    float renderHt = parm2 * parm3;
    if (minY > 1) {
        renderHt -= minY;
        for (auto it = Nodes.begin(); it != Nodes.end(); ++it) {
            for (auto coord = (*it)->Coords.begin(); coord != (*it)->Coords.end(); ++coord) {
                coord->screenY -= minY;
            }
        }
    }
    screenLocation.SetRenderSize(width * parm1 + (parm1 - 1) * _gap, renderHt);
}

void ArchesModel::ExportXlightsModel()
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
    wxString an = ModelXml->GetAttribute("Angle", "0");
    wxString ls = ModelXml->GetAttribute("LayerSizes", "");
    wxString h = ModelXml->GetAttribute("Hollow", "70");

    wxString v = xlights_version_string;
    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<archesmodel \n");
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
    f.Write(wxString::Format("Angle=\"%s\" ", an));
    f.Write(wxString::Format("SourceVersion=\"%s\" ", v));
    f.Write(wxString::Format("LayerSizes=\"%s\" ", ls));
    f.Write(wxString::Format("Hollow=\"%s\" ", h));
    f.Write(ExportSuperStringColors());
    f.Write(" >\n");

    wxString groups = SerialiseGroups();
    if (groups != "") {
        f.Write(groups);
    }
    wxString state = SerialiseState();
    if (state != "") {
        f.Write(state);
    }
    wxString submodel = SerialiseSubmodel();
    if (submodel != "") {
        f.Write(submodel);
    }
    ExportDimensions(f);
    f.Write("</archesmodel>");
    f.Close();
}

void ArchesModel::ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
    if (root->GetName() == "archesmodel") {
        wxString name = root->GetAttribute("name");
        wxString p1 = root->GetAttribute("parm1");
        wxString p2 = root->GetAttribute("parm2");
        wxString p3 = root->GetAttribute("parm3");
        wxString st = root->GetAttribute("StringType");
        wxString ps = root->GetAttribute("PixelSize");
        wxString t = root->GetAttribute("Transparency", "0");
        wxString mb = root->GetAttribute("ModelBrightness", "0");
        wxString a = root->GetAttribute("Antialias");
        wxString an = root->GetAttribute("Angle");
        wxString ss = root->GetAttribute("StartSide");
        wxString dir = root->GetAttribute("Dir");
        wxString sn = root->GetAttribute("StrandNames");
        wxString nn = root->GetAttribute("NodeNames");
        wxString v = root->GetAttribute("SourceVersion");
        wxString da = root->GetAttribute("DisplayAs");
        wxString pc = root->GetAttribute("PixelCount");
        wxString pt = root->GetAttribute("PixelType");
        wxString psp = root->GetAttribute("PixelSpacing");
        wxString ls = root->GetAttribute("LayerSizes");
        wxString h = root->GetAttribute("Hollow");

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
        SetProperty("Angle", an);
        SetProperty("PixelCount", pc);
        SetProperty("PixelType", pt);
        SetProperty("PixelSpacing", psp);
        SetProperty("LayerSizes", ls);
        SetProperty("Hollow", h);

        wxString newname = xlights->AllModels.GenerateModelName(name.ToStdString());
        GetModelScreenLocation().Write(ModelXml);
        SetProperty("name", newname, true);
        ImportSuperStringColours(root);
        ImportModelChildren(root, xlights, newname, min_x, max_x, min_y, max_y);

        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ArchesModel::ImportXlightsModel");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ArchesModel::ImportXlightsModel");
    } else {
        DisplayError("Failure loading Arches model file.");
    }
}

std::string ArchesModel::GetDimension() const
{
    if (GetLayerSizeCount() == 0 && parm1 != 0) {
        return GetModelScreenLocation().GetDimension(1.0 / parm1);
    }
    return GetModelScreenLocation().GetDimension(1.0);
}

void ArchesModel::AddDimensionProperties(wxPropertyGridInterface* grid)
{
    if (GetLayerSizeCount() == 0 && parm1 != 0) {
        GetModelScreenLocation().AddDimensionProperties(grid, 1.0 / parm1);
    } else {
        GetModelScreenLocation().AddDimensionProperties(grid, 1.0);
    }
}

void ArchesModel::OnLayerSizesChange(bool countChanged)
{
    // if string count is 1 then adjust nodes per string to match sum of nodes
    if (parm1 == 1 && GetLayerSizeCount() > 0) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%d", (int)GetLayerSizesTotalNodes()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ArchesModel::OnLayerSizesChange");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ArchesModel::OnLayerSizesChange");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "ArchesModel::OnLayerSizesChange");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "ArchesModel::OnLayerSizesChange");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ArchesModel::OnLayerSizesChange");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ArchesModel::OnLayerSizesChange");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "ArchesModel::OnLayerSizesChange");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ArchesModel::OnLayerSizesChange");
    }
}
