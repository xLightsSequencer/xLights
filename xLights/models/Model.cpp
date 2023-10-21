/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/wx.h>
#include <wx/xml/xml.h>
#include <wx/tokenzr.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/sstream.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/stdpaths.h>

#include "Model.h"
#include "ModelManager.h"
#include "ModelGroup.h"
#include "../xLightsApp.h"
#include "../xLightsMain.h" //for Preview and Other model collections
#include "../xLightsXmlFile.h"
#include "../Color.h"
#include "../DimmingCurve.h"
#include "../StrandNodeNamesDialog.h"
#include "../ModelFaceDialog.h"
#include "../ModelStateDialog.h"
#include "../ModelChainDialog.h"
#include "../ModelDimmingCurveDialog.h"
#include "../StartChannelDialog.h"
#include "../SubModelsDialog.h"
#include "../outputs/Output.h"
#include "../outputs/OutputManager.h"
#include "../outputs/IPOutput.h"
#include "../outputs/Controller.h"
#include "../outputs/ControllerSerial.h"
#include "../VendorModelDialog.h"
#include "../ModelPreview.h"
#include "ModelScreenLocation.h"
#include "SubModel.h"
#include "../UtilFunctions.h"
#include "../xLightsVersion.h"
#include "../controllers/ControllerCaps.h"
#include "../Pixels.h"
#include "../ExternalHooks.h"
#include "CustomModel.h"
#include "RulerObject.h"
#include "../utils/ip_utils.h"

#include <log4cpp/Category.hh>

#include "../xSchedule/wxJSON/jsonreader.h"
#include "CachedFileDownloader.h"

#include <algorithm>

#define MOST_STRINGS_WE_EXPECT 480
#define MOST_CONTROLLER_PORTS_WE_EXPECT 128

const long Model::ID_LAYERSIZE_INSERT = wxNewId();
const long Model::ID_LAYERSIZE_DELETE = wxNewId();

static const int PORTS_PER_SMARTREMOTE = 4;

static const char* NODE_TYPE_VLUES[] = {
    "RGB Nodes", "RBG Nodes", "GBR Nodes", "GRB Nodes",
    "BRG Nodes", "BGR Nodes", "Node Single Color", "3 Channel RGB",
    "4 Channel RGBW", "4 Channel WRGB", "Strobes", "Single Color",
    "Single Color Intensity", "Superstring", "WRGB Nodes", "WRBG Nodes",
    "WGBR Nodes", "WGRB Nodes", "WBRG Nodes", "WBGR Nodes", "RGBW Nodes",
    "RBGW Nodes", "GBRW Nodes", "GRBW Nodes", "BRGW Nodes",  "BGRW Nodes"
};
static wxArrayString NODE_TYPES(26, NODE_TYPE_VLUES);

static const char *RGBW_HANDLING_VALUES[] = {"R=G=B -> W", "RGB Only", "White Only", "Advanced", "White On All"};
static wxArrayString RGBW_HANDLING(5, RGBW_HANDLING_VALUES);

static const char *PIXEL_STYLES_VALUES[] = {"Square", "Smooth", "Solid Circle", "Blended Circle"};
static wxArrayString PIXEL_STYLES(4, PIXEL_STYLES_VALUES);

static const char *CONTROLLER_DIRECTION_VALUES[] = {"Forward", "Reverse"};
static wxArrayString CONTROLLER_DIRECTION(2, CONTROLLER_DIRECTION_VALUES);

static const char *CONTROLLER_COLORORDER_VALUES[] = {
    "RGB", "RBG", "GBR", "GRB", "BRG", "BGR",
    "RGBW", "RBGW", "GBRW", "GRBW", "BRGW", "BGRW",
    "WRGB", "WRBG", "WGBR", "WGRB", "WBRG", "WBGR"};
static wxArrayString CONTROLLER_COLORORDER(18, CONTROLLER_COLORORDER_VALUES);

static wxArrayString LAYOUT_GROUPS;
static wxArrayString CONTROLLERS;
static wxArrayString OTHERMODELLIST;

static const std::string DEFAULT("Default");
static const std::string PER_PREVIEW("Per Preview");
static const std::string SINGLE_LINE("Single Line");
static const std::string AS_PIXEL("As Pixel");
static const std::string VERT_PER_STRAND("Vertical Per Strand");
static const std::string HORIZ_PER_STRAND("Horizontal Per Strand");

static const std::string PER_PREVIEW_NO_OFFSET("Per Preview No Offset");

const std::vector<std::string> Model::DEFAULT_BUFFER_STYLES {DEFAULT, PER_PREVIEW, SINGLE_LINE, AS_PIXEL};

static void clearUnusedProtocolProperties(wxXmlNode* node)
{
    std::string protocol = node->GetAttribute("Protocol");
    bool isDMX = IsSerialProtocol(protocol);
    bool isPixel = IsPixelProtocol(protocol);

    if (!isPixel) {
        node->DeleteAttribute("gamma");
        node->DeleteAttribute("brightness");
        node->DeleteAttribute("nullNodes");
        node->DeleteAttribute("endNullNodes");
        node->DeleteAttribute("colorOrder");
        node->DeleteAttribute("reverse");
        node->DeleteAttribute("groupCount");
        node->DeleteAttribute("zigZag");
        node->DeleteAttribute("ts");
    }
    if (!isDMX) {
        node->DeleteAttribute("channel");
    }
}

static const std::string EFFECT_PREVIEW_CACHE("ModelPreviewEffectCache");
static const std::string MODEL_PREVIEW_CACHE_2D("ModelPreviewCache3D");
static const std::string MODEL_PREVIEW_CACHE_3D("ModelPreviewCache2D");
static const std::string LAYOUT_PREVIEW_CACHE_2D("LayoutPreviewCache3D");
static const std::string LAYOUT_PREVIEW_CACHE_3D("LayoutPreviewCache2D");


Model::Model(const ModelManager& manager) : modelManager(manager)
{
}

Model::~Model()
{
    deleteUIObjects();
    if (modelDimmingCurve != nullptr) {
        delete modelDimmingCurve;
    }
    for (const auto& it : subModels) {
        Model* m = it;
        delete m;
    }
}

static const std::string CLICK_TO_EDIT("--Click To Edit--");
class StrandNodeNamesDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    StrandNodeNamesDialogAdapter(const Model* model)
        : wxPGEditorDialogAdapter(), m_model(model)
    {
    }
    virtual bool DoShowDialog(wxPropertyGrid* propGrid,
        wxPGProperty* WXUNUSED(property)) override
    {
        StrandNodeNamesDialog dlg(propGrid);
        dlg.Setup(m_model,
            m_model->GetModelXml()->GetAttribute("NodeNames").ToStdString(),
            m_model->GetModelXml()->GetAttribute("StrandNames").ToStdString());
        if (dlg.ShowModal() == wxID_OK) {
            m_model->GetModelXml()->DeleteAttribute("NodeNames");
            m_model->GetModelXml()->DeleteAttribute("StrandNames");
            m_model->GetModelXml()->AddAttribute("NodeNames", dlg.GetNodeNames());
            m_model->GetModelXml()->AddAttribute("StrandNames", dlg.GetStrandNames());
            wxVariant v(CLICK_TO_EDIT);
            SetValue(v);
            return true;
        }
        return false;
    }
protected:
    const Model* m_model;
};

class FacesDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    FacesDialogAdapter(Model* model, OutputManager* om) :
        wxPGEditorDialogAdapter(), m_model(model), _outputManager(om)
    {
    }
    virtual bool DoShowDialog(wxPropertyGrid* propGrid,
        wxPGProperty* WXUNUSED(property)) override
    {
        ModelFaceDialog dlg(propGrid, _outputManager);
        dlg.SetFaceInfo(m_model, m_model->faceInfo);
        if (dlg.ShowModal() == wxID_OK) {
            m_model->faceInfo.clear();
            dlg.GetFaceInfo(m_model->faceInfo);
            wxVariant v(CLICK_TO_EDIT);
            SetValue(v);
            return true;
        }
        return false;
    }
protected:
    Model* m_model = nullptr;
    OutputManager* _outputManager = nullptr;
};

class StatesDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    StatesDialogAdapter(Model* model, OutputManager* om)
        : wxPGEditorDialogAdapter(), m_model(model), _outputManager(om)
    {
    }
    virtual bool DoShowDialog(wxPropertyGrid* propGrid,
        wxPGProperty* WXUNUSED(property)) override
    {
        ModelStateDialog dlg(propGrid, _outputManager);
        dlg.SetStateInfo(m_model, m_model->stateInfo);
        if (dlg.ShowModal() == wxID_OK) {
            m_model->stateInfo.clear();
            dlg.GetStateInfo(m_model->stateInfo);
            wxVariant v(CLICK_TO_EDIT);
            SetValue(v);
            return true;
        }
        return false;
    }
protected:
    Model* m_model = nullptr;
    OutputManager* _outputManager = nullptr;
};

std::map<std::string, std::map<std::string, std::string> > Model::GetDimmingInfo() const
{
    std::map<std::string, std::map<std::string, std::string> > dimmingInfo;
    wxXmlNode* f = GetModelXml()->GetChildren();
    while (f != nullptr) {
        if ("dimmingCurve" == f->GetName()) {
            wxXmlNode* dc = f->GetChildren();
            while (dc != nullptr) {
                std::string name = dc->GetName().ToStdString();
                wxXmlAttribute* att = dc->GetAttributes();
                while (att != nullptr) {
                    dimmingInfo[name][att->GetName().ToStdString()] = att->GetValue();
                    att = att->GetNext();
                }
                dc = dc->GetNext();
            }
        }
        f = f->GetNext();
    }

    return dimmingInfo;
}

class DimmingCurveDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    DimmingCurveDialogAdapter(const Model *model)
    : wxPGEditorDialogAdapter(), m_model(model) {
    }
    virtual bool DoShowDialog(wxPropertyGrid* propGrid,
                              wxPGProperty* WXUNUSED(property) ) override {
        ModelDimmingCurveDialog dlg(propGrid);
        auto dimmingInfo = m_model->GetDimmingInfo();
        if(dimmingInfo.empty()) {
            wxString b = m_model->GetModelXml()->GetAttribute("ModelBrightness","0");
            if (b.empty()) {
                b = "0";
            }
            dimmingInfo["all"]["gamma"] = "1.0";
            dimmingInfo["all"]["brightness"] = b;
        }
        dlg.Init(dimmingInfo);
        if (dlg.ShowModal() == wxID_OK) {
            dimmingInfo.clear();
            dlg.Update(dimmingInfo);
            wxXmlNode *f1 = m_model->GetModelXml()->GetChildren();
            while (f1 != nullptr) {
                if ("dimmingCurve" == f1->GetName()) {
                    m_model->GetModelXml()->RemoveChild(f1);
                    delete f1;
                    f1 = m_model->GetModelXml()->GetChildren();
                } else {
                    f1 = f1->GetNext();
                }
            }
            f1 = new wxXmlNode(wxXML_ELEMENT_NODE , "dimmingCurve");
            m_model->GetModelXml()->AddChild(f1);
            for (const auto& it : dimmingInfo) {
                wxXmlNode *dc = new wxXmlNode(wxXML_ELEMENT_NODE , it.first);
                f1->AddChild(dc);
                for (const auto& it2 : it.second) {
                    dc->AddAttribute(it2.first, it2.second);
                }
            }
            wxVariant v(CLICK_TO_EDIT);
            SetValue(v);
            return true;
        }
        return false;
    }
protected:
    const Model *m_model;
};
class SubModelsDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    SubModelsDialogAdapter(Model *model, OutputManager* om) :
        wxPGEditorDialogAdapter(), m_model(model), _outputManager(om)
    {
    }
    virtual bool DoShowDialog(wxPropertyGrid* propGrid,
                              wxPGProperty* WXUNUSED(property) ) override {
        SubModelsDialog dlg(propGrid, _outputManager);
        dlg.Setup(m_model);
        if (dlg.ShowModal() == wxID_OK) {
            dlg.Save();
            wxVariant v(CLICK_TO_EDIT);
            SetValue(v);
            return true;
        }
        if (dlg.ReloadLayout) {//force grid to reload
            wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
            wxPostEvent(m_model->GetModelManager().GetXLightsFrame(), eventForceRefresh);
            m_model->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "Model::SubModelsDialog::SubModels");
            m_model->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::SubModelsDialog::SubModels");
            m_model->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::SubModelsDialog::SubModels");
        }
        return false;
    }
protected:
    Model *m_model = nullptr;
    OutputManager* _outputManager = nullptr;
};

class ModelChainDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    ModelChainDialogAdapter(Model *model)
        : wxPGEditorDialogAdapter(), m_model(model) {
    }
    virtual bool DoShowDialog(wxPropertyGrid* propGrid,
        wxPGProperty* property) override {
        ModelChainDialog dlg(propGrid);
        dlg.Set(m_model, m_model->GetModelManager());
        if (dlg.ShowModal() == wxID_OK) {
            wxVariant v(dlg.Get());
            SetValue(v);
            return true;
        }
        return false;
    }
protected:
    Model *m_model;
};

class PopupDialogProperty : public wxStringProperty
{
public:
    PopupDialogProperty(Model *m,
                        OutputManager* om,
                        const wxString& label,
                        const wxString& name,
                        const wxString& value,
                        int type) :
        wxStringProperty(label, name, value), m_model(m), m_tp(type), _outputManager(om)
    {
    }
    // Set editor to have button
    virtual const wxPGEditor* DoGetEditorClass() const override {
        return wxPGEditor_TextCtrlAndButton;
    }
    // Set what happens on button click
    virtual wxPGEditorDialogAdapter* GetEditorDialog() const override {
        switch (m_tp) {
        case 1:
            return new StrandNodeNamesDialogAdapter(m_model);
        case 2:
            return new FacesDialogAdapter(m_model, _outputManager);
        case 3:
            return new DimmingCurveDialogAdapter(m_model);
        case 4:
            return new StatesDialogAdapter(m_model, _outputManager);
        case 5:
            return new SubModelsDialogAdapter(m_model, _outputManager);
        default:
            break;
        }
        return nullptr;
    }
protected:
    Model *m_model = nullptr;
    OutputManager* _outputManager = nullptr;
    int m_tp;
};

class StartChannelDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    StartChannelDialogAdapter(Model *model, std::string preview)
    : wxPGEditorDialogAdapter(), m_model(model), _preview(preview) {
    }
    virtual bool DoShowDialog(wxPropertyGrid* propGrid,
                              wxPGProperty* property) override {
        StartChannelDialog dlg(propGrid);
        dlg.Set(property->GetValue().GetString(), m_model->GetModelManager(), _preview, m_model);
        if (dlg.ShowModal() == wxID_OK) {
            wxVariant v(dlg.Get());
            SetValue(v);
            return true;
        }
        return false;
    }
protected:
    Model *m_model = nullptr;
    std::string _preview;
};

class StartChannelProperty : public wxStringProperty
{
public:
    StartChannelProperty(Model* m,
                         int strand,
                         const wxString& label,
                         const wxString& name,
                         const wxString& value,
                         std::string preview) :
        wxStringProperty(label, name, value), m_model(m), _preview(preview), m_strand(strand)
    {
    }

    // Set editor to have button
    virtual const wxPGEditor* DoGetEditorClass() const override
    {
        return wxPGEditor_TextCtrlAndButton;
    }

    // Set what happens on button click
    virtual wxPGEditorDialogAdapter* GetEditorDialog() const override
    {
        return new StartChannelDialogAdapter(m_model, _preview);
    }

protected:
    Model* m_model = nullptr;
    std::string _preview;
    int m_strand = 0;
};

class ModelChainProperty : public wxStringProperty
{
public:
    ModelChainProperty(Model *m,
        const wxString& label,
        const wxString& name,
        const wxString& value)
        : wxStringProperty(label, name, value), m_model(m) {
    }
    // Set editor to have button
    virtual const wxPGEditor* DoGetEditorClass() const override {
        return wxPGEditor_TextCtrlAndButton;
    }
    // Set what happens on button click
    virtual wxPGEditorDialogAdapter* GetEditorDialog() const override {
        return new ModelChainDialogAdapter(m_model);
    }
protected:
    Model *m_model = nullptr;
};

wxArrayString Model::GetLayoutGroups(const ModelManager& mm)
{
    wxArrayString lg;
    lg.push_back("Default");
    lg.push_back("All Previews");
    lg.push_back("Unassigned");

    wxXmlNode* layouts_node = mm.GetLayoutsNode();
    for (wxXmlNode* e = layouts_node->GetChildren(); e != nullptr; e = e->GetNext())
    {
        if (e->GetName() == "layoutGroup")
        {
            wxString grp_name = e->GetAttribute("name");
            if (!grp_name.IsEmpty())
            {
                lg.push_back(grp_name.ToStdString());
            }
        }
    }

    return lg;
}

void Model::Rename(std::string const& newName) {
    name = Trim(newName);
    ModelXml->DeleteAttribute("name");
    ModelXml->AddAttribute("name", name);
}

void Model::SetStartChannel(std::string const& startChannel) {
    //wxASSERT(!StartsWith(startChannel, "!:"));

    if (startChannel == ModelXml->GetAttribute("StartChannel", "xyzzy_kw")) return;

    ModelXml->DeleteAttribute("StartChannel");
    ModelXml->AddAttribute("StartChannel", startChannel);
    ModelStartChannel = startChannel;
    if (ModelXml->GetAttribute("Advanced") == "1") {
        ModelXml->DeleteAttribute(StartChanAttrName(0));
        ModelXml->AddAttribute(StartChanAttrName(0), startChannel);
    }
    AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "Model::SetStartChannel");
    IncrementChangeCount();
}

void Model::SetProperty(wxString const& property, wxString const& value, bool apply) {
    if (ModelXml->HasAttribute(property))
    {
        ModelXml->DeleteAttribute(property);
        ModelXml->AddAttribute(property, value);
    }
    else
    {
        ModelXml->AddAttribute(property, value);
    }
    if (apply)
    {
        modelManager.GetXLightsFrame()->AbortRender();
        SetFromXml(ModelXml);
    }
}

void Model::UpdateProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {

    UpdateTypeProperties(grid);

    if (grid->GetPropertyByName("Controller") != nullptr) {
        grid->GetPropertyByName("Controller")->Enable(outputManager->GetAutoLayoutControllerNames().size() > 0);
    }

    if (HasOneString(DisplayAs) && grid->GetPropertyByName("ModelStartChannel") != nullptr) {
        grid->GetPropertyByName("ModelStartChannel")->Enable(GetControllerName() == "" || _controller == 0);
    }
    else {
        if (grid->GetPropertyByName("ModelIndividualStartChannels") != nullptr) {
            grid->GetPropertyByName("ModelIndividualStartChannels")->Enable(parm1 > 1 && (GetControllerName() == "" || _controller == 0));
            if (parm1 > 1 && (GetControllerName() != "" && _controller != 0)) {
                grid->GetPropertyByName("ModelIndividualStartChannels")->SetHelpString("Individual start channels cannot be set if you have assigned a model to a controller rather than using start channels.");
            }
            else {
                grid->GetPropertyByName("ModelIndividualStartChannels")->SetHelpString("");
            }
        }
        if (grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel") != nullptr) {
            grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel")->Enable(GetControllerName() == "" || _controller == 0);
        }
    }

    if (grid->GetPropertyByName("ModelChain") != nullptr) {
        grid->GetPropertyByName("ModelChain")->Enable(GetControllerName() != "" && GetControllerProtocol() != "" && GetControllerPort() != 0 && _controller != 0);
    }

    UpdateControllerProperties(grid);

    if (grid->GetPropertyByName("ModelStringType") != nullptr) {
        int i = grid->GetPropertyByName("ModelStringType")->GetValue().GetLong();
        if (NODE_TYPES[i] == "Single Color" || NODE_TYPES[i] == "Single Color Intensity" || NODE_TYPES[i] == "Node Single Color") {
            if (grid->GetPropertyByName("ModelStringColor") != nullptr) {
                grid->GetPropertyByName("ModelStringColor")->Enable();
            }
            if (NODE_TYPES[i] == "Node Single Color") {
                grid->GetPropertyByName("ModelStringType")->SetHelpString("This represents a string of single color LEDS which are individually controlled. These are very uncommon.");
            }
            else {
                grid->GetPropertyByName("ModelStringType")->SetHelpString("");
            }
        }
        else {
            if (grid->GetPropertyByName("ModelStringColor") != nullptr) {
                grid->GetPropertyByName("ModelStringColor")->Enable(false);
            }
        }
    }

    if (grid->GetPropertyByName("ModelRGBWHandling") != nullptr) {
        grid->GetPropertyByName("ModelRGBWHandling")->Enable(!(HasSingleChannel(StringType) || GetNodeChannelCount(StringType) != 4));
    }
}

void Model::ColourClashingChains(wxPGProperty* p)
{
    if (p == nullptr) return;

    std::string tip;
    if (GetControllerName() != "" && _controller != 0 && GetControllerProtocol() != "" && GetControllerPort() != 0 && p->IsEnabled()) {
        if (!modelManager.IsValidControllerModelChain(this, tip)) {
            p->SetHelpString(tip);
            p->SetBackgroundColour(*wxRED);
        } else {
            p->SetHelpString("");
            p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        }
    } else {
        p->SetHelpString("");
        p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
    }
}

uint32_t Model::ApplyLowDefinition(uint32_t val) const
{
    if (_lowDefFactor == 100 || !SupportsLowDefinitionRender() || !GetModelManager().GetXLightsFrame()->IsLowDefinitionRender())
        return val;
    return (val * _lowDefFactor) /  100;
}

std::string Model::GetPixelStyleDescription(PIXEL_STYLE pixelStyle)
{
    if ((int)pixelStyle < PIXEL_STYLES.size()) {
        return PIXEL_STYLES[(int)pixelStyle];
    }
    return "";
}

int Model::GetNumPhysicalStrings() const
{
    int ts = GetSmartTs();
    if (ts <= 1) {
        return parm1;
    }
    else {
        int strings = parm1 / ts;
        if (strings == 0) strings = 1;
        return strings;
    }
}

ControllerCaps* Model::GetControllerCaps() const
{
    auto c = GetController();
    if (c == nullptr) return nullptr;
    return c->GetControllerCaps();
}

Controller* Model::GetController() const
{
    std::string controller = GetControllerName();
    if (controller == "") {
        if (StartsWith(ModelStartChannel, "!") && Contains(ModelStartChannel, ":")) {
            controller = Trim(BeforeFirst(AfterFirst(ModelStartChannel, '!'), ':'));
        }
    }
    if (controller == "") {
        int32_t start;
        Controller *cp = modelManager.GetXLightsFrame()->GetOutputManager()->GetController(GetFirstChannel() + 1, start);
        if (cp != nullptr) {
            return cp;
        }
    }
    if (controller == "") return nullptr;

    return modelManager.GetXLightsFrame()->GetOutputManager()->GetController(controller);
}

void Model::AddProperties(wxPropertyGridInterface* grid, OutputManager* outputManager)
{
    LAYOUT_GROUPS = Model::GetLayoutGroups(modelManager);

    wxPGProperty* sp;

    wxPGProperty* p;
    grid->Append(new wxPropertyCategory(DisplayAs, "ModelType"));

    AddTypeProperties(grid, outputManager);

    if (SupportsLowDefinitionRender()) {
        p = grid->Append(new wxUIntProperty("Low Definition Factor", "LowDefinition", _lowDefFactor));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 100);
        p->SetEditor("SpinCtrl");
    }

    _controller = 0;
    CONTROLLERS.clear();
    CONTROLLERS.Add(USE_START_CHANNEL);
    CONTROLLERS.Add(NO_CONTROLLER);

    if (GetControllerName() == NO_CONTROLLER) {
        _controller = 1;
    }

    for (const auto& it : outputManager->GetAutoLayoutControllerNames()) {
        if (GetControllerName() == it) {
            _controller = CONTROLLERS.size();
        }
        CONTROLLERS.Add(it);
    }

    if (CONTROLLERS.size() > 1) {
        p = grid->Append(new wxEnumProperty("Controller", "Controller", CONTROLLERS, wxArrayInt(), _controller));
        p->Enable(CONTROLLERS.size() > 0);
    }

    if (HasOneString(DisplayAs)) {
        p = grid->Append(new StartChannelProperty(this, 0, "Start Channel", "ModelStartChannel", ModelXml->GetAttribute("StartChannel", "1"), modelManager.GetXLightsFrame()->GetSelectedLayoutPanelPreview()));
        p->Enable(GetControllerName() == "" || _controller == 0);
    }
    else {
        bool hasIndiv = ModelXml->GetAttribute("Advanced", "0") == "1";
        p = grid->Append(new wxBoolProperty("Indiv Start Chans", "ModelIndividualStartChannels", hasIndiv));
        p->SetAttribute("UseCheckbox", true);
        p->Enable(parm1 > 1 && (GetControllerName() == "" || _controller == 0));
        if (parm1 > 1 && (GetControllerName() != "" && _controller != 0)) {
            p->SetHelpString("Individual start channels cannot be set if you have assigned a model to a controller rather than using start channels.");
        }
        else {
            p->SetHelpString("");
        }
        sp = grid->AppendIn(p, new StartChannelProperty(this, 0, "Start Channel", "ModelStartChannel", ModelXml->GetAttribute("StartChannel", "1"), modelManager.GetXLightsFrame()->GetSelectedLayoutPanelPreview()));
        sp->Enable(GetControllerName() == "" || _controller == 0);
        if (hasIndiv) {
            int c = Model::HasOneString(DisplayAs) ? 1 : parm1;
            for (int x = 0; x < c; ++x) {
                wxString nm = StartChanAttrName(x);
                std::string val = ModelXml->GetAttribute(nm).ToStdString();
                if (val == "") {
                    val = ComputeStringStartChannel(x);
                    ModelXml->DeleteAttribute(nm);
                    ModelXml->AddAttribute(nm, val);
                }
                if (x == 0) {
                    sp->SetLabel(nm);
                    sp->SetValue(val);
                }
                else {
                    sp = grid->AppendIn(p, new StartChannelProperty(this, x, nm, nm, val, modelManager.GetXLightsFrame()->GetSelectedLayoutPanelPreview()));
                }
            }
        }
        else {
            // remove per strand start channels if individual isnt selected
            for (uint32_t x = 0; x < MOST_STRINGS_WE_EXPECT; ++x) {
                wxString nm = StartChanAttrName(x);
                ModelXml->DeleteAttribute(nm);
            }
            wxASSERT(!ModelXml->HasAttribute(StartChanAttrName(MOST_STRINGS_WE_EXPECT))); // if this fires in debug then our magic # just isnt big enough
        }
    }

    if (CONTROLLERS.size() > 1 && GetControllerName() != "" && GetControllerProtocol() != "" && GetControllerPort() != 0 && _controller != 0) {
        p = grid->Append(new ModelChainProperty(this, "Model Chain", "ModelChain", GetModelChain() == "" ? _("Beginning").ToStdString() : GetModelChain()));
        p->Enable(GetControllerName() != "" && GetControllerProtocol() != "" && GetControllerPort() != 0 && _controller != 0);
    }

    int shadowModelFor = 0;
    OTHERMODELLIST.clear();
    OTHERMODELLIST.Add("");

    for (const auto& it : modelManager) {
        auto da = it.second->GetDisplayAs();
        if (da != "ModelGroup" && it.first != GetName()) {
            if (GetShadowModelFor() == it.first) {
                shadowModelFor = OTHERMODELLIST.size();
            }
            OTHERMODELLIST.Add(it.first);
        }
    }

    grid->Append(new wxEnumProperty("Shadow Model For", "ShadowModelFor", OTHERMODELLIST, wxArrayInt(), shadowModelFor));

    int layout_group_number = 0;
    for (int grp = 0; grp < LAYOUT_GROUPS.Count(); grp++) {
        if (LAYOUT_GROUPS[grp] == layout_group) {
            layout_group_number = grp;
            break;
        }
    }

    grid->Append(new wxStringProperty("Description", "Description", description));
    grid->Append(new wxEnumProperty("Preview", "ModelLayoutGroup", LAYOUT_GROUPS, wxArrayInt(), layout_group_number));

    p = grid->Append(new PopupDialogProperty(this, outputManager, "Strand/Node Names", "ModelStrandNodeNames", CLICK_TO_EDIT, 1));
    grid->LimitPropertyEditing(p);
    p = grid->Append(new PopupDialogProperty(this, outputManager, "Faces", "ModelFaces", CLICK_TO_EDIT, 2));
    grid->LimitPropertyEditing(p);
    p = grid->Append(new PopupDialogProperty(this, outputManager, "Dimming Curves", "ModelDimmingCurves", CLICK_TO_EDIT, 3));
    grid->LimitPropertyEditing(p);
    p = grid->Append(new PopupDialogProperty(this, outputManager, "States", "ModelStates", CLICK_TO_EDIT, 4));
    grid->LimitPropertyEditing(p);
    p = grid->Append(new PopupDialogProperty(this, outputManager, "SubModels", "SubModels", CLICK_TO_EDIT, 5));
    grid->LimitPropertyEditing(p);

    auto modelGroups = modelManager.GetGroupsContainingModel(this);
    if (modelGroups.size() > 0) {
        std::string mgs;
        std::string mgscr;
        for (const auto& it : modelGroups) {
            if (mgs != "") {
                mgs += ", ";
                mgscr += "\n";
            }
            mgs += it;
            mgscr += it;
        }
        p = grid->Append(new wxStringProperty("In Model Groups", "MGS", mgs));
        p->SetHelpString(mgscr);
        p->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
        p->ChangeFlag(wxPG_PROP_READONLY, true);
    }

    AddControllerProperties(grid);

    p = grid->Append(new wxPropertyCategory("String Properties", "ModelStringProperties"));
    int i = NODE_TYPES.Index(StringType);
    if (i == wxNOT_FOUND) {
        i = NODE_TYPES.Index("Single Color");
    }
    grid->AppendIn(p, new wxEnumProperty("String Type", "ModelStringType", NODE_TYPES, wxArrayInt(), i));
    if (NODE_TYPES[i] == "Single Color" || NODE_TYPES[i] == "Single Color Intensity" || NODE_TYPES[i] == "Node Single Color") {
        //get the color
        wxColor v;
        if (StringType == "Single Color Red") {
            v = *wxRED;
        }
        else if (StringType == "Single Color Green" || StringType == "G") {
            v = *wxGREEN;
        }
        else if (StringType == "Single Color Blue" || StringType == "B") {
            v = *wxBLUE;
        }
        else if (StringType == "Single Color White" || StringType == "W") {
            v = *wxWHITE;
        }
        else if (StringType == "Single Color Custom" || StringType == "Single Color Intensity" || StringType == "Node Single Color") {
            v = customColor.asWxColor();
        }
        else if (StringType[0] == '#') {
            v = xlColor(StringType).asWxColor();
        }
        grid->AppendIn(p, new wxColourProperty("Color", "ModelStringColor", v));
        if (NODE_TYPES[i] == "Node Single Color") {
            grid->GetPropertyByName("ModelStringType")->SetHelpString("This represents a string of single color LEDS which are individually controlled. These are very uncommon.");
        }
    }
    else if (NODE_TYPES[i] == "Superstring") {
        if (superStringColours.size() == 0) {
            superStringColours.push_back(xlRED);
            SaveSuperStringColours();
        }
        sp = grid->AppendIn(p, new wxIntProperty("Colours", "SuperStringColours", superStringColours.size()));
        sp->SetAttribute("Min", 1);
        sp->SetAttribute("Max", 32);
        sp->SetEditor("SpinCtrl");
        for (int i = 0; i < superStringColours.size(); ++i) {
            grid->AppendIn(p, new wxColourProperty(wxString::Format("Colour %d", i + 1), wxString::Format("SuperStringColour%d", i), superStringColours[i].asWxColor()));
        }
    }
    else {
        sp = grid->AppendIn(p, new wxColourProperty("Color", "ModelStringColor", *wxRED));
        sp->Enable(false);
    }
    sp = grid->AppendIn(p, new wxEnumProperty("RGBW Color Handling", "ModelRGBWHandling", RGBW_HANDLING, wxArrayInt(), rgbwHandlingType));
    if (HasSingleChannel(StringType) || GetNodeChannelCount(StringType) != 4) {
        sp->Enable(false);
    }

    p = grid->Append(new wxPropertyCategory("Appearance", "ModelAppearance"));
    sp = grid->AppendIn(p, new wxBoolProperty("Active", "Active", IsActive()));
    sp->SetAttribute("UseCheckbox", true);
    sp = grid->AppendIn(p, new wxUIntProperty("Pixel Size", "ModelPixelSize", pixelSize));
    sp->SetAttribute("Min", 1);
    sp->SetAttribute("Max", 300);
    sp->SetEditor("SpinCtrl");

    grid->AppendIn(p, new wxEnumProperty("Pixel Style", "ModelPixelStyle", PIXEL_STYLES, wxArrayInt(), (int)_pixelStyle));
    sp = grid->AppendIn(p, new wxUIntProperty("Transparency", "ModelPixelTransparency", transparency));
    sp->SetAttribute("Min", 0);
    sp->SetAttribute("Max", 100);
    sp->SetEditor("SpinCtrl");
    sp = grid->AppendIn(p, new wxUIntProperty("Black Transparency", "ModelPixelBlackTransparency", blackTransparency));
    sp->SetAttribute("Min", 0);
    sp->SetAttribute("Max", 100);
    sp->SetEditor("SpinCtrl");
    grid->AppendIn(p, new wxColourProperty("Tag Color", "ModelTagColour", modelTagColour));
    UpdateControllerProperties(grid);
    DisableUnusedProperties(grid);
}

void Model::ClearIndividualStartChannels()
{
    // dont clear custom models
    if (IsCustom()) return;

    ModelXml->DeleteAttribute("Advanced");
    // remove per strand start channels if individual isnt selected
    for (int x = 0; x < MOST_STRINGS_WE_EXPECT; ++x) {
        ModelXml->DeleteAttribute(StartChanAttrName(x));
    }
    wxASSERT(!ModelXml->HasAttribute(StartChanAttrName(MOST_STRINGS_WE_EXPECT))); // if this fires then # is not the right magic number
}

