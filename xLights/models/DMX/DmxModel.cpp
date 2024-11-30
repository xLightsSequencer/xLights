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
#include <wx/msgdlg.h>

#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "DmxModel.h"
#include "DmxColorAbility.h"
#include "DmxColorAbilityRGB.h"
#include "DmxColorAbilityCMY.h"
#include "DmxPresetAbility.h"
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

void DmxModel::GetBufferSize(const std::string& type, const std::string& camera, const std::string& transform, int& BufferWi, int& BufferHi, int stagger) const
{
    BufferHi = 1;
    BufferWi = GetNodeCount();
}

void DmxModel::InitRenderBufferNodes(const std::string& type, const std::string& camera, const std::string& transform,
    std::vector<NodeBaseClassPtr>& newNodes, int& BufferWi, int& BufferHi, int stagger, bool deep) const
{
    BufferHi = 1;
    BufferWi = GetNodeCount();

    for (int cur = 0; cur < BufferWi; cur++) {
        newNodes.push_back(NodeBaseClassPtr(Nodes[cur]->clone()));
        for (size_t c = 0; c < newNodes[cur]->Coords.size(); c++) {
            newNodes[cur]->Coords[c].bufX = cur;
            newNodes[cur]->Coords[c].bufY = 0;
        }
    }
}

void DmxModel::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager)
{
    wxPGProperty* p = grid->Append(new wxUIntProperty("# Channels", "DmxChannelCount", parm1));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    if (nullptr != preset_ability ) {
        preset_ability->AddProperties(grid, ModelXml);
    }
}

void DmxModel::DisableUnusedProperties(wxPropertyGridInterface* grid)
{
    // disable string type properties.  Only Single Color White allowed.
    wxPGProperty* p = grid->GetPropertyByName("ModelStringType");
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

void DmxModel::UpdateChannelCount(int num_channels, bool do_work)
{
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

int DmxModel::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event)
{
    IncrementChangeCount();
    if ("DmxChannelCount" == event.GetPropertyName()) {
        IncrementChangeCount();
        UpdateChannelCount((int)event.GetPropertyValue().GetLong(), true);
        return 0;
    }

    if (nullptr != preset_ability && preset_ability->OnPropertyGridChange(grid, event, ModelXml, this) == 0) {
        IncrementChangeCount();
        return 0;
    }

    return Model::OnPropertyGridChange(grid, event);
}

void DmxModel::InitModel()
{
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
        Nodes[curNode]->ActChan = stringStartChan[0] + curNode * GetNodeChannelCount(StringType);
        Nodes[curNode]->StringNum = 0;
        // the screenx/screeny positions are used to fake it into giving a bigger selection area
        if (x == 0) {
            Nodes[curNode]->Coords[0].screenX = -0.5f;
            Nodes[curNode]->Coords[0].screenY = -0.5f;
        }
        else if (x == 1) {
            Nodes[curNode]->Coords[0].screenX = 0.5f;
            Nodes[curNode]->Coords[0].screenY = 0.5f;
        }
        else {
            Nodes[curNode]->Coords[0].screenX = 0;
            Nodes[curNode]->Coords[0].screenY = 0;
        }
        Nodes[curNode]->Coords[0].bufX = 0;
        Nodes[curNode]->Coords[0].bufY = 0;
        curNode++;
    }
    SetBufferSize(1, parm1);
    preset_ability = std::make_unique<DmxPresetAbility>(ModelXml);
}

int DmxModel::GetChannelValue(int channel, bool bits16)
{
    xlColor color_angle;
    int lsb = 0;
    int msb = 0;
    int ret_val = 0;

    if (bits16) {
        if (Nodes.size() > channel + 1) {
            Nodes[channel]->GetColor(color_angle);
            msb = color_angle.red;
            Nodes[channel + 1]->GetColor(color_angle);
            lsb = color_angle.red;
            ret_val = ((msb << 8) | lsb);
        }
    }
    else {
        if (Nodes.size() > channel) {
            Nodes[channel]->GetColor(color_angle);
            ret_val = color_angle.red;
        }
    }
    return ret_val;
}

// support for moving heads or devices where coarse and fine channels are not contiguous
int DmxModel::GetChannelValue(int channel_coarse, int channel_fine)
{
    xlColor color_angle;
    int lsb = 0;
    int msb = 0;
    int ret_val = 0;

    if (Nodes.size() > channel_coarse + 1) {
        Nodes[channel_coarse]->GetColor(color_angle);
        msb = color_angle.red;
    }
    if( channel_fine >= 0 )
    if (Nodes.size() > channel_fine + 1) {
        Nodes[channel_fine]->GetColor(color_angle);
        lsb = color_angle.red;
    }
    ret_val = ((msb << 8) | lsb);

    return ret_val;
}

