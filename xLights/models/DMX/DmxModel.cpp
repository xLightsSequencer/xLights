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

#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "DmxModel.h"
#include "../ModelScreenLocation.h"
#include "../../ModelPreview.h"
#include "../../RenderBuffer.h"
#include "../../xLightsVersion.h"
#include "../../xLightsMain.h"
#include "../../UtilFunctions.h"

DmxModel::DmxModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased)
    : ModelWithScreenLocation(manager), color_ability(nullptr)
{
}

DmxModel::~DmxModel()
{
    //dtor
}

void DmxModel::GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform, int &BufferWi, int &BufferHi) const {
        BufferHi = 1;
        BufferWi = GetNodeCount();
}

void DmxModel::InitRenderBufferNodes(const std::string &type, const std::string &camera, const std::string &transform,
                                     std::vector<NodeBaseClassPtr> &newNodes, int &BufferWi, int &BufferHi) const {
    BufferHi = 1;
    BufferWi = GetNodeCount();

    for (int cur=0; cur < BufferWi; cur++) {
        newNodes.push_back(NodeBaseClassPtr(Nodes[cur]->clone()));
        for(size_t c=0; c < newNodes[cur]->Coords.size(); c++) {
            newNodes[cur]->Coords[c].bufX=cur;
            newNodes[cur]->Coords[c].bufY=0;
        }
    }
}

void DmxModel::AddTypeProperties(wxPropertyGridInterface *grid) {

    wxPGProperty *p = grid->Append(new wxUIntProperty("# Channels", "DmxChannelCount", parm1));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");
}

void DmxModel::DisableUnusedProperties(wxPropertyGridInterface *grid)
{
    // disable string type properties.  Only Single Color White allowed.
    wxPGProperty *p = grid->GetPropertyByName("ModelStringType");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("ModelStringColor");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("ModelFaces");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("ModelDimmingCurves");
    if (p != nullptr) {
        p->Enable(false);
    }

    // Don't remove ModelStates ... these can be used for DMX devices that use a value range to set a colour or behaviour
}

void DmxModel::UpdateChannelCount(int num_channels, bool do_work) {
    parm1 = num_channels;
    ModelXml->DeleteAttribute("parm1");
    ModelXml->AddAttribute("parm1", wxString::Format("%d", num_channels));
    if (do_work) {
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxModel::UpdateChannelCount::DMXChannelCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxModel::UpdateChannelCount::DMXChannelCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxModel::UpdateChannelCount::DMXChannelCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "DmxModel::UpdateChannelCount::DMXChannelCount");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxModel::UpdateChannelCount::DMXChannelCount");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "DmxModel::UpdateChannelCount::DMXChannelCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "DmxModel::UpdateChannelCount::DMXChannelCount");
    }
}

int DmxModel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {

    if ("DmxChannelCount" == event.GetPropertyName()) {
        UpdateChannelCount((int)event.GetPropertyValue().GetLong(), true);
        return 0;
    } 

    return Model::OnPropertyGridChange(grid, event);
}

void DmxModel::InitModel() {
    if (DisplayAs.empty()) {
        DisplayAs = "DMX";
    }
    StringType = "Single Color White";
    parm2 = 1;
    parm3 = 1;

    int numChannels = parm1;
    SetNodeCount(numChannels, 1, rgbOrder);

    int curNode = 0;
    for (int x = 0; x < numChannels; x++) {
        Nodes[curNode]->ActChan = stringStartChan[0] + curNode*GetNodeChannelCount(StringType);
        Nodes[curNode]->StringNum=0;
        // the screenx/screeny positions are used to fake it into giving a bigger selection area
        if( x == 0 ) {
            Nodes[curNode]->Coords[0].screenX = -0.5f;
            Nodes[curNode]->Coords[0].screenY = -0.5f;
        } else if( x == 1 ) {
            Nodes[curNode]->Coords[0].screenX = 0.5f;
            Nodes[curNode]->Coords[0].screenY = 0.5f;
        } else {
            Nodes[curNode]->Coords[0].screenX = 0;
            Nodes[curNode]->Coords[0].screenY = 0;
        }
        Nodes[curNode]->Coords[0].bufX = 0;
        Nodes[curNode]->Coords[0].bufY = 0;
        curNode++;
    }
    SetBufferSize(1,parm1);
}

void DmxModel::DisplayEffectOnWindow(ModelPreview* preview, double pointSize)
{
    if (!IsActive()) return;

    bool success = preview->StartDrawing(pointSize);

    if(success) {
        DrawGLUtils::xlAccumulator va(maxVertexCount);

        float sx,sy;
        xlColor color, proxy;
        int w, h;

        GetModelScreenLocation().PrepareToDraw(false, false);

        va.PreAlloc(maxVertexCount);

        preview->GetSize(&w, &h);

        sx=w/2;
        sy=h/2;

        DrawModelOnWindow(preview, va, nullptr, sx, sy, true);

        DrawGLUtils::Draw(va);

        preview->EndDrawing();

    }

}