void Model::GetControllerProtocols(wxArrayString& cp, int& idx)
{
    auto caps = GetControllerCaps();
    Controller *c = GetController();
    wxString protocol = GetControllerProtocol();
    if (c) {
        ControllerSerial *cs = dynamic_cast<ControllerSerial*>(c);
        if (cs) {
            wxString cprotocol = cs->GetProtocol();
            if (cprotocol != protocol) {
                GetControllerConnection()->DeleteAttribute("Protocol");
                GetControllerConnection()->AddAttribute("Protocol", cprotocol);
                clearUnusedProtocolProperties(GetControllerConnection());
            }
            cp.push_back(cprotocol);
            idx = 0;
            return;
        }
    }
    wxString protocolLC = protocol;
    protocolLC.LowerCase();

    if (caps == nullptr) {
        for (const auto& it : GetAllPixelTypes(true, false)) {
            cp.push_back(it);
        }
    } else {
        auto controllerProtocols = caps->GetAllProtocols();
        for (const auto& it : GetAllPixelTypes(controllerProtocols, true, true, true)) {
            cp.push_back(it);
        }
    }

    // if this protocol is not supported by the controller ... choose a compatible one if one exists ... otherwise we blank it out
    if (std::find(begin(cp), end(cp), protocol) == end(cp) && std::find(begin(cp), end(cp), protocolLC) == end(cp)) {
        // not in the list ... maybe there is a compatible protocol we can choose
        std::string np = "";
        if (caps != nullptr) {
            auto controllerProtocols = caps->GetAllProtocols();
            if (::IsPixelProtocol(protocol)) {
                np = ChooseBestControllerPixel(controllerProtocols, protocol);
            } else {
                np = ChooseBestControllerSerial(controllerProtocols, protocol);
            }
        }
        if (protocol != np) SetControllerProtocol(np);
    }

    // now work out the index
    int i = 0;
    for (const auto& it : cp) {
        if (it == protocol || it == protocolLC) {
            idx = i;
            break;
        }
        i++;
    }
}

wxArrayString Model::GetSmartRemoteValues(int smartRemoteCount)
{
    wxArrayString res;
    //res.push_back("None");
    for (int i = 0; i < smartRemoteCount; i++) {
        res.push_back(wxString((char)(65 + i)));
    }
    return res;
}

void Model::AddControllerProperties(wxPropertyGridInterface* grid)
{

    auto caps = GetControllerCaps();

    wxString protocol = GetControllerProtocol();

    wxPGProperty* p = grid->Append(new wxPropertyCategory("Controller Connection", "ModelControllerConnectionProperties"));

    wxPGProperty* sp = grid->AppendIn(p, new wxUIntProperty("Port", "ModelControllerConnectionPort", GetControllerPort(1)));
    sp->SetAttribute("Min", 0);
    if (caps == nullptr || protocol == "") {
        sp->SetAttribute("Max", MOST_CONTROLLER_PORTS_WE_EXPECT);
    } else {
        if (IsSerialProtocol()) {
            sp->SetAttribute("Max", caps->GetMaxSerialPort());
        } else if (IsPixelProtocol()) {
            sp->SetAttribute("Max", caps->GetMaxPixelPort());
        } else if (IsLEDPanelMatrixProtocol()) {
            sp->SetAttribute("Max", caps->GetMaxLEDPanelMatrixPort());
        } else if (IsVirtualMatrixProtocol()) {
            sp->SetAttribute("Max", caps->GetMaxVirtualMatrixPort());
        } else {
            sp->SetAttribute("Max", MOST_CONTROLLER_PORTS_WE_EXPECT);
        }
    }
    sp->SetEditor("SpinCtrl");

    wxArrayString cp;
    int idx = -1;
    GetControllerProtocols(cp, idx);

    if (IsPixelProtocol()) {
        int smartRemoteCount = 15;
        if (caps != nullptr) {
            smartRemoteCount = caps->GetSmartRemoteCount();
        }
        if (smartRemoteCount != 0)
        {
            int sr = GetSmartRemote();

            sp = grid->AppendIn(p, new wxBoolProperty("Use Smart Remote", "UseSmartRemote", sr));
            sp->SetAttribute("UseCheckbox", true);
            p->SetHelpString("Enable Smart Remote for this Model.");

            if (sr != 0) {
                if (GetSmartRemote() != 0) {
                    auto const& srTypes = GetSmartRemoteTypes();
                    if (srTypes.size() > 1) {
                        wxArrayString srlist;
                        for (auto const& typ : srTypes) {
                            srlist.Add(typ);
                        }
                        grid->AppendIn(p, new wxEnumProperty("Smart Remote Type", "SmartRemoteType", srlist, wxArrayInt(), GetSmartRemoteTypeIndex(GetSmartRemoteType())));
                    }
                    else {
                        std::string type = GetSmartRemoteType();
                        auto smt = grid->AppendIn(p, new wxStringProperty("Smart Remote Type", "SmartRemoteType", type));
                        smt->ChangeFlag(wxPG_PROP_READONLY, true);
                        smt->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
                    }
                }

                wxArrayString srv = GetSmartRemoteValues(smartRemoteCount);
                grid->AppendIn(p, new wxEnumProperty("Smart Remote", "SmartRemote", srv, wxArrayInt(), sr-1));

                if (GetNumPhysicalStrings() > 1 )
                {
                    sp = grid->AppendIn(p, new wxUIntProperty("Max Cascade Remotes", "MaxCascadeRemotes", GetSRMaxCascade()));
                    sp->SetAttribute("Min", 1);
                    sp->SetAttribute("Max", smartRemoteCount);
                    p->SetHelpString("This is the number of smart remotes on a chain to use so if start is B and this is 2 then B and C remotes will be used.");
                    sp->SetEditor("SpinCtrl");

                    sp = grid->AppendIn(p, new wxBoolProperty("Cascade On Port", "CascadeOnPort", GetSRCascadeOnPort()));
                    sp->SetAttribute("UseCheckbox", true);
                    p->SetHelpString("When selected order is 1A 1B 1C etc. When not selected order is 1A 2A 3A 4A 1B etc.");
                }
            }
        }
    }

    if (cp.size() > 0) {
        sp = grid->AppendIn(p, new wxEnumProperty("Protocol", "ModelControllerConnectionProtocol", cp, wxArrayInt(), idx));
        if (cp.size() == 1 && idx == 0) {
            grid->DisableProperty(sp);
        }
    }

    wxXmlNode* node = GetControllerConnection();
    if (IsSerialProtocol()) {
        int chan = wxAtoi(node->GetAttribute("channel", "1"));
        sp = grid->AppendIn(p, new wxUIntProperty(protocol + " Channel", "ModelControllerConnectionDMXChannel", chan));
        sp->SetAttribute("Min", 1);
        if (caps == nullptr) {
            sp->SetAttribute("Max", 512);
        }
        else {
            sp->SetAttribute("Max", caps->GetMaxSerialPortChannels());
        }
        sp->SetEditor("SpinCtrl");
    } else if (IsPixelProtocol()) {

        if (caps == nullptr || caps->SupportsPixelPortNullPixels()) {
            sp = grid->AppendIn(p, new wxBoolProperty("Set Start Null Pixels", "ModelControllerConnectionPixelSetNullNodes", node->HasAttribute("nullNodes")));
            sp->SetAttribute("UseCheckbox", true);
            auto sp2 = grid->AppendIn(sp, new wxUIntProperty("Start Null Pixels", "ModelControllerConnectionPixelNullNodes",
                wxAtoi(GetControllerConnection()->GetAttribute("nullNodes", "0"))));
            sp2->SetAttribute("Min", 0);
            sp2->SetAttribute("Max", 100);
            sp2->SetEditor("SpinCtrl");
            if (!node->HasAttribute("nullNodes")) {
                grid->DisableProperty(sp2);
                grid->Collapse(sp);
            }
        }

        if (caps == nullptr || caps->SupportsPixelPortEndNullPixels()) {
            sp = grid->AppendIn(p, new wxBoolProperty("Set End Null Pixels", "ModelControllerConnectionPixelSetEndNullNodes", node->HasAttribute("endNullNodes")));
            sp->SetAttribute("UseCheckbox", true);
            auto sp2 = grid->AppendIn(sp, new wxUIntProperty("End Null Pixels", "ModelControllerConnectionPixelEndNullNodes",
                wxAtoi(GetControllerConnection()->GetAttribute("endNullNodes", "0"))));
            sp2->SetAttribute("Min", 0);
            sp2->SetAttribute("Max", 100);
            sp2->SetEditor("SpinCtrl");
            if (!node->HasAttribute("endNullNodes")) {
                grid->DisableProperty(sp2);
                grid->Collapse(sp);
            }
        }

        if (caps == nullptr || caps->SupportsPixelPortBrightness()) {
            sp = grid->AppendIn(p, new wxBoolProperty("Set Brightness", "ModelControllerConnectionPixelSetBrightness", node->HasAttribute("brightness")));
            sp->SetAttribute("UseCheckbox", true);
            auto sp2 = grid->AppendIn(sp, new wxUIntProperty("Brightness", "ModelControllerConnectionPixelBrightness",
                wxAtoi(GetControllerConnection()->GetAttribute("brightness", "100"))));
            sp2->SetAttribute("Min", 0);
            sp2->SetAttribute("Max", 100);
            sp2->SetEditor("SpinCtrl");
            if (!node->HasAttribute("brightness")) {
                grid->DisableProperty(sp2);
                grid->Collapse(sp);
            }
        }

        if (caps == nullptr || caps->SupportsPixelPortGamma()) {
            sp = grid->AppendIn(p, new wxBoolProperty("Set Gamma", "ModelControllerConnectionPixelSetGamma", node->HasAttribute("gamma")));
            sp->SetAttribute("UseCheckbox", true);
            double gamma = wxAtof(GetControllerConnection()->GetAttribute("gamma", "1.0"));
            auto sp2 = grid->AppendIn(sp, new wxFloatProperty("Gamma", "ModelControllerConnectionPixelGamma", gamma));
            sp2->SetAttribute("Min", 0.1);
            sp2->SetAttribute("Max", 5.0);
            sp2->SetAttribute("Precision", 1);
            sp2->SetAttribute("Step", 0.1);
            sp2->SetEditor("SpinCtrl");
            if (!node->HasAttribute("gamma")) {
                grid->DisableProperty(sp2);
                grid->Collapse(sp);
            }
        }

        if (caps == nullptr || caps->SupportsPixelPortColourOrder()) {
            sp = grid->AppendIn(p, new wxBoolProperty("Set Color Order", "ModelControllerConnectionPixelSetColorOrder", node->HasAttribute("colorOrder")));
            sp->SetAttribute("UseCheckbox", true);
            int cidx = CONTROLLER_COLORORDER.Index(GetControllerConnection()->GetAttribute("colorOrder", "RGB"));
            auto sp2 = grid->AppendIn(sp, new wxEnumProperty("Color Order", "ModelControllerConnectionPixelColorOrder", CONTROLLER_COLORORDER, wxArrayInt(), cidx < 0 ? 0 : cidx));
            if (!node->HasAttribute("colorOrder")) {
                grid->DisableProperty(sp2);
                grid->Collapse(sp);
            }
        }

        if (caps == nullptr || caps->SupportsPixelPortDirection()) {
            sp = grid->AppendIn(p, new wxBoolProperty("Set Pixel Direction", "ModelControllerConnectionPixelSetDirection", node->HasAttribute("reverse")));
            sp->SetAttribute("UseCheckbox", true);
            auto sp2 = grid->AppendIn(sp, new wxEnumProperty("Direction", "ModelControllerConnectionPixelDirection", CONTROLLER_DIRECTION, wxArrayInt(),
                wxAtoi(GetControllerConnection()->GetAttribute("reverse", "0"))));
            if (!node->HasAttribute("reverse")) {
                grid->DisableProperty(sp2);
                grid->Collapse(sp);
            }
        }

        if (caps == nullptr || caps->SupportsPixelPortGrouping()) {
            sp = grid->AppendIn(p, new wxBoolProperty("Set Group Count", "ModelControllerConnectionPixelSetGroupCount", node->HasAttribute("groupCount")));
            sp->SetAttribute("UseCheckbox", true);
            auto sp2 = grid->AppendIn(sp, new wxUIntProperty("Group Count", "ModelControllerConnectionPixelGroupCount",
                                                             wxAtoi(GetControllerConnection()->GetAttribute("groupCount", "1"))));
            sp2->SetAttribute("Min", 0);
            sp2->SetAttribute("Max", 500);
            sp2->SetEditor("SpinCtrl");
            if (!node->HasAttribute("groupCount")) {
                grid->DisableProperty(sp2);
                grid->Collapse(sp);
            }
        }

        if (caps == nullptr || caps->SupportsPixelZigZag()) {
            sp = grid->AppendIn(p, new wxBoolProperty("Set Zig Zag", "ModelControllerConnectionPixelSetZigZag", node->HasAttribute("zigZag")));
            sp->SetAttribute("UseCheckbox", true);
            auto sp2 = grid->AppendIn(sp, new wxUIntProperty("Zig Zag", "ModelControllerConnectionPixelZigZag",
                                                             wxAtoi(GetControllerConnection()->GetAttribute("zigZag", "0"))));
            sp2->SetAttribute("Min", 0);
            sp2->SetAttribute("Max", 1000);
            sp2->SetEditor("SpinCtrl");
            if (!node->HasAttribute("zigZag")) {
                grid->DisableProperty(sp2);
                grid->Collapse(sp);
            }
        }

        if (caps == nullptr || caps->SupportsTs()) {
            sp = grid->AppendIn(p, new wxBoolProperty("Set Smart Ts", "ModelControllerConnectionPixelSetTs", node->HasAttribute("ts")));
            sp->SetAttribute("UseCheckbox", true);
            auto sp2 = grid->AppendIn(sp, new wxUIntProperty("Smart Ts", "ModelControllerConnectionPixelTs",
                GetSmartTs()));
            sp2->SetAttribute("Min", 0);
            sp2->SetAttribute("Max", 20);
            sp2->SetEditor("SpinCtrl");
            if (!node->HasAttribute("ts")) {
                grid->DisableProperty(sp2);
                grid->Collapse(sp);
            }
        }
    }
}

void Model::UpdateControllerProperties(wxPropertyGridInterface* grid) {

    auto caps = GetControllerCaps();

    auto p = grid->GetPropertyByName("ModelControllerConnectionPort");
    if (p != nullptr) {
        if (GetControllerName() != "" && _controller != 0 && GetControllerPort(1) == 0) {
            if (caps != nullptr && caps->GetMaxPixelPort() == 0 && caps->GetMaxSerialPort() == 0) {
                // we let this be 0
            } else {
                p->SetHelpString("When using controller name instead of start channels then port must be specified if the controller has ports.");
                p->SetBackgroundColour(*wxRED);
            }
        } else {
            p->SetHelpString("");
            p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        }
    }

    p = grid->GetPropertyByName("ModelControllerConnectionProtocol");
    if (p != nullptr) {
        if (GetControllerName() != "" && _controller != 0 && (GetControllerPort() == 0 || GetControllerProtocol() == "")) {
            p->SetHelpString("When using controller name instead of start channels then protocol must be specified.");
            p->SetBackgroundColour(*wxRED);
        }
        else {
            p->SetHelpString("");
            grid->GetPropertyByName("ModelControllerConnectionProtocol")->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        }
    }

    if (grid->GetPropertyByName("ModelChain") != nullptr) {
        ColourClashingChains(grid->GetPropertyByName("ModelChain"));
    }

    wxXmlNode* node = GetControllerConnection();
    if (IsPixelProtocol()) {
        if (grid->GetPropertyByName("ModelControllerConnectionPixelSetNullNodes") != nullptr)
        {
            if (!node->HasAttribute("nullNodes")) {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetNullNodes")->SetExpanded(false);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetNullNodes.ModelControllerConnectionPixelNullNodes")->Enable(false);
            }
            else {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetNullNodes")->SetExpanded(true);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetNullNodes.ModelControllerConnectionPixelNullNodes")->Enable();
            }
        }

        if (grid->GetPropertyByName("ModelControllerConnectionPixelSetEndNullNodes") != nullptr) {
            if (!node->HasAttribute("endNullNodes")) {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetEndNullNodes")->SetExpanded(false);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetEndNullNodes.ModelControllerConnectionPixelEndNullNodes")->Enable(false);
            }
            else {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetEndNullNodes")->SetExpanded(true);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetEndNullNodes.ModelControllerConnectionPixelEndNullNodes")->Enable();
            }
        }

        if (grid->GetPropertyByName("ModelControllerConnectionPixelSetBrightness") != nullptr)
        {
            if (!node->HasAttribute("brightness")) {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetBrightness")->SetExpanded(false);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetBrightness.ModelControllerConnectionPixelBrightness")->Enable(false);
            }
            else {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetBrightness")->SetExpanded(true);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetBrightness.ModelControllerConnectionPixelBrightness")->Enable();
            }
        }

        if (grid->GetPropertyByName("ModelControllerConnectionPixelSetGamma") != nullptr)
        {
            if (!node->HasAttribute("gamma")) {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetGamma")->SetExpanded(false);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetGamma.ModelControllerConnectionPixelGamma")->Enable(false);
            }
            else {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetGamma")->SetExpanded(true);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetGamma.ModelControllerConnectionPixelGamma")->Enable();
            }
        }

        if (grid->GetPropertyByName("ModelControllerConnectionPixelSetColorOrder") != nullptr)
        {
            if (!node->HasAttribute("colorOrder")) {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetColorOrder")->SetExpanded(false);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetColorOrder.ModelControllerConnectionPixelColorOrder")->Enable(false);
            }
            else {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetColorOrder")->SetExpanded(true);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetColorOrder.ModelControllerConnectionPixelColorOrder")->Enable();
            }
        }

        if (grid->GetPropertyByName("ModelControllerConnectionPixelSetDirection") != nullptr)
        {
            if (!node->HasAttribute("reverse")) {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetDirection")->SetExpanded(false);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetDirection.ModelControllerConnectionPixelDirection")->Enable(false);
            }
            else {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetDirection")->SetExpanded(true);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetDirection.ModelControllerConnectionPixelDirection")->Enable();
            }
        }

        if (grid->GetPropertyByName("ModelControllerConnectionPixelSetGroupCount") != nullptr)
        {
            if (!node->HasAttribute("groupCount")) {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetGroupCount")->SetExpanded(false);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetGroupCount.ModelControllerConnectionPixelGroupCount")->Enable(false);
            }
            else {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetGroupCount")->SetExpanded(true);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetGroupCount.ModelControllerConnectionPixelGroupCount")->Enable();
            }
        }

        if (grid->GetPropertyByName("ModelControllerConnectionPixelSetZigZag") != nullptr) {
            if (!node->HasAttribute("zigZag")) {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetZigZag")->SetExpanded(false);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetZigZag.ModelControllerConnectionPixelZigZag")->Enable(false);
            } else {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetZigZag")->SetExpanded(true);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetZigZag.ModelControllerConnectionPixelZigZag")->Enable();
            }
        }

        if (grid->GetPropertyByName("ModelControllerConnectionPixelSetTs") != nullptr) {
            if (!node->HasAttribute("ts")) {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetTs")->SetExpanded(false);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetTs.ModelControllerConnectionPixelTs")->Enable(false);
            }
            else {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetTs")->SetExpanded(true);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetTs.ModelControllerConnectionPixelTs")->Enable();
            }
        }
    }
    grid->RefreshGrid();
}

static wxString GetColorString(wxPGProperty *p, xlColor &xc) {
    wxString tp = "Single Color Custom";
    if (p != nullptr) {
        wxColour c;
        c << p->GetValue();
        if (c == *wxRED) {
            tp = "Single Color Red";
        }
        else if (c == *wxBLUE) {
            tp = "Single Color Blue";
        }
        else if (c == *wxGREEN) {
            tp = "Single Color Green";
        }
        else if (c == *wxWHITE) {
            tp = "Single Color White";
        }
        else {
            xc = c;
        }
    }
    return tp;
}

bool Model::HasIndividualStartChannels() const
{
    return ModelXml->GetAttribute("Advanced", "0") == "1";
}

wxString Model::GetIndividualStartChannel(size_t s) const
{
    return ModelXml->GetAttribute(StartChanAttrName(s), "");
}

