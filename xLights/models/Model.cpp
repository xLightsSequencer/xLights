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

#include "Model.h"
#include "ModelManager.h"
#include "ModelGroup.h"
#include "../xLightsApp.h"
#include "../xLightsMain.h" //for Preview and Other model collections
#include "../xLightsXmlFile.h"
#include "../Color.h"
#include "../DrawGLUtils.h"
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
#include "../VendorModelDialog.h"
#include "../ModelPreview.h"
#include "ModelScreenLocation.h"
#include "SubModel.h"
#include "../UtilFunctions.h"
#include "xLightsVersion.h"
#include "../controllers/ControllerCaps.h"

#include <log4cpp/Category.hh>

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

static const char *RGBW_HANDLING_VALUES[] = {"R=G=B -> W", "RGB Only", "White Only"};
static wxArrayString RGBW_HANDLING(3, RGBW_HANDLING_VALUES);

static const char *PIXEL_STYLES_VALUES[] = {"Square", "Smooth", "Solid Circle", "Blended Circle"};
static wxArrayString PIXEL_STYLES(4, PIXEL_STYLES_VALUES);


static const char *CONTROLLER_PROTOCOLS_VALUES[] = {
    "", "ws2811", "gece", "tm18xx", "lx1203",
    "ws2801", "tls3001", "lpd6803", "apa102", "dmx",
    "open dmx", "pixelnet", "renard", "lor", "open pixelnet"};
static wxArrayString CONTROLLER_PROTOCOLS(15, CONTROLLER_PROTOCOLS_VALUES);

static const char *SMART_REMOTES_VALUES[] = {"N/A", "*A*->b->c", "a->*B*->c", "a->b->*C*", "*A*->*B*->*C*", "a->*B*->*C*"};
static wxArrayString SMART_REMOTES(6, SMART_REMOTES_VALUES);

static const char *CONTROLLER_DIRECTION_VALUES[] = {"Forward", "Reverse"};
static wxArrayString CONTROLLER_DIRECTION(2, CONTROLLER_DIRECTION_VALUES);

static const char *CONTROLLER_COLORORDER_VALUES[] = {
    "RGB", "RBG", "GBR", "GRB", "BRG", "BGR",
    "RGBW", "RBGW", "GBRW", "GRBW", "BRGW", "BGRW",
    "WRGB", "WRBG", "WGBR", "WGRB", "WBRG", "WBGR"};
static wxArrayString CONTROLLER_COLORORDER(18, CONTROLLER_COLORORDER_VALUES);


static wxArrayString LAYOUT_GROUPS;
static wxArrayString CONTROLLERS;


static const std::string DEFAULT("Default");
static const std::string PER_PREVIEW("Per Preview");
static const std::string SINGLE_LINE("Single Line");
static const std::string AS_PIXEL("As Pixel");
static const std::string VERT_PER_STRAND("Vertical Per Strand");
static const std::string HORIZ_PER_STRAND("Horizontal Per Strand");

static const std::string PER_PREVIEW_NO_OFFSET("Per Preview No Offset");

const std::vector<std::string> Model::DEFAULT_BUFFER_STYLES {DEFAULT, PER_PREVIEW, SINGLE_LINE, AS_PIXEL};

Model::Model(const ModelManager &manager) : modelDimmingCurve(nullptr),
    parm1(0), parm2(0), parm3(0), pixelStyle(1), pixelSize(2), transparency(0), blackTransparency(0),
    StrobeRate(0), modelManager(manager), CouldComputeStartChannel(false), maxVertexCount(0),
    rgbwHandlingType(0), BufferDp(0), _controller(0), modelTagColour(*wxBLACK)
{
    // These member vars were not initialised so give them some defaults.
    BufferHt = 0;
    BufferWi = 0;
    IsLtoR = true;
    isBotToTop = true;
    SingleNode = false;
    zeroBased = false;
    SingleChannel = false;
}

Model::~Model() {
    if (modelDimmingCurve != nullptr) {
        delete modelDimmingCurve;
    }
    for (const auto& it : subModels) {
        Model *m = it;
        delete m;
    }
}

static const std::string CLICK_TO_EDIT("--Click To Edit--");
class StrandNodeNamesDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    StrandNodeNamesDialogAdapter(const Model *model)
    : wxPGEditorDialogAdapter(), m_model(model) {
    }
    virtual bool DoShowDialog(wxPropertyGrid* propGrid,
                              wxPGProperty* WXUNUSED(property) ) override {
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
    const Model *m_model;
};

class FacesDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    FacesDialogAdapter(Model *model)
    : wxPGEditorDialogAdapter(), m_model(model) {
    }
    virtual bool DoShowDialog(wxPropertyGrid* propGrid,
                              wxPGProperty* WXUNUSED(property) ) override {
        ModelFaceDialog dlg(propGrid);
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
    Model *m_model;
};
class StatesDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    StatesDialogAdapter(Model *model)
        : wxPGEditorDialogAdapter(), m_model(model) {
    }
    virtual bool DoShowDialog(wxPropertyGrid* propGrid,
        wxPGProperty* WXUNUSED(property)) override {
        ModelStateDialog dlg(propGrid);
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
    Model *m_model;
};
class DimmingCurveDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    DimmingCurveDialogAdapter(const Model *model)
    : wxPGEditorDialogAdapter(), m_model(model) {
    }
    virtual bool DoShowDialog(wxPropertyGrid* propGrid,
                              wxPGProperty* WXUNUSED(property) ) override {
        ModelDimmingCurveDialog dlg(propGrid);
        std::map<std::string, std::map<std::string,std::string> > dimmingInfo;
        wxXmlNode *f = m_model->GetModelXml()->GetChildren();
        while (f != nullptr) {
            if ("dimmingCurve" == f->GetName()) {
                wxXmlNode *dc = f->GetChildren();
                while (dc != nullptr) {
                    std::string name = dc->GetName().ToStdString();
                    wxXmlAttribute *att = dc->GetAttributes();
                    while (att != nullptr) {
                        dimmingInfo[name][att->GetName().ToStdString()] = att->GetValue();
                        att = att->GetNext();
                    }
                    dc = dc->GetNext();
                }
            }
            f = f->GetNext();
        }
        if(dimmingInfo.empty()) {
            wxString b = m_model->GetModelXml()->GetAttribute("ModelBrightness","0");
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
            for (auto it = dimmingInfo.begin(); it != dimmingInfo.end(); ++it) {
                wxXmlNode *dc = new wxXmlNode(wxXML_ELEMENT_NODE , it->first);
                f1->AddChild(dc);
                for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
                    dc->AddAttribute(it2->first, it2->second);
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
    SubModelsDialogAdapter(Model *model)
    : wxPGEditorDialogAdapter(), m_model(model) {
    }
    virtual bool DoShowDialog(wxPropertyGrid* propGrid,
                              wxPGProperty* WXUNUSED(property) ) override {
        SubModelsDialog dlg(propGrid);
        dlg.Setup(m_model);
        if (dlg.ShowModal() == wxID_OK) {
            dlg.Save();
            wxVariant v(CLICK_TO_EDIT);
            SetValue(v);
            return true;
        }
        return false;
    }
protected:
    Model *m_model;
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
                        const wxString& label,
                        const wxString& name,
                        const wxString& value,
                        int type)
    : wxStringProperty(label, name, value), m_model(m), m_tp(type) {
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
            return new FacesDialogAdapter(m_model);
        case 3:
            return new DimmingCurveDialogAdapter(m_model);
        case 4:
            return new StatesDialogAdapter(m_model);
        case 5:
            return new SubModelsDialogAdapter(m_model);
        default:
            break;
        }
        return nullptr;
    }
protected:
    Model *m_model = nullptr;
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
    StartChannelProperty(Model *m,
                         int strand,
                        const wxString& label,
                        const wxString& name,
                        const wxString& value,
                        std::string preview)
        : wxStringProperty(label, name, value), m_model(m), m_strand(strand), _preview(preview) {
    }
    // Set editor to have button
    virtual const wxPGEditor* DoGetEditorClass() const override {
        return wxPGEditor_TextCtrlAndButton;
    }
    // Set what happens on button click
    virtual wxPGEditorDialogAdapter* GetEditorDialog() const override {
        return new StartChannelDialogAdapter(m_model, _preview);
    }
protected:
    Model *m_model;
    std::string _preview;
    int m_strand;
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
    Model *m_model;
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

void Model::Rename(std::string newName)
{
    name = Trim(newName);
    ModelXml->DeleteAttribute("name");
    ModelXml->AddAttribute("name", name);
}

void Model::SetStartChannel(std::string startChannel)
{
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

void Model::SetProperty(wxString property, wxString value, bool apply)
{
    wxString val = ModelXml->GetAttribute(property);
    if (val != "")
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
        SetFromXml(ModelXml);
    }
}

void Model::UpdateProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    
    UpdateTypeProperties(grid);

    if (grid->GetPropertyByName("Controller") != nullptr) {
        grid->GetPropertyByName("Controller")->Enable(outputManager->GetAutoLayoutControllerNames().size() > 0);
    }

    if (HasOneString(DisplayAs) && grid->GetPropertyByName("ModelStartChannel") != nullptr) {
        grid->GetPropertyByName("ModelStartChannel")->Enable(GetControllerName() == "" && _controller == 0);
    }
    else {
        if (grid->GetPropertyByName("ModelIndividualStartChannels") != nullptr) {
            grid->GetPropertyByName("ModelIndividualStartChannels")->Enable(parm1 > 1 && GetControllerName() == "" && _controller == 0);
        }
        if (grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel") != nullptr) {
            grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel")->Enable(GetControllerName() == "" && _controller == 0);
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

std::string Model::GetPixelStyleDescription(int pixelStyle)
{
    if (pixelStyle < PIXEL_STYLES.size())
    {
        return PIXEL_STYLES[pixelStyle];
    }
    return "";
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
    if (controller == "")
    {
        if (StartsWith(ModelStartChannel, "!") && Contains(ModelStartChannel, ":"))
        {
            controller = Trim(BeforeFirst(AfterFirst(ModelStartChannel, '!'), ':'));
        }
    }
    if (controller == "") return nullptr;

    return modelManager.GetXLightsFrame()->GetOutputManager()->GetController(controller);
}

void Model::AddProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    LAYOUT_GROUPS = Model::GetLayoutGroups(modelManager);

    wxPGProperty* sp;

    wxPGProperty* p = grid->Append(new wxPropertyCategory(DisplayAs, "ModelType"));

    AddTypeProperties(grid);

    _controller = 0;
    CONTROLLERS.clear();
    CONTROLLERS.Add("Use Start Channel");

    for (const auto& it : outputManager->GetAutoLayoutControllerNames())
    {
        if (GetControllerName() == it)
        {
            _controller = CONTROLLERS.size();
        }
        CONTROLLERS.Add(it);
    }

    if (CONTROLLERS.size() > 1)
    {
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
        p->Enable(parm1 > 1 && GetControllerName() == "" && _controller == 0);
        sp = grid->AppendIn(p, new StartChannelProperty(this, 0, "Start Channel", "ModelStartChannel", ModelXml->GetAttribute("StartChannel", "1"), modelManager.GetXLightsFrame()->GetSelectedLayoutPanelPreview()));
        sp->Enable(GetControllerName() == "" || _controller == 0);
        if (hasIndiv) {
            int c = Model::HasOneString(DisplayAs) ? 1 : parm1;
            for (int x = 0; x < c; x++) {
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
        else
        {
            // remove per strand start channels if individual isnt selected
            for (int x = 0; x < 100; x++) {
                wxString nm = StartChanAttrName(x);
                ModelXml->DeleteAttribute(nm);
            }
        }
    }

    if (CONTROLLERS.size() > 1 && GetControllerName() != "" && GetControllerProtocol() != "" && GetControllerPort() != 0 && _controller != 0)
    {
        p = grid->Append(new ModelChainProperty(this, "Model Chain", "ModelChain", GetModelChain() == "" ? _("Beginning").ToStdString() : GetModelChain()));
        p->Enable(GetControllerName() != "" && GetControllerProtocol() != "" && GetControllerPort() != 0 && _controller != 0);
    }

    int layout_group_number = 0;
    for (int grp = 0; grp < LAYOUT_GROUPS.Count(); grp++)
    {
        if (LAYOUT_GROUPS[grp] == layout_group)
        {
            layout_group_number = grp;
            break;
        }
    }

    grid->Append(new wxStringProperty("Description", "Description", description));
    grid->Append(new wxEnumProperty("Preview", "ModelLayoutGroup", LAYOUT_GROUPS, wxArrayInt(), layout_group_number));

    p = grid->Append(new PopupDialogProperty(this, "Strand/Node Names", "ModelStrandNodeNames", CLICK_TO_EDIT, 1));
    grid->LimitPropertyEditing(p);
    p = grid->Append(new PopupDialogProperty(this, "Faces", "ModelFaces", CLICK_TO_EDIT, 2));
    grid->LimitPropertyEditing(p);
    p = grid->Append(new PopupDialogProperty(this, "Dimming Curves", "ModelDimmingCurves", CLICK_TO_EDIT, 3));
    grid->LimitPropertyEditing(p);
    p = grid->Append(new PopupDialogProperty(this, "States", "ModelStates", CLICK_TO_EDIT, 4));
    grid->LimitPropertyEditing(p);
    p = grid->Append(new PopupDialogProperty(this, "Sub-Models", "SubModels", CLICK_TO_EDIT, 5));
    grid->LimitPropertyEditing(p);

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
    } else if (NODE_TYPES[i] == "Superstring") {
        if (superStringColours.size() == 0)
        {
            superStringColours.push_back(xlRED);
            SaveSuperStringColours();
        }
        sp = grid->AppendIn(p, new wxIntProperty("Colours", "SuperStringColours", superStringColours.size()));
        sp->SetAttribute("Min", 1);
        sp->SetAttribute("Max", 32);
        sp->SetEditor("SpinCtrl");
        for (int i = 0; i < superStringColours.size(); i++)
        {
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

    grid->AppendIn(p, new wxEnumProperty("Pixel Style", "ModelPixelStyle", PIXEL_STYLES, wxArrayInt(), pixelStyle));
    sp = grid->AppendIn(p, new wxUIntProperty("Transparency", "ModelPixelTransparency", transparency));
    sp->SetAttribute("Min", 0);
    sp->SetAttribute("Max", 100);
    sp->SetEditor("SpinCtrl");
    sp = grid->AppendIn(p, new wxUIntProperty("Black Transparency", "ModelPixelBlackTransparency", blackTransparency));
    sp->SetAttribute("Min", 0);
    sp->SetAttribute("Max", 100);
    sp->SetEditor("SpinCtrl");
    sp = grid->AppendIn(p, new wxColourProperty("Tag Color", "ModelTagColour", modelTagColour));
    UpdateControllerProperties(grid);
    DisableUnusedProperties(grid);
}

void Model::GetControllerProtocols(wxArrayString& cp, int& idx) {

    auto caps = GetControllerCaps();
    wxString protocol = GetControllerProtocol();
    protocol.LowerCase();

    int i = 0;
    if (caps == nullptr) {
        for (const auto& it : CONTROLLER_PROTOCOLS) {
            cp.push_back(it);
            if (protocol == it.Lower()) {
                idx = i;
            }
            i++;
        }
    }
    else {
        for (const auto& it : caps->GetAllProtocols())
        {
            cp.push_back(it);
            if (protocol == Lower(it)) {
                idx = i;
            }
            i++;
        }
    }
}

void Model::AddControllerProperties(wxPropertyGridInterface *grid) {

    auto caps = GetControllerCaps();

    wxString protocol = GetControllerProtocol();
    protocol.LowerCase();

    wxPGProperty *p = grid->Append(new wxPropertyCategory("Controller Connection", "ModelControllerConnectionProperties"));

    wxPGProperty *sp = grid->AppendIn(p, new wxUIntProperty("Port", "ModelControllerConnectionPort", GetControllerPort(1)));
    sp->SetAttribute("Min", 0);
    if (caps == nullptr || protocol == "") {
        sp->SetAttribute("Max", 48);
    }
    else
    {
        if (IsPixelProtocol(GetControllerProtocol())) {
            sp->SetAttribute("Max", caps->GetMaxPixelPort());
        }
        else {
            sp->SetAttribute("Max", caps->GetMaxSerialPort());
        }
    }
    sp->SetEditor("SpinCtrl");

    wxArrayString cp;
    int idx = -1;
    GetControllerProtocols(cp, idx);

    if (caps == nullptr || caps->SupportsSmartRemotes()) {
        if (Model::IsPixelProtocol(protocol)) {
            sp = grid->AppendIn(p, new wxEnumProperty("Smart Remote", "SmartRemote", SMART_REMOTES, wxArrayInt(), GetSmartRemote()));
        }
    }

    sp = grid->AppendIn(p, new wxEnumProperty("Protocol", "ModelControllerConnectionProtocol", cp, wxArrayInt(), idx));

    wxXmlNode *node = GetControllerConnection();
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
    } else if (IsPixelProtocol(protocol)) {

        if (caps == nullptr || caps->SupportsPixelPortNullPixels())
        {
            sp = grid->AppendIn(p, new wxBoolProperty("Set Null Pixels", "ModelControllerConnectionPixelSetNullNodes", node->HasAttribute("nullNodes")));
            sp->SetAttribute("UseCheckbox", true);
            auto sp2 = grid->AppendIn(sp, new wxUIntProperty("Null Pixels", "ModelControllerConnectionPixelNullNodes",
                wxAtoi(GetControllerConnection()->GetAttribute("nullNodes", "0"))));
            sp2->SetAttribute("Min", 0);
            sp2->SetAttribute("Max", 100);
            sp2->SetEditor("SpinCtrl");
            if (!node->HasAttribute("nullNodes")) {
                grid->DisableProperty(sp2);
                grid->Collapse(sp);
            }
        }

        if (caps == nullptr || caps->SupportsPixelPortBrightness())
        {
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

        if (caps == nullptr || caps->SupportsPixelPortGamma())
        {
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

        if (caps == nullptr || caps->SupportsPixelPortColourOrder())
        {
            sp = grid->AppendIn(p, new wxBoolProperty("Set Color Order", "ModelControllerConnectionPixelSetColorOrder", node->HasAttribute("colorOrder")));
            sp->SetAttribute("UseCheckbox", true);
            int cidx = CONTROLLER_COLORORDER.Index(GetControllerConnection()->GetAttribute("colorOrder", "RGB"));
            auto sp2 = grid->AppendIn(sp, new wxEnumProperty("Color Order", "ModelControllerConnectionPixelColorOrder", CONTROLLER_COLORORDER, wxArrayInt(), cidx < 0 ? 0 : cidx));
            if (!node->HasAttribute("colorOrder")) {
                grid->DisableProperty(sp2);
                grid->Collapse(sp);
            }
        }

        if (caps == nullptr || caps->SupportsPixelPortDirection())
        {
            sp = grid->AppendIn(p, new wxBoolProperty("Set Pixel Direction", "ModelControllerConnectionPixelSetDirection", node->HasAttribute("reverse")));
            sp->SetAttribute("UseCheckbox", true);
            auto sp2 = grid->AppendIn(sp, new wxEnumProperty("Direction", "ModelControllerConnectionPixelDirection", CONTROLLER_DIRECTION, wxArrayInt(),
                wxAtoi(GetControllerConnection()->GetAttribute("reverse", "0"))));
            if (!node->HasAttribute("reverse")) {
                grid->DisableProperty(sp2);
                grid->Collapse(sp);
            }
        }

        if (caps == nullptr || caps->SupportsPixelPortGrouping())
        {
            sp = grid->AppendIn(p, new wxBoolProperty("Set Group Count", "ModelControllerConnectionPixelSetGroupCount", node->HasAttribute("groupCount")));
            sp->SetAttribute("UseCheckbox", true);
            auto sp2 = grid->AppendIn(sp, new wxUIntProperty("Group Count", "ModelControllerConnectionPixelGroupCount",
                wxAtoi(GetControllerConnection()->GetAttribute("groupCount", "1"))));
            sp2->SetAttribute("Min", 0);
            sp2->SetAttribute("Max", 50);
            sp2->SetEditor("SpinCtrl");
            if (!node->HasAttribute("groupCount")) {
                grid->DisableProperty(sp2);
                grid->Collapse(sp);
            }
        }
    }
}

void Model::UpdateControllerProperties(wxPropertyGridInterface* grid) {

    auto p = grid->GetPropertyByName("ModelControllerConnectionPort");
    if (GetControllerName() != "" && _controller != 0 && GetControllerPort(1) == 0) {
        p->SetHelpString("When using controller name instead of start channels then port must be specified.");
        p->SetBackgroundColour(*wxRED);
    }
    else
    {
        p->SetHelpString("");
        p->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
    }

    p = grid->GetPropertyByName("ModelControllerConnectionProtocol");
    if (GetControllerName() != "" && _controller != 0 && (GetControllerPort() == 0 || GetControllerProtocol() == ""))
    {
        p->SetHelpString("When using controller name instead of start channels then protocol must be specified.");
        p->SetBackgroundColour(*wxRED);
    }
    else
    {
        p->SetHelpString("");
        grid->GetPropertyByName("ModelControllerConnectionProtocol")->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
    }

    if (grid->GetPropertyByName("ModelChain") != nullptr)
    {
        ColourClashingChains(grid->GetPropertyByName("ModelChain"));
    }

    wxXmlNode* node = GetControllerConnection();
    wxString protocol = GetControllerProtocol();
    protocol.LowerCase();
    if (IsPixelProtocol(protocol)) {
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
    }
    grid->RefreshGrid();
}

static wxString GetColorString(wxPGProperty *p, xlColor &xc) {
    wxString tp = "Single Color Custom";
    wxColour c;
    c << p->GetValue();
    if (c == *wxRED) {
        tp = "Single Color Red";
    } else if (c == *wxBLUE) {
        tp = "Single Color Blue";
    } else if (c == *wxGREEN) {
        tp = "Single Color Green";
    } else if (c == *wxWHITE) {
        tp = "Single Color White";
    } else {
        xc = c;
    }
    return tp;
}

static void clearUnusedProtocolProperties(wxXmlNode *node) {
    std::string protocol = node->GetAttribute("Protocol");
    bool isDMX = Model::IsSerialProtocol(protocol);
    bool isPixel = Model::IsPixelProtocol(protocol);

    if (!isPixel) {
        node->DeleteAttribute("gamma");
        node->DeleteAttribute("brightness");
        node->DeleteAttribute("nullNodes");
        node->DeleteAttribute("colorOrder");
        node->DeleteAttribute("reverse");
        node->DeleteAttribute("groupCount");
    }
    if (!isDMX) {
        node->DeleteAttribute("channel");
    }

}

int Model::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    auto caps = GetControllerCaps();

    if (event.GetPropertyName() == "ModelPixelSize") {
        pixelSize = event.GetValue().GetLong();
        ModelXml->DeleteAttribute("PixelSize");
        ModelXml->AddAttribute("PixelSize", wxString::Format(wxT("%i"), pixelSize));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelPixelSize");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Model::OnPropertyGridChange::ModelPixelSize");
        IncrementChangeCount();
        return 0;
    } else if (event.GetPropertyName() == "ModelPixelStyle") {
        pixelStyle = event.GetValue().GetLong();
        ModelXml->DeleteAttribute("Antialias");
        ModelXml->AddAttribute("Antialias", wxString::Format(wxT("%i"), pixelStyle));
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
    else if (event.GetPropertyName() == "Controller") {
        if (GetControllerName() != CONTROLLERS[event.GetValue().GetInteger()])
        {
            SetControllerName(CONTROLLERS[event.GetValue().GetInteger()]);
            if (GetControllerPort() != 0 && IsPixelProtocol())
            {
                SetModelChain(">" + modelManager.GetLastModelOnPort(CONTROLLERS[event.GetValue().GetInteger()], GetControllerPort(), GetName(), GetControllerProtocol()));
            }
            else
            {
                SetModelChain("");
            }
        }
        if (GetControllerName() == "")
        {
            SetModelChain("");
            // dont set start channel to one. that way model holds its previously calculated start channel
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
        else
        {
            ModelXml->DeleteAttribute("Advanced");
            AdjustStringProperties(grid, parm1);
            if (grid->GetPropertyByName("ModelStartChannel") != nullptr) {
                grid->GetPropertyByName("ModelStartChannel")->SetValue(ModelXml->GetAttribute("StartChannel", "1"));
                grid->GetPropertyByName("ModelStartChannel")->Enable(false);
            }
            else if (grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel") != nullptr) {
                grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel")->Enable(false);
                grid->GetPropertyByName("ModelIndividualStartChannels")->Enable(false);
                grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel")->SetValue(ModelXml->GetAttribute("StartChannel", "1"));
            }
        }
        if (grid->GetPropertyByName("ModelChain") != nullptr) {
            grid->GetPropertyByName("ModelChain")->Enable(GetControllerName() != "");
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
        if (GetControllerPort() != event.GetValue().GetLong())
        {
            SetControllerPort(event.GetValue().GetLong());

            if (GetControllerPort(1) > 0 && GetControllerProtocol() == "")
            {
                if (caps == nullptr)
                {
                    SetControllerProtocol(CONTROLLER_PROTOCOLS[1]);
                }
                else
                {
                    if (caps->GetPixelProtocols().size() > 0)
                    {
                        SetControllerProtocol(caps->GetPixelProtocols().front());
                    }
                    else if (caps->GetSerialProtocols().size() > 0)
                    {
                        SetControllerProtocol(caps->GetSerialProtocols().front());
                    }
                }
                protocolChanged = true;
            }

            if (GetControllerName() != "" && _controller != 0)
            {
                if (grid->GetPropertyByName("ModelStartChannel") != nullptr)
                {
                    grid->GetPropertyByName("ModelStartChannel")->SetValue(ModelXml->GetAttribute("StartChannel", "1"));
                }
                else if (grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel") != nullptr)
                {
                    grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel")->SetValue(ModelXml->GetAttribute("StartChannel", "1"));
                }
                if (IsPixelProtocol())
                {
                    // we only do this for pixels as dmx have the channel to base it off
                    // when the port changes we have to assume any existing model chain will break
                    SetModelChain(">" + modelManager.GetLastModelOnPort(GetControllerName(), event.GetValue().GetLong(), GetName(), GetControllerProtocol()));
                }
                else {
                    SetModelChain("Beginning");
                }
            }
        }

        if (grid->GetPropertyByName("ModelChain") != nullptr) {
            grid->GetPropertyByName("ModelChain")->Enable(GetControllerName() != "" && GetControllerProtocol() != "" && GetControllerPort() != 0 && _controller != 0);
        }

        UpdateControllerProperties(grid);
        if (protocolChanged)
        {
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
        return 0;
    } else if (event.GetPropertyName() == "SmartRemote") {
        SetSmartRemote(wxAtoi(event.GetValue().GetString()));

        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionProtocol") {
        wxArrayString cp;
        int idx;
        GetControllerProtocols(cp, idx);
        std::string oldProtocol = GetControllerProtocol();
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

        if (!IsPixelProtocol(newProtocol)) {
            if (GetControllerConnection()->GetAttribute("channel", "-1") == "-1") {
                GetControllerConnection()->AddAttribute("channel", "1");
            }
        }
        if (
            ((newProtocol == "DMX" || newProtocol == "PixelNet" || newProtocol == "Renard" || newProtocol == "LOR") && IsPixelProtocol(oldProtocol)) ||
            ((oldProtocol == "DMX" || oldProtocol == "PixelNet" || oldProtocol == "Renard" || oldProtocol == "LOR") && IsPixelProtocol(newProtocol)) ||
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
    } else if (event.GetPropertyName() == "SubModels") {
        // We cant know which submodels changed so increment all their change counts to ensure anything using them knows they may have changed
        for (auto& it : GetSubModels()) {
            it->IncrementChangeCount();
        }
        IncrementChangeCount();
        wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
        wxPostEvent(modelManager.GetXLightsFrame(), eventForceRefresh);
        AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "Model::OnPropertyGridChange::Submodels");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::Submodels");
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::Submodels");

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
    }
    else if (event.GetPropertyName().StartsWith("SuperStringColours"))
    {
        SetSuperStringColours(event.GetValue().GetLong());
        return 0;
    } else if (event.GetPropertyName().StartsWith("SuperStringColour")) {
        int index = wxAtoi(event.GetPropertyName().substr(17));
        wxColor c;
        c << event.GetValue();
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
            else if (NODE_TYPES[i] == "Node Single Color") {
                tp = "Node Single Color";
                wxColor cc;
                cc << p->GetValue();
                c = cc;
            }
            p->Enable();
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
            for (int x = 0; x < 100; x++) {
                ModelXml->DeleteAttribute(StartChanAttrName(x));
            }
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

    GetModelScreenLocation().Write(ModelXml);
    AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Model::OnPropertyGridChange::ModelLayoutGroup");

    return i;
}

void Model::AdjustStringProperties(wxPropertyGridInterface *grid, int newNum) {
    wxPropertyGrid *pg = static_cast<wxPropertyGrid*>(grid);
    wxPGProperty *p = grid->GetPropertyByName("ModelIndividualStartChannels");
    if (p != nullptr) {
        pg->Freeze();
        p->Enable(GetControllerName() == "" && _controller == 0);
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
                p->Enable(GetControllerName() == "" && _controller == 0);
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
        p->Enable(parm1 > 1 && (GetControllerName() == "" || GetModelChain() == ""));
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
        for (auto it = faceInfo.begin(); it != faceInfo.end(); ++it) {
            wxXmlNode *f = new wxXmlNode(rootXml, wxXML_ELEMENT_NODE , "faceInfo");
            std::string name = it->first;
            f->AddAttribute("Name", name);
            for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
                f->AddAttribute(it2->first, it2->second);
            }
        }
    }
}

void Model::AddFace(wxXmlNode* n)
{
    ParseFaceInfo(n, faceInfo);
    Model::WriteFaceInfo(ModelXml, faceInfo);
}

void Model::AddState(wxXmlNode* n)
{
    ParseStateInfo(n, stateInfo);
    Model::WriteStateInfo(ModelXml, stateInfo);
}

void Model::AddSubmodel(wxXmlNode* n)
{
    ParseSubModel(n);

    // this may break if the submodel format changes and the user loads an old format ... if that happens this needs to go through a upgrade routine
    wxXmlNode *f = new wxXmlNode(wxXML_ELEMENT_NODE, "subModel");
    ModelXml->AddChild(f);
    for (auto a = n->GetAttributes(); a!= nullptr; a = a->GetNext())
    {
        f->AddAttribute(a->GetName(), a->GetValue());
    }
}

wxString Model::SerialiseFace() const
{
    wxString res = "";

    if (!faceInfo.empty()) {
        for (auto it = faceInfo.begin(); it != faceInfo.end(); ++it) {
            res += "    <faceInfo Name=\"" + it->first + "\" ";
            for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
                res += it2->first + "=\"" + it2->second + "\" ";
            }
            res += "/>\n";
        }
    }

    return res;
}

void Model::ParseStateInfo(wxXmlNode *f, std::map<std::string, std::map<std::string, std::string> > &stateInfo) {
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
        if (att->GetName() != "Name")
        {
                stateInfo[name][att->GetName().ToStdString()] = att->GetValue();
        }
        att = att->GetNext();
    }
}

void Model::WriteStateInfo(wxXmlNode *rootXml, const std::map<std::string, std::map<std::string, std::string> > &stateInfo, bool forceCustom) {
    if (!stateInfo.empty()) {
        for (auto it = stateInfo.begin(); it != stateInfo.end(); ++it) {
            std::string name = it->first;
            if (wxString(name).Trim(true).Trim(false) != "")
            {
                wxXmlNode *f = new wxXmlNode(rootXml, wxXML_ELEMENT_NODE, "stateInfo");
                f->AddAttribute("Name", name);
                if (forceCustom)
                {
                    f->AddAttribute("CustomColors", "1");
                }
                for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
                    if (wxString(it2->first).Trim(true).Trim(false) != "")
                        f->AddAttribute(it2->first, it2->second);
                }
            }
        }
    }
}

wxString Model::SerialiseState() const
{
    wxString res = "";

    if (!stateInfo.empty()) {
        for (auto it = stateInfo.begin(); it != stateInfo.end(); ++it) {
            res += "    <stateInfo Name=\"" + it->first + "\" ";
            for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
                if (wxString(it2->first).Trim(false).Trim(true) != "")
                {
                    res += it2->first + "=\"" + it2->second + "\" ";
                }
            }
            res += "/>\n";
        }
    }

    return res;
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
            if (IsIPValidOrHostname(ip.ToStdString()) && o != nullptr &&
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
    int output = 1;
    valid = true;
    if (sc.find(":") != std::string::npos) {
        std::string start = sc.substr(0, sc.find(":"));
        sc = sc.substr(sc.find(":") + 1);
        if (start[0] == '@' || start[0] == '<' || start[0] == '>') {
            int returnChannel = wxAtoi(sc);
            bool fromStart = start[0] == '@';
            start = Trim(start.substr(1, start.size()));
            if (start == GetName() && !CouldComputeStartChannel)
            {
                valid = false;
                output = 1;
            }
            else
            {
                if (start != GetName()) {
                    dependsonmodel = start;
                }
                Model *m = modelManager[start];
                if (m != nullptr && m->CouldComputeStartChannel) {
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
                    output = 1;
                }
            }
        }
        else if (start[0] == '!')
        {
            wxString ss = wxString(str);
            wxArrayString cs = wxSplit(ss.SubString(1, ss.Length()), ':');
            if (cs.Count() == 2) {
                Controller* c = modelManager.GetOutputManager()->GetController(cs[0].Trim(false).Trim(true).ToStdString());
                if (c != nullptr) {
                    return c->GetStartChannel() - 1 + wxAtoi(cs[1]);
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

void Model::SetFromXml(wxXmlNode* ModelNode, bool zb) {
    if (modelDimmingCurve != nullptr) {
        delete modelDimmingCurve;
        modelDimmingCurve = nullptr;
    }
    for (auto it = subModels.begin(); it != subModels.end(); ++it) {
        Model *m = *it;
        delete m;
    }
    subModels.clear();
    superStringColours.clear();

    wxString channelstr;

    zeroBased = zb;
    ModelXml=ModelNode;
    StrobeRate=0;
    Nodes.clear();

    name=ModelNode->GetAttribute("name").Trim(true).Trim(false).ToStdString();
    DisplayAs=ModelNode->GetAttribute("DisplayAs").ToStdString();
    StringType=ModelNode->GetAttribute("StringType").ToStdString();
    _pixelCount = ModelNode->GetAttribute("PixelCount", "").ToStdString();
    _pixelType = ModelNode->GetAttribute("PixelType", "").ToStdString();
    _pixelSpacing = ModelNode->GetAttribute("PixelSpacing", "").ToStdString();
    _active = ModelNode->GetAttribute("Active", "1") == "1";
    SingleNode = HasSingleNode(StringType);
    int ncc = GetNodeChannelCount(StringType);
    SingleChannel = (ncc == 1);
    if (SingleNode) {
        rgbOrder = "RGB";
    } else if (ncc == 4 && StringType[0] == 'W') {
        rgbOrder = StringType.substr(1, 4);
    } else {
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
    tempstr=ModelNode->GetAttribute("parm2");
    tempstr.ToLong(&parm2);
    tempstr=ModelNode->GetAttribute("parm3");
    tempstr.ToLong(&parm3);
    tempstr=ModelNode->GetAttribute("StrandNames");
    strandNames.clear();
    while (tempstr.size() > 0) {
        std::string t2 = tempstr.ToStdString();
        if (tempstr[0] == ',') {
            t2 = "";
            tempstr = tempstr(1, tempstr.length());
        } else if (tempstr.Contains(",")) {
            t2 = tempstr.SubString(0, tempstr.Find(",") - 1);
            tempstr = tempstr.SubString(tempstr.Find(",") + 1, tempstr.length());
        } else {
            tempstr = "";
        }
        strandNames.push_back(t2);
    }
    tempstr=ModelNode->GetAttribute("NodeNames");
    nodeNames.clear();
    while (tempstr.size() > 0) {
        std::string t2 = tempstr.ToStdString();
        if (tempstr[0] == ',') {
            t2 = "";
            tempstr = tempstr(1, tempstr.length());
        } else if (tempstr.Contains(",")) {
            t2 = tempstr.SubString(0, tempstr.Find(",") - 1);
            tempstr = tempstr.SubString(tempstr.Find(",") + 1, tempstr.length());
        } else {
            tempstr = "";
        }
        nodeNames.push_back(t2);
    }

    bool found = true;
    int index = 0;
    while (found)
    {
        auto an = wxString::Format("SuperStringColour%d", index);
        auto v = ModelXml->GetAttribute(an, "");
        if (v == "")
        {
            found = false;
        }
        else
        {
            superStringColours.push_back(wxColour(v));
        }
        index++;
    }

    CouldComputeStartChannel = false;
    std::string  dependsonmodel;
    int32_t StartChannel = GetNumberFromChannelString(ModelNode->GetAttribute("StartChannel","1").ToStdString(), CouldComputeStartChannel, dependsonmodel);
    tempstr=ModelNode->GetAttribute("Dir");
    IsLtoR = tempstr != "R";
    if (ModelNode->HasAttribute("StartSide")) {
        tempstr=ModelNode->GetAttribute("StartSide");
        isBotToTop = (tempstr == "B");
    } else {
        isBotToTop=true;
    }
    customColor = xlColor(ModelNode->GetAttribute("CustomColor", "#000000").ToStdString());

    long n;
    tempstr=ModelNode->GetAttribute("Antialias","1");
    tempstr.ToLong(&n);
    pixelStyle = n;
    tempstr=ModelNode->GetAttribute("PixelSize","2");
    tempstr.ToLong(&n);
    pixelSize = n;
    tempstr=ModelNode->GetAttribute("Transparency","0");
    tempstr.ToLong(&n);
    transparency = n;
    blackTransparency = wxAtoi(ModelNode->GetAttribute("BlackTransparency","0"));
    modelTagColour = wxColour(ModelNode->GetAttribute("TagColour", "Black"));
    layout_group = ModelNode->GetAttribute("LayoutGroup","Unassigned");

    ModelStartChannel = ModelNode->GetAttribute("StartChannel");

    // calculate starting channel numbers for each string
    size_t NumberOfStrings= HasOneString(DisplayAs) ? 1 : parm1;
    int ChannelsPerString=CalcCannelsPerString();

    SetStringStartChannels(zeroBased, NumberOfStrings, StartChannel, ChannelsPerString);
    GetModelScreenLocation().Read(ModelNode);

    InitModel();

    size_t NodeCount=GetNodeCount();
    for(size_t i=0; i<NodeCount; i++) {
        Nodes[i]->sparkle = rand() % 10000;
    }

    wxXmlNode *f = ModelNode->GetChildren();
    faceInfo.clear();
    stateInfo.clear();
    wxXmlNode * dimmingCurveNode = nullptr;
    wxXmlNode * controllerConnectionNode = nullptr;
    while (f != nullptr) {
        if ("faceInfo" == f->GetName()) {
            ParseFaceInfo(f, faceInfo);
        } else if ("stateInfo" == f->GetName()) {
            ParseStateInfo(f, stateInfo);
        } else if ("dimmingCurve" == f->GetName()) {
            dimmingCurveNode = f;
            modelDimmingCurve = DimmingCurve::createFromXML(f);
        } else if ("subModel" == f->GetName()) {
            ParseSubModel(f);
        } else if ("ControllerConnection" == f->GetName()) {
            controllerConnectionNode = f;
        }
        f = f->GetNext();
    }

    wxString cc = ModelNode->GetAttribute("ControllerConnection").ToStdString();
    if (cc != "") {
        ModelNode->DeleteAttribute("ControllerConnection");
        wxArrayString ar = wxSplit(cc, ':');
        if (controllerConnectionNode == nullptr) {
            controllerConnectionNode = new wxXmlNode(wxXML_ELEMENT_NODE , "ControllerConnection");
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
        int b = wxAtoi(ModelNode->GetAttribute("ModelBrightness"));
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

    wxXmlAttribute* att = GetControllerConnection()->GetAttributes();
    while (att != nullptr) {
        if (att->GetName() == "SmartRemote") {
            ret += ":SmartRemote=" + DecodeSmartRemote(wxAtoi(att->GetValue()));
        }
        else if (att->GetName() != "Port" && att->GetName() != "Protocol") {
            ret += ":" + att->GetName() + "=" + att->GetValue();
        }
        att = att->GetNext();
    }
    return ret;
}

std::string Model::GetControllerConnectionRangeString() const
{
    if (GetControllerProtocol() == "") return "";
    std::string ret = wxString::Format("%s:%d", GetControllerProtocol(), GetControllerPort(1)).ToStdString();
    if (GetControllerPort(1) == 0)
    {
        ret = wxString::Format("%s", GetControllerProtocol()).ToStdString();
    }
    if (GetNumPhysicalStrings() > 1 && GetControllerPort(1) != 0)
    {
        ret = wxString::Format("%s-%d", ret, GetControllerPort(GetNumPhysicalStrings())).ToStdString();
    }

    wxXmlAttribute* att = GetControllerConnection()->GetAttributes();
    while (att != nullptr) {
        if (att->GetName() == "SmartRemote") {
            ret += ":SmartRemote=" + DecodeSmartRemote(wxAtoi(att->GetValue()));
        }
        else if (att->GetName() != "Port" && att->GetName() != "Protocol") {
            ret += ":" + att->GetName() + "=" + att->GetValue();
        }
        att = att->GetNext();
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
    switch (sr) {
    case 0: return "None";
    case 1: return "A";
    case 2: return "B";
    case 3: return "C";
    case 4: return "A->B->C";
    case 5: return "B->C";
    }
    return wxString::Format("Invalid (%d)", sr).ToStdString();
}

wxXmlNode *Model::GetControllerConnection() const {
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

    if (SingleChannel)
    {
        return EncodeColour(GetNodeMaskColor(0));
    }

    return GetChannelColorLetter((absoluteChannel - fc - 1) % GetChanCountPerNode());
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

std::string Model::GetChannelInStartChannelFormat(OutputManager* outputManager, unsigned int channel)
{
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

    if (modelFormat != "")
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
        return wxString::Format("%u", channel).ToStdString();
    }
    else
    {
        // This used to be output:sc ... but that is no longer valid
        return wxString::Format("%u", channel).ToStdString();
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

    if (GetModelScreenLocation().IsLocked()) return;

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
    return SingleNode ? 1 : parm2;
}

int32_t Model::NodeStartChannel(size_t nodenum) const {
    return Nodes.size() && nodenum < Nodes.size() ? Nodes[nodenum]->ActChan: 0; //avoid memory access error if no nods -DJ
}

const std::string &Model::NodeType(size_t nodenum) const {
    return Nodes.size() && nodenum < Nodes.size() ? Nodes[nodenum]->GetNodeType(): NodeBaseClass::RGB; //avoid memory access error if no nods -DJ
}

void Model::GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform, int &bufferWi, int &bufferHi) const {
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
        InitRenderBufferNodes(type, camera, "None", newNodes, bufferWi, bufferHi);
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

void Model::ApplyTransform(const std::string &type,
                    std::vector<NodeBaseClassPtr> &newNodes,
                    int &bufferWi, int &bufferHi) const {
    //"Rotate CC 90", "Rotate CW 90", "Rotate 180", "Flip Vertical", "Flip Horizontal",
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
        int tmp = bufferHi;
        bufferHi = bufferWi;
        bufferWi = tmp;
    } else if (type == "Rotate CC 90") {
        for (int x = 0; x < newNodes.size(); x++) {
            for (auto& it2 : newNodes[x]->Coords) {
                SetCoords(it2, it2.bufY, bufferWi - it2.bufX - 1);
            }
        }
        int tmp = bufferHi;
        bufferHi = bufferWi;
        bufferWi = tmp;
    }
}

void Model::InitRenderBufferNodes(const std::string &type, const std::string &camera,
    const std::string &transform,
    std::vector<NodeBaseClassPtr> &newNodes, int &bufferWi, int &bufferHt) const {

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    int firstNode = newNodes.size();

    // want to see if i can catch something that causes this to crash
    if (firstNode + Nodes.size() <= 0)
    {
        // This seems to happen when an effect is dropped on a strand with zero pixels
        // Like a polyline segment with no nodes
        logger_base.warn("Model::InitRenderBufferNodes firstNode + Nodes.size() = %d. %s::'%s'. This commonly happens on a polyline segement with zero pixels or a cutom model with no nodes but with effects dropped on it.", (int32_t)firstNode + Nodes.size(), (const char *)GetDisplayAs().c_str(), (const char *)GetFullName().c_str());
    }

    // Don't add model group nodes if its a 3D preview render buffer
    if ( !((camera != "2D") && GetDisplayAs() == "ModelGroup" && (type == PER_PREVIEW || type == PER_PREVIEW_NO_OFFSET)) ) {
        newNodes.reserve(firstNode + Nodes.size());
        for (auto it = Nodes.begin(); it != Nodes.end(); ++it) {
            newNodes.push_back(NodeBaseClassPtr(it->get()->clone()));
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
        int strandLen = GetStrandLength(0);
        for (int x = firstNode; x < newNodes.size();) {
            if (cnt >= strandLen) {
                strand++;
                strandLen = GetStrandLength(strand);
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
        int strandLen = GetStrandLength(0);
        for (int x = firstNode; x < newNodes.size();) {
            if (cnt >= strandLen) {
                strand++;
                strandLen = GetStrandLength(strand);
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

        if (pcamera != nullptr && camera != "2D")
        {
            GetModelScreenLocation().PrepareToDraw(true, false);
        }
        else
        {
            GetModelScreenLocation().PrepareToDraw(false, false);
        }

        // We save the transformed coordinates here so we dont have to calculate them all twice
        std::list<float> outx;
        std::list<float> outy;

        // For 3D render view buffers recursively process each individual model...should be able to handle nested model groups
        if (GetDisplayAs() == "ModelGroup" && camera != "2D")
        {
            std::vector<Model *> models;
            wxArrayString mn = wxSplit(ModelXml->GetAttribute("models"), ',');
            int nc = 0;
            for (int x = 0; x < mn.size(); x++) {
                Model *c = modelManager.GetModel(mn[x].Trim(true).Trim(false).ToStdString());
                if (c != nullptr) {
                    models.push_back(c);
                    nc += c->GetNodeCount();
                }
                else if (mn[x] == "")
                {
                    // silently ignore blank models
                }
            }

            if (nc) {
                newNodes.reserve(nc);
            }
            for (Model *c : models) {
                int bw, bh;
                c->InitRenderBufferNodes("Per Preview No Offset", camera, transform, newNodes, bw, bh);
            }
        }

        for (int x = firstNode; x < newNodes.size(); x++) {
            if (newNodes[x] == nullptr)
            {
                logger_base.crit("CCC Model::InitRenderBufferNodes newNodes[x] is null ... this is going to crash.");
                wxASSERT(false);
            }
            for (auto& it2 : newNodes[x]->Coords) {
                float sx = it2.screenX;
                float sy = it2.screenY;

                if (ModelXml == nullptr)
                {
                    // if model xml is null then this isnt a real model which means it doesnt have a real location
                    // which means translatePoint is going to do strange things ... so dont call it
                }
                else
                {
                    if (pcamera == nullptr || camera == "2D")
                    {
                        // Handle all of the 2D classic transformations
                        //float sz = 0;
                        // reintroducing the z coordinate as otherwise with some rotations we end up with a zero width buffer
                        float sz = it2.screenZ;
                        GetModelScreenLocation().TranslatePoint(sx, sy, sz);
                    }
                    else
                    {
                        if (GetDisplayAs() != "ModelGroup")  // ignore for groups since they will have already calculated their node positions from recursion call above
                        {
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
        if (pcamera != nullptr && camera != "2D" && GetDisplayAs() == "ModelGroup" && type == PER_PREVIEW)
        {
            int maxDimension = ((ModelGroup*)this)->GetGridSize();
            if (maxDimension != 0 && (maxX - minX > maxDimension || maxY - minY > maxDimension))
            {
                // we need to resize all the points by this amount
                factor = std::max(((float)(maxX - minX)) / (float)maxDimension, ((float)(maxY - minY)) / (float)maxDimension);
                // But if it is already smaller we dont want to make it bigger
                if (factor < 1.0)
                {
                    factor = 1.0;
                }
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
                if (newNodes[x] == nullptr)
                {
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
        logger_base.warn("Model::InitRenderBufferNodes BufferHt was 0 ... overidden to be 1.");
        bufferHt = 1;
    }
    if (bufferWi == 0)
    {
        logger_base.warn("Model::InitRenderBufferNodes BufferWi was 0 ... overidden to be 1.");
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
                Nodes.push_back(NodeBaseClassPtr(new NodeClassSuperString(n, NodesPerString, superStringColours, GetNextName())));
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
        Nodes.push_back(NodeBaseClassPtr(new NodeBaseClass(0, 0, rgbOrder, GetNextName())));
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
        for(n = 0; n < numnodes; n++) {
            Nodes.push_back(NodeBaseClassPtr(new NodeClassRGBW(n/NodesPerString, 1, rgbOrder, wLast, rgbwHandlingType, GetNextName())));
            Nodes.back()->model = this;
        }
    }
}

int Model::GetNodeChannelCount(const std::string & nodeType) const {
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
        return superStringColours.size();
    }
    else if (nodeType == "Node Single Color") {
        return 1;
    }
    return 3;
}

// returns a number where the first node is 1
uint32_t Model::GetNodeNumber(size_t nodenum) const {
    if (nodenum >= Nodes.size()) return 0;
    int sn=Nodes[nodenum]->StringNum;
    return (Nodes[nodenum]->ActChan - stringStartChan[sn]) / 3 + sn*NodesPerString() + 1;
}

uint32_t Model::GetNodeNumber(int bufY, int bufX) const
{
    uint32_t count = 0;
    for (const auto& it : Nodes)
    {
        if (it->Coords.size() > 0)
        {
            if (it->Coords[0].bufX == bufX && it->Coords[0].bufY == bufY)
            {
                return count;
            }
        }
        count++;
    }
    return 0;
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

void Model::GetNodeScreenCoords(int nodeidx, std::vector<wxRealPoint> &pts) {
    for (int x = 0; x < Nodes[nodeidx]->Coords.size(); x++) {
        pts.push_back(wxPoint(Nodes[nodeidx]->Coords[x].screenX, Nodes[nodeidx]->Coords[x].screenY));
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
    if ((nodeinx < 0) || (nodeinx >= GetNodeCount())) return "";
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

void Model::ExportAsCustomXModel() const {

    wxString name = ModelXml->GetAttribute("name").Trim(true).Trim(false);
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (filename.IsEmpty()) return;

    wxFile f(filename);
    //    bool isnew = !wxFile::Exists(filename);
    if (!f.Create(filename, true) || !f.IsOpened()) DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());

    wxString cm = "";

    float minsx = 99999;
    float minsy = 99999;
    float maxsx = -1;
    float maxsy = -1;

    size_t nodeCount = GetNodeCount();
    for (size_t i = 0; i < nodeCount; i++) {
        float Sbufx = Nodes[i]->Coords[0].screenX;
        float Sbufy = Nodes[i]->Coords[0].screenY;
        if (Sbufx < minsx) minsx = Sbufx;
        if (Sbufx > maxsx) maxsx = Sbufx;
        if (Sbufy < minsy) minsy = Sbufy;
        if (Sbufy > maxsy) maxsy = Sbufy;
    }

    int minx = std::floor(minsx);
    int miny = std::floor(minsy);
    int maxx = std::ceil(maxsx);
    int maxy = std::ceil(maxsy);
    int sizex = maxx - minx + 1;
    int sizey = maxy - miny + 1;

    int* nodeLayout = (int*)malloc(sizey * sizex * sizeof(int));
    memset(nodeLayout, 0x00, sizey * sizex * sizeof(int));

    for (int i = 0; i < nodeCount; i++)
    {
        int x = Nodes[i]->Coords[0].screenX - minx;
        int y = sizey - (Nodes[i]->Coords[0].screenY - miny) - 1;
        nodeLayout[y*sizex + x] = i+1;
    }

    for (int i = 0; i < sizey * sizex; i++)
    {
        if (i != 0)
        {
            if (i % sizex == 0)
            {
                cm += ";";
            }
            else
            {
                cm += ",";
            }
        }
        if (nodeLayout[i] != 0)
        {
            cm += wxString::Format("%i", nodeLayout[i]);
        }
    }

    free(nodeLayout);

    wxString p1 = wxString::Format("%i", sizex);
    wxString p2 = wxString::Format("%i", sizey);
    wxString st = ModelXml->GetAttribute("StringType");
    wxString ps = ModelXml->GetAttribute("PixelSize");
    wxString t = ModelXml->GetAttribute("Transparency");
    wxString mb = ModelXml->GetAttribute("ModelBrightness");
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
    f.Write(" >\n");
    wxString face = SerialiseFace();
    if (face != "")
    {
        f.Write(face);
    }
    wxString state = SerialiseState();
    if (state != "")
    {
        f.Write(state);
    }
    wxString submodel = SerialiseSubmodel();
    if (submodel != "")
    {
        f.Write(submodel);
    }
    f.Write("</custommodel>");
    f.Close();
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

    if (c != nullptr)
    {
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
            int s = Nodes[n-1]->StringNum + 1;
            wxString bgcolor = s%2 == 1 ? "#ADD8E6" : "#90EE90";
            while (n > NodesPerString())
            {
                n -= NodesPerString();
            }
            html += wxString::Format("<td bgcolor='" + bgcolor + "'>n%ds%d</td>", n, s);
        }
        html += "</tr>";
    } else if (BufferHt > 1) {
        // horizontal or vertical matrix or frame
        for (size_t i = 0; i < NodeCount; i++) {
            size_t idx = Nodes[i]->Coords[0].bufY * BufferWi + Nodes[i]->Coords[0].bufX;
            if (idx < chmap.size())
            {
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
                    int s = Nodes[n-1]->StringNum + 1;
                    wxString bgcolor = (s%2 == 1) ? "#ADD8E6" : "#90EE90";
                    while (n > NodesPerString())
                    {
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
void Model::CopyBufCoord2ScreenCoord() {
    size_t NodeCount = GetNodeCount();
    int xoffset = BufferWi / 2;
    int yoffset = BufferHt / 2;
    for (size_t n = 0; n < NodeCount; n++) {
        size_t CoordCount=GetCoordCount(n);
        for (size_t c = 0; c < CoordCount; c++) {
            Nodes[n]->Coords[c].screenX = Nodes[n]->Coords[c].bufX - xoffset;
            Nodes[n]->Coords[c].screenY = Nodes[n]->Coords[c].bufY - yoffset;
        }
    }
    GetModelScreenLocation().SetRenderSize(BufferWi, BufferHt, GetModelScreenLocation().GetRenderDp());
}

void Model::UpdateXmlWithScale() {
    GetModelScreenLocation().Write(ModelXml);
    ModelXml->DeleteAttribute("StartChannel");
    if (ModelXml->HasAttribute("versionNumber"))
        ModelXml->DeleteAttribute("versionNumber");
    ModelXml->AddAttribute("versionNumber", CUR_MODEL_POS_VER);
    ModelXml->AddAttribute("StartChannel", ModelStartChannel);
    IncrementChangeCount();
}

bool Model::HitTest(ModelPreview* preview, glm::vec3& ray_origin, glm::vec3& ray_direction) {
    return GetModelScreenLocation().HitTest(ray_origin, ray_direction);
}

wxCursor Model::InitializeLocation(int &handle, wxCoord x, wxCoord y, ModelPreview* preview) {
    return GetModelScreenLocation().InitializeLocation(handle, x, y, Nodes, preview);
}

//#define SUPPRESSCOLOURTRANSPARENCY
void Model::ApplyTransparency(xlColor& color, int transparency, int blackTransparency) const
{
    if (color == xlBLACK) {
        float t = 100.0f - blackTransparency;
        t *= 2.55f;
        int i = std::floor(t);
        color.alpha = i > 255 ? 255 : (i < 0 ? 0 : i);
    } else {
#ifdef SUPPRESSCOLOURTRANSPARENCY
        if (transparency) {
            float t = 100.0f - transparency;
            t *= 2.55f;
            int i = std::floor(t);
            color.alpha = i > 255 ? 255 : (i < 0 ? 0 : i);
        }
#else
        int maxCol = std::max(color.red, std::max(color.green, color.blue));
        int colorAlpha = 255;
        if (transparency) {
            float t = 100.0f - transparency;
            t *= 2.55f;
            colorAlpha = std::floor(t);
        }
        if (maxCol < 64) {
            //if we're getting close to black, we'll start migrating toward the black's transparency setting
            float t = 100.0f - blackTransparency;
            t *= 2.55f;
            int blackAlpha = std::floor(t);
            t = maxCol * colorAlpha + (64 - maxCol) * blackAlpha;
            t /= 64;
            colorAlpha = std::floor(t);
        }
        color.alpha = colorAlpha;
    }
#endif
}

// display model using colors stored in each node
// used when preview is running
void Model::DisplayModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &sva, DrawGLUtils::xlAccumulator &tva, float& minx, float& miny, float& maxx, float& maxy, bool is_3d, const xlColor *c, bool allowSelected) {
    if (!IsActive() && preview->IsNoCurrentModel()) { return; }
    size_t NodeCount = Nodes.size();
    xlColor color;
    if (c != nullptr) {
        color = *c;
    }

    int w, h;
    preview->GetVirtualCanvasSize(w, h);

    ModelScreenLocation& screenLocation = GetModelScreenLocation();

    screenLocation.PrepareToDraw(is_3d, allowSelected);
    //UpdateBoundgingBox depends on "matrix" which is set in PrepareToDraw
    screenLocation.UpdateBoundingBox(Nodes);  // FIXME: Temporary...really only want to do this when something causes a boundary change

    size_t vcount = 0;
    for (const auto& it : Nodes) {
        vcount += it.get()->Coords.size();
    }
    if (pixelStyle > 1) {
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
    if (pixelStyle == 3 || transparency != 0 || blackTransparency != 0) {
        needTransparent = true;
    }
    DrawGLUtils::xlAccumulator &va = needTransparent ? tva : sva;
    va.PreAlloc(maxVertexCount);

    int first = 0;
    int last = NodeCount;
    int buffFirst = -1;
    int buffLast = -1;
    bool left = true;

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
        if (c == nullptr) {
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
        size_t CoordCount=GetCoordCount(n);
        for(size_t c2=0; c2 < CoordCount; c2++) {
            // draw node on screen
            float sx = Nodes[n]->Coords[c2].screenX;;
            float sy = Nodes[n]->Coords[c2].screenY;
            float sz = Nodes[n]->Coords[c2].screenZ;
            GetModelScreenLocation().TranslatePoint(sx, sy, sz);

            if (sx < minx) minx = sx;
            if (sy < miny) miny = sy;
            if (sx > maxx) maxx = sx;
            if (sy > maxy) maxy = sy;

            if (pixelStyle < 2) {
                xlColor c3(color);
                ApplyTransparency(c3, transparency, blackTransparency);
                va.AddVertex(sx, sy, c3);
            } else {
                xlColor ccolor(color);
                xlColor ecolor(color);
                ApplyTransparency(ccolor, transparency, blackTransparency);
                if (pixelStyle == 2) {
                    ecolor = ccolor;
                } else {
                    ecolor.alpha = 0;
                }
                va.AddTrianglesCircle(sx, sy, ((float)pixelSize) / 2.0f, ccolor, ecolor);
            }
        }
    }
    if (pixelStyle > 1) {
        va.Finish(GL_TRIANGLES);
    } else {
        va.Finish(GL_POINTS, pixelStyle == 1 ? GL_POINT_SMOOTH : 0, preview->calcPixelSize(pixelSize));
    }
    if ((Selected || (Highlighted && is_3d)) && c != nullptr && allowSelected) {
        GetModelScreenLocation().DrawHandles(va, preview->GetCameraZoomForHandles(), preview->GetHandleScale());
    }
}

// display model using colors stored in each node
// used when preview is running
void Model::DisplayModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator &sva, DrawGLUtils::xl3Accumulator &tva, DrawGLUtils::xl3Accumulator& lva, bool is_3d, const xlColor *c, bool allowSelected, bool wiring, bool highlightFirst, int highlightpixel) {
    if (!IsActive() && preview->IsNoCurrentModel()) { return; }
    size_t NodeCount = Nodes.size();
    xlColor color;
    if (c != nullptr) {
        color = *c;
    }

    int w, h;
    preview->GetVirtualCanvasSize(w, h);

	 ModelScreenLocation& screenLocation = GetModelScreenLocation();

    screenLocation.UpdateBoundingBox(Nodes);  // FIXME: Temporary...really only want to do this when something causes a boundary change
    screenLocation.PrepareToDraw(is_3d, allowSelected);

    unsigned int vcount = 0;
    for (const auto& it : Nodes) {
        vcount += it.get()->Coords.size();
    }
    if (pixelStyle > 1) {
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
    if (pixelStyle == 3 || transparency != 0 || blackTransparency != 0) {
        needTransparent = true;
    }

    DrawGLUtils::xl3Accumulator& va = needTransparent ? tva : sva;
    va.PreAlloc(maxVertexCount);

    DrawGLUtils::xl3Accumulator& vaLines = lva;
    vaLines.PreAlloc(2*maxVertexCount);

    int first = 0;
    int last = NodeCount;
    int buffFirst = -1;
    int buffLast = -1;
    bool left = true;

    float lastX = -99999999.0f;
    float lastY = -99999999.0f;
    float lastZ = -99999999.0f;
    uint32_t lastChan = 0;
    xlColor cLine(0x49, 0x80, 0x49);

    bool replaceVertices = allowSelected && GroupSelected && DisplayAs == "SubModel";
    bool firstNode = true;

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
        if (c == nullptr) {
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
        size_t CoordCount=GetCoordCount(n);
        for (size_t c2=0; c2 < CoordCount; c2++) {
            // draw node on screen
            float sx = Nodes[n]->Coords[c2].screenX;;
            float sy = Nodes[n]->Coords[c2].screenY;
            float sz = Nodes[n]->Coords[c2].screenZ;

            if (pixelStyle < 2) {
                GetModelScreenLocation().TranslatePoint(sx, sy, sz);
            
                xlColor c3(color);

                if (n + 1 == highlightpixel) {
                    c3 = xlMAGENTA;
                } else  if (firstNode && highlightFirst) {
                    c3 = xlYELLOW;
                }

                ApplyTransparency(c3, transparency, blackTransparency);
                va.AddVertex(sx, sy, sz, c3, replaceVertices);
            } else {
                xlColor ccolor(color);
                xlColor ecolor(color);

                if (n+1 == highlightpixel) {
                    ccolor = xlMAGENTA;
                    ecolor = xlMAGENTA;
                }
                else if (firstNode && highlightFirst) {
                    ccolor = xlYELLOW;
                    ecolor = xlYELLOW;
                }

                ApplyTransparency(ccolor, transparency, blackTransparency);
                if (pixelStyle == 2) {
                    ecolor = ccolor;
                } else {
                    ecolor.alpha = 0;
                }
                va.AddTrianglesCircle(sx, sy, sz, ((float)pixelSize) / 2.0f, ccolor, ecolor,
                                      [this](float &x, float &y, float &z) {
                                          GetModelScreenLocation().TranslatePoint(x, y, z);
                                      }, replaceVertices);
            }
            firstNode = false;
            if (wiring) {
                if (Nodes[n]->ActChan == lastChan + 3) {
                    if (lastX != 99999999.0) {
                        vaLines.AddVertex(lastX, lastY, lastZ, cLine, false);
                        vaLines.AddVertex(sx, sy, sz, cLine, false);
                    }
                }
                lastX = sx;
                lastY = sy;
                lastZ = sz;
                lastChan = Nodes[n]->ActChan;
            }
        }
    }

    if (wiring && vaLines.count > 0) {
        vaLines.Finish(GL_LINES, GL_LINE_SMOOTH, 1.7f);
    }
    if (pixelStyle > 1) {
        va.Finish(GL_TRIANGLES);
    }
    else {
        va.Finish(GL_POINTS, pixelStyle == 1 ? GL_POINT_SMOOTH : 0, preview->calcPixelSize(pixelSize));
    }
    if ((Selected || (Highlighted && is_3d)) && c != nullptr && allowSelected) {
        GetModelScreenLocation().DrawHandles(va, preview->GetCameraZoomForHandles(), preview->GetHandleScale());
    }
}

wxString Model::GetNodeNear(ModelPreview* preview, wxPoint pt)
{
    int w, h;
    preview->GetSize(&w, &h);
    float scaleX = float(w) * 0.95 / GetModelScreenLocation().RenderWi;
    float scaleY = float(h) * 0.95 / GetModelScreenLocation().RenderHt;
    float scale = scaleY < scaleX ? scaleY : scaleX;

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
    float py = h - pt.y;

    int i = 1;
    for (auto it = Nodes.begin(); it != Nodes.end(); ++it) {
        auto c = it->get()->Coords;
        for (auto it2 = c.begin(); it2 != c.end(); ++it2)
        {
            float sx=it2->screenX;
            float sy=it2->screenY;

            if (!GetModelScreenLocation().IsCenterBased()) {
                sx -= GetModelScreenLocation().RenderWi / 2.0;
                sy *= GetModelScreenLocation().GetVScaleFactor();
                if (GetModelScreenLocation().GetVScaleFactor() < 0) {
                    sy += GetModelScreenLocation().RenderHt / 2.0;
                } else {
                    sy -= GetModelScreenLocation().RenderHt / 2.0;
                }
            }
            sy = ((sy*scale)+(h/2));
            sx = (sx*scale)+(w/2);

            if (sx >= (px - pointScale) && sx <= (px + pointScale)  &&
                sy >= (py - pointScale) && sy <= (py + pointScale))
            {
                return wxString::Format(wxT("%i"),i);
            }
        }
        i++;
    }
    return "";
}

void Model::DisplayEffectOnWindow(ModelPreview* preview, double pointSize) {
    if (!IsActive() && preview->IsNoCurrentModel()) { return; }
    bool success = preview->StartDrawing(pointSize);

    if(success) {
        int w, h;
        preview->GetSize(&w, &h);

        float ml, mb;
        GetMinScreenXY(ml, mb);
        ml += GetModelScreenLocation().RenderWi / 2;
        mb += GetModelScreenLocation().RenderHt / 2;

        float scaleX = float(w) * 0.95 / GetModelScreenLocation().RenderWi;
        float scaleY = float(h) * 0.95 / GetModelScreenLocation().RenderHt;
        float scale=scaleY < scaleX ? scaleY : scaleX;

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

        LOG_GL_ERRORV(glPointSize(preview->calcPixelSize(pixelSize*pointScale)));
        int lastPixelStyle = pixelStyle;
        int lastPixelSize = pixelSize;

        // layer calculation and map to output
        size_t NodeCount = Nodes.size();
        unsigned int vcount = 0;
        for (const auto& it : Nodes) {
            vcount += it.get()->Coords.size();
        }
        if (vcount > maxVertexCount) {
            maxVertexCount = vcount;
        }
        DrawGLUtils::xlAccumulator va(maxVertexCount);
        int first = 0; int last = NodeCount;
        int buffFirst = -1; int buffLast = -1;
        bool left = true;
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
            size_t CoordCount=GetCoordCount(n);
            for(size_t c=0; c < CoordCount; c++) {
                // draw node on screen
                float sx = Nodes[n]->Coords[c].screenX;
                float sy = Nodes[n]->Coords[c].screenY;

                if (ml < 0)
                {
                    sx -= ml;
                }
                if (mb < 0)
                {
                    sy -= mb;
                }

                if (!GetModelScreenLocation().IsCenterBased()) {
                    sx -= GetModelScreenLocation().RenderWi / 2.0;
                    sy *= GetModelScreenLocation().GetVScaleFactor();
                    if (GetModelScreenLocation().GetVScaleFactor() < 0) {
                        sy += GetModelScreenLocation().RenderHt / 2.0;
                    } else {
                        sy -= GetModelScreenLocation().RenderHt / 2.0;
                    }
                }
                float newsy = ((sy*scale)+(h/2));


                if (lastPixelStyle != Nodes[n]->model->pixelStyle
                    || lastPixelSize != Nodes[n]->model->pixelSize) {

                    if (va.count && (lastPixelStyle < 2 || Nodes[n]->model->pixelStyle < 2)) {

                        if (lastPixelStyle > 1) {
                            va.Finish(GL_TRIANGLES);
                        } else {
                            va.Finish(GL_POINTS, lastPixelStyle == 1 ? GL_POINT_SMOOTH : 0, preview->calcPixelSize(lastPixelSize * pointScale));
                        }
                    }
                    lastPixelStyle = Nodes[n]->model->pixelStyle;
                    lastPixelSize = Nodes[n]->model->pixelSize;
                }

                if (lastPixelStyle < 2) {
                    xlColor c2(color);
                    ApplyTransparency(c2, Nodes[n]->model->transparency, Nodes[n]->model->blackTransparency);
                    sx = (sx*scale)+(w/2);
                    va.AddVertex(sx, newsy, c2);
                } else {
                    xlColor ccolor(color);
                    xlColor ecolor(color);
                    ApplyTransparency(ccolor, Nodes[n]->model->transparency, Nodes[n]->model->blackTransparency);
                    if (lastPixelStyle == 2) {
                        ecolor = color;
                    } else {
                        ecolor.alpha = 0;
                    }
                    va.AddTrianglesCircle((sx*scale)+(w/2), newsy, lastPixelSize*pointScale, ccolor, ecolor);
                }
            }
        }
        if (va.count) {
            if (va.count > maxVertexCount) {
                maxVertexCount = va.count;
            }
            if (lastPixelStyle > 1) {
                va.Finish(GL_TRIANGLES);
            } else {
                va.Finish(GL_POINTS, lastPixelStyle == 1 ? GL_POINT_SMOOTH : 0, preview->calcPixelSize(lastPixelSize * pointScale));
            }
            DrawGLUtils::Draw(va);
        }
        preview->EndDrawing();
    }
}

glm::vec3 Model::MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX,int mouseY) {

    if (GetModelScreenLocation().IsLocked()) return GetModelScreenLocation().GetHandlePosition(handle);

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

    if (GetModelScreenLocation().IsLocked()) return;

    GetModelScreenLocation().InsertHandle(after_handle, zoom, scale);
}

void Model::DeleteHandle(int handle) {

    if (GetModelScreenLocation().IsLocked()) return;

    GetModelScreenLocation().DeleteHandle(handle);
}

int Model::GetStrandLength(int strand) const {
    return GetNodeCount() / GetNumStrands();
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
    if (parm3 == 0) logger_base.crit("Map node to index with illegal parm3 = 0.");
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

    if (GetControllerName() == oldName)
    {
        SetControllerName(newName);
        changed = true;
    }
    if (StartsWith(ModelStartChannel, "!" + oldName))
    {
        ModelStartChannel = "!" + newName + ModelStartChannel.substr(oldName.size() + 1);
        ModelXml->DeleteAttribute("StartChannel");
        ModelXml->AddAttribute("StartChannel", ModelStartChannel);
        changed = true;
    }
    if (ModelXml->GetAttribute("Advanced") == "1") {
        for (int i = 0; i < parm1; i++)
        {
            auto str = StartChanAttrName(i);
            if (ModelXml->HasAttribute(str))
            {
                auto sc = ModelXml->GetAttribute(str);
                if (StartsWith(sc, "!" + oldName))
                {
                    ModelXml->DeleteAttribute(str);
                    ModelXml->AddAttribute(str, "!" + newName + sc.substr(oldName.size() + 1));
                    changed = true;
                }
            }
        }
    }
    return changed;
}

void Model::AddSizeLocationProperties(wxPropertyGridInterface *grid) {
    GetModelScreenLocation().AddSizeLocationProperties(grid);
}

bool Model::SupportsXlightsModel()
{
    return false;
}

void Model::ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
}

void Model::ExportXlightsModel()
{
}

Model* Model::GetXlightsModel(Model* model, std::string &last_model, xLightsFrame* xlights, bool &cancelled, bool download, wxProgressDialog* prog, int low, int high)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (last_model == "")
    {
        if (download)
        {
            xlights->SuspendAutoSave(true);
            VendorModelDialog dlg(xlights, xlights->CurrentDir);
            xlights->SetCursor(wxCURSOR_WAIT);
            if (dlg.DlgInit(prog, low, high))
            {
                if (prog != nullptr)
                {
                    prog->Update(100);
                }
                xlights->SetCursor(wxCURSOR_DEFAULT);
                if (dlg.ShowModal() == wxID_OK)
                {
                    xlights->SuspendAutoSave(false);
                    last_model = dlg.GetModelFile();

                    if (last_model == "")
                    {
                        DisplayError("Failed to download model file.");

                        cancelled = true;
                        return model;
                    }
                }
                else
                {
                    xlights->SuspendAutoSave(false);
                    cancelled = true;
                    return model;
                }
            }
            else
            {
                if (prog != nullptr) prog->Hide();
                xlights->SetCursor(wxCURSOR_DEFAULT);
                xlights->SuspendAutoSave(false);
                cancelled = true;
                return model;
            }
        }
        else
        {
            wxString filename = wxFileSelector(_("Choose model file"), wxEmptyString, wxEmptyString, wxEmptyString, "xLights Model files (*.xmodel)|*.xmodel|LOR prop files (*.lff;*.lpf)|*.lff;*.lpf|General Device Type Format (*.gdtf)|*.gdtf", wxFD_OPEN);
            if (filename.IsEmpty()) {
                cancelled = true;
                return model;
            }
            last_model = filename.ToStdString();
        }
    }

    if (wxString(last_model).Lower().EndsWith(".gdtf"))
    {
        wxFileInputStream fin(last_model);
        wxZipInputStream zin(fin);
        wxZipEntry* ent = zin.GetNextEntry();

        while (ent != nullptr) {
            if (ent->GetName() == "description.xml") {
                wxXmlDocument gdtf_doc;
                gdtf_doc.Load(zin);
                if (gdtf_doc.IsOk())
                {
                    std::map<std::string, wxXmlNode*> modes;

                    for (wxXmlNode* n = gdtf_doc.GetRoot()->GetChildren(); n != nullptr; n = n->GetNext())
                    {
                        if (n->GetName() == "FixtureType")
                        {
                            for (wxXmlNode* nn = n->GetChildren(); nn != nullptr; nn = nn->GetNext())
                            {
                                if (nn->GetName() == "DMXModes")
                                {
                                    for (wxXmlNode* nnn = nn->GetChildren(); nnn != nullptr; nnn = nnn->GetNext())
                                    {
                                        if (nnn->GetName() == "DMXMode")
                                        {
                                            modes[nnn->GetAttribute("Name")] = nnn;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if (modes.size() == 0) break;

                    std::string mode = modes.begin()->first;
                    if (modes.size() > 1)
                    {
                        wxArrayString choices;
                        for (const auto& it : modes)
                        {
                            choices.push_back(it.first);
                        }
                        wxSingleChoiceDialog dlg(xlights, "Select the model mode", "DMX Model Mode", choices);
                        if (dlg.ShowModal() != wxID_OK)
                        {
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
                                if (ss[1] == "1")
                                {
                                    if (channels == 2)
                                    {
                                        return wxAtoi(ss[0]) << 8;
                                    }
                                    return wxAtoi(ss[0]);
                                }
                                else if (ss[1] == "2")
                                {
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
                                if (nn == nullptr)
                                {
                                    _high = _low;
                                }
                                else
                                {
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
                            while (_values.size() > 0)
                            {
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
                            for (wxXmlNode* nn = n->GetChildren(); nn != nullptr; nn = nn->GetNext())
                            {
                                if (nn->GetName() == "LogicalChannel")
                                {
                                    _attribute = nn->GetAttribute("Attribute").ToStdString();
                                    for (wxXmlNode* nnn = nn->GetChildren(); nnn != nullptr; nnn = nnn->GetNext())
                                    {
                                        if (nnn->GetName() == "ChannelFunction")
                                        {
                                            for (wxXmlNode* nnnn = nnn->GetChildren(); nnnn != nullptr; nnnn = nnnn->GetNext())
                                            {
                                                if (nnnn->GetName() == "ChannelSet" && nnnn->GetAttribute("Name") != "")
                                                {
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
                    for (wxXmlNode* n = modes[mode]->GetChildren(); n != nullptr; n = n->GetNext())
                    {
                        if (n->GetName() == "DMXChannels")
                        {
                            for (wxXmlNode* nn = n->GetChildren(); nn != nullptr; nn = nn->GetNext())
                            {
                                if (nn->GetName() == "DMXChannel")
                                {
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
                    if (isMovingHead)
                    {
                        model = xlights->AllModels.CreateDefaultModel("DmxMovingHead3D", startChannel);
                    }
                    else
                    {
                        model = xlights->AllModels.CreateDefaultModel("DmxMovingHead", startChannel);
                        model->GetModelXml()->DeleteAttribute("DmxStyle");
                        model->GetModelXml()->AddAttribute("DmxStyle", "Moving Head Bars");
                    }

                    model->SetHcenterPos(x);
                    model->SetVcenterPos(y);
                    // Multiply by 5 because default custom model has parm1 and parm2 set to 5 and DMX model is 1 pixel
                    ((BoxedScreenLocation&)model->GetModelScreenLocation()).SetScale(w*5, h*5);
                    model->GetModelScreenLocation().Write(model->GetModelXml());
                    model->SetLayoutGroup(lg);

                    // number of channels
                    model->GetModelXml()->DeleteAttribute("parm1");
                    model->GetModelXml()->AddAttribute("parm1", wxString::Format("%d", channels));

                    if (modes.size() > 0)
                    {
                        model->GetModelXml()->DeleteAttribute("Description");
                        model->GetModelXml()->AddAttribute("Description", XmlSafe("Mode: " + mode));
                    }

                    std::vector<std::string> nodeNames = std::vector<std::string>(channels);
                    std::map<std::string, std::map<std::string, std::string> > stateInfo;

                    for (const auto& it : _channels)
                    {
                        if (it->_attribute == "Pan")
                        {
                            model->GetModelXml()->DeleteAttribute("DmxPanChannel");
                            model->GetModelXml()->AddAttribute("DmxPanChannel", wxString::Format("%d", it->_channelStart));
                        }
                        else if (it->_attribute == "Tilt")
                        {
                            model->GetModelXml()->DeleteAttribute("DmxTiltChannel");
                            model->GetModelXml()->AddAttribute("DmxTiltChannel", wxString::Format("%d", it->_channelStart));
                        }
                        else if (it->_attribute == "ColorAdd_W")
                        {
                            model->GetModelXml()->DeleteAttribute("DmxWhiteChannel");
                            model->GetModelXml()->AddAttribute("DmxWhiteChannel", wxString::Format("%d", it->_channelStart));
                        }
                        else if (it->_attribute == "ColorRGB_Red" || it->_attribute == "ColorAdd_R")
                        {
                            model->GetModelXml()->DeleteAttribute("DmxRedChannel");
                            model->GetModelXml()->AddAttribute("DmxRedChannel", wxString::Format("%d", it->_channelStart));
                        }
                        else if (it->_attribute == "ColorRGB_Green" || it->_attribute == "ColorAdd_G")
                        {
                            model->GetModelXml()->DeleteAttribute("DmxGreenChannel");
                            model->GetModelXml()->AddAttribute("DmxGreenChannel", wxString::Format("%d", it->_channelStart));
                        }
                        else if (it->_attribute == "ColorRGB_Blue" || it->_attribute == "ColorAdd_B")
                        {
                            model->GetModelXml()->DeleteAttribute("DmxBlueChannel");
                            model->GetModelXml()->AddAttribute("DmxBlueChannel", wxString::Format("%d", it->_channelStart));
                        }
                        else if (it->_attribute == "Shutter" || it->_attribute == "Shutter1")
                        {
                            model->GetModelXml()->DeleteAttribute("DmxShutterChannel");
                            model->GetModelXml()->AddAttribute("DmxShutterChannel", wxString::Format("%d", it->_channelStart));
                        }

                        if (it->_values.size() > 0)
                        {
                            std::map<std::string, std::string> states;

                            int st = 1;
                            for (const auto& it2 : it->_values)
                            {
                                states[wxString::Format("s%d-Name", st)] = it2->_description;
                                states[wxString::Format("s%d", st)] = wxString::Format("%d", it->_channelStart);
                                states[wxString::Format("s%d-Color", st)] = wxString::Format("#%02x%02x%02x", it2->_low, it2->_low, it2->_low);

                                st++;
                            }

                            stateInfo[it->_attribute] = states;
                        }

                        // Build the node names
                        if (it->_channels == 1)
                        {
                            nodeNames[it->_channelStart - 1] = it->_attribute;
                        }
                        else
                        {
                            for (int i = 0; i < it->_channels; i++)
                            {
                                nodeNames[it->_channelStart + i - 1] = it->_attribute + wxString::Format("-%d", i + 1).ToStdString();
                            }
                        }
                    }

                    std::string nodenames = "";
                    for (const auto& s : nodeNames)
                    {
                        if (nodenames != "") nodenames += ",";
                        nodenames += s;
                    }
                    model->GetModelXml()->DeleteAttribute("NodeNames");
                    model->GetModelXml()->AddAttribute("NodeNames", nodenames);

                    if (stateInfo.size() > 0)
                    {
                        Model::WriteStateInfo(model->GetModelXml(), stateInfo, true);
                    }

                    model->Selected = true;

                    while (_channels.size() > 0)
                    {
                        delete _channels.front();
                        _channels.pop_front();
                    }
                }
                else
                {
                    cancelled = true;
                }
                break;
            }
            ent = zin.GetNextEntry();
        }
        return model;
    }
    else if (!wxString(last_model).Lower().EndsWith(".xmodel"))
    {
        // if it isnt an xmodel then it is custom
        return model;
    }

    wxXmlDocument doc(last_model);
    if (doc.IsOk())
    {
        wxXmlNode* root = doc.GetRoot();

        if (root->GetName() == "custommodel") {
            return model;
        } else if (root->GetName() == "polylinemodel") {
            // not a custom model so delete the default model that was created
            std::string startChannel = model->GetModelXml()->GetAttribute("StartChannel", "1").ToStdString();
            auto lg = model->GetLayoutGroup();
            if( model != nullptr ) {
                xlights->AddTraceMessage("GetXlightsModel converted model to PolyLine");
                delete model;
            }
            model = xlights->AllModels.CreateDefaultModel("Poly Line", startChannel);
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
            model->InitializeLocation(h1, l, b, nullptr);
            ((ThreePointScreenLocation&)model->GetModelScreenLocation()).SetMWidth(std::abs(r-l));
            ((ThreePointScreenLocation&)model->GetModelScreenLocation()).SetHeight(2 * (float)std::abs(t-b) / (float)std::abs(r-l));
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
                 root->GetName() == "dmxservo3d" ) {

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
            model->GetModelScreenLocation().SetScaleMatrix(glm::vec3(1,1,1));
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
        else
        {
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

    wxXmlNode * root = GetModelXml();
    wxXmlNode * child = root->GetChildren();
    while (child != nullptr) {
        if (child->GetName() == "subModel") {
            wxXmlDocument new_doc;
            new_doc.SetRoot(new wxXmlNode(*child));
            wxStringOutputStream stream;
            new_doc.Save(stream);
            wxString s = stream.GetString();
            s = s.SubString(s.Find("\n")+1, s.Length()); // skip over xml format header
            res += s;
        }
        child = child->GetNext();
    }

    return res;
}

bool Model::IsControllerConnectionValid() const
{
    return (Model::IsProtocolValid(GetControllerProtocol()) && GetControllerPort(1) > 0);
}

static std::set<wxString> SERIAL_PROTOCOLS = {
    "dmx", "dmx-open", "dmx-pro",  "pixelnet", "pixelnet-lynx", "pixelnet-open", "renard", "lor", "genericserial"
};

bool Model::IsPixelProtocol(const std::string &p) {
    if (p == "") {
        return false;
    }
    return !IsSerialProtocol(p);
}

bool Model::IsSerialProtocol(const std::string& p) {
    if (p == "") {
        return false;
    }
    wxString protocol = p;
    protocol.MakeLower();
    
    return SERIAL_PROTOCOLS.find(protocol) != SERIAL_PROTOCOLS.end();
}

void Model::SetTagColour(wxColour colour)
{
    ModelXml->DeleteAttribute("TagColour");
    ModelXml->AddAttribute("TagColour", colour.GetAsString());
    IncrementChangeCount();
    AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::SetTagColour");
}

bool Model::IsPixelProtocol() const
{
    return GetControllerPort(1) != 0 && IsPixelProtocol(GetControllerProtocol());
}

bool Model::IsSerialProtocol() const
{
    return GetControllerPort(1) != 0 && IsSerialProtocol(GetControllerProtocol());
}

int Model::GetSmartRemote() const
{
    wxString s = GetControllerConnection()->GetAttribute("SmartRemote", "0");
    return wxAtoi(s);
}

// This sorts the special A->B->C and B->C first to ensure that anything on a particular smart remote comes after things that span multiple ports
int Model::GetSortableSmartRemote() const
{
    wxString s = GetControllerConnection()->GetAttribute("SmartRemote", "0");
    int sr = wxAtoi(s);

    if (sr < 4) return sr += 10;
    return sr;
}

// string is one based
int Model::GetSmartRemoteForString(int string) const
{
    int sr = GetSmartRemote();

    if (sr < 4) return sr;

    wxString s = GetControllerConnection()->GetAttribute("Port", "0");
    int port = wxAtoi(s);

    int perSmartRemote = 3;
    if (sr == 5) perSmartRemote = 2;
    int firstfirstmax = PORTS_PER_SMARTREMOTE - ((port - 1) % PORTS_PER_SMARTREMOTE);
    int firstmax = (3 * PORTS_PER_SMARTREMOTE) - ((port - 1) % PORTS_PER_SMARTREMOTE);
    if (sr == 5) firstmax = (2 * PORTS_PER_SMARTREMOTE) - ((port - 1) % PORTS_PER_SMARTREMOTE);
    int othermax = 3 * PORTS_PER_SMARTREMOTE;
    if (sr == 5) othermax = 2 * PORTS_PER_SMARTREMOTE;

    if (string <= firstfirstmax) {
        sr = ((string - 1) / PORTS_PER_SMARTREMOTE) % perSmartRemote + 4 - perSmartRemote;
    }
    else if (string <= firstmax) {
        sr = 1 + ((string - firstfirstmax - 1) / PORTS_PER_SMARTREMOTE) % perSmartRemote + 4 - perSmartRemote;
    }
    else {
        sr = ((string  - firstmax - 1) / PORTS_PER_SMARTREMOTE) % perSmartRemote + 4 - perSmartRemote;
    }

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

void Model::SetSmartRemote(int sr)
{
    if (GetSmartRemote() != sr)
    {
        // Find the last model on this smart remote
        if (GetControllerName() != "")
        {
            SetModelChain(modelManager.GetLastModelOnPort(GetControllerName(), GetControllerPort(), GetName(), GetControllerProtocol(), sr));
        }
        GetControllerConnection()->DeleteAttribute("SmartRemote");
        if (sr != 0)
        {
            GetControllerConnection()->AddAttribute("SmartRemote", wxString::Format("%d", sr));
        }
    }

    AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::SetSmartRemote");
    AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::SetSmartRemote");
    AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "Model::SetSmartRemote");
    AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "Model::SetSmartRemote");
    //AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Model::SetSmartRemote");
    AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "Model::SetSmartRemote");
    AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::SetSmartRemote");
    IncrementChangeCount();
}

void Model::SetModelChain(const std::string& modelChain)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string mc = modelChain;
    if (mc != "" && mc != "Beginning" && !StartsWith(mc, ">"))
    {
        mc = ">" + mc;
    }

    logger_base.debug("Model '%s' chained to '%s'.", (const char*)GetName().c_str(), (const char*)mc.c_str());
    ModelXml->DeleteAttribute("ModelChain");
    if (mc != "" && mc != "Beginning" && mc != ">")
    {
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
	if (chain == "Beginning")
	{
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

void Model::SetControllerName(const std::string& controller)
{
    auto n = Trim(controller);

    if (n == ModelXml->GetAttribute("Controller", "xyzzy_kw").Trim(true).Trim(false)) return;

    ModelXml->DeleteAttribute("Controller");
    if (n != "" && n != "Use Start Channel")
    {
        ModelXml->AddAttribute("Controller", n);
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

std::string Model::GetControllerName() const
{
    return ModelXml->GetAttribute("Controller", "").Trim(true).Trim(false).ToStdString();
}

std::list<std::string> Model::GetProtocols()
{
    std::list<std::string> res;
    for (auto a : CONTROLLER_PROTOCOLS) {
        if (a != "") {
            res.push_back(a.ToStdString());
        }
    }
    return res;
}

std::list<std::string> Model::GetLCProtocols()
{
    auto protocols = Model::GetProtocols();

    for (auto p = protocols.begin(); p != protocols.end(); ++p)
    {
        *p = wxString(*p).Lower().ToStdString();
    }

    return protocols;
}

bool Model::IsProtocolValid(std::string protocol)
{
    wxString p(protocol);
    auto protocols = Model::GetLCProtocols();
    return (std::find(protocols.begin(), protocols.end(), p.Lower()) != protocols.end());
}

bool Model::CleanupFileLocations(xLightsFrame* frame)
{
    bool rc = false;
    for (auto it = faceInfo.begin(); it != faceInfo.end(); ++it)
    {
        if (it->second.find("Type") != it->second.end() && it->second.at("Type") == "Matrix")
        {
            for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
            {
                if (it2->first != "CustomColors" && it2->first != "ImagePlacement" && it2->first != "Type" && it2->second != "")
                {
                    if (wxFile::Exists(it2->second))
                    {
                        if (!frame->IsInShowFolder(it2->second))
                        {
                            it2->second = frame->MoveToShowFolder(it2->second, wxString(wxFileName::GetPathSeparator()) + "Faces");
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

    for (auto it = faceInfo.begin(); it != faceInfo.end(); ++it)
    {
        if (all || std::find(begin(facesUsed), end(facesUsed), it->first) != facesUsed.end())
        {
            if (it->second.find("Type") != it->second.end() && it->second.at("Type") == "Matrix")
            {
                for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
                {
                    if (it2->first != "CustomColors" && it2->first != "ImagePlacement" && it2->first != "Type" && it2->second != "")
                    {
                        if (all || wxFile::Exists(it2->second))
                        {
                            std::string n = it2->second;
                            if (includeFaceName) n = it->first + "|" + n;
                            res.push_back(n);
                        }
                    }
                }
            }
        }
    }

    return res;
}

std::vector<std::string> Model::GetModelState() const
{
    return modelState;
}

void Model::SaveModelState( std::vector<std::string>& state )
{
    modelState = state;
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

int Model::GetControllerPort(int string) const
{
    wxString p = wxString::Format("%d", string);
    if (GetControllerConnection()->HasAttribute(p)) {
        wxString s = GetControllerConnection()->GetAttribute(p);
        return wxAtoi(s);
    }
    
    wxString s = GetControllerConnection()->GetAttribute("Port", "0");
    int port = wxAtoi(s);
    if (port > 0) {
        int sr = GetSmartRemote();
        if (sr < 4) {
            port += string - 1;
        }
        else {
            int firstfirstmax = PORTS_PER_SMARTREMOTE - ((port - 1) % PORTS_PER_SMARTREMOTE);
            int firstmax = (3* PORTS_PER_SMARTREMOTE) - ((port-1) % PORTS_PER_SMARTREMOTE);
            if (sr == 5) firstmax = (2* PORTS_PER_SMARTREMOTE) - ((port-1) % PORTS_PER_SMARTREMOTE);
            int othermax = 3 * PORTS_PER_SMARTREMOTE;
            if (sr == 5) othermax = 2 * PORTS_PER_SMARTREMOTE;

            if (string <= firstfirstmax)                 {
                port += (string - 1);
            }
            else if (string <= firstmax) {
                port += (string - 1 - firstfirstmax) % PORTS_PER_SMARTREMOTE - ((port - 1) % PORTS_PER_SMARTREMOTE);
            }
            else {
                port += PORTS_PER_SMARTREMOTE + (string - firstmax - 1) % PORTS_PER_SMARTREMOTE + ((string - firstmax - 1) / othermax) * PORTS_PER_SMARTREMOTE - ((port - 1) % PORTS_PER_SMARTREMOTE);
            }
        }
    }
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
        for (auto it2 : it->Coords)
        {
            minx = std::min(minx, it2.screenX);
            miny = std::min(miny, it2.screenY);
        }
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
        SetHeight(_savedHeight, true);
        SetDepth(_savedDepth, true);
    }
}