// display model using colors
void DmxModel::DisplayModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &sva, DrawGLUtils::xlAccumulator &tva, float& minx, float& miny, float& maxx, float&maxy, bool is_3d, const xlColor *c, bool allowSelected) {

    if (!IsActive()) return;

    int w, h;
    preview->GetVirtualCanvasSize(w, h);

    GetModelScreenLocation().PrepareToDraw(false, false);

    tva.PreAlloc(maxVertexCount);

    float sx = 0;
    float sy = 0;
    float sz = 0;
    GetModelScreenLocation().TranslatePoint(sx, sy, sz);

    GetModelScreenLocation().SetDefaultMatrices();
    GetModelScreenLocation().UpdateBoundingBox(Nodes);  // FIXME: Modify to only call this when position changes

    DrawModelOnWindow(preview, tva, c, sx, sy, !allowSelected);

    if (sx < minx) minx = sx;
    if (sy < miny) miny = sy;
    if (sx > maxx) maxx = sx;
    if (sy > maxy) maxy = sy;

    if ((Selected || (Highlighted && is_3d)) && c != nullptr && allowSelected) {
        GetModelScreenLocation().DrawHandles(sva, preview->GetCameraZoomForHandles(), preview->GetHandleScale());
    }
}

// display model using colors
void DmxModel::DisplayModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator &sva, DrawGLUtils::xl3Accumulator &tva, DrawGLUtils::xl3Accumulator& lva, bool is_3d, const xlColor *c, bool allowSelected, bool wiring, bool highlightFirst, int highlightpixel) {

    if (!IsActive()) return;

    int w, h;
    preview->GetVirtualCanvasSize(w, h);

    GetModelScreenLocation().PrepareToDraw(false, false);

    tva.PreAlloc(maxVertexCount);

    float sx = 0;
    float sy = 0;
    float sz = 0;
    GetModelScreenLocation().TranslatePoint(sx, sy, sz);

    GetModelScreenLocation().SetDefaultMatrices();
    GetModelScreenLocation().UpdateBoundingBox(Nodes);  // FIXME: Modify to only call this when position changes

    DrawModelOnWindow(preview, tva, c, sx, sy, sz, !allowSelected);

    if ((Selected || (Highlighted && is_3d)) && c != nullptr && allowSelected) {
        GetModelScreenLocation().DrawHandles(sva, preview->GetCameraZoomForHandles(), preview->GetHandleScale());
    }
}

int DmxModel::GetChannelValue(int channel, bool bits16)
{
    xlColor color_angle;
    int lsb = 0;
    int msb = 0;
    int ret_val = 0;

    if (bits16) {
        Nodes[channel]->GetColor(color_angle);
        msb = color_angle.red;
        Nodes[channel + 1]->GetColor(color_angle);
        lsb = color_angle.red;
        ret_val = ((msb << 8) | lsb);
    }
    else {
        Nodes[channel]->GetColor(color_angle);
        ret_val = color_angle.red;
    }
    return ret_val;
}

void DmxModel::SetNodeNames(const std::string& default_names, bool force) {
    wxString nn = ModelXml->GetAttribute("NodeNames", "");
    bool save_names = false;
    if (nn == "" || force) {
        // provide default node names
        nn = default_names;
        save_names = true;
    }

    wxString tempstr = nn;
    nodeNames.clear();
    while (tempstr.size() > 0) {
        std::string t2 = tempstr.ToStdString();
        if (tempstr[0] == ',') {
            t2 = "";
            tempstr = tempstr(1, tempstr.length());
        }
        else if (tempstr.Contains(",")) {
            t2 = tempstr.SubString(0, tempstr.Find(",") - 1);
            tempstr = tempstr.SubString(tempstr.Find(",") + 1, tempstr.length());
        }
        else {
            tempstr = "";
        }
        nodeNames.push_back(t2);
    }
    if (save_names) {
        SetProperty("NodeNames", nn);
        ModelXml->DeleteAttribute("NodeNames");
        ModelXml->AddAttribute("NodeNames", nn);
    }
}

void DmxModel::ExportBaseParameters(wxFile& f)
{
    wxString p1 = ModelXml->GetAttribute("parm1");
    wxString p2 = ModelXml->GetAttribute("parm2");
    wxString p3 = ModelXml->GetAttribute("parm3");
    wxString st = ModelXml->GetAttribute("StringType");
    wxString ps = ModelXml->GetAttribute("PixelSize");
    wxString t = ModelXml->GetAttribute("Transparency");
    wxString mb = ModelXml->GetAttribute("ModelBrightness");
    wxString a = ModelXml->GetAttribute("Antialias");
    wxString ss = ModelXml->GetAttribute("StartSide");
    wxString dir = ModelXml->GetAttribute("Dir");
    wxString sn = ModelXml->GetAttribute("StrandNames");
    wxString nn = ModelXml->GetAttribute("NodeNames");
    wxString da = ModelXml->GetAttribute("DisplayAs");
    wxString ds = ModelXml->GetAttribute("DmxStyle");
    wxString v = xlights_version_string;

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
    f.Write(wxString::Format("SourceVersion=\"%s\" ", v));
    f.Write(wxString::Format("DmxStyle=\"%s\" ", ds));
}

void DmxModel::ImportBaseParameters(wxXmlNode* root)
{
    wxString p1 = root->GetAttribute("parm1");
    wxString p2 = root->GetAttribute("parm2");
    wxString p3 = root->GetAttribute("parm3");
    wxString st = root->GetAttribute("StringType");
    wxString ps = root->GetAttribute("PixelSize");
    wxString t = root->GetAttribute("Transparency");
    wxString mb = root->GetAttribute("ModelBrightness");
    wxString a = root->GetAttribute("Antialias");
    wxString ss = root->GetAttribute("StartSide");
    wxString dir = root->GetAttribute("Dir");
    wxString sn = root->GetAttribute("StrandNames");
    wxString nn = root->GetAttribute("NodeNames");
    wxString da = root->GetAttribute("DisplayAs");
    wxString ds = root->GetAttribute("DmxStyle");

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
    SetProperty("DmxStyle", ds);
}