int Model::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    auto caps = GetControllerCaps();

    modelManager.GetXLightsFrame()->AddTraceMessage("Model::OnPropertyGridChange : " + event.GetPropertyName() + " : " + (event.GetValue().GetType() == "string" ? event.GetValue().GetString() : "N/A") + " : " + (event.GetValue().GetType() == "long" ? wxString::Format("%ld", event.GetValue().GetLong()) : "N/A"));

    if (HandleLayerSizePropertyChange(grid, event))
    {
        return 0;
    }

    if (event.GetPropertyName() == "ModelPixelSize") {
        pixelSize = event.GetValue().GetLong();
        ModelXml->DeleteAttribute("PixelSize");
        ModelXml->AddAttribute("PixelSize", wxString::Format(wxT("%i"), pixelSize));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelPixelSize");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Model::OnPropertyGridChange::ModelPixelSize");
        IncrementChangeCount();
        return 0;
    } else if (event.GetPropertyName() == "ModelPixelStyle") {
        _pixelStyle = (PIXEL_STYLE)event.GetValue().GetLong();
        ModelXml->DeleteAttribute("Antialias");
        ModelXml->AddAttribute("Antialias", wxString::Format(wxT("%i"), (int)_pixelStyle));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelPixelStyle");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Model::OnPropertyGridChange::ModelPixelStyle");
        return 0;
    } else if (event.GetPropertyName() == "ModelPixelTransparency") {
        transparency = event.GetValue().GetLong();
        ModelXml->DeleteAttribute("Transparency");
        ModelXml->AddAttribute("Transparency", wxString::Format(wxT("%i"), transparency));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelPixelTransparency");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Model::OnPropertyGridChange::ModelPixelTransparency");
        return 0;
    } else if (event.GetPropertyName() == "ModelPixelBlackTransparency") {
        blackTransparency = event.GetValue().GetLong();
        ModelXml->DeleteAttribute("BlackTransparency");
        ModelXml->AddAttribute("BlackTransparency", wxString::Format(wxT("%i"), blackTransparency));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelPixelBlackTransparency");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Model::OnPropertyGridChange::ModelPixelBlackTransparency");
        return 0;
    } else if (event.GetPropertyName() == "LowDefinition") {
        _lowDefFactor = event.GetValue().GetLong();
        ModelXml->DeleteAttribute("LowDefinition");
        ModelXml->AddAttribute("LowDefinition", wxString::Format(wxT("%i"), _lowDefFactor));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::LowDefinition");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "MatrixModel::OnPropertyGridChange::LowDefinition");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "MatrixModel::OnPropertyGridChange::LowDefinition");
        return 0;
    } else if (event.GetPropertyName() == "ModelTagColour") {
        modelTagColour << event.GetProperty()->GetValue();
        ModelXml->DeleteAttribute("TagColour");
        ModelXml->AddAttribute("TagColour", modelTagColour.GetAsString());
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelTagColour");
        return 0;
    } else if (event.GetPropertyName() == "ModelStrandNodeNames") {
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Model::OnPropertyGridChange::ModelStrandNames");
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelStrandNames");
        return 0;
    } else if (event.GetPropertyName() == "ModelDimmingCurves") {
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Model::OnPropertyGridChange::ModelDimmingCurves");
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelDimmingCurves");
        return 0;
    } else if (event.GetPropertyName() == "ModelChain") {
        std::string modelChain = event.GetValue().GetString();
        if (modelChain == "Beginning")
        {
            modelChain = "";
        }
        SetModelChain(modelChain);
        if (modelChain != "")
        {
            ModelXml->DeleteAttribute("Advanced");
            AdjustStringProperties(grid, parm1);
            if (grid->GetPropertyByName("ModelStartChannel") != nullptr)
            {
                grid->GetPropertyByName("ModelStartChannel")->SetValue(ModelXml->GetAttribute("StartChannel", "1"));
                grid->GetPropertyByName("ModelStartChannel")->Enable(false);
            }
            else if (grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel") != nullptr)
            {
                grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel")->Enable(false);
                grid->GetPropertyByName("ModelIndividualStartChannels")->Enable(false);
                grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel")->SetValue(ModelXml->GetAttribute("StartChannel", "1"));
            }
        }
        else
        {
            SetStartChannel("1");
            if (grid->GetPropertyByName("ModelStartChannel") != nullptr)
            {
                grid->GetPropertyByName("ModelStartChannel")->Enable();
            }
            else if (grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel") != nullptr)
            {
                grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel")->Enable();
                grid->GetPropertyByName("ModelIndividualStartChannels")->Enable();
            }
        }
        UpdateControllerProperties(grid);
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Model::OnPropertyGridChange::ModelChain");
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelChain");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::OnPropertyGridChange::ModelChain");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::ModelChain");
        AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::OnPropertyGridChange::ModelChain");
        return 0;
    }
    else if (event.GetPropertyName() == "ShadowModelFor") {
        if (GetShadowModelFor() != OTHERMODELLIST[event.GetValue().GetInteger()]) {
            SetShadowModelFor(OTHERMODELLIST[event.GetValue().GetInteger()]);
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::Controller");
        return 0;
    }
    else if (event.GetPropertyName() == "Controller") {
        if (GetControllerName() != CONTROLLERS[event.GetValue().GetInteger()]) {
            SetControllerName(CONTROLLERS[event.GetValue().GetInteger()]);
            if (GetControllerPort() != 0 && IsPixelProtocol()) {
                SetModelChain(">" + modelManager.GetLastModelOnPort(CONTROLLERS[event.GetValue().GetInteger()], GetControllerPort(), GetName(), GetControllerProtocol()));
            } else {
                SetModelChain("");
            }
        }
        if (GetControllerName() == "") {
            SetModelChain("");
            // dont set start channel to one. that way model holds its previously calculated start channel
            if (grid->GetPropertyByName("ModelStartChannel") != nullptr) {
                grid->GetPropertyByName("ModelStartChannel")->Enable();
            } else if (grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel") != nullptr) {
                grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel")->Enable();
                grid->GetPropertyByName("ModelIndividualStartChannels")->Enable();
            }
        } else {
            ModelXml->DeleteAttribute("Advanced");
            AdjustStringProperties(grid, parm1);
            if (grid->GetPropertyByName("ModelStartChannel") != nullptr) {
                grid->GetPropertyByName("ModelStartChannel")->SetValue(ModelXml->GetAttribute("StartChannel", "1"));
                grid->GetPropertyByName("ModelStartChannel")->Enable(false);
            } else if (grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel") != nullptr) {
                grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel")->Enable(false);
                grid->GetPropertyByName("ModelIndividualStartChannels")->Enable(false);
                grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel")->SetValue(ModelXml->GetAttribute("StartChannel", "1"));
            }
        }
        if (grid->GetPropertyByName("ModelChain") != nullptr) {
            grid->GetPropertyByName("ModelChain")->Enable(GetControllerName() != "" && _controller != 0);
        }
        UpdateControllerProperties(grid);
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::Controller");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::OnPropertyGridChange::Controller");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Model::OnPropertyGridChange::Controller");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::Controller");
        AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::OnPropertyGridChange::Controller");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPort") {

        bool protocolChanged = false;
        if (GetControllerPort() != event.GetValue().GetLong()) {
            SetControllerPort(event.GetValue().GetLong());

            if (GetControllerPort(1) > 0 && GetControllerProtocol() == "") {
                if (caps == nullptr) {
                    SetControllerProtocol(GetAllPixelTypes()[1]);
                } else {
                    if (caps->GetPixelProtocols().size() > 0) {
                        SetControllerProtocol(caps->GetPixelProtocols().front());
                    } else if (caps->GetSerialProtocols().size() > 0) {
                        SetControllerProtocol(caps->GetSerialProtocols().front());
                    } else if (caps->SupportsVirtualMatrix()) {
                        SetControllerProtocol("Virtual Matrix");
                    } else if (caps->SupportsLEDPanelMatrix()) {
                        SetControllerProtocol("LED Panel Matrix");
                    }
                }
                protocolChanged = true;
            }

            if (GetControllerName() != "" && _controller != 0) {
                if (grid->GetPropertyByName("ModelStartChannel") != nullptr) {
                    grid->GetPropertyByName("ModelStartChannel")->SetValue(ModelXml->GetAttribute("StartChannel", "1"));
                } else if (grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel") != nullptr) {
                    grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel")->SetValue(ModelXml->GetAttribute("StartChannel", "1"));
                }
                if (IsPixelProtocol()) {
                    // we only do this for pixels as dmx have the channel to base it off
                    // when the port changes we have to assume any existing model chain will break
                    SetModelChain(">" + modelManager.GetLastModelOnPort(GetControllerName(), event.GetValue().GetLong(), GetName(), GetControllerProtocol()));
                } else {
                    SetModelChain("Beginning");
                }
            }
        }

        if (grid->GetPropertyByName("ModelChain") != nullptr) {
            grid->GetPropertyByName("ModelChain")->Enable(GetControllerName() != "" && GetControllerProtocol() != "" && GetControllerPort() != 0 && _controller != 0);
        }

        UpdateControllerProperties(grid);
        if (protocolChanged) {
            // need to refresh to add protocol specific options
            AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Model::OnPropertyGridChange::ModelControllerConnectionPort");
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPort");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::OnPropertyGridChange::ModelControllerConnectionPort");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Model::OnPropertyGridChange::ModelControllerConnectionPort");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::ModelControllerConnectionPort");
        AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::OnPropertyGridChange::ModelControllerConnectionPort");
        return 0;
    } else if (event.GetPropertyName() == "Active") {
        SetActive(event.GetValue().GetBool());
        IncrementChangeCount();
        return 0;
    }
    else if (event.GetPropertyName() == "UseSmartRemote") {
        auto usr = event.GetValue().GetBool();
        if (!usr) {
            SetSmartRemote(0);
        } else {
            SetSmartRemote(1);
        }
        return 0;
    } else if (event.GetPropertyName() == "SmartRemote") {
        int sr = wxAtoi(event.GetValue().GetString());
        SetSmartRemote(sr + 1);
        return 0;
    } else if (event.GetPropertyName() == "CascadeOnPort") {
        SetSRCascadeOnPort(event.GetValue().GetBool());
        return 0;
    } else if (event.GetPropertyName() == "MaxCascadeRemotes") {
        SetSRMaxCascade(event.GetValue().GetLong());
        return 0;
    } else if (event.GetPropertyName() == "SmartRemoteType") {
        SetSmartRemoteType(GetSmartRemoteTypeName(wxAtoi(event.GetValue().GetString())));
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionProtocol") {
        wxArrayString cp;
        int idx;
        GetControllerProtocols(cp, idx);
        std::string oldProtocol = GetControllerProtocol();

        // This may be why i see some crashes here
        if (event.GetValue().GetLong() >= cp.size()) {
            logger_base.crit("Protocol being set is not in the controller protocols which has %d protocols.", (int)cp.size());
            return 0;
        }

        SetControllerProtocol(cp[event.GetValue().GetLong()]);

        clearUnusedProtocolProperties(GetControllerConnection());

        if (GetControllerName() != "" && _controller != 0) {
            if (grid->GetPropertyByName("ModelStartChannel") != nullptr) {
                grid->GetPropertyByName("ModelStartChannel")->SetValue(ModelXml->GetAttribute("StartChannel", "1"));
            }
            else if (grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel") != nullptr) {
                grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel")->SetValue(ModelXml->GetAttribute("StartChannel", "1"));
            }
        }
        if (grid->GetPropertyByName("ModelChain") != nullptr) {
            grid->GetPropertyByName("ModelChain")->Enable(GetControllerName() != "" && GetControllerProtocol() != "" && GetControllerPort() != 0 && _controller != 0);
        }

        UpdateControllerProperties(grid);
        std::string newProtocol = GetControllerProtocol();

        if (!::IsPixelProtocol(newProtocol)) {
            if (GetControllerConnection()->GetAttribute("channel", "-1") == "-1") {
                GetControllerConnection()->AddAttribute("channel", "1");
            }
        }
        if (
            //FIXME-Matrix
            (::IsSerialProtocol(newProtocol) && ::IsPixelProtocol(oldProtocol)) ||
            (::IsSerialProtocol(oldProtocol) && ::IsPixelProtocol(newProtocol)) ||
            (oldProtocol == "" && newProtocol != "") ||
            (newProtocol == "" && oldProtocol != "")) {
            // if we switch between a DMX and pixel protocol we need to rebuild the properties
            AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Model::OnPropertyGridChange::ModelControllerConnectionProtocol");
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionProtocol");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::OnPropertyGridChange::ModelControllerConnectionProtocol");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::ModelControllerConnectionProtocol");
        AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::OnPropertyGridChange::ModelControllerConnectionProtocol");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionDMXChannel") {
        SetControllerDMXChannel((int)event.GetValue().GetLong());
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetBrightness") {
        GetControllerConnection()->DeleteAttribute("brightness");
        wxPGProperty *prop = grid->GetFirstChild(event.GetProperty());
        grid->EnableProperty(prop, event.GetValue().GetBool());
        if (event.GetValue().GetBool()) {
            GetControllerConnection()->AddAttribute("brightness", "100");
            prop->SetValueFromInt(100);
            grid->Expand(event.GetProperty());
        } else {
            grid->Collapse(event.GetProperty());
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetBrightness");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetBrightness");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetBrightness");
        AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetBrightness");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetBrightness.ModelControllerConnectionPixelBrightness") {
        GetControllerConnection()->DeleteAttribute("brightness");
        if (event.GetValue().GetLong() > 0) {
            GetControllerConnection()->AddAttribute("brightness", wxString::Format("%i", (int)event.GetValue().GetLong()));
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelBrightness");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::OnPropertyGridChange::ModelControllerConnectionPixelBrightness");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::ModelControllerConnectionBrightness");
        AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::OnPropertyGridChange::ModelControllerConnectionBrightness");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetGamma") {
        GetControllerConnection()->DeleteAttribute("gamma");
        wxPGProperty *prop = grid->GetFirstChild(event.GetProperty());
        grid->EnableProperty(prop, event.GetValue().GetBool());
        if (event.GetValue().GetBool()) {
            GetControllerConnection()->AddAttribute("gamma", "1.0");
            prop->SetValue(1.0f);
            grid->Expand(event.GetProperty());
        } else {
            grid->Collapse(event.GetProperty());
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetGamma");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetGamma");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetGamma");
        AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetGamma");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetGamma.ModelControllerConnectionPixelGamma") {
        GetControllerConnection()->DeleteAttribute("gamma");
        GetControllerConnection()->AddAttribute("gamma", wxString::Format("%g", (float)event.GetValue().GetDouble()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelGamma");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::OnPropertyGridChange::ModelControllerConnectionPixelGamma");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::ModelControllerConnectionPixelGamma");
        AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::OnPropertyGridChange::ModelControllerConnectionPixelGamma");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetDirection") {
        GetControllerConnection()->DeleteAttribute("reverse");
        wxPGProperty *prop = grid->GetFirstChild(event.GetProperty());
        grid->EnableProperty(prop, event.GetValue().GetBool());
        if (event.GetValue().GetBool()) {
            GetControllerConnection()->AddAttribute("reverse", "0");
            prop->SetValueFromInt(0);
            grid->Expand(event.GetProperty());
        } else {
            grid->Collapse(event.GetProperty());
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetDirection");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetDirection");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetDirection");
        AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetDirection");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetDirection.ModelControllerConnectionPixelDirection") {
        GetControllerConnection()->DeleteAttribute("reverse");
        if (event.GetValue().GetLong() > 0) {
            GetControllerConnection()->AddAttribute("reverse", wxString::Format("%i", (int)event.GetValue().GetLong()));
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelDirection");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::OnPropertyGridChange::ModelControllerConnectionPixelDirection");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::ModelControllerConnectionPixelDirection");
        AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::OnPropertyGridChange::ModelControllerConnectionPixelDirection");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetColorOrder") {
        GetControllerConnection()->DeleteAttribute("colorOrder");
        wxPGProperty *prop = grid->GetFirstChild(event.GetProperty());
        grid->EnableProperty(prop, event.GetValue().GetBool());
        if (event.GetValue().GetBool()) {
            GetControllerConnection()->AddAttribute("colorOrder", "RGB");
            prop->SetValueFromString("RGB");
            grid->Expand(event.GetProperty());
        } else {
            grid->Collapse(event.GetProperty());
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetColorOrder");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetColorOrder");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetColorOrder");
        AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetColorOrder");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetColorOrder.ModelControllerConnectionPixelColorOrder") {
        GetControllerConnection()->DeleteAttribute("colorOrder");
        GetControllerConnection()->AddAttribute("colorOrder", CONTROLLER_COLORORDER[event.GetValue().GetLong()]);
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelColorOrder");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::OnPropertyGridChange::ModelControllerConnectionPixelColorOrder");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::ModelControllerConnectionPixelColorOrder");
        AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::OnPropertyGridChange::ModelControllerConnectionPixelColorOrder");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetNullNodes") {
        GetControllerConnection()->DeleteAttribute("nullNodes");
        wxPGProperty *prop = grid->GetFirstChild(event.GetProperty());
        grid->EnableProperty(prop, event.GetValue().GetBool());
        if (event.GetValue().GetBool()) {
            GetControllerConnection()->AddAttribute("nullNodes", "0");
            prop->SetValueFromInt(0);
            grid->Expand(event.GetProperty());
        } else {
            grid->Collapse(event.GetProperty());
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetNullNodes");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetNullNodes");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetNullNodes");
        AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetNullNodes");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetEndNullNodes") {
        GetControllerConnection()->DeleteAttribute("endNullNodes");
        wxPGProperty* prop = grid->GetFirstChild(event.GetProperty());
        grid->EnableProperty(prop, event.GetValue().GetBool());
        if (event.GetValue().GetBool()) {
            GetControllerConnection()->AddAttribute("endNullNodes", "0");
            prop->SetValueFromInt(0);
            grid->Expand(event.GetProperty());
        }
        else {
            grid->Collapse(event.GetProperty());
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetEndNullNodes");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetEndNullNodes");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetEndNullNodes");
        AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetEndNullNodes");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetNullNodes.ModelControllerConnectionPixelNullNodes") {
        GetControllerConnection()->DeleteAttribute("nullNodes");
        if (event.GetValue().GetLong() >= 0) {
            GetControllerConnection()->AddAttribute("nullNodes", wxString::Format("%i", (int)event.GetValue().GetLong()));
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelNullNodes");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::OnPropertyGridChange::ModelControllerConnectionPixelNullNodes");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::ModelControllerConnectionPixelNullNodes");
        AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::OnPropertyGridChange::ModelControllerConnectionPixelNullNodes");
        return 0;
    }  else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetEndNullNodes.ModelControllerConnectionPixelEndNullNodes") {
        GetControllerConnection()->DeleteAttribute("endNullNodes");
        if (event.GetValue().GetLong() >= 0) {
            GetControllerConnection()->AddAttribute("endNullNodes", wxString::Format("%i", (int)event.GetValue().GetLong()));
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelEndNullNodes");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::OnPropertyGridChange::ModelControllerConnectionPixelEndNullNodes");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::ModelControllerConnectionPixelEndNullNodes");
        AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::OnPropertyGridChange::ModelControllerConnectionPixelEndNullNodes");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetGroupCount") {
        GetControllerConnection()->DeleteAttribute("groupCount");
        wxPGProperty *prop = grid->GetFirstChild(event.GetProperty());
        grid->EnableProperty(prop, event.GetValue().GetBool());
        if (event.GetValue().GetBool()) {
            GetControllerConnection()->AddAttribute("groupCount", "0");
            prop->SetValueFromInt(0);
            grid->Expand(event.GetProperty());
        } else {
            grid->Collapse(event.GetProperty());
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetGroupCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetGroupCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetGroupCount");
        AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetGroupCount");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetGroupCount.ModelControllerConnectionPixelGroupCount") {
        GetControllerConnection()->DeleteAttribute("groupCount");
        if (event.GetValue().GetLong() >= 0) {
            GetControllerConnection()->AddAttribute("groupCount", wxString::Format("%i", (int)event.GetValue().GetLong()));
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelGroupCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::OnPropertyGridChange::ModelControllerConnectionPixelGroupCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::ModelControllerConnectionPixelGroupCount");
        AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::OnPropertyGridChange::ModelControllerConnectionPixelGroupCount");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetZigZag") {
        GetControllerConnection()->DeleteAttribute("zigZag");
        wxPGProperty* prop = grid->GetFirstChild(event.GetProperty());
        grid->EnableProperty(prop, event.GetValue().GetBool());
        if (event.GetValue().GetBool()) {
            GetControllerConnection()->AddAttribute("zigZag", "0");
            prop->SetValueFromInt(0);
            grid->Expand(event.GetProperty());
        } else {
            grid->Collapse(event.GetProperty());
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetZigZag");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetZigZag");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetZigZag");
        AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetZigZag");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetZigZag.ModelControllerConnectionPixelZigZag") {
        GetControllerConnection()->DeleteAttribute("zigZag");
        if (event.GetValue().GetLong() >= 0) {
            GetControllerConnection()->AddAttribute("zigZag", wxString::Format("%i", (int)event.GetValue().GetLong()));
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelZigZag");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::OnPropertyGridChange::ModelControllerConnectionPixelZigZag");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::ModelControllerConnectionPixelZigZag");
        AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::OnPropertyGridChange::ModelControllerConnectionPixelZigZag");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetTs") {
        GetControllerConnection()->DeleteAttribute("ts");
        wxPGProperty* prop = grid->GetFirstChild(event.GetProperty());
        grid->EnableProperty(prop, event.GetValue().GetBool());
        if (event.GetValue().GetBool()) {
            GetControllerConnection()->AddAttribute("ts", "0");
            prop->SetValueFromInt(0);
            grid->Expand(event.GetProperty());
        }
        else {
            grid->Collapse(event.GetProperty());
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetTs");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetTs");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetTs");
        AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetTs");
        return 0;
    }
    else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetTs.ModelControllerConnectionPixelTs") {
        GetControllerConnection()->DeleteAttribute("ts");
        if (event.GetValue().GetLong() >= 0) {
            GetControllerConnection()->AddAttribute("ts", wxString::Format("%i", (int)event.GetValue().GetLong()));
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelTs");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::OnPropertyGridChange::ModelControllerConnectionPixelTs");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::ModelControllerConnectionPixelTs");
        AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::OnPropertyGridChange::ModelControllerConnectionPixelTs");
        return 0;
    }
    else if (event.GetPropertyName() == "SubModels") {
        // We cant know which subModels changed so increment all their change counts to ensure anything using them knows they may have changed
        for (auto& it : GetSubModels()) {
            it->IncrementChangeCount();
        }
        IncrementChangeCount();
        wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
        wxPostEvent(modelManager.GetXLightsFrame(), eventForceRefresh);
        AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "Model::OnPropertyGridChange::SubModels");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::SubModels");
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::SubModels");

        return 0;
    } else if (event.GetPropertyName() == "Description") {
        description = event.GetValue().GetString();
        ModelXml->DeleteAttribute("Description");
        if (description != "")
        {
            ModelXml->AddAttribute("Description", XmlSafe(description));
        }
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::Description");
        return 0;
    } else if (event.GetPropertyName() == "ModelFaces") {
        wxXmlNode *f = ModelXml->GetChildren();
        while (f != nullptr) {
            if ("faceInfo" == f->GetName()) {
                ModelXml->RemoveChild(f);
                delete f;
                f = ModelXml->GetChildren();
            }
            else {
                f = f->GetNext();
            }
        }
        Model::WriteFaceInfo(ModelXml, faceInfo);
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelFaces");
        return 0;
    }
    else if (event.GetPropertyName() == "ModelStates") {
        wxXmlNode* f = ModelXml->GetChildren();
        while (f != nullptr) {
            if ("stateInfo" == f->GetName()) {
                ModelXml->RemoveChild(f);
                delete f;
                f = ModelXml->GetChildren();
            }
            else {
                f = f->GetNext();
            }
        }
        Model::WriteStateInfo(ModelXml, stateInfo);
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelStates");
        return 0;
    } else if (event.GetPropertyName().StartsWith("SuperStringColours")) {
        IncrementChangeCount();
        SetSuperStringColours(event.GetValue().GetLong());
        return 0;
    } else if (event.GetPropertyName().StartsWith("SuperStringColour")) {
        int index = wxAtoi(event.GetPropertyName().substr(17));
        wxColor c;
        c << event.GetValue();
        IncrementChangeCount();
        SetSuperStringColour(index, c);
        return 0;
    } else if (event.GetPropertyName() == "ModelStringColor"
               || event.GetPropertyName() == "ModelStringType"
               || event.GetPropertyName() == "ModelRGBWHandling") {
        wxPGProperty *p2 = grid->GetPropertyByName("ModelStringType");
        int i = p2->GetValue().GetLong();
        ModelXml->DeleteAttribute("StringType");
        ModelXml->DeleteAttribute("RGBWHandling");
        if (NODE_TYPES[i] == "Single Color"|| NODE_TYPES[i] == "Single Color Intensity" || NODE_TYPES[i] == "Node Single Color") {
            wxPGProperty *p = grid->GetPropertyByName("ModelStringColor");
            xlColor c;
            wxString tp = GetColorString(p, c);
            if (NODE_TYPES[i] == "Single Color Intensity") {
                tp = "Single Color Intensity";
            }
            else if (NODE_TYPES[i] == "Node Single Color" && p != nullptr) {
                tp = "Node Single Color";
                wxColor cc;
                cc << p->GetValue();
                c = cc;
            }
            if (p != nullptr) p->Enable();
            if (tp == "Single Color Custom" || tp == "Single Color Intensity" || tp == "Node Single Color") {
                ModelXml->DeleteAttribute("CustomColor");
                xlColor xc = c;
                ModelXml->AddAttribute("CustomColor", xc);
            }
            ModelXml->AddAttribute("StringType", tp);
        } else {
            ModelXml->AddAttribute("StringType", NODE_TYPES[i]);
            AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Model::OnPropertyGridChange::ModelStringType");
        }
        if (GetNodeChannelCount(ModelXml->GetAttribute("StringType")) == 4) {
            p2 = grid->GetPropertyByName("ModelRGBWHandling");
            ModelXml->AddAttribute("RGBWHandling", RGBW_HANDLING[p2->GetValue().GetLong()]);
        }
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Model::OnPropertyGridChange::ModelStringType");
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelStringType");
        if (event.GetPropertyName() == "ModelStringType")
        {
            AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Model::OnPropertyGridChange::ModelStringType");
            AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::OnPropertyGridChange::ModelStringType");
            AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "Model::OnPropertyGridChange::ModelStringType");
            AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "Model::OnPropertyGridChange::ModelStringType");
            AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::ModelStringType");
        }
        return 0;
    } else if (event.GetPropertyName() == "ModelStartChannel" || event.GetPropertyName() == "ModelIndividualStartChannels.ModelStartChannel") {

        wxString val = event.GetValue().GetString();

        if ((val.StartsWith("@") || val.StartsWith("#") || val.StartsWith(">") || val.StartsWith("!")) && !val.Contains(":"))
        {
            val = val + ":1";
            event.GetProperty()->SetValue(val);
        }

        ModelXml->DeleteAttribute("StartChannel");
        ModelXml->AddAttribute("StartChannel", val);
        SetControllerName("");
        if (ModelXml->GetAttribute("Advanced") == "1") {
            ModelXml->DeleteAttribute(StartChanAttrName(0));
            ModelXml->AddAttribute(StartChanAttrName(0), val);
        }
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelStartChannel");
        //AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Model::OnPropertyGridChange::ModelStartChannel");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::OnPropertyGridChange::ModelStartChannel");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "Model::OnPropertyGridChange::ModelStartChannel");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::ModelStartChannel");
        return 0;
    } else if (event.GetPropertyName() == "ModelIndividualStartChannels") {
        ModelXml->DeleteAttribute("Advanced");
        int c = Model::HasOneString(DisplayAs) ? 1 : parm1;
        if (event.GetValue().GetBool()) {
            ModelXml->AddAttribute("Advanced", "1");
            for (int x = 0; x < c; x++) {
                if (ModelXml->GetAttribute(StartChanAttrName(x)) == "") {
                    ModelXml->DeleteAttribute(StartChanAttrName(x));
                    ModelXml->AddAttribute(StartChanAttrName(x),
                        ComputeStringStartChannel(x));
                }
            }
        } else {
            // overkill but just delete any that are there
            for (int x = 0; x < MOST_STRINGS_WE_EXPECT; x++) {
                ModelXml->DeleteAttribute(StartChanAttrName(x));
            }
            wxASSERT(!ModelXml->HasAttribute(StartChanAttrName(MOST_STRINGS_WE_EXPECT)));
        }
        // Not sure if i can just remove these
        //RecalcStartChannels();
        //AdjustStringProperties(grid, parm1);
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelIndividualStartChannels");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Model::OnPropertyGridChange::ModelIndividualStartChannels");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::OnPropertyGridChange::ModelIndividualStartChannels");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "Model::OnPropertyGridChange::ModelIndividualStartChannels");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::ModelIndividualStartChannels");
        return 0;
    } else if (event.GetPropertyName().StartsWith("ModelIndividualStartChannels.")) {
        wxString str = event.GetPropertyName();
        str = str.SubString(str.Find(".") + 1, str.length());

        wxString val = event.GetValue().GetString();
        if ((val.StartsWith("@") || val.StartsWith("#") || val.StartsWith(">") || val.StartsWith("!")) && !val.Contains(":"))
        {
            val = val + ":1";
            event.GetProperty()->SetValue(val);
        }

        ModelXml->DeleteAttribute(str);
        ModelXml->AddAttribute(str, val);
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelIndividualStartChannels2");
        //AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Model::OnPropertyGridChange::ModelIndividualStartChannels2");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::OnPropertyGridChange::ModelIndividualStartChannels2");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "Model::OnPropertyGridChange::ModelIndividualStartChannels2");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::ModelIndividualStartChannels2");
        return 0;
    } else if (event.GetPropertyName() == "ModelLayoutGroup") {
        layout_group = LAYOUT_GROUPS[event.GetValue().GetLong()];
        ModelXml->DeleteAttribute("LayoutGroup");
        ModelXml->AddAttribute("LayoutGroup", layout_group);
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelLayoutGroup");
        AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "Model::OnPropertyGridChange::ModelLayoutGroup");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::OnPropertyGridChange::ModelLayoutGroup");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Model::OnPropertyGridChange::ModelLayoutGroup");
        //AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Model::OnPropertyGridChange::ModelLayoutGroup");
        return 0;
    }

    int i = GetModelScreenLocation().OnPropertyGridChange(grid, event);
    IncrementChangeCount();
    GetModelScreenLocation().Write(ModelXml);
    AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Model::OnPropertyGridChange::ModelLayoutGroup");

    return i;
}

void Model::AdjustStringProperties(wxPropertyGridInterface *grid, int newNum) {
    wxPropertyGrid *pg = static_cast<wxPropertyGrid*>(grid);
    wxPGProperty *p = grid->GetPropertyByName("ModelIndividualStartChannels");
    if (p != nullptr) {
        pg->Freeze();
        p->Enable(GetControllerName() == "" || _controller == 0);
        bool adv = p->GetValue().GetBool();
        if (adv) {
            int count = p->GetChildCount();
            while (count > newNum) {
                count--;
                wxString nm = StartChanAttrName(count);
                wxPGProperty *sp = grid->GetPropertyByName("ModelIndividualStartChannels." + nm);
                if (sp != nullptr) {
                    grid->DeleteProperty(sp);
                }
                ModelXml->DeleteAttribute(nm);
            }
            while (count < newNum) {
                wxString nm = StartChanAttrName(count);
                std::string val = ModelXml->GetAttribute(nm).ToStdString();
                if (val == "") {
                    val = ComputeStringStartChannel(count);
                    ModelXml->DeleteAttribute(nm);
                    ModelXml->AddAttribute(nm, val);
                }
                grid->AppendIn(p, new StartChannelProperty(this, count, nm, nm, val, modelManager.GetXLightsFrame()->GetSelectedLayoutPanelPreview()));
                p->Enable(GetControllerName() == "" || _controller == 0);
                count++;
            }
        } else if (p->GetChildCount() > 1) {
            int count = p->GetChildCount();
            for (int x = 1; x < count; x++) {
                wxString nm = StartChanAttrName(x);
                wxPGProperty *sp = grid->GetPropertyByName("ModelIndividualStartChannels." + nm);
                if (sp != nullptr) {
                    grid->DeleteProperty(sp);
                }
            }
        }
        wxPGProperty *sp = grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel");
        if (sp != nullptr)
        {
            if (adv) {
                sp->SetLabel(StartChanAttrName(0));
            }
            else {
                sp->SetLabel("Start Channel");
            }
        }
        p->Enable(parm1 > 1 && (GetControllerName() == "" || _controller == 0 || GetModelChain() == ""));
        pg->Thaw();
        pg->RefreshGrid();
    }
}

void Model::ParseFaceInfo(wxXmlNode *f, std::map<std::string, std::map<std::string, std::string> > &faceInfo) {
    std::string name = f->GetAttribute("Name", "SingleNode").ToStdString();
    std::string type = f->GetAttribute("Type", "SingleNode").ToStdString();
    if (name == "") {
        name = type;
        f->DeleteAttribute("Name");
        f->AddAttribute("Name", type);
    }
    if (!(type == "SingleNode" || type == "NodeRange" || type == "Matrix")) {
        if (type == "Coro") {
            type = "SingleNode";
        } else {
            type = "Matrix";
        }
        f->DeleteAttribute("Type");
        f->AddAttribute("Type", type);
    }
    wxXmlAttribute *att = f->GetAttributes();
    while (att != nullptr) {
        if (att->GetName() != "Name")
        {
            if (att->GetName().Left(5) == "Mouth" || att->GetName().Left(4) == "Eyes")
            {
                if (type == "Matrix")
                {
                    faceInfo[name][att->GetName().ToStdString()] = FixFile("", att->GetValue());
                    if (att->GetValue() != faceInfo[name][att->GetName().ToStdString()]) att->SetValue(faceInfo[name][att->GetName().ToStdString()]);
                }
                else {
                    faceInfo[name][att->GetName().ToStdString()] = att->GetValue();
                }
            }
            else
            {
                faceInfo[name][att->GetName().ToStdString()] = att->GetValue();
            }
        }
        att = att->GetNext();
    }
}

void Model::WriteFaceInfo(wxXmlNode *rootXml, const std::map<std::string, std::map<std::string, std::string> > &faceInfo) {
    if (!faceInfo.empty()) {
        for (const auto& it : faceInfo) {
            wxXmlNode *f = new wxXmlNode(rootXml, wxXML_ELEMENT_NODE , "faceInfo");
            std::string name = it.first;
            f->AddAttribute("Name", name);
            for (const auto& it2 : it.second) {
                f->AddAttribute(it2.first, it2.second);
            }
        }
    }
}

void Model::AddFace(wxXmlNode* n)
{
    ParseFaceInfo(n, faceInfo);
    Model::WriteFaceInfo(ModelXml, faceInfo);
    UpdateFaceInfoNodes();
}

void Model::AddState(wxXmlNode* n)
{
    ParseStateInfo(n, stateInfo);
    Model::WriteStateInfo(ModelXml, stateInfo);
    UpdateStateInfoNodes();
}

void Model::ImportShadowModels(wxXmlNode* n, xLightsFrame* xlights)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    int x = 500;
    int y = 800;

    // import the shadow models as well
    for (auto m = n->GetChildren(); m != nullptr; m = m->GetNext()) {
        Model* model = nullptr;
        if (m->GetName() == "matrixmodel") {
            logger_base.debug("Importing shadow matrix model.");

            // grab the attributes I want to keep
            model = xlights->AllModels.CreateDefaultModel("Matrix", "1");
        } else if (m->GetName() == "custommodel") {
            logger_base.debug("Importing shadow custom model.");
            model = xlights->AllModels.CreateDefaultModel("Custom", "1");
        } else {
            logger_base.error("Importing of shadow models of type %s not yet supported.", (const char*)m->GetName().c_str());
            continue;
        }
        if (model != nullptr) {
            x += 20;
            y += 20;
            model->SetHcenterPos(x);
            model->SetVcenterPos(y);
            ((BoxedScreenLocation&)model->GetModelScreenLocation()).SetScale(1, 1);
            model->SetWidth(100, true);
            model->SetHeight(100, true);
            model->UpdateXmlWithScale();
            model->SetLayoutGroup("Unassigned");
            model->Selected = false;
            float min_x = 0;
            float min_y = 0;
            float max_x = 0;
            float max_y = 0;
            model->ImportXlightsModel(m, xlights, min_x, max_x, min_y, max_y);
            model->SetControllerName(NO_CONTROLLER); // this will force the start channel to a non controller start channel ... then the user can associate them using visualiser
            xlights->AllModels.AddModel(model);
            AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "Model::ImportShadowModels");
            AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "Model::ImportShadowModels");
            AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::ImportShadowModels");
            AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::ImportShadowModels");
            AddASAPWork(OutputModelManager::WORK_UPDATE_PROPERTYGRID, "Model::ImportShadowModels");
            AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::ImportShadowModels");
            IncrementChangeCount();
        }
    }
}

void Model::AddSubmodel(wxXmlNode* n)
{
    ParseSubModel(n);

    // this may break if the submodel format changes and the user loads an old format ... if that happens this needs to go through a upgrade routine
    wxXmlNode* f = new wxXmlNode(wxXML_ELEMENT_NODE, "subModel");
    ModelXml->AddChild(f);
    for (auto a = n->GetAttributes(); a != nullptr; a = a->GetNext()) {
        f->AddAttribute(a->GetName(), a->GetValue());
    }
}

wxString Model::SerialiseFace() const
{
    wxString res = "";

    if (!faceInfo.empty()) {
        for (const auto& it : faceInfo) {
            res += "    <faceInfo Name=\"" + it.first + "\" ";
            for (const auto& it2 : it.second) {
                res += it2.first + "=\"" + it2.second + "\" ";
            }
            res += "/>\n";
        }
    }

    return res;
}

void Model::UpdateFaceInfoNodes()
{
    faceInfoNodes.clear();
    for (const auto& it : faceInfo) {
        if (faceInfo[it.first]["Type"] == "NodeRange") {
            for (const auto& it2 : it.second) {
                if (it2.first != "Type" && !Contains(it2.first, "Color") && it2.second != "") {
                    std::list<int> nodes;
                    auto wtkz = wxStringTokenizer(it2.second, ",");
                    while (wtkz.HasMoreTokens()) {
                        wxString valstr = wtkz.GetNextToken();

                        int start, end;
                        if (valstr.Contains("-")) {
                            int idx = valstr.Index('-');
                            start = wxAtoi(valstr.Left(idx));
                            end = wxAtoi(valstr.Right(valstr.size() - idx - 1));
                            if (end < start) {
                                std::swap(start, end);
                            }
                        } else {
                            start = end = wxAtoi(valstr);
                        }
                        if (start > end) {
                            start = end;
                        }
                        start--;
                        end--;
                        for (int n = start; n <= end; n++) {
                            nodes.push_back(n);
                        }
                    }
                    faceInfoNodes[it.first][it2.first] = nodes;
                }
            }
        }
    }
}

void Model::UpdateStateInfoNodes()
{
    stateInfoNodes.clear();
    for (const auto& it : stateInfo) {
        if (stateInfo[it.first]["Type"] == "NodeRange") {
            for (const auto& it2 : it.second) {
                if (it2.first != "Type" && !Contains(it2.first, "Color") && it2.second != "") {
                    std::list<int> nodes;
                    auto wtkz = wxStringTokenizer(it2.second, ",");
                    while (wtkz.HasMoreTokens()) {
                        wxString valstr = wtkz.GetNextToken();

                        int start, end;
                        if (valstr.Contains("-")) {
                            int idx = valstr.Index('-');
                            start = wxAtoi(valstr.Left(idx));
                            end = wxAtoi(valstr.Right(valstr.size() - idx - 1));
                            if (end < start) {
                                std::swap(start, end);
                            }
                        } else {
                            start = end = wxAtoi(valstr);
                        }
                        if (start > end) {
                            start = end;
                        }
                        start--;
                        end--;
                        for (int n = start; n <= end; n++) {
                            nodes.push_back(n);
                        }
                    }
                    stateInfoNodes[it.first][it2.first] = nodes;
                }
            }
        }
    }
}

void Model::ParseStateInfo(wxXmlNode* f, std::map<std::string, std::map<std::string, std::string>>& stateInfo)
{
    std::string name = f->GetAttribute("Name", "SingleNode").ToStdString();
    std::string type = f->GetAttribute("Type", "SingleNode").ToStdString();
    if (name == "") {
        name = type;
        f->DeleteAttribute("Name");
        f->AddAttribute("Name", type);
    }
    if (!(type == "SingleNode" || type == "NodeRange")) {
        if (type == "Coro") {
            type = "SingleNode";
        }
        f->DeleteAttribute("Type");
        f->AddAttribute("Type", type);
    }
    wxXmlAttribute *att = f->GetAttributes();
    while (att != nullptr) {
        if (att->GetName() != "Name") {
            if (att->GetValue() != "") { // we only save non default values to keep xml file small
                stateInfo[name][att->GetName().ToStdString()] = att->GetValue();
            }
        }
        att = att->GetNext();
    }
}

void Model::WriteStateInfo(wxXmlNode *rootXml, const std::map<std::string, std::map<std::string, std::string> > &stateInfo, bool forceCustom) {
    if (!stateInfo.empty()) {
        for (const auto& it : stateInfo) {
            std::string name = it.first;
            if (wxString(name).Trim(true).Trim(false) != "")
            {
                wxXmlNode *f = new wxXmlNode(rootXml, wxXML_ELEMENT_NODE, "stateInfo");
                f->AddAttribute("Name", name);
                if (forceCustom)
                {
                    f->AddAttribute("CustomColors", "1");
                }
                for (const auto& it2 : it.second) {
                    if (wxString(it2.first).Trim(true).Trim(false) != "")
                        f->AddAttribute(it2.first, it2.second);
                }
            }
        }
    }
}

wxString Model::SerialiseState() const
{
    wxString res = "";

    if (!stateInfo.empty()) {
        for (const auto& it : stateInfo) {
            res += "    <stateInfo Name=\"" + it.first + "\" ";
            for (const auto& it2 : it.second) {
                if (wxString(it2.first).Trim(false).Trim(true) != "")
                {
                    res += it2.first + "=\"" + it2.second + "\" ";
                }
            }
            res += "/>\n";
        }
    }

    return res;
}

wxString Model::SerialiseConnection() const
{
    // Generally you dont want controller connection data in exported models
    wxString res = "";

    wxXmlNode* node = GetControllerConnection();
    if (node->HasAttribute("zigZag")) {
        res = "<ControllerConnection zigZag=\"" + node->GetAttribute("zigZag") + "\"/>";
    }

    return res;
}

wxString Model::SerialiseGroups() const
{
    return modelManager.SerialiseModelGroupsForModel(const_cast<Model*>(this));
}

void Model::AddModelGroups(wxXmlNode* n, int w, int h, const wxString& name, bool& merge, bool& ask)
{
    auto grpModels = n->GetAttribute("models");
    if (grpModels.length() == 0) return;

    modelManager.GetXLightsFrame()->AllModels.AddModelGroups(n, w, h, name, merge, ask);
}

std::string Model::ComputeStringStartChannel(int i) {
    if (i == 0) {
        return ModelXml->GetAttribute("StartChannel", "1").ToStdString();
    }

    wxString existingStartChannelAsString = ModelXml->GetAttribute(StartChanAttrName(i));
    if (existingStartChannelAsString != "")
    {
        return existingStartChannelAsString;
    }

    wxString stch = ModelXml->GetAttribute("StartChannel", "1");
    wxString priorStringStartChannelAsString = ModelXml->GetAttribute(StartChanAttrName(i - 1));
    int priorLength = CalcCannelsPerString();
    // This will be required once custom model supports multiple strings ... working on that
    //if (DisplayAs == "Custom")
    //{
    //    priorLength = GetStrandLength(i - 1) * GetChanCountPerNode();
    //}
    int32_t priorStringStartChannel = GetNumberFromChannelString(priorStringStartChannelAsString);
    int32_t startChannel = priorStringStartChannel + priorLength;
    if (stch.Contains(":")) {
        auto comps = wxSplit(priorStringStartChannelAsString, ':');
        if (comps[0].StartsWith("#"))
        {
            int32_t ststch;
            Output* o = modelManager.GetOutputManager()->GetOutput(startChannel, ststch);
            if (comps.size() == 2)
            {
                if (o != nullptr)
                {
                    return wxString::Format("#%i:%d", o->GetUniverse(), ststch).ToStdString();
                }
                else
                {
                    return wxString::Format("%d", startChannel);
                }
            }
            else
            {
                if (o != nullptr)
                {
                    return wxString::Format("%s:%i:%d", comps[0], o->GetUniverse(), ststch).ToStdString();
                }
                else
                {
                    return wxString::Format("%d", startChannel);
                }
            }
        }
        else if (comps[0].StartsWith(">") || comps[0].StartsWith("@") || comps[0].StartsWith("!") )
        {
            return wxString::Format("%s:%d", comps[0], wxAtol(comps[1]) + priorLength);
        }
        else {
            // This used to be on:sc but this is no longer supported
            return wxString::Format("%d", startChannel);
        }
    }
    return wxString::Format("%d", startChannel);
}

int Model::GetNumStrands() const {
    return 1;
}

bool Model::ModelRenamed(const std::string &oldName, const std::string &newName) {
    bool changed = false;
    std::string sc = ModelXml->GetAttribute("StartChannel","1").ToStdString();
    if ((sc[0] == '@' || sc[0] == '<' || sc[0] == '>') && sc.size() > 1) {
        std::string mn = sc.substr(1, sc.find(':')-1);
        if (mn == oldName) {
            sc = sc[0] + newName + sc.substr(sc.find(':'), sc.size());
            ModelXml->DeleteAttribute("StartChannel");
            ModelXml->AddAttribute("StartChannel", sc);
            changed = true;
        }
    }

    if (GetShadowModelFor() == oldName) {
        SetShadowModelFor(newName);
        changed = true;
    }

    std::string mc = ModelXml->GetAttribute("ModelChain", "").ToStdString();
    if (mc == ">" + oldName)
    {
        ModelXml->DeleteAttribute("ModelChain");
        ModelXml->AddAttribute("ModelChain", ">" + newName);
        changed = true;
    }

    for (size_t i=0; i<stringStartChan.size(); i++) {
        std::string tempstr = StartChanAttrName(i);
        if (ModelXml->HasAttribute(tempstr)) {
            sc = ModelXml->GetAttribute(tempstr, "1").ToStdString();
            if ((sc[0] == '@' || sc[0] == '<' || sc[0] == '>') && sc.size() > 1) {
                std::string mn = sc.substr(1, sc.find(':')-1);
                if (mn == oldName) {
                    sc = sc[0] + newName + sc.substr(sc.find(':'), sc.size());
                    ModelXml->DeleteAttribute(tempstr);
                    ModelXml->AddAttribute(tempstr, sc);
                    changed = true;
                }
            }
        }
    }
    if (changed) {
        SetFromXml(ModelXml, zeroBased);
    }
    return changed;
}

bool Model::IsValidStartChannelString() const
{
    wxString sc;

    if (GetDisplayAs() == "SubModel") {
        sc = this->ModelStartChannel;
    } else {
        sc = ModelXml->GetAttribute("StartChannel").Trim(true).Trim(false);
    }

    if (sc.IsNumber() && wxAtol(sc) > 0 && ! sc.Contains('.')) return true; // absolule

    if (!sc.Contains(':')) return false; // all other formats need a colon

    wxArrayString parts = wxSplit(sc, ':');

    if (parts.size() > 3) return false;

    if (parts[0][0] == '#')
    {
        if (parts.size() == 2)
        {
            Output* o = modelManager.GetOutputManager()->GetOutput(wxAtoi(parts[0].substr(1)), "");
            if (o != nullptr &&
                (parts[1].Trim(true).Trim(false).IsNumber() && wxAtol(parts[1]) > 0 && !parts[1].Contains('.')))
            {
                return true;
            }
        }
        else if (parts.size() == 3)
        {
            wxString ip = parts[0].substr(1);
            Output* o = modelManager.GetOutputManager()->GetOutput(wxAtoi(parts[1]), ip.ToStdString());
            if (ip_utils::IsIPValidOrHostname(ip.ToStdString()) && o != nullptr &&
                (parts[2].Trim(true).Trim(false).IsNumber() && wxAtol(parts[2]) > 0 && !parts[2].Contains('.')))
            {
                return true;
            }
        }
    }
    else if (parts[0][0] == '>' || parts[0][0] == '@')
    {
        if ((parts.size() == 2) &&
            (parts[0].Trim(true).Trim(false).substr(1) != GetName()) && // self referencing
            (parts[1].Trim(true).Trim(false).IsNumber() && wxAtol(parts[1]) > 0 && !parts[1].Contains('.')))
        {
            // dont bother checking the model name ... other processes will check for that
            return true;
        }
    }
    else if (parts[0][0] == '!')
    {
        if ((parts.size() == 2) &&
            (modelManager.GetOutputManager()->GetController(Trim(parts[0].substr(1))) != nullptr) &&
            (parts[1].Trim(true).Trim(false).IsNumber() && wxAtol(parts[1]) > 0 && !parts[1].Contains('.')))
        {
            return true;
        }
    }

    return false;
}