void DmxModel::SetNodeNames(const std::string& default_names, bool force)
{
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

std::list<std::string> DmxModel::CheckModelSettings()
{
    std::list<std::string> res;

    if (nullptr != color_ability) {
        res = color_ability->CheckModelSettings(this);
    }

    if (nullptr != preset_ability) {
        res.splice(res.end(), preset_ability->CheckModelSettings(this));
    }

    res.splice(res.end(), Model::CheckModelSettings());
    return res;
}

void DmxModel::DrawInvalid(xlGraphicsProgram* pg, ModelScreenLocation* msl, bool is_3d, bool applyTransform)
{
    if (applyTransform) {
        pg->addStep([msl, is_3d](xlGraphicsContext* ctx) {
            ctx->PushMatrix();
            if (!is_3d) {
                //not 3d, flatten to the 0.5 plane
                ctx->Translate(0, 0, 0.5f);
                ctx->ScaleViewMatrix(1.0f, 1.0f, 0.001f);
            }
            msl->ApplyModelViewMatrices(ctx);
        });
    }
    auto vac = pg->getAccumulator();
    int start = vac->getCount();
    vac->PreAlloc(12);
    vac->AddVertex(-0.5, -0.5, 0, *wxRED);
    vac->AddVertex(-0.5, 0.5, 0, *wxRED);
    vac->AddVertex(-0.5, 0.5, 0, *wxRED);
    vac->AddVertex(0.5, 0.5, 0, *wxRED);
    vac->AddVertex(0.5, 0.5, 0, *wxRED);
    vac->AddVertex(0.5, -0.5, 0, *wxRED);
    vac->AddVertex(0.5, -0.5, 0, *wxRED);
    vac->AddVertex(-0.5, -0.5, 0, *wxRED);
    vac->AddVertex(-0.5, -0.5, 0, *wxRED);
    vac->AddVertex(0.5, 0.5, 0, *wxRED);
    vac->AddVertex(-0.5, 0.5, 0, *wxRED);
    vac->AddVertex(0.5, -0.5, 0, *wxRED);
    int end = vac->getCount();
    pg->addStep([=](xlGraphicsContext* ctx) {
        ctx->drawLines(vac, start, end - start);
    });
    if (applyTransform) {
        pg->addStep([=](xlGraphicsContext* ctx) {
            ctx->PopMatrix();
        });
    }
}

void DmxModel::ExportBaseParameters(wxFile& f)
{
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

    if (nullptr != preset_ability) {
        preset_ability->ExportParameters(f, ModelXml);
    }
}

bool DmxModel::ImportBaseParameters(wxXmlNode* root)
{
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
    wxString da = root->GetAttribute("DisplayAs");

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

    _startSide = ss;

    if (nullptr != preset_ability) {
        preset_ability->ImportParameters(root, this);
    }

    return true;
}

std::vector<std::string> DmxModel::GenerateNodeNames() const
{
    std::vector<std::string> names;
    for (int i=0; i< parm1; ++i) {// parm1 is channel count
        names.push_back("");
    }
    if (nullptr != color_ability) {
        color_ability->SetNodeNames(names);
    }
    if (nullptr != preset_ability) {
        preset_ability->SetNodeNames(names);
    }
    return names;
}

void DmxModel::EnableFixedChannels(xlColorVector& pixelVector) const
{
    if (nullptr != preset_ability) {
         preset_ability->SetPresetValues(pixelVector);
    }
}

void DmxModel::GetPWMOutputs(std::map<uint32_t, PWMOutput> &channels) const {
    if (nullptr != color_ability) {
        color_ability->GetPWMOutputs(channels);
    }
}
std::vector<PWMOutput> DmxModel::GetPWMOutputs() const {
    std::map<uint32_t, PWMOutput> channels;
    GetPWMOutputs(channels);
    std::vector<PWMOutput> ret;
    uint32_t startChannel = GetFirstChannel();
    for (auto &a : channels) {
        if ((a.first - 1) < nodeNames.size() && !nodeNames[a.first - 1].empty()) {
            a.second.label = nodeNames[a.first - 1];
        }
        ret.emplace_back(a.second);
        ret.back().startChannel += startChannel - 1;
    }
    return ret;
}