bool Model::UpdateStartChannelFromChannelString(std::map<std::string, Model*>& models, std::list<std::string>& used)
{
    bool valid = false;

    used.push_back(GetName());

    std::string dependsonmodel;
    ModelStartChannel = ModelXml->GetAttribute("StartChannel");
    int32_t StartChannel = GetNumberFromChannelString(ModelStartChannel, valid, dependsonmodel);
    while (!valid && dependsonmodel != "" && std::find(used.begin(), used.end(), dependsonmodel) == used.end())
    {
        Model* m = models[dependsonmodel];
        if (m != nullptr)
        {
            valid = m->UpdateStartChannelFromChannelString(models, used);
        }
        if (valid)
        {
            StartChannel = GetNumberFromChannelString(ModelStartChannel, valid, dependsonmodel);
        }
    }

    if (valid)
    {
        size_t NumberOfStrings = HasOneString(DisplayAs) ? 1 : parm1;
        int ChannelsPerString = CalcCannelsPerString();
        SetStringStartChannels(zeroBased, NumberOfStrings, StartChannel, ChannelsPerString);
    }

    CouldComputeStartChannel = valid;
    return valid;
}

int Model::GetNumberFromChannelString(const std::string &sc) const {
    bool v = false;
    std::string dependsonmodel;
    return GetNumberFromChannelString(sc, v, dependsonmodel);
}

int Model::GetNumberFromChannelString(const std::string &str, bool &valid, std::string& dependsonmodel) const {
    std::string sc(Trim(str));
    valid = true;
    if (sc.find(":") != std::string::npos) {
        std::string start = sc.substr(0, sc.find(":"));
        sc = sc.substr(sc.find(":") + 1);
        if (start[0] == '@' || start[0] == '<' || start[0] == '>') {
            int returnChannel = wxAtoi(sc);
            bool chain = start[0] == '>';
            bool fromStart = start[0] == '@';
            start = Trim(start.substr(1, start.size()));
            if (start == GetName() && !CouldComputeStartChannel)
            {
                valid = false;
            } else {
                if (start != GetName()) {
                    dependsonmodel = start;
                }
                Model* m = modelManager[start];
                if (m != nullptr && m->CouldComputeStartChannel && (!chain || (chain && m->GetControllerName() != NO_CONTROLLER))) {
                    if (fromStart) {
                        int i = m->GetFirstChannel();
                        if (i == -1 && m == this && stringStartChan.size() > 0) {
                            i = stringStartChan[0];
                        }
                        int res = i + returnChannel;
                        if (res < 1)
                        {
                            valid = false;
                            res = 1;
                        }
                        return res;
                    }
                    else {
                        int res = m->GetLastChannel() + returnChannel + 1;
                        if (res < 1)
                        {
                            valid = false;
                            res = 1;
                        }
                        return res;
                    }
                }
                else {
                    valid = false;
                }
            }
        }  else if (start[0] == '!') {
            if (sc.find_first_of(':') == std::string::npos) {
                std::string cs = Trim(start.substr(1));
                Controller* c = modelManager.GetOutputManager()->GetController(cs);
                if (c != nullptr) {
                    return c->GetStartChannel() - 1 + wxAtoi(sc);
                }
            }
            valid = false;
            return 1;
        }
        else if (start[0] == '#') {
            wxString ss = wxString(str);
            wxArrayString cs = wxSplit(ss.SubString(1, ss.Length()), ':');
            if (cs.Count() == 3)
            {
                // #ip:universe:channel
                int returnUniverse = wxAtoi(cs[1]);
                int returnChannel = wxAtoi(cs[2]);

                int res = modelManager.GetOutputManager()->GetAbsoluteChannel(cs[0].Trim(false).Trim(true).ToStdString(), returnUniverse - 1, returnChannel - 1);
                if (res < 1)
                {
                    res = 1;
                    valid = false;
                }
                return res;
            }
            else if (cs.Count() == 2)
            {
                // #universe:channel
                int returnChannel = wxAtoi(sc);
                int returnUniverse = wxAtoi(ss.SubString(1, ss.Find(":") - 1));

                // find output based on universe number ...
                int res = modelManager.GetOutputManager()->GetAbsoluteChannel("", returnUniverse - 1, returnChannel - 1);
                if (res < 1)
                {
                    res = 1;
                    valid = false;
                }
                return res;
            }
            else
            {
                valid = false;
                return 1;
            }
        }
    }
    int returnChannel = wxAtoi(sc);
    if (returnChannel < 1)
    {
        valid = false;
        returnChannel = 1;
    }

    return returnChannel;
}

std::list<int> Model::ParseFaceNodes(std::string channels)
{
    std::list<int> res;

    wxStringTokenizer wtkz(channels, ",");
    while (wtkz.HasMoreTokens())
    {
        wxString valstr = wtkz.GetNextToken();

        int start, end;
        if (valstr.Contains("-")) {
            int idx = valstr.Index('-');
            start = wxAtoi(valstr.Left(idx));
            end = wxAtoi(valstr.Right(valstr.size() - idx - 1));
            if (end < start) std::swap(start, end);
        }
        else {
            start = end = wxAtoi(valstr);
        }
        if (start > end) {
            start = end;
        }
        start--;
        end--;
        for (int n = start; n <= end; n++) {
            res.push_back(n);
        }
    }

    return res;
}

void Model::SetFromXml(wxXmlNode* ModelNode, bool zb)
{

    if (modelDimmingCurve != nullptr) {
        delete modelDimmingCurve;
        modelDimmingCurve = nullptr;
    }

    while (subModels.size() > 0) {
        delete subModels.back();
        subModels.pop_back();
    }
    superStringColours.clear();

    wxString channelstr;

    zeroBased = zb;
    ModelXml = ModelNode;
    StrobeRate = 0;
    Nodes.clear();

    clearUnusedProtocolProperties(GetControllerConnection());

    DeserialiseLayerSizes(ModelNode->GetAttribute("LayerSizes", ""), false);

    name = ModelNode->GetAttribute("name").Trim(true).Trim(false).ToStdString();
    if (name != ModelNode->GetAttribute("name")) {
        ModelNode->DeleteAttribute("name");
        ModelNode->AddAttribute("name", name);
    }
    DisplayAs = ModelNode->GetAttribute("DisplayAs").ToStdString();
    StringType = ModelNode->GetAttribute("StringType", "RGB Nodes").ToStdString();
    _pixelCount = ModelNode->GetAttribute("PixelCount", "").ToStdString();
    _pixelType = ModelNode->GetAttribute("PixelType", "").ToStdString();
    _pixelSpacing = ModelNode->GetAttribute("PixelSpacing", "").ToStdString();
    _active = ModelNode->GetAttribute("Active", "1") == "1";
    _lowDefFactor = wxAtoi(ModelNode->GetAttribute("LowDefinition", "100"));

    if (GetShadowModelFor() == name) {
        // this is a problem ... models should not be a shadow model for themselves
        SetShadowModelFor("");
    }

    //this needs to be done before GetNodeChannelCount call
    bool found = true;
    int index = 0;
    while (found) {
        auto an = wxString::Format("SuperStringColour%d", index);
        auto v = ModelXml->GetAttribute(an, "");
        if (v == "") {
            found = false;
        }
        else {
            superStringColours.push_back(wxColour(v));
        }
        index++;
    }

    SingleNode = HasSingleNode(StringType);
    int ncc = GetNodeChannelCount(StringType);
    SingleChannel = (ncc == 1) && StringType != "Node Single Color";
    if (SingleNode) {
        rgbOrder = "RGB";
    }
    else if (ncc == 4 && StringType[0] == 'W') {
        rgbOrder = StringType.substr(1, 4);
    }
    else {
        rgbOrder = StringType.substr(0, 3);
    }
    if (ncc == 4) {
        std::string s = ModelNode->GetAttribute("RGBWHandling").ToStdString();
        for (int x = 0; x < RGBW_HANDLING.size(); x++) {
            if (RGBW_HANDLING[x] == s) {
                rgbwHandlingType = x;
            }
        }
    }
    description = UnXmlSafe(ModelNode->GetAttribute("Description"));

    wxString tempstr = ModelNode->GetAttribute("parm1");
    tempstr.ToLong(&parm1);
    tempstr = ModelNode->GetAttribute("parm2");
    tempstr.ToLong(&parm2);
    tempstr = ModelNode->GetAttribute("parm3");
    tempstr.ToLong(&parm3);
    tempstr = ModelNode->GetAttribute("StrandNames");
    strandNames.clear();
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
        strandNames.push_back(t2);
    }
    tempstr = ModelNode->GetAttribute("NodeNames");
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

    CouldComputeStartChannel = false;
    std::string  dependsonmodel;
    int32_t StartChannel = GetNumberFromChannelString(ModelNode->GetAttribute("StartChannel", "1").ToStdString(), CouldComputeStartChannel, dependsonmodel);
    tempstr = ModelNode->GetAttribute("Dir");
    IsLtoR = tempstr != "R";
    if (ModelNode->HasAttribute("StartSide")) {
        tempstr = ModelNode->GetAttribute("StartSide");
        isBotToTop = (tempstr == "B");
    }
    else {
        isBotToTop = true;
    }
    customColor = xlColor(ModelNode->GetAttribute("CustomColor", "#000000").ToStdString());

    long n;
    tempstr = ModelNode->GetAttribute("Antialias", wxString::Format("%d", (int)PIXEL_STYLE::PIXEL_STYLE_SMOOTH));
    tempstr.ToLong(&n);
    _pixelStyle = (PIXEL_STYLE)n;
    tempstr = ModelNode->GetAttribute("PixelSize", "2");
    tempstr.ToLong(&n);
    pixelSize = n;
    tempstr = ModelNode->GetAttribute("Transparency", "0");
    tempstr.ToLong(&n);
    transparency = n;
    blackTransparency = wxAtoi(ModelNode->GetAttribute("BlackTransparency", "0"));
    modelTagColour = wxColour(ModelNode->GetAttribute("TagColour", "Black"));
    layout_group = ModelNode->GetAttribute("LayoutGroup", "Unassigned");

    ModelStartChannel = ModelNode->GetAttribute("StartChannel");

    // calculate starting channel numbers for each string
    size_t NumberOfStrings = HasOneString(DisplayAs) ? 1 : parm1;
    int ChannelsPerString = CalcCannelsPerString();

    SetStringStartChannels(zeroBased, NumberOfStrings, StartChannel, ChannelsPerString);
    GetModelScreenLocation().Read(ModelNode);

    InitModel();

    size_t NodeCount = GetNodeCount();
    for (size_t i = 0; i < NodeCount; i++) {
        Nodes[i]->sparkle = rand() % 10000;
    }

    wxXmlNode* f = ModelNode->GetChildren();
    faceInfo.clear();
    stateInfo.clear();
    wxXmlNode* dimmingCurveNode = nullptr;
    wxXmlNode* controllerConnectionNode = nullptr;
    while (f != nullptr) {
        if ("faceInfo" == f->GetName()) {
            ParseFaceInfo(f, faceInfo);
        }
        else if ("stateInfo" == f->GetName()) {
            ParseStateInfo(f, stateInfo);
        }
        else if ("dimmingCurve" == f->GetName()) {
            dimmingCurveNode = f;
            modelDimmingCurve = DimmingCurve::createFromXML(f);
        }
        else if ("subModel" == f->GetName()) {
            ParseSubModel(f);
        }
        else if ("ControllerConnection" == f->GetName()) {
            controllerConnectionNode = f;
        }
        f = f->GetNext();
    }

    UpdateFaceInfoNodes();
    UpdateStateInfoNodes();

    wxString cc = ModelNode->GetAttribute("ControllerConnection").ToStdString();
    if (cc != "") {
        ModelNode->DeleteAttribute("ControllerConnection");
        wxArrayString ar = wxSplit(cc, ':');
        if (controllerConnectionNode == nullptr) {
            controllerConnectionNode = new wxXmlNode(wxXML_ELEMENT_NODE, "ControllerConnection");
            ModelNode->AddChild(controllerConnectionNode);
        }
        if (ar.size() > 0) {
            controllerConnectionNode->DeleteAttribute("Protocol");
            if (ar[0] != "") {
                controllerConnectionNode->AddAttribute("Protocol", ar[0]);
            }
        }
        if (ar.size() > 1) {
            GetControllerConnection()->DeleteAttribute("Port");
            int port = wxAtoi(ar[1]);
            if (port > 0) {
                GetControllerConnection()->AddAttribute("Port", wxString::Format("%d", port));
            }
        }
    }

    if (ModelNode->HasAttribute("ModelBrightness") && modelDimmingCurve == nullptr) {
        int b = wxAtoi(ModelNode->GetAttribute("ModelBrightness", "0"));
        if (b != 0) {
            modelDimmingCurve = DimmingCurve::createBrightnessGamma(b, 1.0);
        }
    }
    if (modelDimmingCurve == nullptr && dimmingCurveNode != nullptr) {
        ModelNode->RemoveChild(dimmingCurveNode);
    }

    IncrementChangeCount();
}

std::string Model::GetControllerConnectionString() const
{
    if (GetControllerProtocol() == "") return "";
    std::string ret = wxString::Format("%s:%d", GetControllerProtocol(), GetControllerPort(1)).ToStdString();

    ret += GetControllerConnectionAttributeString();
    return ret;
}

std::string Model::GetControllerConnectionRangeString() const
{
    if (GetControllerProtocol() == "") return "";
    std::string ret = wxString::Format("%s:%d", GetControllerProtocol(), GetControllerPort(1)).ToStdString();
    if (GetControllerPort(1) == 0) {
        ret = wxString::Format("%s", GetControllerProtocol()).ToStdString();
    }
    if (GetNumPhysicalStrings() > 1 && GetControllerPort(1) != 0 && !IsMatrixProtocol()) {
        ret = wxString::Format("%s-%d", ret, GetControllerPort(GetNumPhysicalStrings())).ToStdString();
    }

    ret += GetControllerConnectionAttributeString();

    return ret;
}

std::string Model::GetControllerConnectionPortRangeString() const
{
    std::string ret = wxString::Format("%d", GetControllerPort(1)).ToStdString();
    if (GetNumPhysicalStrings() > 1 && GetControllerPort(1) != 0 && !IsMatrixProtocol()) {
        ret = wxString::Format("%s-%d", ret, GetControllerPort(GetNumPhysicalStrings())).ToStdString();
    }
    return ret;
}

bool compare_pairstring(const std::pair<std::string, std::string>& a, const std::pair<std::string, std::string>& b) {
    return a.first > b.first;
}

std::string Model::GetControllerConnectionAttributeString() const
{
    std::string ret;
    std::list<std::pair<std::string, std::string>> props;
    wxXmlAttribute* att = GetControllerConnection()->GetAttributes();
    while (att != nullptr) {
        if (att->GetName() == "SmartRemote") {
            props.push_back({"SmartRemote", DecodeSmartRemote(wxAtoi(att->GetValue()))});
        } else if (att->GetName() != "Port" && att->GetName() != "Protocol" && att->GetName() != "SRMaxCascade" && att->GetName() != "SRCascadeOnPort" && att->GetName() != "SmartRemoteType") {
            props.push_back({att->GetName(), att->GetValue()});
        }
        att = att->GetNext();
    }

    // this adds the properties alphabetically
    props.sort(compare_pairstring);
    for (const auto& it : props) {
        ret += ":" + it.first + "=" + it.second;
    }

    return ret;
}

void Model::ReplaceIPInStartChannels(const std::string& oldIP, const std::string& newIP)
{
    bool changed = false;
    if (Contains(ModelStartChannel, oldIP))
    {
        wxString sc(ModelStartChannel);
        sc.Replace(oldIP, newIP);
        SetStartChannel(sc);
        changed = true;
    }

    size_t NumberOfStrings = HasOneString(DisplayAs) ? 1 : parm1;
    for (int i = 0; i < NumberOfStrings; i++) {
        auto tempstr = StartChanAttrName(i);
        if (ModelXml->HasAttribute(tempstr)) {
            wxString sc = ModelXml->GetAttribute(tempstr, "");
            if (Contains(sc, oldIP))
            {
                sc.Replace(oldIP, newIP);
                ModelXml->DeleteAttribute(tempstr);
                ModelXml->AddAttribute(tempstr, sc);
                changed = true;
            }
        }
    }
    if (changed)
    {
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::ReplaceIPInStartChannels");
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::ReplaceIPInStartChannels");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "Model::ReplaceIPInStartChannels");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "Model::ReplaceIPInStartChannels");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::ReplaceIPInStartChannels");
    }
}

std::string Model::DecodeSmartRemote(int sr)
{
    if(sr == 0) return "None";
    return std::string (1, ('A' + sr - 1));
}

wxXmlNode *Model::GetControllerConnection() const {
    if (GetModelXml() == nullptr) return nullptr;

    wxXmlNode *n = GetModelXml()->GetChildren();
    while (n != nullptr) {
        if (n->GetName() == "ControllerConnection") {
            return n;
        }
        n = n->GetNext();
    }
    n = new wxXmlNode(wxXML_ELEMENT_NODE , "ControllerConnection");
    GetModelXml()->AddChild(n);
    return n;
}

void Model::RemoveSubModel(const std::string &name) {
    for (auto a = subModels.begin(); a != subModels.end(); ++a) {
        Model *m = *a;
        if (m->GetName() == name) {
            delete m;
            subModels.erase(a);
        }
    }
}

Model *Model::GetSubModel(const std::string &name) const {
    for (auto a = subModels.begin(); a != subModels.end(); ++a) {
        if ((*a)->GetName() == name) {
            return *a;
        }
    }
    return nullptr;
}

std::string Model::GenerateUniqueSubmodelName(const std::string suggested) const
{
    if (GetSubModel(suggested) == nullptr) return suggested;

    int i = 2;
    for (;;)
    {
        auto name = wxString::Format("%s_%d", suggested, i++);
        if (GetSubModel(name) == nullptr) return name;
    }
}

void Model::ParseSubModel(wxXmlNode *node) {
    subModels.push_back(new SubModel(this, node));
}

int Model::CalcCannelsPerString() {
    int ChannelsPerString = parm2*GetNodeChannelCount(StringType);
    if (SingleChannel)
        ChannelsPerString=1;
    else if (SingleNode)
        ChannelsPerString=GetNodeChannelCount(StringType);

    return ChannelsPerString;
}

void Model::SetStringStartChannels(bool zeroBased, int NumberOfStrings, int StartChannel, int ChannelsPerString) {
    std::string tempstr = ModelXml->GetAttribute("Advanced", "0").ToStdString();
    bool HasIndividualStartChans=tempstr == "1";
    stringStartChan.clear();
    stringStartChan.resize(NumberOfStrings);
    for (int i=0; i<NumberOfStrings; i++) {
        tempstr = StartChanAttrName(i);
        if (!zeroBased && HasIndividualStartChans && ModelXml->HasAttribute(tempstr)) {
            bool b = false;
            std::string dependsonmodel;
            stringStartChan[i] = GetNumberFromChannelString(ModelXml->GetAttribute(tempstr, "1").ToStdString(), b, dependsonmodel)-1;
            CouldComputeStartChannel &= b;
        } else {
            stringStartChan[i] = (zeroBased? 0 : StartChannel-1) + i*ChannelsPerString;
        }
    }
}

int Model::FindNodeAtXY(int bufx, int bufy)
{
    for (int i = 0; i < Nodes.size(); ++i)
    {
        if ((bufx == -1 || Nodes[i]->Coords[0].bufX == bufx) && (bufy == -1 || Nodes[i]->Coords[0].bufY == bufy))
        {
            return i;
        }
    }
    wxASSERT(false);
    return -1;
}

void Model::InitModel() {
}

void Model::GetNodeChannelValues(size_t nodenum, unsigned char *buf) {
    wxASSERT(nodenum < Nodes.size()); // trying to catch an error i can see in crash reports
    if (nodenum < Nodes.size()) {
        Nodes[nodenum]->GetForChannels(buf);
    }
}

void Model::SetNodeChannelValues(size_t nodenum, const unsigned char *buf) {
    wxASSERT(nodenum < Nodes.size()); // trying to catch an error i can see in crash reports
    if (nodenum < Nodes.size()) {
        Nodes[nodenum]->SetFromChannels(buf);
    }
}

xlColor Model::GetNodeColor(size_t nodenum) const {
    wxASSERT(nodenum < Nodes.size()); // trying to catch an error i can see in crash reports
    xlColor color;
    if (nodenum < Nodes.size()) {
        Nodes[nodenum]->GetColor(color);
    }
    else
    {
        color = xlWHITE;
    }
    return color;
}

xlColor Model::GetNodeMaskColor(size_t nodenum) const {
    if (nodenum >= Nodes.size()) return xlWHITE; // this shouldnt happen but it does if you have a custom model with no nodes in it
    xlColor color;
    Nodes[nodenum]->GetMaskColor(color);
    return color;
}

void Model::SetNodeColor(size_t nodenum, const xlColor &c) {
    wxASSERT(nodenum < Nodes.size()); // trying to catch an error i can see in crash reports
    if (nodenum < Nodes.size()) {
        Nodes[nodenum]->SetColor(c);
    }
}

bool Model::IsNodeInBufferRange(size_t nodeNum, int x1, int y1, int x2, int y2) {
    if (nodeNum < Nodes.size()) {
        for (auto a = Nodes[nodeNum]->Coords.begin(); a != Nodes[nodeNum]->Coords.end(); ++a) {
            if (a->bufX >= x1 && a->bufX <= x2
                && a->bufY >= y1 && a->bufY <= y2) {
                return true;
            }
        }
    }
    return false;
}

// only valid for rgb nodes and dumb strings (not traditional strings)
wxChar Model::GetChannelColorLetter(wxByte chidx) {
    return rgbOrder[chidx];
}

char Model::EncodeColour(const xlColor& c)
{
    if (c.red > 0 && c.green == 0 && c.blue == 0)
    {
        return 'R';
    }
    if (c.red == 0 && c.green > 0 && c.blue == 0)
    {
        return 'G';
    }
    if (c.red == 0 && c.green == 0 && c.blue > 0)
    {
        return 'B';
    }
    if (c.red > 0 && c.red == c.green && c.red == c.blue)
    {
        return 'W';
    }

    return 'X';
}

// Accepts any absolute channel number and if it happens to be used by this model a single character representing the channel colour is returned.
// If the channel does not map to the model this returns ' '
char Model::GetAbsoluteChannelColorLetter(int32_t absoluteChannel)
{
    int32_t fc = GetFirstChannel();
    if (absoluteChannel < fc + 1 || absoluteChannel > (int32_t)GetLastChannel() + 1) return ' ';

    if (SingleChannel) {
        return EncodeColour(GetNodeMaskColor(0));
    }
    int ccpn = std::max(GetChanCountPerNode(), 1);
    return GetChannelColorLetter((absoluteChannel - fc - 1) % ccpn);
}

std::string Model::GetControllerPortSortString() const
{
    auto controller = GetControllerName();
    if (controller.empty()) {
        controller = PadLeft("Z", 'Z', 140);
    }
    auto port = GetControllerPort();
    auto sc = GetFirstChannel(); // we assume within a port models are in channel order

    return wxString::Format("%s:%08d:%08d", controller, port, sc).ToStdString();
}

std::string Model::GetStartChannelInDisplayFormat(OutputManager* outputManager)
{
    auto s = Trim(ModelStartChannel);
    if (!IsValidStartChannelString())
    {
        return "(1)";
    }
    else if (s[0] == '>')
    {
        return s + wxString::Format(" (%u)", GetFirstChannel() + 1);
    }
    else if (s[0] == '@')
    {
        return s + wxString::Format(" (%u)", GetFirstChannel() + 1);
    }
    else if (s[0] == '!')
    {
        return s + wxString::Format(" (%u)", GetFirstChannel() + 1);
    }
    else if (s[0] == '#')
    {
        return GetFirstChannelInStartChannelFormat(outputManager);
    }
    else
    {
        return wxString::Format("%u", GetFirstChannel() + 1);
    }
}

std::string Model::GetLastChannelInStartChannelFormat(OutputManager* outputManager)
{
    return GetChannelInStartChannelFormat(outputManager, GetLastChannel() + 1);
}

std::string Model::GetChannelInStartChannelFormat(OutputManager* outputManager, uint32_t channel) {
    std::list<std::string> visitedModels;
    visitedModels.push_back(GetName());

    std::string modelFormat = Trim(ModelStartChannel);
    char firstChar = modelFormat[0];

    bool done = false;
    while (!done && (firstChar == '@' || firstChar == '>') && CountChar(modelFormat, ':') == 1)
    {
        std::string referencedModel = Trim(modelFormat.substr(1, modelFormat.find(':') - 1));
        Model* m = modelManager[referencedModel];

        if (m != nullptr && std::find(visitedModels.begin(), visitedModels.end(), referencedModel) == visitedModels.end())
        {
            modelFormat = Trim(m->ModelStartChannel);
            firstChar = modelFormat[0];
        }
        else
        {
            done = true;
        }
        visitedModels.push_back(referencedModel);
    }

    if (!modelFormat.empty())
    {
        if (modelFormat[0] == '#')
        {
            firstChar = '#';
        }
        else if (modelFormat[0] == '!')
        {
            firstChar = '!';
        }
        else if (CountChar(modelFormat, ':') == 1)
        {
            firstChar = '0';
        }
    }
    else
    {
        firstChar = '0';
        modelFormat = "0";
    }

    if (firstChar == '#')
    {
        // universe:channel
        int32_t startChannel;
        Output* output = outputManager->GetOutput(channel, startChannel);

        if (output == nullptr) {
            return wxString::Format("%u", channel).ToStdString();
        }

        // This should not be the case any more
        //if (output->IsOutputCollection())
        //{
        //    output = output->GetActualOutput(channel);
        //    startChannel = channel - output->GetStartChannel() + 1;
        //}

        if (CountChar(modelFormat, ':') == 1)
        {
            return wxString::Format("#%d:%d (%u)", output->GetUniverse(), startChannel, channel).ToStdString();
        }
        else
        {
            std::string ip = "<err>";
            if (output->IsIpOutput())
            {
                ip = ((IPOutput*)output)->GetIP();
            }
            return wxString::Format("#%s:%d:%d (%u)", ip, output->GetUniverse(), startChannel, channel).ToStdString();
        }
    }
    else if (firstChar == '!')
    {
        auto comps = wxSplit(modelFormat, ':');
        auto c = outputManager->GetController(Trim(comps[0].substr(1)));
        int32_t start = 1;
        if (c != nullptr)
        {
            start = c->GetStartChannel();
        }
        unsigned int lastChannel = GetLastChannel() + 1;
        return wxString(modelFormat).BeforeFirst(':').Trim(true).Trim(false) + ":" + wxString::Format("%d (%u)", lastChannel - start + 1, lastChannel);
    }
    else if (firstChar == '@' || firstChar == '>' || CountChar(modelFormat, ':') == 0)
    {
        // absolute
        return std::to_string(channel);
    }
    else
    {
        // This used to be output:sc ... but that is no longer valid
        return std::to_string(channel);
    }
}

std::string Model::GetFirstChannelInStartChannelFormat(OutputManager* outputManager)
{
    return GetChannelInStartChannelFormat(outputManager, GetFirstChannel() + 1);
}

uint32_t Model::GetLastChannel() const {
    uint32_t LastChan = 0;
    size_t NodeCount = GetNodeCount();
    for (size_t idx = 0; idx < NodeCount; idx++) {
        if (Nodes[idx]->ActChan == (unsigned int)-1)
        {
            return (unsigned int)NodeCount * Nodes[idx]->GetChanCount() - 1;
        }
        unsigned int lc = std::max(LastChan, Nodes[idx]->ActChan + Nodes[idx]->GetChanCount() - 1);
        if (lc > LastChan)
        {
            LastChan = lc;
        }
    }
    return LastChan;
}

//zero based channel number, i.e. 0 is the first channel
uint32_t Model::GetFirstChannel() const {
    uint32_t FirstChan = 0xFFFFFFFF;
    size_t NodeCount = GetNodeCount();
    for (size_t idx = 0; idx < NodeCount; idx++) {
        uint32_t fc = std::min(FirstChan, Nodes[idx]->ActChan);
        if (fc < FirstChan) {
            FirstChan = fc;
        }
    }
    return FirstChan;
}

unsigned int Model::GetNumChannels() {
    return GetLastChannel() - GetFirstChannel() + 1;
}

void Model::SetPosition(double posx, double posy) {

    if (GetModelScreenLocation().IsLocked() || IsFromBase())
        return;

    GetModelScreenLocation().SetPosition(posx, posy);
    IncrementChangeCount();
}

// initialize screen coordinates
// parm1=Number of Strings/Arches/Canes
// parm2=Pixels Per String/Arch/Cane
void Model::SetLineCoord() {
    float x,y;
    float idx=0;
    size_t NodeCount=GetNodeCount();
    int numlights=parm1*parm2;
    float half=numlights/2;
    GetModelScreenLocation().SetRenderSize(numlights, numlights*2);

    for(size_t n=0; n<NodeCount; n++) {
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++) {
            x=idx;
            x=IsLtoR ? x - half : half - x;
            y=0;
            Nodes[n]->Coords[c].screenX=x;
            Nodes[n]->Coords[c].screenY=y + numlights;
            idx++;
        }
    }
}

void Model::SetBufferSize(int NewHt, int NewWi) {
    BufferHt = NewHt;
    BufferWi = NewWi;
    IncrementChangeCount();
}

// not valid for Frame or Custom
int Model::NodesPerString() const {
    if (SingleNode) {
        return 1;
    }
    else {
        int ts = GetSmartTs();
        if (ts <= 1) {
            return parm2;
        }
        else {
            return parm2 * ts;
        }
    }
}

int32_t Model::NodeStartChannel(size_t nodenum) const {
    return Nodes.size() && nodenum < Nodes.size() ? Nodes[nodenum]->ActChan: 0; //avoid memory access error if no nods -DJ
}
int32_t Model::NodeEndChannel(size_t nodenum) const {
    return Nodes.size() && nodenum < Nodes.size() ? Nodes[nodenum]->ActChan + Nodes[nodenum]->GetChanCount() - 1: 0; //avoid memory access error if no nods -DJ
}

const std::string &Model::NodeType(size_t nodenum) const {
    return Nodes.size() && nodenum < Nodes.size() ? Nodes[nodenum]->GetNodeType(): NodeBaseClass::RGB; //avoid memory access error if no nods -DJ
}

void Model::GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform, int &bufferWi, int &bufferHi, int stagger) const {
    if (type == DEFAULT) {
        bufferHi = this->BufferHt;
        bufferWi = this->BufferWi;
    } else if (type == SINGLE_LINE) {
        bufferHi = 1;
        bufferWi = Nodes.size();
    } else if (type == AS_PIXEL) {
        bufferHi = 1;
        bufferWi = 1;
    } else if (type == VERT_PER_STRAND) {
        bufferHi = GetNumStrands();
        bufferWi = 1;
        for (int x = 0; x < bufferHi; x++) {
            bufferWi = std::max(bufferWi, GetStrandLength(x));
        }
    } else if (type == HORIZ_PER_STRAND) {
        bufferWi = GetNumStrands();
        bufferHi = 1;
        for (int x = 0; x < bufferWi; x++) {
            bufferHi = std::max(bufferHi, GetStrandLength(x));
        }
    } else {
        //if (type == PER_PREVIEW) {
        //default is to go ahead and build the full node buffer
        std::vector<NodeBaseClassPtr> newNodes;
        InitRenderBufferNodes(type, camera, "None", newNodes, bufferWi, bufferHi, stagger);
    }
    AdjustForTransform(transform, bufferWi, bufferHi);
}

void Model::AdjustForTransform(const std::string &transform,
                               int &bufferWi, int &bufferHi) const {
    if (transform == "Rotate CC 90" || transform == "Rotate CW 90") {
        int x = bufferHi;
        bufferHi = bufferWi;
        bufferWi = x;
    }
}

static inline void SetCoords(NodeBaseClass::CoordStruct &it2, int x, int y) {
    it2.bufX = x;
    it2.bufY = y;
}

static inline void SetCoords(NodeBaseClass::CoordStruct &it2, int x, int y, int maxX, int maxY, int scale) {
    if (maxX != -1) {
        x = x * maxX;
        x = x / scale;
    }
    if (maxY != -1) {
        y = y * maxY;
        y = y / scale;
    }
    it2.bufX = x;
    it2.bufY = y;
}

// this is really slow
char GetPixelDump(int x, int y, std::vector<NodeBaseClassPtr> &newNodes)
{
    for (auto n = newNodes.begin(); n != newNodes.end(); ++n)
    {
        for (auto c = (*n)->Coords.begin(); c != (*n)->Coords.end(); ++c)
        {
            if (c->bufX == x && c->bufY == y)
            {
                return '*';
            }
        }
    }

    return '-';
}

void Model::DumpBuffer(std::vector<NodeBaseClassPtr> &newNodes,
    int bufferWi, int bufferHt) const
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("Dumping render buffer for '%s':", (const char*)GetFullName().c_str());
    for (int y = bufferHt - 1; y >= 0; y--)
    {
        std::string line = "";
        for (int x = 0; x < bufferWi; x++)
        {
            line += GetPixelDump(x, y, newNodes);
        }
        logger_base.debug(">    %s", (const char*)line.c_str());
    }
}

void Model::ApplyTransform(const std::string& type,
                           std::vector<NodeBaseClassPtr>& newNodes,
                           int& bufferWi, int& bufferHi) const
{
    //"Rotate CC 90", "Rotate CW 90", "Rotate 180", "Flip Vertical", "Flip Horizontal", "Rotate CC 90 Flip Horizontal", "Rotate CW 90 Flip Horizontal"
    if (type == "None") {
        return;
    } else if (type == "Rotate 180") {
        for (size_t x = 0; x < newNodes.size(); x++) {
            for (auto& it2 : newNodes[x]->Coords) {
                SetCoords(it2, bufferWi - it2.bufX - 1, bufferHi - it2.bufY - 1);
            }
        }
    } else if (type == "Flip Vertical") {
        for (size_t x = 0; x < newNodes.size(); x++) {
            for (auto& it2 : newNodes[x]->Coords) {
                SetCoords(it2, it2.bufX, bufferHi - it2.bufY - 1);
            }
        }
    } else if (type == "Flip Horizontal") {
        for (size_t x = 0; x < newNodes.size(); x++) {
            for (auto& it2 : newNodes[x]->Coords) {
                SetCoords(it2, bufferWi - it2.bufX - 1, it2.bufY);
            }
        }
    } else if (type == "Rotate CW 90") {
        for (size_t x = 0; x < newNodes.size(); x++) {
            for (auto& it2 : newNodes[x]->Coords) {
                SetCoords(it2, bufferHi - it2.bufY - 1, it2.bufX);
            }
        }
        std::swap(bufferWi, bufferHi);
    } else if (type == "Rotate CC 90") {
        for (int x = 0; x < newNodes.size(); x++) {
            for (auto& it2 : newNodes[x]->Coords) {
                SetCoords(it2, it2.bufY, bufferWi - it2.bufX - 1);
            }
        }
        std::swap(bufferWi, bufferHi);
    } else if (type == "Rotate CC 90 Flip Horizontal") {
        for (int x = 0; x < newNodes.size(); x++) {
            for (auto& it2 : newNodes[x]->Coords) {
                SetCoords(it2, it2.bufY, bufferWi - it2.bufX - 1);
            }
        }
        std::swap(bufferWi, bufferHi);

        for (size_t x = 0; x < newNodes.size(); x++) {
            for (auto& it2 : newNodes[x]->Coords) {
                SetCoords(it2, it2.bufX, bufferHi - it2.bufY - 1);
            }
        }
    } else if (type == "Rotate CW 90 Flip Horizontal") {
        for (size_t x = 0; x < newNodes.size(); x++) {
            for (auto& it2 : newNodes[x]->Coords) {
                SetCoords(it2, bufferHi - it2.bufY - 1, it2.bufX);
            }
        }
        std::swap(bufferWi, bufferHi);

        for (size_t x = 0; x < newNodes.size(); x++) {
            for (auto& it2 : newNodes[x]->Coords) {
                SetCoords(it2, it2.bufX, bufferHi - it2.bufY - 1);
            }
        }
    }
}

void Model::InitRenderBufferNodes(const std::string &type, const std::string &camera,
    const std::string &transform,
    std::vector<NodeBaseClassPtr>& newNodes, int& bufferWi, int& bufferHt, int stagger, bool deep) const
{

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    int firstNode = newNodes.size();

    // want to see if i can catch something that causes this to crash
    if (firstNode + Nodes.size() <= 0)
    {
        // This seems to happen when an effect is dropped on a strand with zero pixels
        // Like a polyline segment with no nodes
        logger_base.warn("Model::InitRenderBufferNodes firstNode + Nodes.size() = %d. %s::'%s'. This commonly happens on a polyline segment with zero pixels or a custom model with no nodes but with effects dropped on it.", (int32_t)firstNode + Nodes.size(), (const char *)GetDisplayAs().c_str(), (const char *)GetFullName().c_str());
    }

    // Don't add model group nodes if its a 3D preview render buffer
    if ( !((camera != "2D") && GetDisplayAs() == "ModelGroup" && (type == PER_PREVIEW || type == PER_PREVIEW_NO_OFFSET)) ) {
        newNodes.reserve(firstNode + Nodes.size());
        for (auto& it : Nodes) {
            newNodes.push_back(NodeBaseClassPtr(it.get()->clone()));
        }
    }

    if (type == DEFAULT) {
        bufferHt = this->BufferHt;
        bufferWi = this->BufferWi;
    }
    else if (type == SINGLE_LINE) {
        bufferHt = 1;
        bufferWi = newNodes.size();
        int cnt = 0;
        for (int x = firstNode; x < newNodes.size(); x++) {
            if (newNodes[x] == nullptr)
            {
                logger_base.crit("XXX Model::InitRenderBufferNodes newNodes[x] is null ... this is going to crash.");
                wxASSERT(false);
            }
            for (auto& it2 : newNodes[x]->Coords) {
                SetCoords(it2, cnt, 0);
            }
            cnt++;
        }
    }
    else if (type == AS_PIXEL) {
        bufferHt = 1;
        bufferWi = 1;
        for (int x = firstNode; x < newNodes.size(); x++) {
            if (newNodes[x] == nullptr)
            {
                logger_base.crit("XXX Model::InitRenderBufferNodes newNodes[x] is null ... this is going to crash.");
                wxASSERT(false);
            }
            for (auto& it2 : newNodes[x]->Coords) {
                SetCoords(it2, 0, 0);
            }
        }
    }
    else if (type == HORIZ_PER_STRAND) {
        bufferWi = GetNumStrands();
        bufferHt = 1;
        for (int x = 0; x < bufferWi; x++) {
            bufferHt = std::max(bufferHt, GetStrandLength(x));
        }
        int cnt = 0;
        int strand = 0;
        int strandLen = GetStrandLength(GetMappedStrand(0));
        for (int x = firstNode; x < newNodes.size();) {
            if (cnt >= strandLen) {
                strand++;
                if (strand < GetNumStrands()) {
                    strandLen = GetStrandLength(GetMappedStrand(strand));
                }
                else {
                    // not sure what to do here ... we have more nodes than strands ... so lets just start again
                    strandLen = GetStrandLength(GetMappedStrand(0));
                    strand = 0;
                }
                cnt = 0;
            }
            else {
                if (newNodes[x] == nullptr)
                {
                    logger_base.crit("AAA Model::InitRenderBufferNodes newNodes[x] is null ... this is going to crash.");
                    wxASSERT(false);
                }
                for (auto& it2 : newNodes[x]->Coords) {
                    SetCoords(it2, strand, cnt, -1, bufferHt, strandLen);
                }
                cnt++;
                x++;
            }
        }
    }
    else if (type == VERT_PER_STRAND) {
        bufferHt = GetNumStrands();
        bufferWi = 1;
        for (int x = 0; x < bufferHt; x++) {
            bufferWi = std::max(bufferWi, GetStrandLength(x));
        }
        int cnt = 0;
        int strand = 0;
        int strandLen = GetStrandLength(GetMappedStrand(0));
        for (int x = firstNode; x < newNodes.size();) {
            if (cnt >= strandLen) {
                strand++;
                if (strand < GetNumStrands()) {
                    strandLen = GetStrandLength(GetMappedStrand(strand));
                }
                else {
                    // not sure what to do here ... we have more nodes than strands ... so lets just start again
                    strandLen = GetStrandLength(GetMappedStrand(0));
                    strand = 0;
                }
                cnt = 0;
            }
            else {
                if (newNodes[x] == nullptr)
                {
                    logger_base.crit("BBB Model::InitRenderBufferNodes newNodes[x] is null ... this is going to crash.");
                    wxASSERT(false);
                }
                for (auto& it2 : newNodes[x]->Coords) {
                    SetCoords(it2, cnt, strand, bufferWi, -1, strandLen);
                }
                cnt++;
                x++;
            }
        }
    }
    else if (type == PER_PREVIEW || type == PER_PREVIEW_NO_OFFSET) {
        float maxX = -1000000.0;
        float minX = 1000000.0;
        float maxY = -1000000.0;
        float minY = 1000000.0;

        ModelPreview* modelPreview = nullptr;
        PreviewCamera* pcamera = nullptr;
        if (xLightsApp::GetFrame() != nullptr)
        {
            modelPreview = xLightsApp::GetFrame()->GetHousePreview();
            pcamera = xLightsApp::GetFrame()->viewpoint_mgr.GetNamedCamera3D(camera);
        }

        if (pcamera != nullptr && camera != "2D") {
            GetModelScreenLocation().PrepareToDraw(true, false);
        } else {
            GetModelScreenLocation().PrepareToDraw(false, false);
        }

        // For 3D render view buffers recursively process each individual model...should be able to handle nested model groups
        if (GetDisplayAs() == "ModelGroup" && camera != "2D") {
            std::vector<Model *> models;
            auto mn = Split(ModelXml->GetAttribute("models").ToStdString(), ',', true);
            int nc = 0;
            for (int x = 0; x < mn.size(); x++) {
                Model *c = modelManager.GetModel(mn[x]);
                if (c != nullptr) {
                    models.push_back(c);
                    nc += c->GetNodeCount();
                } else if (mn[x].empty()) {
                    // silently ignore blank models
                }
            }

            if (nc) {
                newNodes.reserve(nc);
            }
            for (Model *c : models) {
                int bw, bh;
                c->InitRenderBufferNodes("Per Preview No Offset", camera, transform, newNodes, bw, bh, stagger);
            }
        }

        // We save the transformed coordinates here so we dont have to calculate them all twice
        std::vector<float> outx;
        std::vector<float> outy;
        outx.reserve(newNodes.size() - firstNode); //common case is one coord per node so size for that
        outy.reserve(newNodes.size() - firstNode);
        for (int x = firstNode; x < newNodes.size(); x++) {
            if (newNodes[x] == nullptr) {
                logger_base.crit("CCC Model::InitRenderBufferNodes newNodes[x] is null ... this is going to crash.");
                wxASSERT(false);
            }
            for (auto& it2 : newNodes[x]->Coords) {
                float sx = it2.screenX;
                float sy = it2.screenY;

                if (ModelXml == nullptr) {
                    // if model xml is null then this isnt a real model which means it doesnt have a real location
                    // which means translatePoint is going to do strange things ... so dont call it
                } else {
                    if (pcamera == nullptr || camera == "2D") {
                        // Handle all of the 2D classic transformations
                        //float sz = 0;
                        // reintroducing the z coordinate as otherwise with some rotations we end up with a zero width buffer
                        float sz = it2.screenZ;
                        GetModelScreenLocation().TranslatePoint(sx, sy, sz);
                    } else {
                        if (GetDisplayAs() != "ModelGroup") { // ignore for groups since they will have already calculated their node positions from recursion call above
                            // Handle 3D from an arbitrary camera position
                            float sz = it2.screenZ;
                            GetModelScreenLocation().TranslatePoint(sx, sy, sz);

                            // really not sure if 400,400 is the best thing to pass in here ... but it seems to work
                            glm::vec2 loc = GetModelScreenLocation().GetScreenPosition(400, 400, modelPreview, pcamera, sx, sy, sz);
                            loc.y *= -1.0f;
                            sx = loc.x;
                            sy = loc.y;
                            it2.screenX = sx;
                            it2.screenY = sy;
                        }
                    }
                }

                // save the transformed value
                outx.push_back(sx);
                outy.push_back(sy);

                if (sx > maxX) {
                    maxX = sx;
                }
                if (sx < minX) {
                    minX = sx;
                }
                if (sy > maxY) {
                    maxY = sy;
                }
                if (sy < minY) {
                    minY = sy;
                }
            }
        }

        // Work out scaling factor for per preview camera views as these can build some
        // exteme locations which translate into crazy sized render buffers
        // this allows us to scale it back to the desired grid size
        float factor = 1.0;
        if (GetDisplayAs() == "ModelGroup" && type == PER_PREVIEW) {
            int maxDimension = ((ModelGroup*)this)->GetGridSize();
            if (maxDimension != 0 && (maxX - minX > maxDimension || maxY - minY > maxDimension)) {
                // we need to resize all the points by this amount
                factor = std::max(((float)(maxX - minX)) / (float)maxDimension, ((float)(maxY - minY)) / (float)maxDimension);
                // But if it is already smaller we dont want to make it bigger
                if (factor < 1.0) {
                    factor = 1.0;
                }
            }
        }
        if ((type !=  PER_PREVIEW_NO_OFFSET) && (((maxX - minX) > 2048) || ((maxY - minY) > 2048))){
            // this will result in a GIANT render buffer, lets reduce to something we can reasonably render
            float fx = ((float)(maxX - minX)) / 2048.0f;
            float fy = ((float)(maxY - minY)) / 2048.0f;
            factor = fx > fy ? fx : fy;
        }

        // if we have a dense model with lots of pixels but (int)(maxx - minx) and (int)(maxy - miny) are really small then it generates a render buffer that is quite small with lots of nodes in each cell
        // We need a factor that scales up the screen locations to separate the pixels
        // The empty space factor is the number of empty cells expected per filled cell in the average model ... of course in models where there are dense and sparse areas this wont necessarily be true
        #define MODEL_EMPTY_SPACE_FACTOR 4.0f
        if (type == PER_PREVIEW && GetDisplayAs() != "ModelGroup" && factor == 1.0 && (newNodes.size() * (MODEL_EMPTY_SPACE_FACTOR + 1.0) > (maxX - minX) * (maxY - minY))) {
            float deltaX = maxX - minX;
            float deltaY = maxY - minY;

            // Don't allow "aspect" (down below) become zero because this will lead to a divide by zero for "factor", then all following calculations will be "nan".
            if (deltaX == 0) {
                deltaX = 0.01f;
            }

            // Don't allow "deltaY" to be zero because this will lead to a divide by zero for "aspect", then all following calculations will be "nan".
            if (deltaY == 0) {
                deltaY = 0.01f;
            }

            float aspect = deltaX / deltaY;
            float mx = ((float)newNodes.size()) * (MODEL_EMPTY_SPACE_FACTOR + 1.0f) * aspect;
            float x = std::sqrt(mx);
            factor = deltaX / x;
            if (std::max(deltaX / factor, deltaY / factor) > 400) { // if this results in an overly large scaling ... ie a buffer > 400 in any dimension
                factor = std::max(deltaX, deltaY) /  400; // work out a scaling that gives a 400x400 buffer
            }
        }

        minX /= factor;
        maxX /= factor;
        minY /= factor;
        maxY /= factor;
        //logger_base.debug("Factor '%f':", factor);

        float offx = minX;
        float offy = minY;
        bool noOff = type == PER_PREVIEW_NO_OFFSET;

        if (noOff) {
            offx = 0;
            offy = 0;
        }
        bufferHt = bufferWi = -1;

        // we can skip all the scaling for individual models that are being recursively handled from a ModelGroup
        if (!(pcamera != nullptr && camera != "2D" && GetDisplayAs() != "ModelGroup" && noOff)) {
            auto itx = outx.begin();
            auto ity = outy.begin();
            for (int x = firstNode; x < newNodes.size(); x++) {
                if (newNodes[x] == nullptr) {
                    logger_base.crit("DDD Model::InitRenderBufferNodes newNodes[x] is null ... this is going to crash.");
                    wxASSERT(false);
                }
                for (auto& it2 : newNodes[x]->Coords) {

                    // grab the previously transformed coordinate
                    float sx = *itx / factor;
                    float sy = *ity / factor;

                    SetCoords(it2, std::round(sx - offx), std::round(sy - offy));
                    if (it2.bufX > bufferWi) {
                        bufferWi = it2.bufX;
                    }
                    if (it2.bufY > bufferHt) {
                        bufferHt = it2.bufY;
                    }

                    if (noOff) {
                        it2.screenX = sx;
                        it2.screenY = sy;
                    }

                    ++itx;
                    ++ity;
                }
            }
        }

        if (!noOff) {
            bufferHt++;
            bufferWi++;
        }
        else {
            bufferHt = std::round(maxY - minY + 1.0f);
            bufferWi = std::round(maxX - minX + 1.0f);
        }
        //DumpBuffer(newNodes, bufferWi, bufferHt);
    }
    else {
        bufferHt = this->BufferHt;
        bufferWi = this->BufferWi;
    }

    // Zero buffer sizes are bad
    // This can happen when a strand is zero length ... maybe also a custom model with no nodes
    if (bufferHt == 0)
    {
        logger_base.warn("Model::InitRenderBufferNodes BufferHt was 0 ... overridden to be 1.");
        bufferHt = 1;
    }
    if (bufferWi == 0)
    {
        logger_base.warn("Model::InitRenderBufferNodes BufferWi was 0 ... overridden to be 1.");
        bufferWi = 1;
    }

    ApplyTransform(transform, newNodes, bufferWi, bufferHt);
}

std::string Model::GetNextName() {
    if (nodeNames.size() > Nodes.size()) {
        return nodeNames[Nodes.size()];
    }
    return "";
}

bool Model::FourChannelNodes() const
{
    // true if string contains WRGB or any variant thereof
    // I do the W search first to try to abort quickly for strings unlikely to be 4 channel
    return (Contains(StringType, "W") &&
        (Contains(StringType, "RGBW") ||
         Contains(StringType, "WRGB") ||
         Contains(StringType, "WRBG") ||
         Contains(StringType, "RBGW") ||
         Contains(StringType, "WGRB") ||
         Contains(StringType, "GRBW") ||
         Contains(StringType, "WGBR") ||
         Contains(StringType, "GBRW") ||
         Contains(StringType, "WBRG") ||
         Contains(StringType, "BRGW") ||
         Contains(StringType, "WBGR") ||
         Contains(StringType, "BGRW")));
}

std::list<std::string> Model::GetShadowedBy() const {
    return GetModelManager().GetModelsShadowing(this);
}

// set size of Nodes vector and each Node's Coords vector
void Model::SetNodeCount(size_t NumStrings, size_t NodesPerString, const std::string &rgbOrder) {
    size_t n;
    if (SingleNode) {
        if (StringType=="Single Color Red") {
            for(n = 0; n < NumStrings; n++) {
                Nodes.push_back(NodeBaseClassPtr(new NodeClassRed(n, NodesPerString, GetNextName())));
                Nodes.back()->model = this;
            }
        } else if (StringType=="Single Color Green") {
            for(n = 0; n < NumStrings; n++) {
                Nodes.push_back(NodeBaseClassPtr(new NodeClassGreen(n, NodesPerString, GetNextName())));
                Nodes.back()->model = this;
            }
        }
        else if (StringType == "Superstring") {
            for (n = 0; n < NumStrings; n++) {
                Nodes.push_back(NodeBaseClassPtr(new NodeClassSuperString(n, NodesPerString, superStringColours, rgbwHandlingType, GetNextName())));
                Nodes.back()->model = this;
            }
        } else if (StringType=="Single Color Blue") {
            for(n = 0; n < NumStrings; n++) {
                Nodes.push_back(NodeBaseClassPtr(new NodeClassBlue(n, NodesPerString, GetNextName())));
                Nodes.back()->model = this;
            }
        } else if (StringType=="Single Color White") {
            for(n = 0; n < NumStrings; n++) {
                Nodes.push_back(NodeBaseClassPtr(new NodeClassWhite(n, NodesPerString, GetNextName())));
                Nodes.back()->model = this;
            }
        } else if (StringType=="Strobes White 3fps" || StringType=="Strobes") {
            StrobeRate=7;  // 1 out of every 7 frames
            for(n = 0; n < NumStrings; n++) {
                Nodes.push_back(NodeBaseClassPtr(new NodeClassWhite(n, NodesPerString, GetNextName())));
                Nodes.back()->model = this;
            }
        } else if (StringType=="Single Color Custom") {
            for(n = 0; n < NumStrings; n++) {
                Nodes.push_back(NodeBaseClassPtr(new NodeClassCustom(n, NodesPerString, customColor, GetNextName())));
                Nodes.back()->model = this;
            }
        } else if (StringType=="Single Color Intensity") {
            for(n = 0; n < NumStrings; n++) {
                Nodes.push_back(NodeBaseClassPtr(new NodeClassIntensity(n, NodesPerString, customColor, GetNextName())));
                Nodes.back()->model = this;
            }
        } else if (StringType=="4 Channel RGBW") {
            for(n = 0; n < NumStrings; n++) {
                Nodes.push_back(NodeBaseClassPtr(new NodeClassRGBW(n, NodesPerString, "RGB", true, rgbwHandlingType, GetNextName())));
                Nodes.back()->model = this;
            }
        }
        else if (StringType == "4 Channel WRGB") {
            for (n = 0; n < NumStrings; n++) {
                Nodes.push_back(NodeBaseClassPtr(new NodeClassRGBW(n, NodesPerString, "RGB", false, rgbwHandlingType, GetNextName())));
                Nodes.back()->model = this;
            }
        } else {
            // 3 Channel RGB
            for(n = 0; n < NumStrings; n++) {
                Nodes.push_back(NodeBaseClassPtr(new NodeBaseClass(n, NodesPerString, "RGB", GetNextName())));
                Nodes.back()->model = this;
            }
        }
    } else if (NodesPerString == 0) {
        if (StringType == "Node Single Color") {
            Nodes.push_back(NodeBaseClassPtr(new NodeClassCustom(0, 0, customColor, GetNextName())));
        }
        else  if (FourChannelNodes()) {
            bool wLast = StringType[3] == 'W';
            Nodes.push_back(NodeBaseClassPtr(new NodeClassRGBW(0, 0, rgbOrder, wLast, rgbwHandlingType, GetNextName())));
        }
        else {
            Nodes.push_back(NodeBaseClassPtr(new NodeBaseClass(0, 0, rgbOrder, GetNextName())));
        }
        Nodes.back()->model = this;
    } else if (StringType[3] == ' ') {
        size_t numnodes = NumStrings * NodesPerString;
        for(n = 0; n < numnodes; n++) {
            Nodes.push_back(NodeBaseClassPtr(new NodeBaseClass(n/NodesPerString, 1, rgbOrder, GetNextName())));
            Nodes.back()->model = this;
        }
    }
    else if (StringType == "Node Single Color") {
        size_t numnodes = NumStrings * NodesPerString;
        for (n = 0; n < numnodes; n++) {
            Nodes.push_back(NodeBaseClassPtr(new NodeClassCustom(n/NodesPerString, 1, customColor, GetNextName())));
            Nodes.back()->model = this;
        }
    } else {
        bool wLast = StringType[3] == 'W';
        size_t numnodes = NumStrings * NodesPerString;
        for (n = 0; n < numnodes; n++) {
            Nodes.push_back(NodeBaseClassPtr(new NodeClassRGBW(n / NodesPerString, 1, rgbOrder, wLast, rgbwHandlingType, GetNextName())));
            Nodes.back()->model = this;
        }
    }
}

size_t Model::GetNodeChannelCount(const std::string & nodeType) const {
    if (nodeType.compare(0, 12, "Single Color") == 0) {
        return 1;
    } else if (nodeType == "Strobes White 3fps") {
        return 1;
    } else if (nodeType == "Strobes") {
        return 1;
    } else if (nodeType == "4 Channel RGBW") {
        return 4;
    } else if (nodeType == "4 Channel WRGB") {
        return 4;
    } else if (nodeType[0] == 'W' || nodeType [3] == 'W') {
        //various WRGB and RGBW types
        return 4;
    }
    else if (nodeType == "Superstring") {
        return std::max(1, (int)superStringColours.size());
    }
    else if (nodeType == "Node Single Color") {
        return 1;
    }
    return 3;
}

void Model::AddLayerSizeProperty(wxPropertyGridInterface* grid)
{
    wxPGProperty* psn = grid->Append(new wxUIntProperty("Layers", "Layers", GetLayerSizeCount()));
    psn->SetAttribute("Min", 1);
    psn->SetAttribute("Max", 100);
    psn->SetEditor("SpinCtrl");

    if (GetLayerSizeCount() > 1) {
        for (int i = 0; i < GetLayerSizeCount(); i++)
        {
            wxString id = wxString::Format("Layer%d", i);
            wxString nm = wxString::Format("Layer %d", i + 1);
            if (i == 0) nm = "Inside";
            else if (i == GetLayerSizeCount() - 1) nm = "Outside";

            wxPGProperty* pls = grid->AppendIn(psn, new wxUIntProperty(nm, id, GetLayerSize(i)));
            pls->SetAttribute("Min", 1);
            pls->SetAttribute("Max", 1000);
            pls->SetEditor("SpinCtrl");
        }
    }
}

bool Model::HandleLayerSizePropertyChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event)
{
    if ("Layers" == event.GetPropertyName()) {
        SetLayerSizeCount(event.GetValue().GetLong());
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::HandleLayerSizePropertyChange::Layers");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::HandleLayerSizePropertyChange::Layers");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Model::HandleLayerSizePropertyChange::Layers");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Model::HandleLayerSizePropertyChange::Layers");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Model::HandleLayerSizePropertyChange::Layers");
        IncrementChangeCount();

        ModelXml->DeleteAttribute("LayerSizes");
        ModelXml->AddAttribute("LayerSizes", SerialiseLayerSizes());

        OnLayerSizesChange(true);

        return true;
    }
    else if (event.GetPropertyName().StartsWith("Layers.Layer")) {
        int layer = wxAtoi(event.GetPropertyName().AfterLast('r'));
        SetLayerSize(layer, event.GetValue().GetLong());
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::HandleLayerSizePropertyChange::Layer");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::HandleLayerSizePropertyChange::Layer");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Model::HandleLayerSizePropertyChange::Layer");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Model::HandleLayerSizePropertyChange::Layer");
        IncrementChangeCount();

        ModelXml->DeleteAttribute("LayerSizes");
        ModelXml->AddAttribute("LayerSizes", SerialiseLayerSizes());

        OnLayerSizesChange(false);

        return true;
    }
    return false;
}

bool Model::ContainsChannel(uint32_t startChannel, uint32_t endChannel) const
{
    // we could just check first and last channel but then that would not work for submodels
    for (const auto& node : Nodes) {
        if (node->ContainsChannel(startChannel, endChannel))
            return true;
    }
    return false;
}

bool Model::ContainsChannel(int strand, uint32_t startChannel, uint32_t endChannel) const
{
    uint32_t sc = GetChannelForNode(strand, 0);
    uint32_t ec = GetChannelForNode(strand, GetStrandLength(strand)-1) + GetChanCountPerNode() - 1;
    return !(endChannel < sc || startChannel > ec);
}

bool Model::ContainsChannel(const std::string& submodelName, uint32_t startChannel, uint32_t endChannel) const
{
    auto sm = GetSubModel(submodelName);
    return sm->ContainsChannel(startChannel, endChannel);
}

void Model::HandlePropertyGridRightClick(wxPropertyGridEvent& event, wxMenu& mnu)
{
    wxString name = event.GetPropertyName();

    if (name.StartsWith("Layers.")) {
        layerSizeMenu = wxAtoi(event.GetPropertyName().AfterLast('r'));
        if (GetLayerSizeCount() != 1) {
            mnu.Append(ID_LAYERSIZE_DELETE, "Delete");
        }
        mnu.Append(ID_LAYERSIZE_INSERT, "Insert");
    }
}

void Model::HandlePropertyGridContextMenu(wxCommandEvent& event)
{
    if (event.GetId() == ID_LAYERSIZE_DELETE) {
        DeleteLayerSize(layerSizeMenu);
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::HandlePropertyGridContextMenu::Delete");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::HandlePropertyGridContextMenu::Delete");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Model::HandlePropertyGridContextMenu::Delete");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Model::HandlePropertyGridContextMenu::Delete");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Model::HandlePropertyGridContextMenu::Delete");
        IncrementChangeCount();
        ModelXml->DeleteAttribute("LayerSizes");
        ModelXml->AddAttribute("LayerSizes", SerialiseLayerSizes());
        OnLayerSizesChange(true);
    } else if (event.GetId() == ID_LAYERSIZE_INSERT) {
        InsertLayerSizeBefore(layerSizeMenu);
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::HandlePropertyGridContextMenu::Insert");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::HandlePropertyGridContextMenu::Insert");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Model::HandlePropertyGridContextMenu::Insert");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Model::HandlePropertyGridContextMenu::Insert");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Model::HandlePropertyGridContextMenu::Insert");
        IncrementChangeCount();
        ModelXml->DeleteAttribute("LayerSizes");
        ModelXml->AddAttribute("LayerSizes", SerialiseLayerSizes());
        OnLayerSizesChange(true);
    }
}

uint32_t Model::GetChannelForNode(int strandIndex, int node) const
{
    if (node < 0)
        return -1;

    size_t nodesOnPriorStrands = 0;
    for (size_t s = 0; s < strandIndex; ++s) {
        nodesOnPriorStrands += GetStrandLength(s);
    }

    size_t n = nodesOnPriorStrands + node;

    if (n >= Nodes.size()) {
        wxASSERT(false);
        return -1;
    }

    return Nodes[n]->ActChan;
}

// returns a number where the first node is 1
uint32_t Model::GetNodeNumber(size_t nodenum) const
{
    if (nodenum >= Nodes.size())
        return 0;
    int sn = Nodes[nodenum]->StringNum;
    return (Nodes[nodenum]->ActChan - stringStartChan[sn]) / 3 + sn * NodesPerString() + 1;
}

uint32_t Model::GetNodeNumber(int bufY, int bufX) const
{
    uint32_t count = 0;
    for (const auto& it : Nodes) {
        if (it->Coords.size() > 0) {
            if (it->Coords[0].bufX == bufX && it->Coords[0].bufY == bufY) {
                return count;
            }
        }
        count++;
    }
    return -1;
}

uint32_t Model::GetNodeCount() const {
    return Nodes.size();
}

uint32_t Model::GetActChanCount() const
{
    uint32_t NodeCnt = GetNodeCount();
    if (NodeCnt == 0) {
        return 0;
    }

    uint32_t count = 0;
    for (uint32_t x = 0; x < NodeCnt; x++) {
        count += Nodes[x]->GetChanCount();
    }

    return count;
}

uint32_t Model::GetChanCount() const {
    size_t NodeCnt=GetNodeCount();
    if (NodeCnt == 0) {
        return 0;
    }
    int min = 999999999;
    int max = 0;

    for (int x = 0; x < NodeCnt; x++) {
        int i = Nodes[x]->ActChan;
        if (i < min) {
            min = i;
        }
        i += Nodes[x]->GetChanCount();
        if (i > max) {
            max = i;
        }
    }
    return max - min;
}

NodeBaseClass* Model::GetNode(uint32_t node) const
{
    if (node < Nodes.size()) {
        return Nodes[node].get();
    }
    return nullptr;
}

int Model::GetChanCountPerNode() const {
    size_t NodeCnt = GetNodeCount();
    if (NodeCnt == 0) {
        return 0;
    }
    return Nodes[0]->GetChanCount();
}

uint32_t Model::GetCoordCount(size_t nodenum) const {
    return nodenum < Nodes.size() ? Nodes[nodenum]->Coords.size() : 0;
}

int Model::GetNodeStringNumber(size_t nodenum) const {
    return nodenum < Nodes.size() ? Nodes[nodenum]->StringNum : 0;
}

void Model::GetNode3DScreenCoords(int nodeidx, std::vector<std::tuple<float, float, float>>& pts) {
    for (int x = 0; x < Nodes[nodeidx]->Coords.size(); x++) {
        pts.push_back(std::make_tuple(Nodes[nodeidx]->Coords[x].screenX, Nodes[nodeidx]->Coords[x].screenY, Nodes[nodeidx]->Coords[x].screenZ));
    }
}

void Model::GetNodeCoords(int nodeidx, std::vector<wxPoint> &pts) {
    if (nodeidx >= Nodes.size()) return;
    for (int x = 0; x < Nodes[nodeidx]->Coords.size(); x++) {
        pts.push_back(wxPoint(Nodes[nodeidx]->Coords[x].bufX, Nodes[nodeidx]->Coords[x].bufY));
    }
}

bool Model::IsCustom(void) {
    return (DisplayAs == "Custom");
}

//convert # to AA format so it matches Custom Model grid display:
//this makes it *so* much easier to visually compare with Custom Model grid display
//A - Z == 1 - 26
//AA - AZ == 27 - 52
//BA - BZ == 53 - 78
//etc
static wxString AA(int x) {
    wxString retval;
    --x;
    //    if (x >= 26 * 26) { retval += 'A' + x / (26 * 26); x %= 26 * 26; }
    if (x >= 26) {
        retval += 'A' + x / 26 - 1;
        x %= 26;
    }
    retval += 'A' + x;
    return retval;
}

//add just the node#s to a choice list:
//NO add parsed info to choice list or check list box:
size_t Model::GetChannelCoords(wxArrayString& choices) { //wxChoice* choices1, wxCheckListBox* choices2, wxListBox* choices3)
    //    if (choices1) choices1->Clear();
    //    if (choices2) choices2->Clear();
    //    if (choices3) choices3->Clear();
    //    if (choices1) choices1->Append(wxT("0: (none)"));
    //    if (choices2) choices2->Append(wxT("0: (none)"));
    //    if (choices3) choices3->Append(wxT("0: (none)"));
    size_t NodeCount = GetNodeCount();
    for (size_t n = 0; n < NodeCount; n++) {
        wxString newstr;
        //        debug(10, "model::node[%d/%d]: #coords %d, ach# %d, str %d", n, NodeCount, Nodes[n]->Coords.size(), Nodes[n]->StringNum, Nodes[n]->ActChan);
        if (Nodes[n]->Coords.empty()) continue;
        //        newstr = wxString::Format(wxT("%i"), GetNodeNumber(n));
        //        choices.Add(newstr);
        choices.Add(GetNodeXY(n));
        //        if (choices1) choices1->Append(newstr);
        //        if (choices2) choices2->Append(newstr);
        //        if (choices3)
        //        {
        //            wxArrayString strary;
        //            choices3->InsertItems(strary, choices3->GetCount() + 0);
        //        }
    }
    return choices.GetCount(); //choices1? choices1->GetCount(): 0) + (choices2? choices2->GetCount(): 0);
}

//get parsed node info:
std::string Model::GetNodeXY(const std::string& nodenumstr) {
    size_t NodeCount = GetNodeCount();
    try {
        int32_t nodenum = std::stod(nodenumstr);
        for (size_t inx = 0; inx < NodeCount; inx++) {
            if (Nodes[inx]->Coords.empty()) continue;
            if (GetNodeNumber(inx) == nodenum) return GetNodeXY(inx);
        }
    } catch ( ... ) {

    }
    return nodenumstr; //not found?
}

std::string Model::GetNodeXY(int nodeinx) {
    if ((nodeinx < 0) || (nodeinx >= (int)GetNodeCount())) return "";
    if (Nodes[nodeinx]->Coords.empty()) return "";
    if (GetCoordCount(nodeinx) > 1) //show count and first + last coordinates
        if (IsCustom())
            return wxString::Format(wxT("%d: %d# @%s%d-%s%d"), GetNodeNumber(nodeinx), GetCoordCount(nodeinx), AA(Nodes[nodeinx]->Coords.front().bufX + 1), BufferHt - Nodes[nodeinx]->Coords.front().bufY, AA(Nodes[nodeinx]->Coords.back().bufX + 1), BufferHt - Nodes[nodeinx]->Coords.back().bufY).ToStdString(); //NOTE: only need first (X,Y) for each channel, but show last and count as well; Y is in reverse order
        else
            return wxString::Format(wxT("%d: %d# @(%d,%d)-(%d,%d"), GetNodeNumber(nodeinx), GetCoordCount(nodeinx), Nodes[nodeinx]->Coords.front().bufX + 1, BufferHt - Nodes[nodeinx]->Coords.front().bufY, Nodes[nodeinx]->Coords.back().bufX + 1, BufferHt - Nodes[nodeinx]->Coords.back().bufY).ToStdString(); //NOTE: only need first (X,Y) for each channel, but show last and count as well; Y is in reverse order
        else //just show singleton
            if (IsCustom())
                return wxString::Format(wxT("%d: @%s%d"), GetNodeNumber(nodeinx), AA(Nodes[nodeinx]->Coords.front().bufX + 1), BufferHt - Nodes[nodeinx]->Coords.front().bufY).ToStdString();
            else
                return wxString::Format(wxT("%d: @(%d,%d)"), GetNodeNumber(nodeinx), Nodes[nodeinx]->Coords.front().bufX + 1, BufferHt - Nodes[nodeinx]->Coords.front().bufY).ToStdString();
}

//extract first (X,Y) from string formatted above:
bool Model::ParseFaceElement(const std::string& multi_str, std::vector<wxPoint>& first_xy) {
    //    first_xy->x = first_xy->y = 0;
    //    first_xy.clear();
    wxStringTokenizer wtkz(multi_str, "+");
    while (wtkz.HasMoreTokens()) {
        wxString str = wtkz.GetNextToken();
        if (str.empty()) continue;
        if (str.Find('@') == wxNOT_FOUND) continue; //return false;

        wxString xystr = str.AfterFirst('@');
        if (xystr.empty()) continue; //return false;
        long xval = 0, yval = 0;
        if (xystr[0] == '(') {
            xystr.Remove(0, 1);
            if (!xystr.BeforeFirst(',').ToLong(&xval)) continue; //return false;
            if (!xystr.AfterFirst(',').BeforeFirst(')').ToLong(&yval)) continue; //return false;
        } else {
            int parts = 0;
            while (!xystr.empty() && (xystr[0] >= 'A') && (xystr[0] <= 'Z')) {
                xval *= 26;
                xval += xystr[0] - 'A' + 1;
                xystr.Remove(0, 1);
                parts |= 1;
            }
            while (!xystr.empty() && (xystr[0] >= '0') && (xystr[0] <= '9')) {
                yval *= 10;
                yval += xystr[0] - '0';
                xystr.Remove(0, 1);
                parts |= 2;
            }
            if (parts != 3) continue; //return false;
            if (!xystr.empty() && (xystr[0] != '-')) continue; //return false;
        }
        wxPoint newxy(xval, yval);
        first_xy.push_back(newxy);
    }

    return !first_xy.empty(); //true;
}

//extract first (X,Y) from string formatted above:
bool Model::ParseStateElement(const std::string& multi_str, std::vector<wxPoint>& first_xy) {
    //    first_xy->x = first_xy->y = 0;
    //    first_xy.clear();
    wxStringTokenizer wtkz(multi_str, "+");
    while (wtkz.HasMoreTokens()) {
        wxString str = wtkz.GetNextToken();
        if (str.empty()) continue;
        if (str.Find('@') == wxNOT_FOUND) continue; //return false;

        wxString xystr = str.AfterFirst('@');
        if (xystr.empty()) continue; //return false;
        long xval = 0, yval = 0;
        if (xystr[0] == '(') {
            xystr.Remove(0, 1);
            if (!xystr.BeforeFirst(',').ToLong(&xval)) continue; //return false;
            if (!xystr.AfterFirst(',').BeforeFirst(')').ToLong(&yval)) continue; //return false;
        }
        else {
            int parts = 0;
            while (!xystr.empty() && (xystr[0] >= 'A') && (xystr[0] <= 'Z')) {
                xval *= 26;
                xval += xystr[0] - 'A' + 1;
                xystr.Remove(0, 1);
                parts |= 1;
            }
            while (!xystr.empty() && (xystr[0] >= '0') && (xystr[0] <= '9')) {
                yval *= 10;
                yval += xystr[0] - '0';
                xystr.Remove(0, 1);
                parts |= 2;
            }
            if (parts != 3) continue; //return false;
            if (!xystr.empty() && (xystr[0] != '-')) continue; //return false;
        }
        wxPoint newxy(xval, yval);
        first_xy.push_back(newxy);
    }

    return !first_xy.empty(); //true;
}

void Model::ExportAsCustomXModel() const
{
    wxString name = ModelXml->GetAttribute("name").Trim(true).Trim(false);
    wxLogNull logNo; // kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (filename.IsEmpty())
        return;

    wxFile f(filename);
    //    bool isnew = !FileExists(filename);
    if (!f.Create(filename, true) || !f.IsOpened())
        DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());

    wxString cm = "";

    float minsx = 99999;
    float minsy = 99999;
    float maxsx = -1;
    float maxsy = -1;

    size_t nodeCount = GetNodeCount();
    for (size_t i = 0; i < nodeCount; i++) {
        float Sbufx = Nodes[i]->Coords[0].screenX;
        float Sbufy = Nodes[i]->Coords[0].screenY;
        if (Sbufx < minsx)
            minsx = Sbufx;
        if (Sbufx > maxsx)
            maxsx = Sbufx;
        if (Sbufy < minsy)
            minsy = Sbufy;
        if (Sbufy > maxsy)
            maxsy = Sbufy;
    }

    int scale = 1;

    while (!FindCustomModelScale(scale)) {
        ++scale;
        if (scale > 100) { // I(Scott) am afraid of infinite while loops
            scale = 1;
            break;
        }
    }

    int minx = std::floor(minsx);
    int miny = std::floor(minsy);
    int maxx = std::ceil(maxsx);
    int maxy = std::ceil(maxsy);
    int sizex = maxx - minx + 1;
    int sizey = maxy - miny + 1;

    sizex *= scale;
    sizey *= scale;

    int* nodeLayout = (int*)malloc(sizey * sizex * sizeof(int));
    memset(nodeLayout, 0x00, sizey * sizex * sizeof(int));

    for (int i = 0; i < nodeCount; ++i) {
        int x = (Nodes[i]->Coords[0].screenX - minx) * scale;
        int y = (sizey - ((Nodes[i]->Coords[0].screenY - miny) * scale) - 1);
        nodeLayout[y * sizex + x] = i + 1;
    }

    for (int i = 0; i < sizey * sizex; ++i) {
        if (i != 0) {
            if (i % sizex == 0) {
                cm += ";";
            } else {
                cm += ",";
            }
        }
        if (nodeLayout[i] != 0) {
            cm += wxString::Format("%i", nodeLayout[i]);
        }
    }

    free(nodeLayout);

    wxString p1 = wxString::Format("%i", sizex);
    wxString p2 = wxString::Format("%i", sizey);
    wxString st = ModelXml->GetAttribute("StringType");
    wxString ps = ModelXml->GetAttribute("PixelSize");
    wxString t = ModelXml->GetAttribute("Transparency", "0");
    wxString mb = ModelXml->GetAttribute("ModelBrightness", "0");
    wxString a = ModelXml->GetAttribute("Antialias");
    wxString sn = ModelXml->GetAttribute("StrandNames");
    wxString nn = ModelXml->GetAttribute("NodeNames");
    wxString pc = ModelXml->GetAttribute("PixelCount");
    wxString pt = ModelXml->GetAttribute("PixelType");
    wxString psp = ModelXml->GetAttribute("PixelSpacing");

    wxString v = xlights_version_string;
    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<custommodel \n");
    f.Write(wxString::Format("name=\"%s\" ", name));
    f.Write(wxString::Format("parm1=\"%s\" ", p1));
    f.Write(wxString::Format("parm2=\"%s\" ", p2));
    f.Write("Depth=\"1\" ");
    f.Write(wxString::Format("StringType=\"%s\" ", st));
    f.Write(wxString::Format("Transparency=\"%s\" ", t));
    f.Write(wxString::Format("PixelSize=\"%s\" ", ps));
    f.Write(wxString::Format("ModelBrightness=\"%s\" ", mb));
    f.Write(wxString::Format("Antialias=\"%s\" ", a));
    f.Write(wxString::Format("StrandNames=\"%s\" ", sn));
    f.Write(wxString::Format("NodeNames=\"%s\" ", nn));
    if (pc != "")
        f.Write(wxString::Format("PixelCount=\"%s\" ", pc));
    if (pt != "")
        f.Write(wxString::Format("PixelType=\"%s\" ", pt));
    if (psp != "")
        f.Write(wxString::Format("PixelSpacing=\"%s\" ", psp));
    f.Write("CustomModel=\"");
    f.Write(cm);
    f.Write("\" ");
    f.Write(wxString::Format("SourceVersion=\"%s\" ", v));
    f.Write(ExportSuperStringColors());
    f.Write(" >\n");
    wxString face = SerialiseFace();
    if (face != "") {
        f.Write(face);
    }
    wxString state = SerialiseState();
    if (state != "") {
        f.Write(state);
    }
    wxString bufsubmodel = CreateBufferAsSubmodel();
    if (bufsubmodel != "") {
        f.Write(bufsubmodel);
    }
    wxString submodel = SerialiseSubmodel();
    if (submodel != "") {
        f.Write(submodel);
    }
    // we only save the dimensions if it is a boxed location
    auto msl = dynamic_cast<const BoxedScreenLocation*>(&GetModelScreenLocation());
    if (msl != nullptr) {
        ExportDimensions(f);
    }
    f.Write("</custommodel>");
    f.Close();
}

wxString Model::ExportSuperStringColors() const
{
    if (superStringColours.size() == 0) {
        return "";
    }
    wxString colors;
    for (int i = 0; i < superStringColours.size(); ++i) {
        wxString c = superStringColours[i];
        colors += wxString::Format("SuperStringColour%d=\"%s\" ", i, c);
    }
    return colors;
}

void Model::ImportSuperStringColours(wxXmlNode* root)
{
    bool found = true;
    int index = 0;
    while (found) {
        auto an = wxString::Format("SuperStringColour%d", index);
        if (root->HasAttribute(an)) {
            superStringColours.push_back(xlColor(root->GetAttribute(an)));
        }
        else {
            found = false;
        }

        index++;
    }

    SaveSuperStringColours();
}

bool Model::FindCustomModelScale(int scale) const
{
    size_t nodeCount = GetNodeCount();
    if (nodeCount <= 1) {
        return true;
    }
    for (int i = 0; i < nodeCount; i++) {
        for (int j = i + 1; j < nodeCount; j++) {
            int x1 = (Nodes[i]->Coords[0].screenX * scale);
            int y1 = (Nodes[i]->Coords[0].screenY * scale);
            int x2 = (Nodes[j]->Coords[0].screenX * scale);
            int y2 = (Nodes[j]->Coords[0].screenY * scale);
            if (x1 == x2 && y1 == y2) {
                return false;
            }
        }
    }
    return true;
}

std::string Model::GetStartLocation() const
{
    if (!IsLtoR) {
     if (!isBotToTop)
         return "Top Right";
     else
         return "Bottom Right";
    }
     else {
     if (!isBotToTop)
         return "Top Left";
     else
         return "Bottom Left";
    }
}

std::string Model::ChannelLayoutHtml(OutputManager* outputManager)
{
    size_t NodeCount = GetNodeCount();

    std::vector<int> chmap;
    chmap.resize(BufferHt * BufferWi, 0);

    std::string direction = GetStartLocation();

    int32_t sc;
    Controller* c = outputManager->GetController(this->GetFirstChannel() + 1, sc);

    std::string html = "<html><body><table border=0>";
    html += "<tr><td>Name:</td><td>" + name + "</td></tr>";
    html += "<tr><td>Display As:</td><td>" + DisplayAs + "</td></tr>";
    html += "<tr><td>String Type:</td><td>" + StringType + "</td></tr>";
    html += "<tr><td>Start Corner:</td><td>" + direction + "</td></tr>";
    html += wxString::Format("<tr><td>Total nodes:</td><td>%d</td></tr>", (int)NodeCount);
    html += wxString::Format("<tr><td>Height:</td><td>%d</td></tr>", BufferHt);

    if (c != nullptr) {
        html += wxString::Format("<tr><td>Controller:</td><td>%s</td></tr>", c->GetLongDescription());
    }

    if (GetControllerProtocol() != "") {
        html += wxString::Format("<tr><td>Pixel protocol:</td><td>%s</td></tr>", GetControllerProtocol().c_str());
        if (GetNumStrings() == 1) {
            html += wxString::Format("<tr><td>Controller Connection:</td><td>%d</td></tr>", GetControllerPort(1));
        } else {
            html += wxString::Format("<tr><td>Controller Connections:</td><td>%d-%d</td></tr>", GetControllerPort(1), GetControllerPort(GetNumPhysicalStrings()));
        }
    }
    html += "</table><p>Node numbers starting with 1 followed by string number:</p><table border=1>";

    if (BufferHt == 1) {
        // single line or arch or cane
        html += "<tr>";
        for (size_t i = 1; i <= NodeCount; i++) {
            int n = IsLtoR ? i : NodeCount - i + 1;
            int s = Nodes[n - 1]->StringNum + 1;
            wxString bgcolor = s % 2 == 1 ? "#ADD8E6" : "#90EE90";
            while (n > NodesPerString()) {
                n -= NodesPerString();
            }
            html += wxString::Format("<td bgcolor='" + bgcolor + "'>n%ds%d</td>", n, s);
        }
        html += "</tr>";
    } else if (BufferHt > 1) {
        // horizontal or vertical matrix or frame
        for (size_t i = 0; i < NodeCount; i++) {
            size_t idx = Nodes[i]->Coords[0].bufY * BufferWi + Nodes[i]->Coords[0].bufX;
            if (idx < chmap.size()) {
                chmap[idx] = i + 1;
            }
        }
        for (int y = BufferHt - 1; y >= 0; y--) {
            html += "<tr>";
            for (int x = 0; x < BufferWi; x++) {
                int n = chmap[y * BufferWi + x];
                if (n == 0) {
                    html += "<td></td>";
                } else {
                    int s = Nodes[n - 1]->StringNum + 1;
                    wxString bgcolor = (s % 2 == 1) ? "#ADD8E6" : "#90EE90";
                    while (n > NodesPerString()) {
                        n -= NodesPerString();
                    }
                    html += wxString::Format("<td bgcolor='" + bgcolor + "'>n%ds%d</td>", n, s);
                }
            }
            html += "</tr>";
        }
    } else {
        html += "<tr><td>Error - invalid height</td></tr>";
    }
    html += "</table></body></html>";
    return html;
}

// initialize screen coordinates
void Model::CopyBufCoord2ScreenCoord()
{
    size_t NodeCount = GetNodeCount();
    int xoffset = BufferWi / 2;
    int yoffset = BufferHt / 2;
    for (size_t n = 0; n < NodeCount; n++) {
        size_t CoordCount = GetCoordCount(n);
        for (size_t c = 0; c < CoordCount; c++) {
            Nodes[n]->Coords[c].screenX = Nodes[n]->Coords[c].bufX - xoffset;
            Nodes[n]->Coords[c].screenY = Nodes[n]->Coords[c].bufY - yoffset;
        }
    }
    GetModelScreenLocation().SetRenderSize(BufferWi, BufferHt, GetModelScreenLocation().GetRenderDp());
}

void Model::UpdateXmlWithScale()
{
    GetModelScreenLocation().Write(ModelXml);
    ModelXml->DeleteAttribute("StartChannel");
    if (ModelXml->HasAttribute("versionNumber"))
        ModelXml->DeleteAttribute("versionNumber");
    ModelXml->AddAttribute("versionNumber", CUR_MODEL_POS_VER);
    ModelXml->AddAttribute("StartChannel", ModelStartChannel);
    IncrementChangeCount();
}

bool Model::HitTest(ModelPreview* preview, glm::vec3& ray_origin, glm::vec3& ray_direction)
{
    return GetModelScreenLocation().HitTest(ray_origin, ray_direction);
}

wxCursor Model::InitializeLocation(int &handle, wxCoord x, wxCoord y, ModelPreview* preview) {
    return GetModelScreenLocation().InitializeLocation(handle, x, y, Nodes, preview);
}

void Model::ApplyTransparency(xlColor& color, int transparency, int blackTransparency)
{
    int colorAlpha = 255;
    if (color == xlBLACK) {
        if (blackTransparency) {
            float t = 100.0f - blackTransparency;
            t *= 2.55f;
            int i = std::floor(t);
            colorAlpha = i > 255 ? 255 : (i < 0 ? 0 : i);
        }
    } else if (transparency || blackTransparency){
        int maxCol = std::max(color.red, std::max(color.green, color.blue));
        if (transparency) {
            float t = 100.0f - transparency;
            t *= 2.55f;
            colorAlpha = std::floor(t);
        }
        if (maxCol < 64 && blackTransparency) {
            //if we're getting close to black, we'll start migrating toward the black's transparency setting
            float t = 100.0f - blackTransparency;
            t *= 2.55f;
            int blackAlpha = std::floor(t);
            t = maxCol * colorAlpha + (64 - maxCol) * blackAlpha;
            t /= 64;
            colorAlpha = std::floor(t);
        }
    }
    color.alpha = colorAlpha;
}

void Model::DisplayModelOnWindow(ModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *solidProgram, xlGraphicsProgram *transparentProgram, bool is_3d,
                                  const xlColor* c, bool allowSelected, bool wiring, bool highlightFirst, int highlightpixel,
                                  float *boundingBox) {

    if (!IsActive() && preview->IsNoCurrentModel()) { return; }
    size_t NodeCount = Nodes.size();
    xlColor color;
    xlColor saveColor;
    if (c != nullptr) {
        color = *c;
        saveColor = *c;
    }

    int w, h;
    preview->GetVirtualCanvasSize(w, h);

    ModelScreenLocation& screenLocation = GetModelScreenLocation();
    screenLocation.PrepareToDraw(is_3d, allowSelected);

    const std::string &cacheKey = allowSelected
        ? (is_3d ? LAYOUT_PREVIEW_CACHE_3D : LAYOUT_PREVIEW_CACHE_2D)
        : (is_3d ? MODEL_PREVIEW_CACHE_3D : MODEL_PREVIEW_CACHE_2D);
    if (uiObjectsInvalid) {
        deleteUIObjects();
    }
    bool created = false;
    auto cache = uiCaches[cacheKey];
    // nothing in the cache is dependent on preview size/rotation/etc..., the cached program is
    // size indepentent and thus can be re-used
    if (cache == nullptr) {
        screenLocation.UpdateBoundingBox(Nodes);
        cache = new PreviewGraphicsCacheInfo();
        uiCaches[cacheKey] = cache;
        created = true;

        size_t vcount = 0;
        for (const auto& it : Nodes) {
            vcount += it.get()->Coords.size();
        }
        if (_pixelStyle == PIXEL_STYLE::PIXEL_STYLE_SOLID_CIRCLE ||  _pixelStyle == PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE) {
            int f = pixelSize;
            if (pixelSize < 16) {
                f = 16;
            }
            vcount = vcount * f * 3;
        }
        if (vcount > maxVertexCount) {
            maxVertexCount = vcount;
        }
        bool needTransparent = false;
        if (_pixelStyle == PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE || transparency != 0 || blackTransparency != 0) {
            needTransparent = true;
        }
        cache->isTransparent = needTransparent;
        cache->program = ctx->createGraphicsProgram();
        cache->vica = ctx->createVertexIndexedColorAccumulator();
        cache->vica->SetName(GetName() + (is_3d ? " - 3DPreview" : " - 2DPreview"));
        cache->vica->PreAlloc(vcount);
        cache->vica->SetColorCount(_pixelStyle == PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE ? NodeCount * 2 : NodeCount);

        float modelPixelSize = pixelSize;
        //pixelSize is in world coordinate sizes, not model size.  Thus, we need to reverse the matrices to
        //get the size to use for the pixelStyle 3/4 that use triangles
        if (_pixelStyle == PIXEL_STYLE::PIXEL_STYLE_SOLID_CIRCLE || _pixelStyle == PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE) {
            modelPixelSize = preview->calcPixelSize(pixelSize);

            float x1 = -1, y1 = -1, z1 = -1;
            float x2 = 1, y2 = 1, z2 = 1;
            GetModelScreenLocation().TranslatePoint(x1, y1, z1);
            GetModelScreenLocation().TranslatePoint(x2, y2, z2);

            glm::vec3 a = glm::vec3(x2, y2, z2) - glm::vec3(x1, y1, z1);
            float length =std::max(std::max(std::abs(a.x), std::abs(a.y)), std::abs(a.z));
            modelPixelSize /= std::abs(length);
        }


        int first = 0;
        int last = NodeCount;
        int buffFirst = -1;
        int buffLast = -1;
        bool left = true;
        //int lastChan = -999;
        while (first < last) {
            int n;
            if (left) {
                n = first;
                first++;
                if (NodeRenderOrder() == 1) {
                    if (buffFirst == -1) {
                        buffFirst = Nodes[n]->Coords[0].bufX;
                    }
                    if (first < NodeCount && buffFirst != Nodes[first]->Coords[0].bufX) {
                        left = false;
                    }
                }
            } else {
                last--;
                n = last;
                if (buffLast == -1) {
                    buffLast = Nodes[n]->Coords[0].bufX;
                }
                if (last > 0 && buffFirst != Nodes[last - 1]->Coords[0].bufX) {
                    left = true;
                }
            }

            size_t CoordCount=GetCoordCount(n);
            for(size_t c2=0; c2 < CoordCount; ++c2) {
                // draw node on screen
                float sx = Nodes[n]->Coords[c2].screenX;
                float sy = Nodes[n]->Coords[c2].screenY;
                float sz = Nodes[n]->Coords[c2].screenZ;

                if (n == 0 && c2 == 0) {
                    cache->boundingBox[0] = sx;
                    cache->boundingBox[1] = sy;
                    cache->boundingBox[2] = sz;
                    cache->boundingBox[3] = sx;
                    cache->boundingBox[4] = sy;
                    cache->boundingBox[5] = sz;
                } else {
                    cache->boundingBox[0] = std::min(sx, cache->boundingBox[0]);
                    cache->boundingBox[1] = std::min(sy, cache->boundingBox[1]);
                    cache->boundingBox[2] = std::min(sz, cache->boundingBox[2]);
                    cache->boundingBox[3] = std::max(sx, cache->boundingBox[3]);
                    cache->boundingBox[4] = std::max(sy, cache->boundingBox[4]);
                    cache->boundingBox[5] = std::max(sz, cache->boundingBox[5]);
                }
                if (_pixelStyle == PIXEL_STYLE::PIXEL_STYLE_SQUARE || _pixelStyle == PIXEL_STYLE::PIXEL_STYLE_SMOOTH) {
                    cache->vica->AddVertex(sx, sy, sz, n);
                } else {
                    int eidx = n;
                    if (_pixelStyle == PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE) {
                        eidx += NodeCount;
                    }
                    cache->vica->AddCircleAsTriangles(sx, sy, sz, ((float)modelPixelSize) / 2.0f, n, eidx, pixelSize);
                }
            }

            //lastChan = Nodes[n]->ActChan;
        }
        cache->program->addStep([=](xlGraphicsContext *ctx) {
            if (_pixelStyle == PIXEL_STYLE::PIXEL_STYLE_SOLID_CIRCLE || _pixelStyle == PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE) {
                ctx->drawTriangles(cache->vica, 0, cache->vica->getCount());
            } else {
                ModelPreview *preview = (ModelPreview *)ctx->getWindow();
                float pointSize = preview->calcPixelSize(pixelSize);
                ctx->drawPoints(cache->vica, pointSize, _pixelStyle == PIXEL_STYLE::PIXEL_STYLE_SMOOTH, 0, cache->vica->getCount());
            }
        });
    }
    for (int n = 0; n < NodeCount; ++n) {
        if (n+1 == highlightpixel) {
            color = xlMAGENTA;
        } else if (highlightFirst && Nodes.size() > 1) {
            if (IsNodeFirst(n)) {
                color = xlCYAN;
            } else {
                color = saveColor;
            }
        } else if (c == nullptr) {
            Nodes[n]->GetColor(color);
            if (Nodes[n]->model->modelDimmingCurve != nullptr) {
                Nodes[n]->model->modelDimmingCurve->reverse(color);
            }
            if (Nodes[n]->model->StrobeRate) {
                int r = rand() % 5;
                if (r != 0) {
                    color = xlBLACK;
                }
            }
        }
        ApplyTransparency(color, transparency, blackTransparency);
        cache->vica->SetColor(n, color);
        if (_pixelStyle == PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE) {
            xlColor c2(color);
            c2.alpha = 0;
            cache->vica->SetColor(n + NodeCount, c2);
        }
    }
    if (created) {
        cache->vica->Finalize(false, true);
    } else {
        cache->vica->FlushColors(0, _pixelStyle == PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE ? NodeCount * 2 : NodeCount);
    }
    if (boundingBox) {
        boundingBox[0] = cache->boundingBox[0];
        boundingBox[1] = cache->boundingBox[1];
        boundingBox[2] = cache->boundingBox[2];
        boundingBox[3] = cache->boundingBox[3];
        boundingBox[4] = cache->boundingBox[4];
        boundingBox[5] = cache->boundingBox[5];
    }
    xlGraphicsProgram *p =  cache->isTransparent ? transparentProgram : solidProgram;
    if (wiring && NodeCount > 1 && cache->va == nullptr) {
        cache->va = ctx->createVertexAccumulator();
        cache->va->SetName(GetName() + (is_3d ? " - 3DPWiring" : " - 2DWiring"));
        cache->va->PreAlloc(NodeCount);
        for (int x = 0; x < NodeCount; x++) {
            float sx = Nodes[x]->Coords[0].screenX;
            float sy = Nodes[x]->Coords[0].screenY;
            float sz = Nodes[x]->Coords[0].screenZ;
            cache->va->AddVertex(sx, sy, sz);
        }
        cache->va->Finalize(false);
    }

    p->addStep([=](xlGraphicsContext *ctx) {
        // cache has the model in model coordinates
        // we need to scale/translate/etc.... to world
        ctx->PushMatrix();
        if (!is_3d) {
            //not 3d, flatten to the 0 plane
            ctx->Scale(1.0, 1.0, 0.0);
        }
        GetModelScreenLocation().ApplyModelViewMatrices(ctx);
        cache->program->runSteps(ctx);
        if (wiring && cache->va != nullptr) {
            ctx->drawLineStrip(cache->va, xlColor(0x49, 0x80, 0x49));
        }
        ctx->PopMatrix();
    });


    if ((Selected || (Highlighted && is_3d)) && c != nullptr && allowSelected) {
        if (is_3d) {
            GetModelScreenLocation().DrawHandles(transparentProgram, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), Highlighted, IsFromBase());
        } else {
            GetModelScreenLocation().DrawHandles(transparentProgram, preview->GetCameraZoomForHandles(), preview->GetHandleScale(), IsFromBase());
        }
    }
}

float Model::GetPreviewDimScale(ModelPreview* preview, int& w, int& h)
{
    preview->GetSize(&w, &h);
    float scaleX = float(w) * 0.95 / GetModelScreenLocation().RenderWi;
    float scaleY = float(h) * 0.95 / GetModelScreenLocation().RenderHt;
    float scale = scaleY < scaleX ? scaleY : scaleX;

    return scale;
}

void Model::GetScreenLocation(float& sx, float& sy, const NodeBaseClass::CoordStruct& it2, int w, int h, float scale)
{
    sx = it2.screenX;
    sy = it2.screenY;

    if (!GetModelScreenLocation().IsCenterBased()) {
        sx -= GetModelScreenLocation().RenderWi / 2.0;
        sy *= GetModelScreenLocation().GetVScaleFactor();
        if (GetModelScreenLocation().GetVScaleFactor() < 0) {
            sy += GetModelScreenLocation().RenderHt / 2.0;
        } else {
            sy -= GetModelScreenLocation().RenderHt / 2.0;
        }
    }
    sy = ((sy * scale) + (h / 2));
    sx = (sx * scale) + (w / 2);
}

wxString Model::GetNodeNear(ModelPreview* preview, wxPoint pt, bool flip)
{
    int w, h;
    float scale = GetPreviewDimScale(preview, w, h);

    float pointScale = scale;
    if (pointScale > 2.5) {
        pointScale = 2.5;
    }
    if (pointScale > GetModelScreenLocation().RenderHt) {
        pointScale = GetModelScreenLocation().RenderHt;
    }
    if (pointScale > GetModelScreenLocation().RenderWi) {
        pointScale = GetModelScreenLocation().RenderWi;
    }

    float px = pt.x;
    float py = pt.y;
    if (flip)
        py = h - pt.y;

    int i = 1;
    for (const auto& it : Nodes) {
        auto c = it.get()->Coords;
        for (const auto& it2 : c) {
            float sx, sy;
            GetScreenLocation(sx, sy, it2, w, h, scale);

            if (sx >= (px - pointScale) && sx <= (px + pointScale) &&
                sy >= (py - pointScale) && sy <= (py + pointScale)) {
                return wxString::Format(wxT("%i"), i);
            }
        }
        i++;
    }
    return "";
}

bool Model::GetScreenLocations(ModelPreview* preview, std::map<int, std::pair<float, float>>& coords)
{
    int w, h;
    float scale = GetPreviewDimScale(preview, w, h);

    int i = 1;
    for (const auto& it : Nodes) {
        auto c = it.get()->Coords;
        if (c.size() != 1)
            return false;
        for (const auto& it2 : c) {
            float sx, sy;
            GetScreenLocation(sx, sy, it2, w, h, scale);
            coords[i] = std::make_pair(sx, sy);
        }
        ++i;
    }
    return true;
}

std::vector<int> Model::GetNodesInBoundingBox(ModelPreview* preview, wxPoint start, wxPoint end)
{
    int w, h;
    float scale = GetPreviewDimScale(preview, w, h);

    std::vector<int> nodes;

    float startpx = start.x;
    float startpy = /*h -*/ start.y;
    float endpx = end.x;
    float endpy = /*h -*/ end.y;

    if (startpx > endpx) {
        float tmp = startpx;
        startpx = endpx;
        endpx = tmp;
    }

    if (startpy > endpy) {
        float tmp = startpy;
        startpy = endpy;
        endpy = tmp;
    }

    int i = 1;
    for (const auto& it : Nodes) {
        auto c = it.get()->Coords;
        for (const auto& it2 : c) {
            float sx, sy;
            GetScreenLocation(sx, sy, it2, w, h, scale);

            if (sx >= startpx && sx <= endpx &&
                sy >= startpy && sy <= endpy)
            {
                nodes.push_back(i);
            }
        }
        i++;
    }
    return nodes;
}

bool Model::IsMultiCoordsPerNode() const
{
    for (const auto& it : Nodes)         {
        if (it.get()->Coords.size() > 1) return true;
    }
    return false;
}

void Model::DisplayEffectOnWindow(ModelPreview* preview, double pointSize)
{
    if (!IsActive() && preview->IsNoCurrentModel()) {
        return;
    }

    bool mustEnd = false;
    xlGraphicsContext* ctx = preview->getCurrentGraphicsContext();
    if (ctx == nullptr) {
        bool success = preview->StartDrawing(pointSize);
        if (success) {
            ctx = preview->getCurrentGraphicsContext();
            mustEnd = true;
        }
    }

    if (ctx) {
        if (uiObjectsInvalid) {
            deleteUIObjects();
        }

        int w, h;
        float scale = GetPreviewDimScale(preview, w, h);

        size_t NodeCount = Nodes.size();
        bool created = false;

        int renderWi = GetModelScreenLocation().RenderWi;
        int renderHi = GetModelScreenLocation().RenderHt;

        float ml, mb;
        GetMinScreenXY(ml, mb);
        ml += GetModelScreenLocation().RenderWi / 2;
        mb += GetModelScreenLocation().RenderHt / 2;

        auto cache = uiCaches[EFFECT_PREVIEW_CACHE];
        // nothing in the cache is dependent on preview size, the cached program is
        // size indepentent and thus can be re-used unless the models rendeWi/Hi
        // changes (which should trigger the uiObjectsInvalid and clear
        // the cache anyway)
        if (cache == nullptr || cache->renderWi != renderWi || cache->renderHi != renderHi
            || cache->modelChangeCount != this->changeCount) {
            if (cache != nullptr) {
                delete cache;
            }
            cache = new PreviewGraphicsCacheInfo();
            uiCaches[EFFECT_PREVIEW_CACHE] = cache;

            cache->width = w;
            cache->height = h;
            cache->renderWi = renderWi;
            cache->renderHi = renderHi;
            cache->modelChangeCount = this->changeCount;

            created = true;

            float pointScale = scale;
            if (pointScale > 2.5) {
                pointScale = 2.5;
            }
            if (pointScale > GetModelScreenLocation().RenderHt) {
                pointScale = GetModelScreenLocation().RenderHt;
            }
            if (pointScale > GetModelScreenLocation().RenderWi) {
                pointScale = GetModelScreenLocation().RenderWi;
            }

            PIXEL_STYLE lastPixelStyle = _pixelStyle;
            int lastPixelSize = pixelSize;

            // layer calculation and map to output
            unsigned int vcount = 0;
            for (const auto& it : Nodes) {
                vcount += it.get()->Coords.size();
            }
            if (vcount > maxVertexCount) {
                maxVertexCount = vcount;
            }

            cache->vica = ctx->createVertexIndexedColorAccumulator();
            cache->vica->SetName(GetName() + " - Preview");
            cache->program = ctx->createGraphicsProgram();

            cache->vica->SetColorCount(NodeCount * 2); // upper one is for the clear edges of blended circles
            cache->vica->PreAlloc(maxVertexCount);

            int startVertex = 0;

            int first = 0;
            int last = NodeCount;
            int buffFirst = -1;
            int buffLast = -1;
            bool left = true;
            while (first < last) {
                int n;
                if (left) {
                    n = first;
                    ++first;
                    if (NodeRenderOrder() == 1) {
                        if (buffFirst == -1) {
                            buffFirst = Nodes[n]->Coords[0].bufX;
                        }
                        if (first < NodeCount && buffFirst != Nodes[first]->Coords[0].bufX) {
                            left = false;
                        }
                    }
                } else {
                    --last;
                    n = last;
                    if (buffLast == -1) {
                        buffLast = Nodes[n]->Coords[0].bufX;
                    }
                    if (last > 0 && buffFirst != Nodes[last - 1]->Coords[0].bufX) {
                        left = true;
                    }
                }

                size_t CoordCount = GetCoordCount(n);
                for (size_t c = 0; c < CoordCount; ++c) {
                    // draw node on screen
                    float newsx = Nodes[n]->Coords[c].screenX;
                    float newsy = Nodes[n]->Coords[c].screenY;

                    if (lastPixelStyle != Nodes[n]->model->_pixelStyle || lastPixelSize != Nodes[n]->model->pixelSize) {
                        if (cache->vica->getCount() && (lastPixelStyle == PIXEL_STYLE::PIXEL_STYLE_SQUARE ||
                                                        lastPixelStyle == PIXEL_STYLE::PIXEL_STYLE_SMOOTH ||
                                                        Nodes[n]->model->_pixelStyle == PIXEL_STYLE::PIXEL_STYLE_SQUARE ||
                                                        Nodes[n]->model->_pixelStyle == PIXEL_STYLE::PIXEL_STYLE_SMOOTH))
                        {
                            int count = cache->vica->getCount();
                            cache->program->addStep([=](xlGraphicsContext* ctx) {
                                if (lastPixelStyle == PIXEL_STYLE::PIXEL_STYLE_SOLID_CIRCLE || lastPixelStyle == PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE) {
                                    ctx->drawTriangles(cache->vica, startVertex, count - startVertex);
                                } else {
                                    ModelPreview* preview = (ModelPreview*)ctx->getWindow();
                                    float pointSize = preview->calcPixelSize(lastPixelSize * pointScale);
                                    ctx->drawPoints(cache->vica, pointSize, lastPixelStyle == PIXEL_STYLE::PIXEL_STYLE_SMOOTH, startVertex, count - startVertex);
                                }
                            });
                            startVertex = count;
                        }
                        lastPixelStyle = Nodes[n]->model->_pixelStyle;
                        lastPixelSize = Nodes[n]->model->pixelSize;
                    }

                    if (lastPixelStyle == PIXEL_STYLE::PIXEL_STYLE_SQUARE ||
                        lastPixelStyle == PIXEL_STYLE::PIXEL_STYLE_SMOOTH) {
                        cache->vica->AddVertex(newsx, newsy, n);
                    } else {
                        uint32_t ecolor = n;
                        if (lastPixelStyle == PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE) {
                            ecolor += NodeCount;
                        }
                        cache->vica->AddCircleAsTriangles(newsx, newsy, 0, lastPixelSize * pointScale, n, ecolor);
                    }
                }
            }
            if (int(cache->vica->getCount()) > startVertex) {
                int count = cache->vica->getCount();
                cache->program->addStep([=](xlGraphicsContext* ctx) {
                    if (lastPixelStyle == PIXEL_STYLE::PIXEL_STYLE_SOLID_CIRCLE || lastPixelStyle == PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE) {
                        ctx->drawTriangles(cache->vica, startVertex, count - startVertex);
                    } else {
                        ModelPreview* preview = (ModelPreview*)ctx->getWindow();
                        float pointSize = preview->calcPixelSize(lastPixelSize * pointScale);
                        ctx->drawPoints(cache->vica, pointSize, lastPixelStyle == PIXEL_STYLE::PIXEL_STYLE_SMOOTH, startVertex, count - startVertex);
                    }
                });
            }
        }
        int maxFlush = NodeCount;
        for (int n = 0; n < NodeCount; ++n) {
            xlColor color;
            Nodes[n]->GetColor(color);
            if (Nodes[n]->model->modelDimmingCurve != nullptr) {
                Nodes[n]->model->modelDimmingCurve->reverse(color);
            }
            if (Nodes[n]->model->StrobeRate) {
                int r = rand() % 5;
                if (r != 0) {
                    color = xlBLACK;
                }
            }
            xlColor c2(color);
            ApplyTransparency(c2, Nodes[n]->model->transparency, Nodes[n]->model->blackTransparency);
            cache->vica->SetColor(n, c2);
            if (Nodes[n]->model->_pixelStyle == PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE) {
                c2.alpha = 0;
                cache->vica->SetColor(n + NodeCount, c2);
                maxFlush = n + NodeCount + 1;
            }
        }
        if (created) {
            cache->vica->Finalize(false, true);
        } else {
            cache->vica->FlushColors(0, maxFlush);
        }

        preview->getCurrentSolidProgram()->addStep([=](xlGraphicsContext* ctx) {
            // cache has the model in model coordinates
            // we need to scale/translate/etc.... to world
            ctx->PushMatrix();
            ctx->Translate(w / 2.0f - (ml < 0.0f ? ml : 0.0f),
                           h / 2.0f - (mb < 0.0f ? mb : 0.0f), 0.0f);
            ctx->Scale(scale, scale, 1.0);
            if (!GetModelScreenLocation().IsCenterBased()) {
                ctx->Translate(-GetModelScreenLocation().RenderWi / 2.0,
                               GetModelScreenLocation().GetVScaleFactor() < 0 ? GetModelScreenLocation().RenderHt / 2.0 : -GetModelScreenLocation().RenderHt / 2.0,
                               0.0f);
                ctx->Scale(1.0, GetModelScreenLocation().GetVScaleFactor(), 1.0);
            }
            cache->program->runSteps(ctx);
            ctx->PopMatrix();
        });
        if (mustEnd) {
            preview->EndDrawing();
        }
    }
}

glm::vec3 Model::MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX,int mouseY) {

    if (GetModelScreenLocation().IsLocked() || IsFromBase())
        return GetModelScreenLocation().GetHandlePosition(handle);

    int i = GetModelScreenLocation().MoveHandle(preview, handle, ShiftKeyPressed, mouseX, mouseY);
    GetModelScreenLocation().Write(ModelXml);
    if (i) {
        SetFromXml(ModelXml);
    }
    IncrementChangeCount();

    return GetModelScreenLocation().GetHandlePosition(handle);
}

int Model::GetSelectedHandle() {
    return GetModelScreenLocation().GetSelectedHandle();
}

int Model::GetNumHandles() {
    return GetModelScreenLocation().GetNumHandles();
}

int Model::GetSelectedSegment() {
    return GetModelScreenLocation().GetSelectedSegment();
}

bool Model::SupportsCurves() {
    return GetModelScreenLocation().SupportsCurves();
}

bool Model::HasCurve(int segment) {
    return GetModelScreenLocation().HasCurve(segment);
}

void Model::SetCurve(int segment, bool create) {
    return GetModelScreenLocation().SetCurve(segment, create);
}

void Model::AddHandle(ModelPreview* preview, int mouseX, int mouseY) {
    GetModelScreenLocation().AddHandle(preview, mouseX, mouseY);
}

void Model::InsertHandle(int after_handle, float zoom, int scale) {

    if (GetModelScreenLocation().IsLocked() || IsFromBase())
        return;

    GetModelScreenLocation().InsertHandle(after_handle, zoom, scale);
}

void Model::DeleteHandle(int handle) {

    if (GetModelScreenLocation().IsLocked() || IsFromBase())
        return;

    GetModelScreenLocation().DeleteHandle(handle);
}

int Model::GetStrandLength(int strand) const {
    int numStrands = std::max( 1, GetNumStrands() );
    return GetNodeCount() / numStrands;
}

int Model::MapToNodeIndex(int strand, int node) const {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    //if ((DisplayAs == wxT("Vert Matrix") || DisplayAs == wxT("Horiz Matrix") || DisplayAs == wxT("Matrix")) && SingleChannel) {
    //    return node;
    //}
    if (GetNumStrands() == 1) {
        return node;
    }
    if (SingleNode) {
        return strand;
    }
    if (parm3 == 0) {
        logger_base.crit("Map node to index with illegal parm3 = 0.");
        return node;
    }
    return (strand * parm2 / parm3) + node;
}

void Model::RecalcStartChannels()
{
    modelManager.RecalcStartChannels();
}

bool Model::RenameController(const std::string& oldName, const std::string& newName)
{
    wxASSERT(newName != "");

    bool changed = false;

    if (GetControllerName() == oldName) {
        SetControllerName(newName);
        changed = true;
    }
    if (StartsWith(ModelStartChannel, "!" + oldName)) {
        ModelStartChannel = "!" + newName + ModelStartChannel.substr(oldName.size() + 1);
        ModelXml->DeleteAttribute("StartChannel");
        ModelXml->AddAttribute("StartChannel", ModelStartChannel);
        changed = true;
    }
    if (ModelXml->GetAttribute("Advanced") == "1") {
        for (int i = 0; i < parm1; i++) {
            auto str = StartChanAttrName(i);
            if (ModelXml->HasAttribute(str)) {
                auto sc = ModelXml->GetAttribute(str);
                if (StartsWith(sc, "!" + oldName)) {
                    ModelXml->DeleteAttribute(str);
                    ModelXml->AddAttribute(str, "!" + newName + sc.substr(oldName.size() + 1));
                    changed = true;
                }
            }
        }
    }
    return changed;
}

void Model::AddSizeLocationProperties(wxPropertyGridInterface* grid)
{
    GetModelScreenLocation().AddSizeLocationProperties(grid);
}

void Model::AddDimensionProperties(wxPropertyGridInterface* grid)
{
    GetModelScreenLocation().AddDimensionProperties(grid);
}

std::string Model::GetDimension() const
{
    return GetModelScreenLocation().GetDimension();
}

void Model::ImportModelChildren(wxXmlNode* root, xLightsFrame* xlights, wxString const& newname, float& min_x, float& max_x, float& min_y, float& max_y)
{
    bool merge = false;
    bool showPopup = true;
    for (wxXmlNode* n = root->GetChildren(); n != nullptr; n = n->GetNext())
    {
        if (n->GetName() == "stateInfo") {
            AddState(n);
        }
        else if (n->GetName() == "subModel") {
            AddSubmodel(n);
        }
        else if (n->GetName() == "faceInfo") {
            AddFace(n);
        } else if (n->GetName() == "ControllerConnection") {
            if (n->HasAttribute("zigZag")) {
                wxXmlNode* nn = GetControllerConnection();
                if (nn->HasAttribute("zigZag")) {
                    nn->DeleteAttribute("zigZag");
                }
                nn->AddAttribute("zigZag", n->GetAttribute("zigZag"));
            }
        } else if (n->GetName() == "modelGroup") {
            AddModelGroups(n, xlights->GetLayoutPreview()->GetVirtualCanvasWidth(),
                xlights->GetLayoutPreview()->GetVirtualCanvasHeight(), newname, merge, showPopup);
        } else if (n->GetName() == "shadowmodels") {
            ImportShadowModels(n, xlights);
        } else if (n->GetName() == "dimensions") {

            if (RulerObject::GetRuler() != nullptr) {
                std::string units = n->GetAttribute("units", "mm");
                float width = wxAtof(n->GetAttribute("width", "1000"));
                float height = wxAtof(n->GetAttribute("height", "1000"));
                float depth = wxAtof(n->GetAttribute("depth", "0"));
                ApplyDimensions(units, width, height, depth, min_x, max_x, min_y, max_y);
            }
        }
    }
}

Model* Model::GetXlightsModel(Model* model, std::string& last_model, xLightsFrame* xlights, bool& cancelled, bool download, wxProgressDialog* prog, int low, int high, ModelPreview* modelPreview)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxXmlDocument doc;
    bool docLoaded = false;
    if (last_model.empty()) {
        if (download) {
            xlights->SuspendAutoSave(true);
            VendorModelDialog dlg(xlights, xlights->CurrentDir);
            xlights->SetCursor(wxCURSOR_WAIT);
            if (dlg.DlgInit(prog, low, high)) {
                if (prog != nullptr) {
                    prog->Update(100);
                }
                xlights->SetCursor(wxCURSOR_DEFAULT);
                if (dlg.ShowModal() == wxID_OK) {
                    xlights->SuspendAutoSave(false);
                    last_model = dlg.GetModelFile();

                    if (last_model.empty()) {
                        DisplayError("Failed to download model file.");

                        cancelled = true;
                        return model;
                    }
                } else {
                    xlights->SuspendAutoSave(false);
                    cancelled = true;
                    return model;
                }
            } else {
                if (prog != nullptr) prog->Hide();
                xlights->SetCursor(wxCURSOR_DEFAULT);
                xlights->SuspendAutoSave(false);
                cancelled = true;
                return model;
            }
        } else {
            wxString filename = wxFileSelector(_("Choose model file"), wxEmptyString, wxEmptyString, wxEmptyString, "xLights Model files (*.xmodel)|*.xmodel|LOR prop files (*.lff;*.lpf)|*.lff;*.lpf|General Device Type Format (*.gdtf)|*.gdtf", wxFD_OPEN);
            if (filename.IsEmpty()) {
                cancelled = true;
                return model;
            }
            last_model = filename.ToStdString();

            if (wxString(last_model).Lower().EndsWith(".xmodel")) {
                doc.Load(last_model);
                if (doc.IsOk() && doc.GetRoot()->GetAttribute("name", "") != "") {
                    docLoaded = true;
                    wxString modelName = doc.GetRoot()->GetAttribute("name", "");
#ifdef __WXMSW__
                    // If a windows user does not want vendor recommendations then dont go looking for them at all
                    // I have allowed this to be off (ie it does the vendor recommendation check) by default but once
                    // the user has decided they dont want it then treat them like an adult
                    if (!xlights->GetIgnoreVendorModelRecommendations()) {
#endif
                        wxURI mappingJson("https://raw.githubusercontent.com/smeighan/xLights/master/download/model_vendor_mapping.json");
                        std::string json = CachedFileDownloader::GetDefaultCache().GetFile(mappingJson, CACHETIME_DAY);
                        if (json == "") {
                            json = wxStandardPaths::Get().GetResourcesDir() + "/model_vendor_mapping.json";
                        }
                        if (json != "" && !FileExists(json)) {
                            json = "";
                        }
                        if (json != "") {
                            wxJSONValue origJson;
                            wxJSONReader reader;
                            wxFileInputStream f(json);
                            int errors = reader.Parse(f, &origJson);
                            if (!errors) {
                                VendorModelDialog* dlg = nullptr;
#ifndef __WXMSW__
                                bool block = false;
#endif
                                wxString vendorBlock;
                                for (auto& name : origJson["mappings"].GetMemberNames()) {
                                    wxJSONValue v = origJson["mappings"][name];
                                    bool matches = false;
                                    wxString newModelName = modelName;
                                    bool localBlock = false;
                                    if (v.HasMember("regex") && v["regex"].AsBool()) {
                                        wxRegEx regex;
                                        if (regex.Compile(name)) {
                                            if (regex.Matches(modelName)) {
                                                wxString nmodel = v["model"].AsString();
                                                regex.ReplaceAll(&newModelName, nmodel);
                                                matches = true;
                                                if (v.HasMember("block")) {
                                                    localBlock = v["block"].AsBool();
                                                }
                                            }
                                        }
                                    }
                                    else if (name == modelName) {
                                        matches = true;
                                        newModelName = v["model"].AsString();
                                        if (v.HasMember("block")) {
                                            localBlock = v["block"].AsBool();
                                        }
                                    }
                                    if (matches) {
                                        wxString vendor = v["vendor"].AsString();
                                        if (dlg == nullptr) {
                                            dlg = new VendorModelDialog(xlights, xlights->CurrentDir);
                                            dlg->DlgInit(prog, low, high);
                                        }
                                        if (localBlock) {
                                            vendorBlock = vendor;
#ifndef __WXMSW__
                                            block = true;
#endif
                                        }
                                        if (dlg->FindModelFile(vendor, newModelName)) {
                                            if (localBlock) {
                                                wxString msg = "'" + vendor + "' provides a certified model for '" + newModelName + "' in the xLights downloads.  The "
                                                    + "vendor has requested that the model they provide be the model that is used."
                                                    + "Use the Vendor provided model instead?";
                                                if (wxMessageBox(msg, "Use Vendor Certified Model?", wxYES_NO | wxICON_QUESTION, xlights) == wxYES) {
                                                    last_model = dlg->GetModelFile();
                                                }
                                                else {
                                                    last_model = "";
                                                }
                                                docLoaded = false;
                                                break;
                                            }
                                            else if (!xlights->GetIgnoreVendorModelRecommendations()) {
                                                // I do not believe we should be saying xLights recommends this as fom what I have seen this claim on quality is historically dubious and I do not believe we have
                                                // ever actually assessed the quality of their models. My own experience has been the quality of some models is poor or worse. Others are fine. No vendor in
                                                // my experience is noticably better or worse than any other ... they all have had their poor models.
                                                // If you want to change the message back then have an OSX specific phrasing.
                                                wxString msg = "xLights found a '" + vendor + "' provided and certified model for '" + newModelName + "' in the xLights downloads.  The "
                                                    + "Vendor provided models are strongly recommended by the vendor due to their claimed quality and ease of use.\n\nWould you prefer to "
                                                    + "use the Vendor provided model instead?";
                                                if (wxMessageBox(msg, "Use Vendor Certified Model?", wxYES_NO | wxICON_QUESTION, xlights) == wxYES) {
                                                    last_model = dlg->GetModelFile();
                                                    docLoaded = false;
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }
#ifndef __WXMSW__
                                // I wont incude this code in the windows release ... it is a step way too far ... the vendors should not be dictating what models a user can use
                                if (block) {
                                    wxString msg = "'" + vendorBlock + "' has requested that the models they provide be the models that are used.";
                                    wxMessageBox(msg, "Loading of Model Blocked", wxOK | wxICON_ERROR, xlights);
                                    last_model = "";
                                }
#endif
                                if (dlg) {
                                    delete dlg;
                                }
                            }
                        }
                    #ifdef __WXMSW__
                    }
                    #endif
                }
            }
        }
    }
    if (wxString(last_model).Lower().EndsWith(".gdtf")) {
        wxFileInputStream fin(last_model);
        wxZipInputStream zin(fin);
        wxZipEntry* ent = zin.GetNextEntry();

        while (ent != nullptr) {
            if (ent->GetName() == "description.xml") {
                wxXmlDocument gdtf_doc;
                gdtf_doc.Load(zin);
                if (gdtf_doc.IsOk()) {
                    std::map<std::string, wxXmlNode*> modes;

                    for (wxXmlNode* n = gdtf_doc.GetRoot()->GetChildren(); n != nullptr; n = n->GetNext()) {
                        if (n->GetName() == "FixtureType") {
                            for (wxXmlNode* nn = n->GetChildren(); nn != nullptr; nn = nn->GetNext()) {
                                if (nn->GetName() == "DMXModes") {
                                    for (wxXmlNode* nnn = nn->GetChildren(); nnn != nullptr; nnn = nnn->GetNext()) {
                                        if (nnn->GetName() == "DMXMode") {
                                            modes[nnn->GetAttribute("Name")] = nnn;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if (modes.size() == 0) break;

                    std::string mode = modes.begin()->first;
                    if (modes.size() > 1) {
                        wxArrayString choices;
                        for (const auto& it : modes) {
                            choices.push_back(it.first);
                        }
                        wxSingleChoiceDialog dlg(xlights, "Select the model mode", "DMX Model Mode", choices);
                        if (dlg.ShowModal() != wxID_OK) {
                            cancelled = true;
                            break;
                        }
                        mode = choices[dlg.GetSelection()];
                    }

                    bool isMovingHead = false;

                    class DMXChannel
                    {
                    public:

                        class DMXValue
                        {
                            int ParseValue(wxString s, int channels)
                            {
                                wxArrayString ss = wxSplit(s, '/');
                                wxASSERT(ss.size() == 2);
                                if (ss[1] == "1") {
                                    if (channels == 2) {
                                        return wxAtoi(ss[0]) << 8;
                                    }
                                    return wxAtoi(ss[0]);
                                }
                                else if (ss[1] == "2") {
                                    wxASSERT(channels == 2);
                                    return wxAtoi(ss[0]);
                                }

                                wxASSERT(false);
                                return 0;
                            }

                        public:
                            std::string _description;
                            int _low;
                            int _high;

                            DMXValue(wxXmlNode* n, wxXmlNode* nn, int channels)
                            {
                                _description = n->GetAttribute("Name").ToStdString();
                                wxASSERT(_description != "");
                                _low = ParseValue(n->GetAttribute("DMXFrom"), channels);
                                if (nn == nullptr) {
                                    _high = _low;
                                }
                                else {
                                    _high = ParseValue(nn->GetAttribute("DMXFrom"), channels) - 1;
                                }
                            }
                        };

                        std::string _attribute;
                        int _channelStart;
                        int _channels;
                        std::list<DMXValue*> _values;

                        bool IsMovingHead() const
                        {
                            return _attribute == "Pan" || _attribute == "Tilt";
                        }

                        ~DMXChannel()
                        {
                            while (_values.size() > 0) {
                                delete _values.front();
                                _values.pop_front();
                            }
                        }

                        DMXChannel(wxXmlNode* n)
                        {
                            wxString offset = n->GetAttribute("Offset");
                            wxArrayString os = wxSplit(offset, ',');
                            wxASSERT(os.size() > 0);
                            _channelStart = wxAtoi(os[0]);
                            _channels = os.size();
                            for (wxXmlNode* nn = n->GetChildren(); nn != nullptr; nn = nn->GetNext()) {
                                if (nn->GetName() == "LogicalChannel") {
                                    _attribute = nn->GetAttribute("Attribute").ToStdString();
                                    for (wxXmlNode* nnn = nn->GetChildren(); nnn != nullptr; nnn = nnn->GetNext()) {
                                        if (nnn->GetName() == "ChannelFunction") {
                                            for (wxXmlNode* nnnn = nnn->GetChildren(); nnnn != nullptr; nnnn = nnnn->GetNext()) {
                                                if (nnnn->GetName() == "ChannelSet" && !nnnn->GetAttribute("Name").empty()) {
                                                    _values.push_back(new DMXValue(nnnn, nnnn->GetNext(), _channels));
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    };

                    std::list<DMXChannel*> _channels;

                    int channels = 0;
                    for (wxXmlNode* n = modes[mode]->GetChildren(); n != nullptr; n = n->GetNext()) {
                        if (n->GetName() == "DMXChannels") {
                            for (wxXmlNode* nn = n->GetChildren(); nn != nullptr; nn = nn->GetNext()) {
                                if (nn->GetName() == "DMXChannel") {
                                    DMXChannel* c = new DMXChannel(nn);
                                    _channels.push_back(c);
                                    channels = std::max(channels, c->_channelStart + c->_channels - 1);
                                    isMovingHead |= c->IsMovingHead();
                                }
                            }
                        }
                    }

                    std::string startChannel = model->GetModelXml()->GetAttribute("StartChannel", "1").ToStdString();
                    auto x = model->GetHcenterPos();
                    auto y = model->GetVcenterPos();
                    auto w = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetScaleX();
                    auto h = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetScaleY();
                    auto lg = model->GetLayoutGroup();
                    if (model != nullptr) {
                        xlights->AddTraceMessage("GetXlightsModel converted model to DMX");
                        delete model;
                    }
                    if (isMovingHead) {
                        model = xlights->AllModels.CreateDefaultModel("DmxMovingHead3D", startChannel);
                    }
                    else {
                        model = xlights->AllModels.CreateDefaultModel("DmxMovingHead", startChannel);
                        model->GetModelXml()->DeleteAttribute("DmxStyle");
                        model->GetModelXml()->AddAttribute("DmxStyle", "Moving Head Bars");
                    }

                    model->SetHcenterPos(x);
                    model->SetVcenterPos(y);
                    // Multiply by 5 because default custom model has parm1 and parm2 set to 5 and DMX model is 1 pixel
                    ((BoxedScreenLocation&)model->GetModelScreenLocation()).SetScale(w * 5, h * 5);
                    model->GetModelScreenLocation().Write(model->GetModelXml());
                    model->SetLayoutGroup(lg);

                    // number of channels
                    model->GetModelXml()->DeleteAttribute("parm1");
                    model->GetModelXml()->AddAttribute("parm1", wxString::Format("%d", channels));

                    if (modes.size() > 0) {
                        model->GetModelXml()->DeleteAttribute("Description");
                        model->GetModelXml()->AddAttribute("Description", XmlSafe("Mode: " + mode));
                    }

                    std::vector<std::string> nodeNames = std::vector<std::string>(channels);
                    std::map<std::string, std::map<std::string, std::string> > stateInfo;

                    for (const auto& it : _channels) {
                        if (it->_attribute == "Pan") {
                            model->GetModelXml()->DeleteAttribute("DmxPanChannel");
                            model->GetModelXml()->AddAttribute("DmxPanChannel", wxString::Format("%d", it->_channelStart));
                        }
                        else if (it->_attribute == "Tilt") {
                            model->GetModelXml()->DeleteAttribute("DmxTiltChannel");
                            model->GetModelXml()->AddAttribute("DmxTiltChannel", wxString::Format("%d", it->_channelStart));
                        }
                        else if (it->_attribute == "ColorAdd_W") {
                            model->GetModelXml()->DeleteAttribute("DmxWhiteChannel");
                            model->GetModelXml()->AddAttribute("DmxWhiteChannel", wxString::Format("%d", it->_channelStart));
                        }
                        else if (it->_attribute == "ColorRGB_Red" || it->_attribute == "ColorAdd_R") {
                            model->GetModelXml()->DeleteAttribute("DmxRedChannel");
                            model->GetModelXml()->AddAttribute("DmxRedChannel", wxString::Format("%d", it->_channelStart));
                        }
                        else if (it->_attribute == "ColorRGB_Green" || it->_attribute == "ColorAdd_G") {
                            model->GetModelXml()->DeleteAttribute("DmxGreenChannel");
                            model->GetModelXml()->AddAttribute("DmxGreenChannel", wxString::Format("%d", it->_channelStart));
                        }
                        else if (it->_attribute == "ColorRGB_Blue" || it->_attribute == "ColorAdd_B") {
                            model->GetModelXml()->DeleteAttribute("DmxBlueChannel");
                            model->GetModelXml()->AddAttribute("DmxBlueChannel", wxString::Format("%d", it->_channelStart));
                        }
                        else if (it->_attribute == "Shutter" || it->_attribute == "Shutter1") {
                            model->GetModelXml()->DeleteAttribute("DmxShutterChannel");
                            model->GetModelXml()->AddAttribute("DmxShutterChannel", wxString::Format("%d", it->_channelStart));
                        }

                        if (it->_values.size() > 0) {
                            std::map<std::string, std::string> states;

                            int st = 1;
                            for (const auto& it2 : it->_values) {
                                states[wxString::Format("s%d-Name", st)] = it2->_description;
                                states[wxString::Format("s%d", st)] = wxString::Format("%d", it->_channelStart);
                                states[wxString::Format("s%d-Color", st)] = wxString::Format("#%02x%02x%02x", it2->_low, it2->_low, it2->_low);

                                st++;
                            }

                            stateInfo[it->_attribute] = states;
                        }

                        // Build the node names
                        if (it->_channels == 1) {
                            nodeNames[it->_channelStart - 1] = it->_attribute;
                        }
                        else {
                            for (int i = 0; i < it->_channels; i++) {
                                nodeNames[it->_channelStart + i - 1] = it->_attribute + wxString::Format("-%d", i + 1).ToStdString();
                            }
                        }
                    }

                    std::string nodenames = "";
                    for (const auto& s : nodeNames) {
                        if (!nodenames.empty()) nodenames += ",";
                        nodenames += s;
                    }
                    model->GetModelXml()->DeleteAttribute("NodeNames");
                    model->GetModelXml()->AddAttribute("NodeNames", nodenames);

                    if (stateInfo.size() > 0) {
                        Model::WriteStateInfo(model->GetModelXml(), stateInfo, true);
                    }

                    model->Selected = true;

                    while (_channels.size() > 0) {
                        delete _channels.front();
                        _channels.pop_front();
                    }
                }
                else {
                    cancelled = true;
                }
                break;
            }
            ent = zin.GetNextEntry();
        }
        return model;
    } else if (!wxString(last_model).Lower().EndsWith(".xmodel")) {
        // if it isnt an xmodel then it is custom
        return model;
    }

    if (!docLoaded) {
        doc.Load(last_model);
    }
    if (doc.IsOk()) {
        wxXmlNode* root = doc.GetRoot();

        if (root->GetName() == "custommodel") {
            return model;
        }
        else if (root->GetName() == "polylinemodel") {
            // not a custom model so delete the default model that was created
            std::string startChannel = model->GetModelXml()->GetAttribute("StartChannel", "1").ToStdString();
            auto lg = model->GetLayoutGroup();
            if (model != nullptr) {
                xlights->AddTraceMessage("GetXlightsModel converted model to PolyLine");
                delete model;
            }
            model = xlights->AllModels.CreateDefaultModel("Poly Line", startChannel);
            model->SetLayoutGroup(lg);
            model->Selected = true;
            return model;
        }
        else if (root->GetName() == "multipointmodel") {
            std::string startChannel = model->GetModelXml()->GetAttribute("StartChannel", "1").ToStdString();
            auto lg = model->GetLayoutGroup();
            if (model != nullptr) {
                xlights->AddTraceMessage("GetXlightsModel converted model to MultiPoint");
                delete model;
            }
            model = xlights->AllModels.CreateDefaultModel("MultiPoint", startChannel);
            model->SetLayoutGroup(lg);
            model->Selected = true;
            return model;
        }
        else if (root->GetName() == "matrixmodel") {

            // grab the attributes I want to keep
            std::string startChannel = model->GetModelXml()->GetAttribute("StartChannel", "1").ToStdString();
            auto x = model->GetHcenterPos();
            auto y = model->GetVcenterPos();
            auto w = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetScaleX();
            auto h = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetScaleY();
            auto lg = model->GetLayoutGroup();

            // not a custom model so delete the default model that was created
            if (model != nullptr) {
                xlights->AddTraceMessage("GetXlightsModel converted model to Matrix");
                delete model;
            }
            model = xlights->AllModels.CreateDefaultModel("Matrix", startChannel);
            model->SetHcenterPos(x);
            model->SetVcenterPos(y);
            ((BoxedScreenLocation&)model->GetModelScreenLocation()).SetScale(w, h);
            model->SetLayoutGroup(lg);
            model->Selected = true;
            return model;
        }
        else if (root->GetName() == "archesmodel") {

            // grab the attributes I want to keep
            std::string startChannel = model->GetModelXml()->GetAttribute("StartChannel", "1").ToStdString();
            int l = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetLeft();
            int r = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetRight();
            int t = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetTop();
            int b = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetBottom();
            auto lg = model->GetLayoutGroup();

            // not a custom model so delete the default model that was created
            if (model != nullptr) {
                xlights->AddTraceMessage("GetXlightsModel converted model to Arches");
                delete model;
            }
            model = xlights->AllModels.CreateDefaultModel("Arches", startChannel);

            int h1 = 1;
            model->InitializeLocation(h1, l, b, modelPreview);
            ((ThreePointScreenLocation&)model->GetModelScreenLocation()).SetMWidth(std::abs(r - l));
            ((ThreePointScreenLocation&)model->GetModelScreenLocation()).SetRight(r);
            ((ThreePointScreenLocation&)model->GetModelScreenLocation()).SetLeft(l);
            ((ThreePointScreenLocation&)model->GetModelScreenLocation()).SetBottom(b);
            ((ThreePointScreenLocation&)model->GetModelScreenLocation()).SetHeight(2 * (float)std::abs(t - b) / (float)std::abs(r - l));
            model->SetLayoutGroup(lg);
            model->Selected = true;
            return model;
        }
        else if (root->GetName() == "starmodel") {

            // grab the attributes I want to keep
            std::string startChannel = model->GetModelXml()->GetAttribute("StartChannel", "1").ToStdString();
            auto x = model->GetHcenterPos();
            auto y = model->GetVcenterPos();
            auto w = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetScaleX();
            auto h = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetScaleY();
            auto lg = model->GetLayoutGroup();

            // not a custom model so delete the default model that was created
            if (model != nullptr) {
                xlights->AddTraceMessage("GetXlightsModel converted model to Star");
                delete model;
            }
            model = xlights->AllModels.CreateDefaultModel("Star", startChannel);
            model->SetHcenterPos(x);
            model->SetVcenterPos(y);
            ((BoxedScreenLocation&)model->GetModelScreenLocation()).SetScale(w, h);
            model->SetLayoutGroup(lg);
            model->Selected = true;
            return model;
        }
        else if (root->GetName() == "treemodel") {

            // grab the attributes I want to keep
            std::string startChannel = model->GetModelXml()->GetAttribute("StartChannel", "1").ToStdString();
            auto x = model->GetHcenterPos();
            auto y = model->GetVcenterPos();
            auto w = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetScaleX();
            auto h = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetScaleY();
            auto lg = model->GetLayoutGroup();

            // not a custom model so delete the default model that was created
            if (model != nullptr) {
                xlights->AddTraceMessage("GetXlightsModel converted model to Tree");
                delete model;
            }
            model = xlights->AllModels.CreateDefaultModel("Tree", startChannel);
            model->SetHcenterPos(x);
            model->SetVcenterPos(y);
            ((BoxedScreenLocation&)model->GetModelScreenLocation()).SetScale(w, h);
            model->SetLayoutGroup(lg);
            model->Selected = true;
            return model;
        }
        else if (root->GetName() == "dmxmodel") {

            // grab the attributes I want to keep
            std::string startChannel = model->GetModelXml()->GetAttribute("StartChannel", "1").ToStdString();
            auto x = model->GetHcenterPos();
            auto y = model->GetVcenterPos();
            auto w = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetScaleX();
            auto h = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetScaleY();
            auto lg = model->GetLayoutGroup();

            std::string dmx_type = root->GetAttribute("DisplayAs");
            // not a custom model so delete the default model that was created
            if (model != nullptr) {
                xlights->AddTraceMessage("GetXlightsModel converted model to DMX");
                delete model;
            }
            // Upgrade older DMX models
            if (dmx_type == "DMX") {
                std::string style = root->GetAttribute("DmxStyle", "DMX");
                if (style == "Moving Head Top" ||
                    style == "Moving Head Side" ||
                    style == "Moving Head Bars" ||
                    style == "Moving Head TopBars" ||
                    style == "Moving Head SideBars") {
                    dmx_type = "DmxMovingHead";
                }
                else if (style == "Moving Head 3D") {
                    dmx_type = "DmxMovingHead3D";
                }
                else if (style == "Flood Light") {
                    dmx_type = "DmxFloodlight";
                }
                else if (style == "Skulltronix Skull") {
                    dmx_type = "DmxSkulltronix";
                }
            }
            model = xlights->AllModels.CreateDefaultModel(dmx_type, startChannel);
            model->SetHcenterPos(x);
            model->SetVcenterPos(y);
            // Multiply by 5 because default custom model has parm1 and parm2 set to 5 and DMX model is 1 pixel
            ((BoxedScreenLocation&)model->GetModelScreenLocation()).SetScale(w * 5, h * 5);
            model->SetLayoutGroup(lg);
            model->Selected = true;
            return model;
        }
        else if (root->GetName() == "dmxgeneral") {

            // grab the attributes I want to keep
            std::string startChannel = model->GetModelXml()->GetAttribute("StartChannel", "1").ToStdString();
            auto x = model->GetHcenterPos();
            auto y = model->GetVcenterPos();
            auto w = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetScaleX();
            auto h = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetScaleY();
            auto lg = model->GetLayoutGroup();

            std::string dmx_type = root->GetAttribute("DisplayAs");
            // not a custom model so delete the default model that was created
            if (model != nullptr) {
                xlights->AddTraceMessage("GetXlightsModel converted model to DMX");
                delete model;
            }
            model = xlights->AllModels.CreateDefaultModel(dmx_type, startChannel);
            model->SetHcenterPos(x);
            model->SetVcenterPos(y);
            // Multiply by 5 because default custom model has parm1 and parm2 set to 5 and DMX model is 1 pixel
            ((BoxedScreenLocation&)model->GetModelScreenLocation()).SetScale(w * 5, h * 5);
            model->SetLayoutGroup(lg);
            model->Selected = true;
            return model;
        }
        else if (root->GetName() == "dmxservo") {

            // grab the attributes I want to keep
            std::string startChannel = model->GetModelXml()->GetAttribute("StartChannel", "1").ToStdString();
            auto w = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetScaleX();
            auto h = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetScaleY();
            auto x = model->GetHcenterPos();
            auto y = model->GetVcenterPos();
            auto lg = model->GetLayoutGroup();

            std::string dmx_type = root->GetAttribute("DisplayAs");
            // not a custom model so delete the default model that was created
            if (model != nullptr) {
                xlights->AddTraceMessage("GetXlightsModel converted model to DMX");
                delete model;
            }
            model = xlights->AllModels.CreateDefaultModel(dmx_type, startChannel);
            model->SetHcenterPos(x);
            model->SetVcenterPos(y);
            // Multiply by 5 because default custom model has parm1 and parm2 set to 5 and DMX model is 1 pixel
            ((BoxedScreenLocation&)model->GetModelScreenLocation()).SetScale(w * 5, h * 5);
            model->SetLayoutGroup(lg);
            model->Selected = true;
            return model;
        }
        else if (root->GetName() == "dmxservo3axis" ||
            root->GetName() == "dmxservo3d") {

            // grab the attributes I want to keep
            std::string startChannel = model->GetModelXml()->GetAttribute("StartChannel", "1").ToStdString();
            auto x = model->GetHcenterPos();
            auto y = model->GetVcenterPos();
            auto lg = model->GetLayoutGroup();

            std::string dmx_type = root->GetAttribute("DisplayAs");
            // not a custom model so delete the default model that was created
            if (model != nullptr) {
                xlights->AddTraceMessage("GetXlightsModel converted model to DMX");
                delete model;
            }
            model = xlights->AllModels.CreateDefaultModel(dmx_type, startChannel);
            model->SetHcenterPos(x);
            model->SetVcenterPos(y);
            model->GetModelScreenLocation().SetScaleMatrix(glm::vec3(1, 1, 1));
            model->SetLayoutGroup(lg);
            model->Selected = true;
            return model;
        }
        else if (root->GetName() == "circlemodel") {

            // grab the attributes I want to keep
            std::string startChannel = model->GetModelXml()->GetAttribute("StartChannel", "1").ToStdString();
            auto x = model->GetHcenterPos();
            auto y = model->GetVcenterPos();
            auto lg = model->GetLayoutGroup();

            // not a custom model so delete the default model that was created
            if (model != nullptr) {
                xlights->AddTraceMessage("GetXlightsModel converted model to Circle");
                delete model;
            }
            model = xlights->AllModels.CreateDefaultModel("Circle", startChannel);
            model->SetHcenterPos(x);
            model->SetVcenterPos(y);
            //((BoxedScreenLocation&)model->GetModelScreenLocation()).SetScale(w, h);
            model->SetLayoutGroup(lg);
            model->Selected = true;
            return model;
        }
        else if (root->GetName() == "spheremodel") {
            // grab the attributes I want to keep
            std::string startChannel = model->GetModelXml()->GetAttribute("StartChannel", "1").ToStdString();
            auto x = model->GetHcenterPos();
            auto y = model->GetVcenterPos();
            auto scale = ((BoxedScreenLocation&)model->GetModelScreenLocation()).GetScaleMatrix();
            auto lg = model->GetLayoutGroup();

            // not a custom model so delete the default model that was created
            if (model != nullptr) {
                xlights->AddTraceMessage("GetXlightsModel converted model to Sphere");
                delete model;
            }
            model = xlights->AllModels.CreateDefaultModel("Sphere", startChannel);
            model->SetHcenterPos(x);
            model->SetVcenterPos(y);
            ((BoxedScreenLocation&)model->GetModelScreenLocation()).SetScaleMatrix(scale);
            model->SetLayoutGroup(lg);
            model->Selected = true;
            return model;
        }
        else {
            logger_base.error("GetXlightsModel no code to convert to " + root->GetName());
            xlights->AddTraceMessage("GetXlightsModel no code to convert to " + root->GetName());
            cancelled = true;
        }
    }
    return model;
}

wxString Model::SerialiseSubmodel() const
{
    wxString res = "";

    wxXmlNode* root = GetModelXml();
    wxXmlNode* child = root->GetChildren();
    while (child != nullptr) {
        if (child->GetName() == "subModel") {
            wxXmlDocument new_doc;
            new_doc.SetRoot(new wxXmlNode(*child));
            wxStringOutputStream stream;
            new_doc.Save(stream);
            wxString s = stream.GetString();
            s = s.SubString(s.Find("\n") + 1, s.Length()); // skip over xml format header
            res += s;
        }
        child = child->GetNext();
    }

    return res;
}

wxString Model::CreateBufferAsSubmodel() const
{
    int buffW = GetDefaultBufferWi();
    int buffH = GetDefaultBufferHt();
    std::vector<std::vector<wxString>> nodearray(buffH, std::vector<wxString>(buffW, ""));
    uint32_t nodeCount = GetNodeCount();
    for (uint32_t i = 0; i < nodeCount; i++) {
        int bufx = Nodes[i]->Coords[0].bufX;
        int bufy = Nodes[i]->Coords[0].bufY;
        nodearray[bufy][bufx] = wxString::Format("%d", i + 1);
    }
    wxXmlNode* child = new wxXmlNode(wxXML_ELEMENT_NODE, "subModel");
    child->AddAttribute("name", "DefaultRenderBuffer");
    child->AddAttribute("layout", "horizontal");
    child->AddAttribute("type", "ranges");

    for (int x = 0; x < nodearray.size(); x++) {
        child->AddAttribute(wxString::Format("line%d", x), CompressNodes(wxJoin(nodearray[x], ',')));
    }

    wxXmlDocument new_doc;
    new_doc.SetRoot(new wxXmlNode(*child));
    wxStringOutputStream stream;
    new_doc.Save(stream);
    wxString s = stream.GetString();
    s = s.SubString(s.Find("\n") + 1, s.Length()); // skip over xml format header
    return s;
}

std::list<std::string> Model::CheckModelSettings()
{
    std::list<std::string> res;

    if (modelDimmingCurve != nullptr) {
        auto dimmingInfo = GetDimmingInfo();
        if (dimmingInfo.size() > 0) {
            float maxGamma = 0.0;
            int maxBrightness = -100;
            for (auto& it : dimmingInfo) {
                maxGamma = std::max(maxGamma, (float)wxAtof(it.second["gamma"]));
                maxBrightness = std::max(maxBrightness, wxAtoi(it.second["brightness"]));
            }
            if (maxGamma == 0.0) {
                res.push_back(wxString::Format("    ERR: Model %s has a dimming curve gamma of 0.0 ... this will essentially blank the model so no effects will ever show on it.", GetName()));
            }
            if (maxBrightness == -100) {
                res.push_back(wxString::Format("    ERR: Model %s has a dimming curve brightness of -100 ... this will essentially blank the model so no effects will ever show on it.", GetName()));
            }
        }
    }

    res.splice(res.end(), BaseObject::CheckModelSettings());
    return res;
}

bool Model::IsControllerConnectionValid() const
{
    return ((IsPixelProtocol() || IsSerialProtocol() || IsMatrixProtocol()) && GetControllerPort(1) > 0);
}

void Model::SetTagColour(wxColour colour)
{
    ModelXml->DeleteAttribute("TagColour");
    ModelXml->AddAttribute("TagColour", colour.GetAsString());
    IncrementChangeCount();
    AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::SetTagColour");
}
void Model::SetPixelSize(int size) {
    if (size != pixelSize) {
        pixelSize = size;
        ModelXml->DeleteAttribute("PixelSize");
        ModelXml->AddAttribute("PixelSize", wxString::Format(wxT("%i"), pixelSize));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::SetPixelSize");
    }
}

void Model::SetTransparency(int t) {
    if (t != transparency) {
        transparency = t;
        ModelXml->DeleteAttribute("Transparency");
        ModelXml->AddAttribute("Transparency", wxString::Format(wxT("%i"), transparency));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::SetTransparency");
    }
}

void Model::SetBlackTransparency(int t) {
    if (t != blackTransparency) {
        blackTransparency = t;
        ModelXml->DeleteAttribute("BlackTransparency");
        ModelXml->AddAttribute("BlackTransparency", wxString::Format(wxT("%i"), blackTransparency));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::SetBlackTransparency");
    }
}

void Model::SetPixelStyle(PIXEL_STYLE style) {
    if (_pixelStyle != style) {
        _pixelStyle = style;
        ModelXml->DeleteAttribute("Antialias");
        ModelXml->AddAttribute("Antialias", wxString::Format(wxT("%i"), (int)_pixelStyle));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::SetPixelStyle");
    }
}

int32_t Model::GetStringStartChan(int x) const
{
    int ts = GetSmartTs();
    if (ts <= 1) {
        if (x < stringStartChan.size()) {
            return stringStartChan[x];
        }
        return 1;
    }
    else {
        int str = x * ts;
        if (str < stringStartChan.size()) {
            return stringStartChan[str];
        }
        return 1;
    }
}

int Model::GetSmartRemote() const
{
    wxString s = GetControllerConnection()->GetAttribute("SmartRemote", "0");
    return wxAtoi(s);
}

bool Model::GetSRCascadeOnPort() const
{
    return GetControllerConnection()->GetAttribute("SRCascadeOnPort", "FALSE") == "TRUE";
}

int Model::GetSRMaxCascade() const
{
    wxString s = GetControllerConnection()->GetAttribute("SRMaxCascade", "1");
    return wxAtoi(s);
}

char Model::GetSmartRemoteLetter() const
{
    wxString s = GetControllerConnection()->GetAttribute("SmartRemote", "0");
    int l = wxAtoi(s);
    if (l == 0) return ' ';
    return char('A' + l - 1);
}

char Model::GetSmartRemoteLetterForString(int string) const
{
    auto sr = GetSmartRemoteForString(string);
    if (sr == 0) return ' ';
    return char('A' + sr - 1);
}

// This sorts the special A->B->C and B->C first to ensure that anything on a particular smart remote comes after things that span multiple ports
int Model::GetSortableSmartRemote() const
{
    int sr = GetSmartRemote();
    int max = GetSRMaxCascade();
    if (max == 1) return sr + 200;
    return sr;
}

int Model::GetSmartTs() const
{
    if (GetControllerConnection() == nullptr) return 0;

    return wxAtoi(GetControllerConnection()->GetAttribute("ts", "0"));
}

// string is one based
int Model::GetSmartRemoteForString(int string) const
{
    int port;
    int sr;
    GetPortSR(string, port, sr);
    return sr;
}

void Model::SetControllerDMXChannel(int ch)
{
    if (GetControllerDMXChannel() != ch)
    {
        GetControllerConnection()->DeleteAttribute("channel");
        if (ch > 0) {
            GetControllerConnection()->AddAttribute("channel", wxString::Format("%i", ch));
            clearUnusedProtocolProperties(GetControllerConnection());
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::SetControllerDMXChannel");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::OnPropertyGridChange::SetControllerDMXChannel");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "Model::SetControllerDMXChannel");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "Model::SetControllerDMXChannel");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::SetControllerDMXChannel");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Model::SetControllerDMXChannel");
        AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::SetControllerDMXChannel");
        IncrementChangeCount();
    }
}

void Model::SetSRCascadeOnPort(bool cascade)
{
    if (GetSRCascadeOnPort() != cascade) {
        GetControllerConnection()->DeleteAttribute("SRCascadeOnPort");
        GetControllerConnection()->AddAttribute("SRCascadeOnPort", cascade ? "TRUE": "FALSE");

        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::SetSRCascadeOnPort");
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::SetSRCascadeOnPort");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "Model::SetSRCascadeOnPort");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "Model::SetSRCascadeOnPort");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Model::SetSRCascadeOnPort");
        AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::SetSRCascadeOnPort");
        IncrementChangeCount();
    }
}

void Model::SetSRMaxCascade(int max)
{
    if (GetSRMaxCascade() != max) {

        GetControllerConnection()->DeleteAttribute("SRMaxCascade");
        GetControllerConnection()->AddAttribute("SRMaxCascade", wxString::Format("%d", max));

        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::SetSRMaxCascade");
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::SetSRMaxCascade");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "Model::SetSRMaxCascade");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "Model::SetSRMaxCascade");
        AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::SetSRMaxCascade");
        IncrementChangeCount();
    }
}

void Model::SetSmartRemote(int sr)
{
    if (GetSmartRemote() != sr)
    {
        // Find the last model on this smart remote
        if (!GetControllerName().empty()) {
            SetModelChain(modelManager.GetLastModelOnPort(GetControllerName(), GetControllerPort(), GetName(), GetControllerProtocol(), sr));
        }
        GetControllerConnection()->DeleteAttribute("SmartRemote");
        if (sr != 0) {
            GetControllerConnection()->AddAttribute("SmartRemote", wxString::Format("%d", sr));
        } else {
            SetSRMaxCascade(1);
            SetSRCascadeOnPort(false);
        }

        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::SetSmartRemote");
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::SetSmartRemote");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "Model::SetSmartRemote");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "Model::SetSmartRemote");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::SetSmartRemote");
        AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::SetSmartRemote");
        IncrementChangeCount();
    }
}

void Model::SetSmartRemoteType(const std::string& type) {
    if (GetSmartRemoteType() != type) {
        GetControllerConnection()->DeleteAttribute("SmartRemoteType");
        if (!type.empty()) {
            GetControllerConnection()->AddAttribute("SmartRemoteType", type);
        }
    }

    AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::SmartRemoteType");
    AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::SmartRemoteType");
    AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "Model::SmartRemoteType");
    AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "Model::SmartRemoteType");
    AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Model::SmartRemoteType");
    AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::SmartRemoteType");
    IncrementChangeCount();
}

void Model::SetModelChain(const std::string& modelChain)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string mc = modelChain;
    if (!mc.empty() && mc != "Beginning" && !StartsWith(mc, ">")) {
        mc = ">" + mc;
    }

    logger_base.debug("Model '%s' chained to '%s'.", (const char*)GetName().c_str(), (const char*)mc.c_str());
    ModelXml->DeleteAttribute("ModelChain");
    if (!mc.empty() && mc != "Beginning" && mc != ">") {
        ModelXml->AddAttribute("ModelChain", mc);
    }
    AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::SetModelChain");
    AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::SetModelChain");
    AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "Model::SetModelChain");
    AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "Model::SetModelChain");
    //AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Model::SetModelChain");
    AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::SetModelChain");
    IncrementChangeCount();
}

std::string Model::GetModelChain() const
{
    const std::string chain = ModelXml->GetAttribute("ModelChain", "").ToStdString();
    if (chain == "Beginning") {
        return "";
    }
    return chain;
}

void Model::SaveSuperStringColours()
{
    bool found = true;
    int index = 0;
    while (found)
    {
        auto an = wxString::Format("SuperStringColour%d", index);
        if (ModelXml->GetAttribute(an, "") == "")
        {
            found = false;
        }
        else
        {
            ModelXml->DeleteAttribute(an);
        }
        index++;
    }

    for (int i = 0; i < superStringColours.size(); i++)
    {
        auto an = wxString::Format("SuperStringColour%d", i);
        ModelXml->AddAttribute(an, superStringColours[i]);
    }
}

void Model::SetSuperStringColours(int count)
{
    while (superStringColours.size() < count)
    {
        bool r = false;
        bool g = false;
        bool b = false;
        bool w = false;
        bool y = false;

        for (const auto& it : superStringColours)
        {
            if (it == xlRED) r = true;
            if (it == xlGREEN) g = true;
            if (it == xlBLUE) b = true;
            if (it == xlWHITE) w = true;
            if (it == xlYELLOW) y = true;
        }

        if (!r) superStringColours.push_back(xlRED); else
            if (!g) superStringColours.push_back(xlGREEN); else
                if (!b) superStringColours.push_back(xlBLUE); else
                    if (!w) superStringColours.push_back(xlWHITE); else
                        if (!y) superStringColours.push_back(xlYELLOW); else
                            superStringColours.push_back(xlRED);
    }

    while (superStringColours.size() > count)
    {
        superStringColours.pop_back();
    }

    SaveSuperStringColours();

    IncrementChangeCount();
    AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Model::SetSuperStringColours");
    AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::SetSuperStringColours");
    AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Model::SetSuperStringColours");
    AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::SetSuperStringColours");
    AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "Model::SetSuperStringColours");
    AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "Model::SetSuperStringColours");
    AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::SetSuperStringColours");
}

void Model::SetSuperStringColour(int index, xlColor c)
{
    superStringColours[index] = c;
    SaveSuperStringColours();

    IncrementChangeCount();
    AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Model::SetSuperStringColour");
    AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::SetSuperStringColour");
    AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Model::SetSuperStringColour");
    AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::SetSuperStringColour");
    AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "Model::SetSuperStringColour");
    AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "Model::SetSuperStringColour");
    AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::SetSuperStringColour");
}

void Model::AddSuperStringColour(xlColor c, bool saveToXml)
{
    superStringColours.push_back(c);
    if (saveToXml) {
        SaveSuperStringColours();
    }
}

void Model::SetShadowModelFor(const std::string& shadowModelFor)
{
    ModelXml->DeleteAttribute("ShadowModelFor");
    ModelXml->AddAttribute("ShadowModelFor", shadowModelFor);
    //AddASAPWork(OutputModelManager::WORK_UPDATE_PROPERTYGRID, "Model::SetControllerName");
    IncrementChangeCount();
}

void Model::SetControllerName(const std::string& controller) {
    auto n = Trim(controller);

    if (n == ModelXml->GetAttribute("Controller", "xyzzy_kw").Trim(true).Trim(false))
        return;

    ModelXml->DeleteAttribute("Controller");
    if (!n.empty() && n != USE_START_CHANNEL) {
        ModelXml->AddAttribute("Controller", n);
    }

    // if we are moving the model to no contoller then clear the start channel and model chain
    if (n == NO_CONTROLLER) {
        SetStartChannel("");
        SetModelChain("");
        SetControllerPort(0);
    }

    AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "Model::SetControllerName");
    AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "Model::SetControllerName");
    AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::SetControllerName");
    AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::SetControllerName");
    AddASAPWork(OutputModelManager::WORK_UPDATE_PROPERTYGRID, "Model::SetControllerName");
    //AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Model::SetControllerName");
    AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::SetControllerName");
    IncrementChangeCount();
}

void Model::SetControllerProtocol(const std::string& protocol)
{
    if (protocol == ModelXml->GetAttribute("Protocol", "xyzzy_kw")) return;

    GetControllerConnection()->DeleteAttribute("Protocol");
    if (protocol != "") {
        GetControllerConnection()->AddAttribute("Protocol", protocol);
    }

    AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::SetControllerProtocol");
    AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::SetControllerProtocol");
    AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "Model::SetControllerProtocol");
    AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "Model::SetControllerProtocol");
    //AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Model::SetControllerProtocol");
    AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::SetControllerProtocol");
    IncrementChangeCount();
}

void Model::SetControllerPort(int port)
{
    if (port == wxAtoi(ModelXml->GetAttribute("Port", "-999"))) return;

    GetControllerConnection()->DeleteAttribute("Port");
    if (port > 0) {
        GetControllerConnection()->AddAttribute("Port", wxString::Format("%d", port));
    }

    AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::SetControllerPort");
    AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::SetControllerPort");
    AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "Model::SetControllerPort");
    AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "Model::SetControllerPort");
    //AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Model::SetControllerPort");
    AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::SetControllerPort");
    IncrementChangeCount();
}

void Model::SetControllerBrightness(int brightness)
{
    if (brightness == wxAtoi(ModelXml->GetAttribute("brightness", "-1"))) return;

    GetControllerConnection()->DeleteAttribute("brightness");
    GetControllerConnection()->AddAttribute("brightness", wxString::Format("%d", brightness));

    AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::SetConnectionPixelBrightness");
    AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::SetConnectionPixelBrightness");
    AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::SetConnectionBrightness");
    AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::SetConnectionBrightness");
    IncrementChangeCount();
}

void Model::ClearControllerBrightness()
{
    if (GetControllerConnection()->HasAttribute("brightness")) {
        GetControllerConnection()->DeleteAttribute("brightness");
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::ClearControllerBrightness");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::ClearControllerBrightness");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::ClearControllerBrightness");
        AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::ClearControllerBrightness");
        IncrementChangeCount();
    }
}

bool Model::IsControllerBrightnessSet() const
{
    return GetControllerConnection()->HasAttribute("brightness");
}

int Model::GetControllerBrightness() const
{
    return wxAtoi(GetControllerConnection()->GetAttribute("brightness", "100"));
}

bool Model::IsShadowModel() const
{
    return ModelXml->GetAttribute("ShadowModelFor", "").size() > 0;
}

std::string Model::GetShadowModelFor() const
{
    return ModelXml->GetAttribute("ShadowModelFor", "").ToStdString();
}

std::string Model::GetControllerName() const
{
    return ModelXml->GetAttribute("Controller", "").Trim(true).Trim(false).ToStdString();
}

//std::list<std::string> Model::GetProtocols()
//{
//    std::list<std::string> res;
//    for (auto a : GetAllPixelTypes()) {
//        if (a != "") {
//            res.push_back(a);
//        }
//    }
//    for (auto a : SERIAL_PROTOCOLS) {
//        if (a != "") {
//            res.push_back(a.ToStdString());
//        }
//    }
//    return res;
//}

//std::list<std::string> Model::GetLCProtocols()
//{
//    auto protocols = Model::GetProtocols();

//    for (auto p = protocols.begin(); p != protocols.end(); ++p)
//    {
//        *p = wxString(*p).Lower().ToStdString();
//    }

//    return protocols;
//}

//bool Model::IsProtocolValid(std::string protocol)
//{
//    wxString p(protocol);
//    std::string prot = p.Lower().ToStdString();
//    auto protocols = Model::GetLCProtocols();
//    return (std::find(protocols.begin(), protocols.end(), prot) != protocols.end());
//}

bool Model::CleanupFileLocations(xLightsFrame* frame)
{
    bool rc = false;
    for (auto& it : faceInfo)
    {
        if (it.second.find("Type") != it.second.end() && it.second.at("Type") == "Matrix")
        {
            for (auto& it2 : it.second)
            {
                if (it2.first != "CustomColors" && it2.first != "ImagePlacement" && it2.first != "Type" && it2.second != "")
                {
                    if (FileExists(it2.second))
                    {
                        if (!frame->IsInShowFolder(it2.second))
                        {
                            it2.second = frame->MoveToShowFolder(it2.second, wxString(wxFileName::GetPathSeparator()) + "Faces");
                            rc = true;
                        }
                    }
                }
            }
        }
    }

    if (rc)
    {
        wxXmlNode *f = ModelXml->GetChildren();
        while (f != nullptr) {
            if ("faceInfo" == f->GetName()) {
                ModelXml->RemoveChild(f);
                delete f;
                f = ModelXml->GetChildren();
            }
            else {
                f = f->GetNext();
            }
        }
        Model::WriteFaceInfo(ModelXml, faceInfo);
    }

    return BaseObject::CleanupFileLocations(frame) || rc;
}

// all when true includes all image files ... even if they dont really exist
std::list<std::string> Model::GetFaceFiles(const std::list<std::string>& facesUsed, bool all, bool includeFaceName) const
{
    std::list<std::string> res;

    for (const auto& it : faceInfo)
    {
        if (all || std::find(begin(facesUsed), end(facesUsed), it.first) != facesUsed.end())
        {
            if (it.second.find("Type") != it.second.end() && it.second.at("Type") == "Matrix")
            {
                for (const auto& it2 : it.second)
                {
                    if (it2.first != "CustomColors" && it2.first != "ImagePlacement" && it2.first != "Type" && it2.second != "")
                    {
                        if (all || FileExists(it2.second))
                        {
                            std::string n = it2.second;
                            if (includeFaceName) n = it.first + "|" + n;
                            res.push_back(n);
                        }
                    }
                }
            }
        }
    }

    return res;
}

bool Model::HasState(std::string const& state) const {
    auto s = Lower(state);
    for (const auto& it : stateInfo) {
        if (it.first == s) {
            return true;
        }
    }
    return false;
}

std::string Model::GetControllerProtocol() const
{
    wxString s = GetControllerConnection()->GetAttribute("Protocol");
    return s.ToStdString();
}

int Model::GetControllerDMXChannel() const
{
    return wxAtoi(GetControllerConnection()->GetAttribute("channel", "1"));
}

void Model::GetPortSR(int string, int& outport, int& outsr) const
{
    // we need to work with 0 based strings
    string = string - 1;

    wxString s = GetControllerConnection()->GetAttribute("Port", "0");
    int port = wxAtoi(s);
    int sr = GetSmartRemote();

    if (port == 0 || string == 0) {
        outport = port;
        outsr = sr;
    }
    else if (sr == 0) {
        outport = port + string;
        outsr = 0;
    }
    else {
        bool cascadeOnPort = GetSRCascadeOnPort();
        int max = GetSRMaxCascade();

        if (cascadeOnPort) {
            outport = port + string / max;
            outsr = sr + (string % max);
        }
        else {
            int currp = port;
            int currsr = sr;

            for (int p = 0; p < string; p++) {
                int newp = currp + 1;
                if ((newp - 1) / PORTS_PER_SMARTREMOTE != (currp - 1) / PORTS_PER_SMARTREMOTE) {
                    int newsr = currsr + 1;
                    if (newsr - sr >= max) {
                        currsr = sr;
                        currp = newp;
                    }
                    else {
                        currsr = newsr;
                        currp = ((currp - 1) / PORTS_PER_SMARTREMOTE) * PORTS_PER_SMARTREMOTE + 1;
                    }
                }
                else {
                    currp = newp;
                }
            }

            outport = currp;
            outsr = currsr;
        }
    }
}

int Model::GetControllerPort(int string) const
{
    wxString p = wxString::Format("%d", string);
    if (GetControllerConnection()->HasAttribute(p)) {
        wxString s = GetControllerConnection()->GetAttribute(p);
        return wxAtoi(s);
    }

    int port;
    int sr;
    GetPortSR(string, port, sr);
    return port;
}

void Model::GetMinScreenXY(float& minx, float& miny) const
{
    if (Nodes.size() == 0)
    {
        minx = 0;
        miny = 0;
        return;
    }

    minx = 99999999.0f;
    miny = 99999999.0f;
    for (const auto& it : Nodes)
    {
        for (const auto& it2 : it->Coords)
        {
            minx = std::min(minx, it2.screenX);
            miny = std::min(miny, it2.screenY);
        }
    }
}

void Model::ApplyDimensions(const std::string& units, float width, float height, float depth, float& min_x, float& max_x, float& min_y, float& max_y)
{
    auto ruler = RulerObject::GetRuler();

    if (ruler != nullptr && width != 0 && height != 0) {
        float w = ruler->ConvertDimension(units, width);
        float h = ruler->ConvertDimension(units, height);
        float d = ruler->ConvertDimension(units, depth);

        GetModelScreenLocation().SetMWidth(ruler->UnMeasure(w));
        GetModelScreenLocation().SetMHeight(ruler->UnMeasure(h));
        if (depth != 0) {
            GetModelScreenLocation().SetMDepth(ruler->UnMeasure(d));
        }
    }
}

void Model::ExportDimensions(wxFile& f) const
{
    auto ruler = RulerObject::GetRuler();

    if (ruler != nullptr) {
        std::string u = "mm";
        switch (ruler->GetUnits()) {
        case RULER_UNITS_INCHES:
            u = "i";
            break;
        case RULER_UNITS_FEET:
            u = "f";
            break;
        case RULER_UNITS_YARDS:
            u = "y";
            break;
        case RULER_UNITS_MM:
            u = "mm";
            break;
        case RULER_UNITS_CM:
            u = "cm";
            break;
        case RULER_UNITS_M:
            u = "m";
            break;
        }
        f.Write(wxString::Format("<dimensions units=\"%s\" width=\"%f\" height=\"%f\" depth=\"%f\"/>", u, GetModelScreenLocation().GetRealWidth(), GetModelScreenLocation().GetRealHeight(), GetModelScreenLocation().GetRealDepth()));
    }
}

void Model::SaveDisplayDimensions()
{
    _savedWidth = GetModelScreenLocation().GetRestorableMWidth();
    if (_savedWidth == 0) _savedWidth = 1;
    _savedHeight = GetModelScreenLocation().GetRestorableMHeight();
    if (_savedHeight == 0) _savedHeight = 1;
    _savedDepth = GetModelScreenLocation().GetRestorableMDepth();
    if (_savedDepth == 0) _savedDepth = 1;
}

void Model::RestoreDisplayDimensions()
{
    if ((DisplayAs.rfind("Dmx", 0) != 0) && DisplayAs != "Image")
    {
        SetWidth(_savedWidth, true);
        // We dont want to set the height of three point models
        if (dynamic_cast<const ThreePointScreenLocation*>(&(GetModelScreenLocation())) == nullptr)
        {
            SetHeight(_savedHeight, true);
        }
        SetDepth(_savedDepth, true);
    }
}

// This is deliberately ! serial so that it defaults to thinking it is pixel
bool Model::IsPixelProtocol() const {
    return GetControllerPort(1) != 0 && !::IsSerialProtocol(GetControllerProtocol()) && !::IsMatrixProtocol(GetControllerProtocol());
}
bool Model::IsSerialProtocol() const {
    return GetControllerPort(1) != 0 && ::IsSerialProtocol(GetControllerProtocol());
}
bool Model::IsMatrixProtocol() const {
    return GetControllerPort(1) != 0 && ::IsMatrixProtocol(GetControllerProtocol());
}
bool Model::IsLEDPanelMatrixProtocol() const {
    return GetControllerPort(1) != 0 && ::IsLEDPanelMatrixProtocol(GetControllerProtocol());
}
bool Model::IsVirtualMatrixProtocol() const {
    return GetControllerPort(1) != 0 && ::IsVirtualMatrixProtocol(GetControllerProtocol());
}

std::vector<std::string> Model::GetSmartRemoteTypes() const {
    auto caps = GetControllerCaps();
    if (caps == nullptr) {
        return { "" };
    }
    return caps->GetSmartRemoteTypes();
}

std::string Model::GetSmartRemoteType() const {
    auto types = GetSmartRemoteTypes();
    if (types.empty()) {
        return "";
    }
    std::string t = GetSmartRemoteTypes().front();
    wxString s = GetControllerConnection()->GetAttribute("SmartRemoteType", t);

    if (std::find(types.begin(), types.end(), s) == types.end()) {
        return t;
    }
    return s;
}

int Model::GetSmartRemoteTypeIndex(const std::string& srType) const {
    auto caps = GetControllerCaps();
    int i = 0;
    if (caps != nullptr) {
        for (const auto& it : caps->GetSmartRemoteTypes()) {
            if (srType == Lower(it)) {
                return i;
            }
            i++;
        }
    }

    return 0;
}

std::string Model::GetSmartRemoteTypeName(int idx) const {
    auto caps = GetControllerCaps();
    if (caps != nullptr) {
        const auto srList = caps->GetSmartRemoteTypes();
        if (idx < srList.size() && idx >= 0) {
            auto it = srList.begin();
            std::advance(it, idx);
            return *it;
        }
    }

    return std::string();
}

int Model::GetSmartRemoteCount() const {
    auto caps = GetControllerCaps();
    if (caps != nullptr) {
        return caps->GetSmartRemoteCount();
    }
    return 3;
}


Model::PreviewGraphicsCacheInfo::~PreviewGraphicsCacheInfo() {
    if (vica) delete vica;
    if (program) delete program;
    if (vca) delete vca;
    if (va) delete va;
};
void Model::deleteUIObjects() {
    for (auto &a : uiCaches) {
        delete a.second;
    }
    uiCaches.clear();
    uiObjectsInvalid = false;
}

bool wxDropPatternProperty::ValidateValue(wxVariant& value, wxPGValidationInfo& validationInfo) const
{
    for (auto c : value.GetString()) {
        if ((c < '0' || c > '9') && c != ',' && c != '-') return false;
    }
    return true;
}

void Model::ImportXlightsModel(std::string const& filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
    // these have already been dealt with
    if (EndsWith(filename, "gdtf"))
        return;

    std::string lower = Lower(filename);
    if (!EndsWith(lower, "xmodel")) {
        CustomModel* cm = dynamic_cast<CustomModel*>(this);
        if (cm != nullptr) {
            return cm->ImportLORModel(filename, xlights, min_x, max_x, min_y, max_y);
        }
        DisplayError("Attempt to import non-xmodel onto a non custom model.");
        return;
    }

    wxXmlDocument doc(filename);
    if (doc.IsOk()) {
        wxXmlNode* root = doc.GetRoot();
        ImportXlightsModel(root, xlights, min_x, max_x, min_y, max_y);
    } else {
        DisplayError("Failure loading model file: " + filename);
    }
}

std::string Model::GetAttributesAsJSON() const
{
    std::string json = "{";
    bool first{true};
    for (wxXmlAttribute* attrp = ModelXml->GetAttributes(); attrp; attrp = attrp->GetNext())
    {
        wxString value = attrp->GetValue();
        if (!value.empty())
        {
            if(!first)
            {
                json += ",";
            }
            json += "\"" + attrp->GetName().ToStdString() + "\":\"" + JSONSafe(value.ToStdString()) + "\"";
            first = false;
        }
    }
    json += ",\"ControllerConnection\":{";
    wxXmlNode* cc = GetControllerConnection();
    bool first2{true};
    for (wxXmlAttribute* attrp = cc->GetAttributes(); attrp; attrp = attrp->GetNext())
    {
        wxString value = attrp->GetValue();
        if (!value.empty())
        {
            if(!first2)
            {
                json += ",";
            }
            json += "\"" + attrp->GetName().ToStdString() + "\":\"" + JSONSafe(value.ToStdString()) + "\"";
            first2 = false;
        }
    }
    json += "}}";
    return json;
}

