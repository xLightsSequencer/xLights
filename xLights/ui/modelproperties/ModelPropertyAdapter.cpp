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
#include <wx/wx.h>

#include "ModelPropertyAdapter.h"
#include "PropertyGridHelpers.h"
#include "ScreenLocationPropertyHelper.h"
#include "../../models/Model.h"
#include "../../models/ModelManager.h"
#include "../../models/SubModel.h"
#include "../../models/RulerObject.h"
#include "../../OutputModelManager.h"
#include "../../xLightsMain.h"
#include "../../outputs/OutputManager.h"
#include "../../outputs/Controller.h"
#include "../../outputs/ControllerSerial.h"
#include "../../controllers/ControllerCaps.h"
#include "../../LayoutGroup.h"
#include "../../Color.h"
#include "../../utils/string_utils.h"
#include "../../models/ControllerConnection.h"
#include "../../UtilFunctions.h"
#include "../../models/Pixels.h"
#include "../../ui/wxUtilities.h"

// Dialog adapters - these need to stay accessible for PopupDialogProperty etc.
#include "../../StrandNodeNamesDialog.h"
#include "../../ModelFaceDialog.h"
#include "../../ModelDimmingCurveDialog.h"
#include "../../ModelStateDialog.h"
#include "../../SubModelsDialog.h"
#include "../../EditAliasesDialog.h"
#include "../../ModelChainDialog.h"
#include "../../StartChannelDialog.h"

#include <log.h>

#define MOST_STRINGS_WE_EXPECT 480
#define MOST_CONTROLLER_PORTS_WE_EXPECT 128

// Mutable statics for property grid choice lists
static wxArrayString LAYOUT_GROUPS;
static wxArrayString CONTROLLERS;
static wxArrayString OTHERMODELLIST;

//--- Dialog adapter classes (moved from Model.cpp) ---

class StrandNodeNamesDialogAdapter : public wxPGEditorDialogAdapter {
public:
    StrandNodeNamesDialogAdapter(Model* model) : wxPGEditorDialogAdapter(), m_model(model) {}
    virtual bool DoShowDialog(wxPropertyGrid* propGrid, wxPGProperty* WXUNUSED(property)) override {
        StrandNodeNamesDialog dlg(propGrid);
        dlg.Setup(m_model, m_model->GetNodeNames(), m_model->GetStrandNames());
        if (dlg.ShowModal() == wxID_OK) {
            m_model->SetNodeNames(dlg.GetNodeNames());
            m_model->SetStrandNames(dlg.GetStrandNames());
            wxVariant v(CLICK_TO_EDIT);
            SetValue(v);
            return true;
        }
        return false;
    }
protected:
    Model* m_model;
};

class FacesDialogAdapter : public wxPGEditorDialogAdapter {
public:
    FacesDialogAdapter(Model* model, OutputManager* om) : wxPGEditorDialogAdapter(), m_model(model), _outputManager(om) {}
    virtual bool DoShowDialog(wxPropertyGrid* propGrid, wxPGProperty* WXUNUSED(property)) override {
        ModelFaceDialog dlg(propGrid, _outputManager);
        dlg.SetFaceInfo(m_model, m_model->GetFaceInfo());
        if (dlg.ShowModal() == wxID_OK) {
            m_model->SetFaceInfo(dlg.GetFaceInfo());
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

class AliasDialogAdapter : public wxPGEditorDialogAdapter {
public:
    AliasDialogAdapter(Model* model) : wxPGEditorDialogAdapter(), m_model(model) {}
    virtual bool DoShowDialog(wxPropertyGrid* propGrid, wxPGProperty* WXUNUSED(property)) override {
        EditAliasesDialog dlg(propGrid, m_model);
        if (dlg.ShowModal() == wxID_OK) {
            wxVariant v(CLICK_TO_EDIT);
            SetValue(v);
            return true;
        }
        return false;
    }
protected:
    Model* m_model = nullptr;
};

class StatesDialogAdapter : public wxPGEditorDialogAdapter {
public:
    StatesDialogAdapter(Model* model, OutputManager* om) : wxPGEditorDialogAdapter(), m_model(model), _outputManager(om) {}
    virtual bool DoShowDialog(wxPropertyGrid* propGrid, wxPGProperty* WXUNUSED(property)) override {
        ModelStateDialog dlg(propGrid, _outputManager);
        dlg.SetStateInfo(m_model, m_model->GetStateInfo());
        if (dlg.ShowModal() == wxID_OK) {
            m_model->SetStateInfo(dlg.GetStateInfo());
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

class DimmingCurveDialogAdapter : public wxPGEditorDialogAdapter {
public:
    DimmingCurveDialogAdapter(Model* model) : wxPGEditorDialogAdapter(), m_model(model) {}
    virtual bool DoShowDialog(wxPropertyGrid* propGrid, wxPGProperty* WXUNUSED(property)) override {
        ModelDimmingCurveDialog dlg(propGrid);
        auto di = m_model->GetDimmingInfo();
        if (di.empty()) {
            di["all"]["gamma"] = "1.0";
            di["all"]["brightness"] = "0";
        }
        dlg.Init(di);
        if (dlg.ShowModal() == wxID_OK) {
            di.clear();
            dlg.Update(di);
            m_model->SetDimmingInfo(di);
            wxVariant v(CLICK_TO_EDIT);
            SetValue(v);
            return true;
        }
        return false;
    }
protected:
    Model* m_model;
};

class SubModelsDialogAdapter : public wxPGEditorDialogAdapter {
public:
    SubModelsDialogAdapter(Model* model, OutputManager* om) : wxPGEditorDialogAdapter(), m_model(model), _outputManager(om) {}
    virtual bool DoShowDialog(wxPropertyGrid* propGrid, wxPGProperty* WXUNUSED(property)) override {
        SubModelsDialog dlg(propGrid, _outputManager);
        dlg.Setup(m_model);
        if (dlg.ShowModal() == wxID_OK) {
            dlg.Save();
            wxVariant v(CLICK_TO_EDIT);
            SetValue(v);
            return true;
        }
        if (dlg.ReloadLayout) {
            wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
            wxPostEvent(m_model->GetModelManager().GetXLightsFrame(), eventForceRefresh);
            m_model->AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS |
                                 OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
                                 OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::SubModelsDialog::SubModels");
        }
        return false;
    }
protected:
    Model* m_model = nullptr;
    OutputManager* _outputManager = nullptr;
};

class ModelChainDialogAdapter : public wxPGEditorDialogAdapter {
public:
    ModelChainDialogAdapter(Model* model) : wxPGEditorDialogAdapter(), m_model(model) {}
    virtual bool DoShowDialog(wxPropertyGrid* propGrid, wxPGProperty* property) override {
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
    Model* m_model = nullptr;
};

class StartChannelDialogAdapter : public wxPGEditorDialogAdapter {
public:
    StartChannelDialogAdapter(Model* model, std::string preview) : wxPGEditorDialogAdapter(), m_model(model), _preview(preview) {}
    virtual bool DoShowDialog(wxPropertyGrid* propGrid, wxPGProperty* property) override {
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
    Model* m_model = nullptr;
    std::string _preview;
};

class PopupDialogProperty : public wxStringProperty {
public:
    PopupDialogProperty(Model* m, OutputManager* om, const wxString& label, const wxString& name,
                        const wxString& value, int type)
        : wxStringProperty(label, name, value), m_model(m), m_tp(type), _outputManager(om) {}
    virtual const wxPGEditor* DoGetEditorClass() const override { return wxPGEditor_TextCtrlAndButton; }
    virtual wxPGEditorDialogAdapter* GetEditorDialog() const override {
        switch (m_tp) {
        case 1: return new StrandNodeNamesDialogAdapter(m_model);
        case 2: return new FacesDialogAdapter(m_model, _outputManager);
        case 3: return new DimmingCurveDialogAdapter(m_model);
        case 4: return new StatesDialogAdapter(m_model, _outputManager);
        case 5: return new SubModelsDialogAdapter(m_model, _outputManager);
        case 6: return new AliasDialogAdapter(m_model);
        default: break;
        }
        return nullptr;
    }
protected:
    Model* m_model = nullptr;
    OutputManager* _outputManager = nullptr;
    int m_tp = 0;
};

class StartChannelProperty : public wxStringProperty {
public:
    StartChannelProperty(Model* m, int strand, const wxString& label, const wxString& name,
                         const wxString& value, std::string preview)
        : wxStringProperty(label, name, value), m_model(m), _preview(preview), m_strand(strand) {}
    virtual const wxPGEditor* DoGetEditorClass() const override { return wxPGEditor_TextCtrlAndButton; }
    virtual wxPGEditorDialogAdapter* GetEditorDialog() const override {
        return new StartChannelDialogAdapter(m_model, _preview);
    }
protected:
    Model* m_model = nullptr;
    std::string _preview;
    int m_strand = 0;
};

class ModelChainProperty : public wxStringProperty {
public:
    ModelChainProperty(Model* m, const wxString& label, const wxString& name, const wxString& value)
        : wxStringProperty(label, name, value), m_model(m) {}
    virtual const wxPGEditor* DoGetEditorClass() const override { return wxPGEditor_TextCtrlAndButton; }
    virtual wxPGEditorDialogAdapter* GetEditorDialog() const override {
        return new ModelChainDialogAdapter(m_model);
    }
protected:
    Model* m_model = nullptr;
};

//--- ModelPropertyAdapter implementation ---

ModelPropertyAdapter::ModelPropertyAdapter(Model& model) : _model(model) {}

void ModelPropertyAdapter::AddProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    
    wxStopWatch sw;

    auto layoutGroupsList = Model::GetLayoutGroups(_model.GetModelManager());
    LAYOUT_GROUPS.clear();
    for (const auto& g : layoutGroupsList) {
        LAYOUT_GROUPS.Add(wxString(g));
    }

    wxPGProperty* sp;
    wxPGProperty* p;
    grid->Append(new wxPropertyCategory(DisplayAsTypeToString(_model.GetDisplayAs()), "ModelType"));

    AddTypeProperties(grid, outputManager);

    if (_model.SupportsLowDefinitionRender()) {
        p = grid->Append(new wxUIntProperty("Low Definition Factor", "LowDefinition", _model.GetLowDefFactor()));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 100);
        p->SetEditor("SpinCtrl");
    }

    _model._controller = 0;
    CONTROLLERS.clear();
    CONTROLLERS.Add(USE_START_CHANNEL);
    CONTROLLERS.Add(NO_CONTROLLER);

    if (_model.GetControllerName() == NO_CONTROLLER) {
        _model._controller = 1;
    }

    for (const auto& it : outputManager->GetAutoLayoutControllerNames()) {
        if (_model.GetControllerName() == it) {
            _model._controller = CONTROLLERS.size();
        }
        CONTROLLERS.Add(it);
    }

    if (CONTROLLERS.size() > 1) {
        p = grid->Append(new wxEnumProperty("Controller", "Controller", CONTROLLERS, wxArrayInt(), _model._controller));
        p->Enable(CONTROLLERS.size() > 0);
    }

    if (_model.HasOneString(_model.GetDisplayAs())) {
        p = grid->Append(new StartChannelProperty(&_model, 0, "Start Channel", "ModelStartChannel",
            _model.GetModelStartChannel(), _model.GetModelManager().GetXLightsFrame()->GetSelectedLayoutPanelPreview()));
        p->Enable(_model.GetControllerName() == "" || _model._controller == 0);
    } else {
        p = grid->Append(new wxBoolProperty("Indiv Start Chans", "ModelIndividualStartChannels", _model.HasIndividualStartChannels()));
        p->SetAttribute("UseCheckbox", true);
        p->Enable(_model.GetNumStrings() > 1 && (_model.GetControllerName() == "" || _model._controller == 0));
        if (_model.GetNumStrings() > 1 && (_model.GetControllerName() != "" && _model._controller != 0)) {
            p->SetHelpString("Individual start channels cannot be set if you have assigned a model to a controller rather than using start channels.");
        } else {
            p->SetHelpString("");
        }
        sp = grid->AppendIn(p, new StartChannelProperty(&_model, 0, "Start Channel", "ModelStartChannel",
            _model.GetModelStartChannel(), _model.GetModelManager().GetXLightsFrame()->GetSelectedLayoutPanelPreview()));
        sp->Enable(_model.GetControllerName() == "" || _model._controller == 0);
        if (_model.HasIndividualStartChannels()) {
            int c = _model.GetNumStrings();
            while ((int)_model.IndivStartChannelCount() < c) {
                _model.AddIndivStartChannel(_model.ComputeStringStartChannel(_model.IndivStartChannelCount()));
            }
            while ((int)_model.IndivStartChannelCount() > c) {
                _model.PopIndivStartChannel();
            }
            for (int x = 0; x < c; ++x) {
                std::string nm = _model.StartChanAttrName(x);
                std::string val = _model.GetIndividualStartChannel(x);
                if (x == 0) {
                    sp->SetLabel(nm);
                    sp->SetValue(val);
                } else {
                    sp = grid->AppendIn(p, new StartChannelProperty(&_model, x, nm, nm, val,
                        _model.GetModelManager().GetXLightsFrame()->GetSelectedLayoutPanelPreview()));
                }
            }
        } else {
            _model.ClearIndividualStartChannels();
        }
    }

    if (CONTROLLERS.size() > 1 && _model.GetControllerName() != "" && _model.GetControllerProtocol() != "" && _model.GetControllerPort() != 0 && _model._controller != 0) {
        p = grid->Append(new ModelChainProperty(&_model, "Model Chain", "ModelChain",
            _model.GetModelChain() == "" ? _("Beginning").ToStdString() : _model.GetModelChain()));
        p->Enable(_model.GetControllerName() != "" && _model.GetControllerProtocol() != "" && _model.GetControllerPort() != 0 && _model._controller != 0);
    }

    int shadowModelFor = 0;
    OTHERMODELLIST.clear();
    OTHERMODELLIST.Add("");

    for (const auto& it : _model.GetModelManager()) {
        auto da = it.second->GetDisplayAs();
        if (da != DisplayAsType::ModelGroup && it.first != _model.GetName()) {
            if (_model.GetShadowModelFor() == it.first) {
                shadowModelFor = OTHERMODELLIST.size();
            }
            OTHERMODELLIST.Add(it.first);
        }
    }

    grid->Append(new wxEnumProperty("Shadow Model For", "ShadowModelFor", OTHERMODELLIST, wxArrayInt(), shadowModelFor));

    int layout_group_number = 0;
    for (int grp = 0; grp < LAYOUT_GROUPS.Count(); ++grp) {
        if (LAYOUT_GROUPS[grp] == _model.GetLayoutGroup()) {
            layout_group_number = grp;
            break;
        }
    }

    grid->Append(new wxStringProperty("Description", "Description", _model.GetDescription()));
    grid->Append(new wxEnumProperty("Preview", "ModelLayoutGroup", LAYOUT_GROUPS, wxArrayInt(), layout_group_number));

    p = grid->Append(new PopupDialogProperty(&_model, outputManager, "Strand/Node Names", "ModelStrandNodeNames", CLICK_TO_EDIT, 1));
    grid->LimitPropertyEditing(p);
    p = grid->Append(new PopupDialogProperty(&_model, outputManager, "Faces", "ModelFaces", CLICK_TO_EDIT, 2));
    grid->LimitPropertyEditing(p);
    p = grid->Append(new PopupDialogProperty(&_model, outputManager, "Dimming Curves", "ModelDimmingCurves", CLICK_TO_EDIT, 3));
    grid->LimitPropertyEditing(p);
    p = grid->Append(new PopupDialogProperty(&_model, outputManager, "States", "ModelStates", CLICK_TO_EDIT, 4));
    grid->LimitPropertyEditing(p);
    p = grid->Append(new PopupDialogProperty(&_model, outputManager, "SubModels", "SubModels", CLICK_TO_EDIT, 5));
    grid->LimitPropertyEditing(p);
    p = grid->Append(new PopupDialogProperty(&_model, outputManager, "Aliases", "Aliases", CLICK_TO_EDIT, 6));
    grid->LimitPropertyEditing(p);
    p->SetHelpString("Aliases are used in mapping to provide alternate names for this model which might match a model in a sequence you are importing from. To use it use the Auto Map button.");

    auto modelGroups = _model.GetModelManager().GetGroupsContainingModel(&_model);
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
        p->ChangeFlag(wxPGFlags::ReadOnly, true);
    }

    AddControllerProperties(grid);

    p = grid->Append(new wxPropertyCategory("String Properties", "ModelStringProperties"));
    int i = NODE_TYPES.Index(_model.GetStringType());
    if (i == wxNOT_FOUND) {
        i = NODE_TYPES.Index("Single Color");
    }
    grid->AppendIn(p, new wxEnumProperty("String Type", "ModelStringType", NODE_TYPES, wxArrayInt(), i));
    if (NODE_TYPES[i] == "Single Color" || NODE_TYPES[i] == "Single Color Intensity" || NODE_TYPES[i] == "Node Single Color") {
        wxColor v;
        std::string st = _model.GetStringType();
        if (st == "Single Color Red") {
            v = *wxRED;
        } else if (st == "Single Color Green" || st == "G") {
            v = *wxGREEN;
        } else if (st == "Single Color Blue" || st == "B") {
            v = *wxBLUE;
        } else if (st == "Single Color White" || st == "W") {
            v = *wxWHITE;
        } else if (st == "Single Color Custom" || st == "Single Color Intensity" || st == "Node Single Color") {
            v = xlColorToWxColour(_model.GetCustomColor());
        } else if (st[0] == '#') {
            v = xlColorToWxColour(xlColor(st));
        }
        grid->AppendIn(p, new wxColourProperty("Color", "ModelStringColor", v));
        if (NODE_TYPES[i] == "Node Single Color") {
            grid->GetPropertyByName("ModelStringType")->SetHelpString("This represents a string of single color LEDS which are individually controlled. These are very uncommon.");
        }
    } else if (NODE_TYPES[i] == "Superstring") {
        auto& ssc = _model.GetSuperStringColours();
        if (ssc.size() == 0) {
            _model.InitSuperStringColours();
        }
        auto& ssc2 = _model.GetSuperStringColours();
        sp = grid->AppendIn(p, new wxIntProperty("Colours", "SuperStringColours", ssc2.size()));
        sp->SetAttribute("Min", 1);
        sp->SetAttribute("Max", 32);
        sp->SetEditor("SpinCtrl");
        for (int j = 0; j < (int)ssc2.size(); ++j) {
            grid->AppendIn(p, new wxColourProperty(wxString::Format("Colour %d", j + 1),
                wxString::Format("SuperStringColour%d", j), xlColorToWxColour(ssc2[j])));
        }
    } else {
        sp = grid->AppendIn(p, new wxColourProperty("Color", "ModelStringColor", *wxRED));
        sp->Enable(false);
    }
    sp = grid->AppendIn(p, new wxEnumProperty("RGBW Color Handling", "ModelRGBWHandling", RGBW_HANDLING, wxArrayInt(), _model.GetRGBWHandlingType()));
    if (_model.HasSingleChannel(_model.GetStringType()) || _model.GetNodeChannelCount(_model.GetStringType()) < 4) {
        sp->Enable(false);
    }

    p = grid->Append(new wxPropertyCategory("Appearance", "ModelAppearance"));
    sp = grid->AppendIn(p, new wxBoolProperty("Active", "Active", _model.IsActive()));
    sp->SetHelpString("If unchecked the model will not be shown in the layout screen.");
    sp->SetAttribute("UseCheckbox", true);
    sp = grid->AppendIn(p, new wxUIntProperty("Pixel Size", "ModelPixelSize", _model.GetPixelSize()));
    sp->SetAttribute("Min", 1);
    sp->SetAttribute("Max", 300);
    sp->SetEditor("SpinCtrl");
    sp->SetHelpString("By increasing the pixel size, the appearance of the element can be made to display a bigger size.");

    sp = grid->AppendIn(p, new wxEnumProperty("Pixel Style", "ModelPixelStyle", PIXEL_STYLES, wxArrayInt(), (int)_model.GetPixelStyle()));
    sp->SetHelpString("A visual representation of a pixel.");
    sp = grid->AppendIn(p, new wxUIntProperty("Transparency", "ModelPixelTransparency", _model.GetTransparency()));
    sp->SetHelpString("Adjust how opaque the element is on the display.");
    sp->SetAttribute("Min", 0);
    sp->SetAttribute("Max", 100);
    sp->SetEditor("SpinCtrl");
    sp = grid->AppendIn(p, new wxUIntProperty("Black Transparency", "ModelPixelBlackTransparency", _model.GetBlackTransparency()));
    sp->SetHelpString("Adjust how transparent the element is on the display.");
    sp->SetAttribute("Min", 0);
    sp->SetAttribute("Max", 100);
    sp->SetEditor("SpinCtrl");
    sp = grid->AppendIn(p, new wxColourProperty("Tag Color", "ModelTagColour", xlColorToWxColour(_model.GetTagColour())));
    sp->SetHelpString("A visual color assigned to the model in the model list.");
    UpdateControllerProperties(grid);
    DisableUnusedProperties(grid);

    if (sw.Time() > 500)
        spdlog::debug("        ModelPropertyAdapter::AddProperties took {}ms", sw.Time());
}

void ModelPropertyAdapter::UpdateProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    UpdateTypeProperties(grid);

    if (grid->GetPropertyByName("Controller") != nullptr) {
        grid->GetPropertyByName("Controller")->Enable(outputManager->GetAutoLayoutControllerNames().size() > 0);
    }

    if (_model.HasOneString(_model.GetDisplayAs()) && grid->GetPropertyByName("ModelStartChannel") != nullptr) {
        grid->GetPropertyByName("ModelStartChannel")->Enable(_model.GetControllerName() == "" || _model._controller == 0);
    } else {
        if (grid->GetPropertyByName("ModelIndividualStartChannels") != nullptr) {
            grid->GetPropertyByName("ModelIndividualStartChannels")->Enable(_model.GetNumStrings() > 1 && (_model.GetControllerName() == "" || _model._controller == 0));
            if (_model.GetNumStrings() > 1 && (_model.GetControllerName() != "" && _model._controller != 0)) {
                grid->GetPropertyByName("ModelIndividualStartChannels")->SetHelpString("Individual start channels cannot be set if you have assigned a model to a controller rather than using start channels.");
            } else {
                grid->GetPropertyByName("ModelIndividualStartChannels")->SetHelpString("");
            }
        }
        if (grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel") != nullptr) {
            grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel")->Enable(_model.GetControllerName() == "" || _model._controller == 0);
        }
    }

    if (grid->GetPropertyByName("ModelChain") != nullptr) {
        grid->GetPropertyByName("ModelChain")->Enable(_model.GetControllerName() != "" && _model.GetControllerProtocol() != "" && _model.GetControllerPort() != 0 && _model._controller != 0);
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
            } else {
                grid->GetPropertyByName("ModelStringType")->SetHelpString("");
            }
        } else {
            if (grid->GetPropertyByName("ModelStringColor") != nullptr) {
                grid->GetPropertyByName("ModelStringColor")->Enable(false);
            }
        }
    }

    if (grid->GetPropertyByName("ModelRGBWHandling") != nullptr) {
        grid->GetPropertyByName("ModelRGBWHandling")->Enable(!(_model.HasSingleChannel(_model.GetStringType()) || _model.GetNodeChannelCount(_model.GetStringType()) != 4));
    }
}

void ModelPropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
}

void ModelPropertyAdapter::UpdateTypeProperties(wxPropertyGridInterface* grid) {
}

void ModelPropertyAdapter::AddControllerProperties(wxPropertyGridInterface* grid) {
    auto caps = _model.GetControllerCaps();

    wxString protocol = _model.GetControllerProtocol();

    wxPGProperty* p = grid->Append(new wxPropertyCategory("Controller Connection", "ModelControllerConnectionProperties"));

    wxPGProperty* sp = grid->AppendIn(p, new wxUIntProperty("Port", "ModelControllerConnectionPort", _model.GetControllerPort(1)));
    sp->SetAttribute("Min", 0);
    if (caps == nullptr || protocol == "") {
        sp->SetAttribute("Max", MOST_CONTROLLER_PORTS_WE_EXPECT);
    } else {
        if (_model.IsSerialProtocol()) {
            sp->SetAttribute("Max", caps->GetMaxSerialPort());
        } else if (_model.IsPixelProtocol()) {
            sp->SetAttribute("Max", caps->GetMaxPixelPort());
        } else if (_model.IsLEDPanelMatrixProtocol()) {
            sp->SetAttribute("Max", caps->GetMaxLEDPanelMatrixPort());
        } else if (_model.IsVirtualMatrixProtocol()) {
            sp->SetAttribute("Max", caps->GetMaxVirtualMatrixPort());
        } else if (_model.IsPWMProtocol()) {
            sp->SetAttribute("Max", caps->GetMaxPWMPort());
        } else {
            sp->SetAttribute("Max", MOST_CONTROLLER_PORTS_WE_EXPECT);
        }
    }
    sp->SetEditor("SpinCtrl");

    std::vector<std::string> cp;
    int idx = -1;
    _model.GetControllerProtocols(cp, idx);

    std::vector<std::string> cs;
    int idxs = -1;
    if (idx >= 0 && idx < (int)cp.size()) {
        _model.GetSerialProtocolSpeeds(cp[idx], cs, idxs);
    }

    if (_model.IsPixelProtocol()) {
        int smartRemoteCount = 15;
        if (caps != nullptr) {
            smartRemoteCount = caps->GetSmartRemoteCount();
        }
        if (smartRemoteCount != 0) {
            int sr = _model.GetSmartRemote();

            sp = grid->AppendIn(p, new wxBoolProperty("Use Smart Remote", "UseSmartRemote", _model.IsCtrlPropertySet(CtrlProps::USE_SMART_REMOTE)));
            sp->SetAttribute("UseCheckbox", true);
            p->SetHelpString("Enable Smart Remote for this Model.");

            if (_model.IsCtrlPropertySet(CtrlProps::USE_SMART_REMOTE)) {
                if (_model.GetSmartRemote() != 0) {
                    auto const& srTypes = _model.GetSmartRemoteTypes();
                    if (srTypes.size() > 1) {
                        wxArrayString srlist;
                        for (auto const& typ : srTypes) {
                            srlist.Add(typ);
                        }
                        grid->AppendIn(p, new wxEnumProperty("Smart Remote Type", "SmartRemoteType", srlist, wxArrayInt(), _model.GetSmartRemoteTypeIndex(_model.GetSmartRemoteType())));
                    } else {
                        std::string type = _model.GetSmartRemoteType();
                        auto smt = grid->AppendIn(p, new wxStringProperty("Smart Remote Type", "SmartRemoteType", type));
                        smt->ChangeFlag(wxPGFlags::ReadOnly, true);
                        smt->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
                    }
                }

                wxArrayString srv;
                for (const auto& v : _model.GetSmartRemoteValues(smartRemoteCount)) srv.Add(wxString(v));
                grid->AppendIn(p, new wxEnumProperty("Smart Remote", "SmartRemote", srv, wxArrayInt(), sr - 1));

                if (_model.GetNumPhysicalStrings() > 1) {
                    sp = grid->AppendIn(p, new wxUIntProperty("Max Cascade Remotes", "MaxCascadeRemotes", _model.GetSRMaxCascade()));
                    sp->SetAttribute("Min", 1);
                    sp->SetAttribute("Max", smartRemoteCount);
                    p->SetHelpString("This is the number of smart remotes on a chain to use so if start is B and this is 2 then B and C remotes will be used.");
                    sp->SetEditor("SpinCtrl");

                    sp = grid->AppendIn(p, new wxBoolProperty("Cascade On Port", "CascadeOnPort", _model.GetSRCascadeOnPort()));
                    sp->SetAttribute("UseCheckbox", true);
                    p->SetHelpString("When selected order is 1A 1B 1C etc. When not selected order is 1A 2A 3A 4A 1B etc.");
                }
            }
        }
    }

    if (cp.size() > 0) {
        wxArrayString cpWx;
        for (const auto& s : cp) cpWx.Add(wxString(s));
        sp = grid->AppendIn(p, new wxEnumProperty("Protocol", "ModelControllerConnectionProtocol", cpWx, wxArrayInt(), idx));
        if (cp.size() == 1 && idx == 0) {
            grid->DisableProperty(sp);
        }
    }

    if (_model.IsSerialProtocol()) {
        sp = grid->AppendIn(p, new wxUIntProperty(protocol + " Channel", "ModelControllerConnectionDMXChannel", _model.GetControllerDMXChannel()));
        sp->SetAttribute("Min", 1);
        if (caps == nullptr) {
            sp->SetAttribute("Max", 512);
        } else {
            sp->SetAttribute("Max", caps->GetMaxSerialPortChannels());
        }
        sp->SetEditor("SpinCtrl");
        if (idx >= 0 && idx < (int)cp.size() && (cp[idx] != "dmx" || Contains(protocol, "DMX"))) {
            // non dmx protocols support speeds
            wxArrayString csWx;
            for (const auto& s : cs) csWx.Add(wxString(s));
            sp = grid->AppendIn(p, new wxEnumProperty("Speed", "ModelControllerConnectionSpeed", csWx, wxArrayInt(), idxs));
            if (cs.size() == 1 && idxs == 0) {
                grid->DisableProperty(sp);
            }
        }

    } else if (_model.IsPWMProtocol()) {
        if (!IsDmxDisplayType(_model.GetDisplayAs())) { //DMX models handle this themselves
            auto sp2 = grid->AppendIn(sp, new wxFloatProperty("Gamma", "ModelControllerConnectionPixelGamma", _model.GetControllerGamma()));
            sp2->SetAttribute("Min", 0.1);
            sp2->SetAttribute("Max", 5.0);
            sp2->SetAttribute("Precision", 1);
            sp2->SetAttribute("Step", 0.1);
            sp2->SetEditor("SpinCtrl");

            sp2 = grid->AppendIn(sp, new wxUIntProperty("Brightness", "ModelControllerConnectionPixelBrightness", _model.GetControllerBrightness()));
            sp2->SetAttribute("Min", 0);
            sp2->SetAttribute("Max", 100);
            sp2->SetEditor("SpinCtrl");
        }
    } else if (_model.IsPixelProtocol()) {
        if (caps == nullptr || caps->SupportsPixelPortNullPixels()) {
            sp = grid->AppendIn(p, new wxBoolProperty("Set Start Null Pixels", "ModelControllerConnectionPixelSetNullNodes", _model.IsCtrlPropertySet(CtrlProps::START_NULLS_ACTIVE)));
            sp->SetAttribute("UseCheckbox", true);
            auto sp2 = grid->AppendIn(sp, new wxUIntProperty("Start Null Pixels", "ModelControllerConnectionPixelNullNodes", _model.GetControllerStartNulls()));
            sp2->SetAttribute("Min", 0);
            sp2->SetAttribute("Max", 100);
            sp2->SetEditor("SpinCtrl");
            if (!_model.IsCtrlPropertySet(CtrlProps::START_NULLS_ACTIVE)) {
                grid->DisableProperty(sp2);
                grid->Collapse(sp);
            }
        }

        if (caps == nullptr || caps->SupportsPixelPortEndNullPixels()) {
            sp = grid->AppendIn(p, new wxBoolProperty("Set End Null Pixels", "ModelControllerConnectionPixelSetEndNullNodes", _model.IsCtrlPropertySet(CtrlProps::END_NULLS_ACTIVE)));
            sp->SetAttribute("UseCheckbox", true);
            auto sp2 = grid->AppendIn(sp, new wxUIntProperty("End Null Pixels", "ModelControllerConnectionPixelEndNullNodes", _model.GetControllerEndNulls()));
            sp2->SetAttribute("Min", 0);
            sp2->SetAttribute("Max", 100);
            sp2->SetEditor("SpinCtrl");
            if (!_model.IsCtrlPropertySet(CtrlProps::END_NULLS_ACTIVE)) {
                grid->DisableProperty(sp2);
                grid->Collapse(sp);
            }
        }

        if (caps == nullptr || caps->SupportsPixelPortBrightness()) {
            sp = grid->AppendIn(p, new wxBoolProperty("Set Brightness", "ModelControllerConnectionPixelSetBrightness", _model.IsCtrlPropertySet(CtrlProps::BRIGHTNESS_ACTIVE)));
            sp->SetAttribute("UseCheckbox", true);
            auto sp2 = grid->AppendIn(sp, new wxUIntProperty("Brightness", "ModelControllerConnectionPixelBrightness", _model.GetControllerBrightness()));
            sp2->SetAttribute("Min", 0);
            sp2->SetAttribute("Max", 100);
            sp2->SetEditor("SpinCtrl");
            if (!_model.IsCtrlPropertySet(CtrlProps::BRIGHTNESS_ACTIVE)) {
                grid->DisableProperty(sp2);
                grid->Collapse(sp);
            }
        }

        if (caps == nullptr || caps->SupportsPixelPortGamma()) {
            sp = grid->AppendIn(p, new wxBoolProperty("Set Gamma", "ModelControllerConnectionPixelSetGamma", _model.IsCtrlPropertySet(CtrlProps::GAMMA_ACTIVE)));
            sp->SetAttribute("UseCheckbox", true);
            auto sp2 = grid->AppendIn(sp, new wxFloatProperty("Gamma", "ModelControllerConnectionPixelGamma", _model.GetControllerGamma()));
            sp2->SetAttribute("Min", 0.1);
            sp2->SetAttribute("Max", 5.0);
            sp2->SetAttribute("Precision", 1);
            sp2->SetAttribute("Step", 0.1);
            sp2->SetEditor("SpinCtrl");
            if (!_model.IsCtrlPropertySet(CtrlProps::GAMMA_ACTIVE)) {
                grid->DisableProperty(sp2);
                grid->Collapse(sp);
            }
        }

        if (caps == nullptr || caps->SupportsPixelPortColourOrder()) {
            sp = grid->AppendIn(p, new wxBoolProperty("Set Color Order", "ModelControllerConnectionPixelSetColorOrder", _model.IsCtrlPropertySet(CtrlProps::COLOR_ORDER_ACTIVE)));
            sp->SetAttribute("UseCheckbox", true);
            wxArrayString colorOrderChoices;
            for (const auto& co : Model::CONTROLLER_COLORORDER) colorOrderChoices.Add(wxString(co));
            int cidx = colorOrderChoices.Index(_model.GetControllerColorOrder());
            auto sp2 = grid->AppendIn(sp, new wxEnumProperty("Color Order", "ModelControllerConnectionPixelColorOrder", colorOrderChoices, wxArrayInt(), cidx < 0 ? 0 : cidx));
            if (!_model.IsCtrlPropertySet(CtrlProps::COLOR_ORDER_ACTIVE)) {
                grid->DisableProperty(sp2);
                grid->Collapse(sp);
            }
        }

        if (caps == nullptr || caps->SupportsPixelPortDirection()) {
            sp = grid->AppendIn(p, new wxBoolProperty("Set Pixel Direction", "ModelControllerConnectionPixelSetDirection", _model.IsCtrlPropertySet(CtrlProps::REVERSE_ACTIVE)));
            sp->SetAttribute("UseCheckbox", true);
            auto sp2 = grid->AppendIn(sp, new wxEnumProperty("Direction", "ModelControllerConnectionPixelDirection", CONTROLLER_DIRECTION, wxArrayInt(), _model.GetControllerReverse()));
            if (!_model.IsCtrlPropertySet(CtrlProps::REVERSE_ACTIVE)) {
                grid->DisableProperty(sp2);
                grid->Collapse(sp);
            }
        }

        if (caps == nullptr || caps->SupportsPixelPortGrouping()) {
            sp = grid->AppendIn(p, new wxBoolProperty("Set Group Count", "ModelControllerConnectionPixelSetGroupCount", _model.IsCtrlPropertySet(CtrlProps::GROUP_COUNT_ACTIVE)));
            sp->SetAttribute("UseCheckbox", true);
            auto sp2 = grid->AppendIn(sp, new wxUIntProperty("Group Count", "ModelControllerConnectionPixelGroupCount", _model.GetControllerGroupCount()));
            sp2->SetAttribute("Min", 1);
            sp2->SetAttribute("Max", 500);
            sp2->SetEditor("SpinCtrl");
            if (!_model.IsCtrlPropertySet(CtrlProps::GROUP_COUNT_ACTIVE)) {
                grid->DisableProperty(sp2);
                grid->Collapse(sp);
            }
        }

        if (caps == nullptr || caps->SupportsPixelZigZag()) {
            sp = grid->AppendIn(p, new wxBoolProperty("Set Zig Zag", "ModelControllerConnectionPixelSetZigZag", _model.IsCtrlPropertySet(CtrlProps::ZIG_ZAG_ACTIVE)));
            sp->SetAttribute("UseCheckbox", true);
            auto sp2 = grid->AppendIn(sp, new wxUIntProperty("Zig Zag", "ModelControllerConnectionPixelZigZag", _model.GetControllerZigZag()));
            sp2->SetAttribute("Min", 0);
            sp2->SetAttribute("Max", 1000);
            sp2->SetEditor("SpinCtrl");
            if (!_model.IsCtrlPropertySet(CtrlProps::ZIG_ZAG_ACTIVE)) {
                grid->DisableProperty(sp2);
                grid->Collapse(sp);
            }
        }

        if (caps == nullptr || caps->SupportsTs()) {
            sp = grid->AppendIn(p, new wxBoolProperty("Set Smart Ts", "ModelControllerConnectionPixelSetTs", _model.IsCtrlPropertySet(CtrlProps::TS_ACTIVE)));
            sp->SetAttribute("UseCheckbox", true);
            auto sp2 = grid->AppendIn(sp, new wxUIntProperty("Smart Ts", "ModelControllerConnectionPixelTs", _model.GetSmartTs()));
            sp2->SetAttribute("Min", 0);
            sp2->SetAttribute("Max", 20);
            sp2->SetEditor("SpinCtrl");
            if (!_model.IsCtrlPropertySet(CtrlProps::TS_ACTIVE)) {
                grid->DisableProperty(sp2);
                grid->Collapse(sp);
            }
        }
    }
}

void ModelPropertyAdapter::UpdateControllerProperties(wxPropertyGridInterface* grid) {
    auto caps = _model.GetControllerCaps();

    auto p = grid->GetPropertyByName("ModelControllerConnectionPort");
    if (p != nullptr) {
        if (_model.GetControllerName() != "" && _model._controller != 0 && _model.GetControllerPort(1) == 0) {
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
        if (_model.GetControllerName() != "" && _model._controller != 0 && (_model.GetControllerPort() == 0 || _model.GetControllerProtocol() == "")) {
            p->SetHelpString("When using controller name instead of start channels then protocol must be specified.");
            p->SetBackgroundColour(*wxRED);
        } else {
            p->SetHelpString("");
            grid->GetPropertyByName("ModelControllerConnectionProtocol")->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        }
    }

    if (grid->GetPropertyByName("ModelChain") != nullptr) {
        ColourClashingChains(grid->GetPropertyByName("ModelChain"));
    }

    if (_model.IsPixelProtocol()) {
        if (grid->GetPropertyByName("ModelControllerConnectionPixelSetNullNodes") != nullptr) {
            if (!_model.IsCtrlPropertySet(CtrlProps::START_NULLS_ACTIVE)) {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetNullNodes")->SetExpanded(false);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetNullNodes.ModelControllerConnectionPixelNullNodes")->Enable(false);
            } else {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetNullNodes")->SetExpanded(true);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetNullNodes.ModelControllerConnectionPixelNullNodes")->Enable();
            }
        }

        if (grid->GetPropertyByName("ModelControllerConnectionPixelSetEndNullNodes") != nullptr) {
            if (!_model.IsCtrlPropertySet(CtrlProps::END_NULLS_ACTIVE)) {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetEndNullNodes")->SetExpanded(false);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetEndNullNodes.ModelControllerConnectionPixelEndNullNodes")->Enable(false);
            } else {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetEndNullNodes")->SetExpanded(true);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetEndNullNodes.ModelControllerConnectionPixelEndNullNodes")->Enable();
            }
        }

        if (grid->GetPropertyByName("ModelControllerConnectionPixelSetBrightness") != nullptr) {
            if (!_model.IsCtrlPropertySet(CtrlProps::BRIGHTNESS_ACTIVE)) {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetBrightness")->SetExpanded(false);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetBrightness.ModelControllerConnectionPixelBrightness")->Enable(false);
            } else {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetBrightness")->SetExpanded(true);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetBrightness.ModelControllerConnectionPixelBrightness")->Enable();
            }
        }

        if (grid->GetPropertyByName("ModelControllerConnectionPixelSetGamma") != nullptr) {
            if (!_model.IsCtrlPropertySet(CtrlProps::GAMMA_ACTIVE)) {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetGamma")->SetExpanded(false);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetGamma.ModelControllerConnectionPixelGamma")->Enable(false);
            } else {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetGamma")->SetExpanded(true);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetGamma.ModelControllerConnectionPixelGamma")->Enable();
            }
        }

        if (grid->GetPropertyByName("ModelControllerConnectionPixelSetColorOrder") != nullptr) {
            if (!_model.IsCtrlPropertySet(CtrlProps::COLOR_ORDER_ACTIVE)) {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetColorOrder")->SetExpanded(false);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetColorOrder.ModelControllerConnectionPixelColorOrder")->Enable(false);
            } else {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetColorOrder")->SetExpanded(true);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetColorOrder.ModelControllerConnectionPixelColorOrder")->Enable();
            }
        }

        if (grid->GetPropertyByName("ModelControllerConnectionPixelSetDirection") != nullptr) {
            if (!_model.IsCtrlPropertySet(CtrlProps::REVERSE_ACTIVE)) {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetDirection")->SetExpanded(false);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetDirection.ModelControllerConnectionPixelDirection")->Enable(false);
            } else {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetDirection")->SetExpanded(true);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetDirection.ModelControllerConnectionPixelDirection")->Enable();
            }
        }

        if (grid->GetPropertyByName("ModelControllerConnectionPixelSetGroupCount") != nullptr) {
            if (!_model.IsCtrlPropertySet(CtrlProps::GROUP_COUNT_ACTIVE)) {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetGroupCount")->SetExpanded(false);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetGroupCount.ModelControllerConnectionPixelGroupCount")->Enable(false);
            } else {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetGroupCount")->SetExpanded(true);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetGroupCount.ModelControllerConnectionPixelGroupCount")->Enable();
            }
        }

        if (grid->GetPropertyByName("ModelControllerConnectionPixelSetZigZag") != nullptr) {
            if (!_model.IsCtrlPropertySet(CtrlProps::ZIG_ZAG_ACTIVE)) {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetZigZag")->SetExpanded(false);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetZigZag.ModelControllerConnectionPixelZigZag")->Enable(false);
            } else {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetZigZag")->SetExpanded(true);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetZigZag.ModelControllerConnectionPixelZigZag")->Enable();
            }
        }

        if (grid->GetPropertyByName("ModelControllerConnectionPixelSetTs") != nullptr) {
            if (!_model.IsCtrlPropertySet(CtrlProps::TS_ACTIVE)) {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetTs")->SetExpanded(false);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetTs.ModelControllerConnectionPixelTs")->Enable(false);
            } else {
                grid->GetPropertyByName("ModelControllerConnectionPixelSetTs")->SetExpanded(true);
                grid->GetPropertyByName("ModelControllerConnectionPixelSetTs.ModelControllerConnectionPixelTs")->Enable();
            }
        }
    }
    grid->RefreshGrid();
}

void ModelPropertyAdapter::AddSizeLocationProperties(wxPropertyGridInterface* grid) {
    ScreenLocationPropertyHelper::AddSizeLocationProperties(_model.GetModelScreenLocation(), grid);
}

void ModelPropertyAdapter::AddDimensionProperties(wxPropertyGridInterface* grid) {
    ScreenLocationPropertyHelper::AddDimensionProperties(_model.GetModelScreenLocation(), grid);
}

void ModelPropertyAdapter::ColourClashingChains(wxPGProperty* p) {
    if (p == nullptr)
        return;

    std::string tip;
    if (_model.GetControllerName() != "" && _model._controller != 0 && _model.GetControllerProtocol() != "" && _model.GetControllerPort() != 0 && p->IsEnabled()) {
        if (!_model.modelManager.IsValidControllerModelChain(&_model, tip)) {
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

void ModelPropertyAdapter::AdjustStringProperties(wxPropertyGridInterface* grid, int newNum) {
    wxPropertyGrid* pg = static_cast<wxPropertyGrid*>(grid);
    wxPGProperty* p = grid->GetPropertyByName("ModelIndividualStartChannels");
    if (p != nullptr) {
        pg->Freeze();
        p->Enable(_model.GetControllerName() == "" || _model._controller == 0);
        bool adv = p->GetValue().GetBool();
        if (adv) {
            int count = p->GetChildCount();
            while (count > newNum) {
                count--;
                wxString nm = _model.StartChanAttrName(count);
                wxPGProperty* sp = grid->GetPropertyByName("ModelIndividualStartChannels." + nm);
                if (sp != nullptr) {
                    grid->DeleteProperty(sp);
                }
                _model._indivStartChannels.pop_back();
            }
            while (count < newNum) {
                wxString nm = _model.StartChanAttrName(count);
                std::string val = _model.ComputeStringStartChannel(count);
                _model.SetIndividualStartChannel(count, val);
                grid->AppendIn(p, new StartChannelProperty(&_model, count, nm, nm, val, _model.modelManager.GetXLightsFrame()->GetSelectedLayoutPanelPreview()));
                p->Enable(_model.GetControllerName() == "" || _model._controller == 0);
                count++;
            }
        } else if (p->GetChildCount() > 1) {
            int count = p->GetChildCount();
            for (int x = 1; x < count; ++x) {
                wxString nm = _model.StartChanAttrName(x);
                wxPGProperty* sp = grid->GetPropertyByName("ModelIndividualStartChannels." + nm);
                if (sp != nullptr) {
                    grid->DeleteProperty(sp);
                }
            }
        }
        wxPGProperty* sp = grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel");
        if (sp != nullptr) {
            if (adv) {
                sp->SetLabel(_model.StartChanAttrName(0));
            } else {
                sp->SetLabel("Start Channel");
            }
        }
        p->Enable(_model.GetNumStrings() > 1 && (_model.GetControllerName() == "" || _model._controller == 0 || _model.GetModelChain() == ""));
        pg->Thaw();
        pg->RefreshGrid();
    }
}

void ModelPropertyAdapter::AddLayerSizeProperty(wxPropertyGridInterface* grid) {
    wxPGProperty* psn = grid->Append(new wxUIntProperty("Layers", "Layers", _model.GetLayerSizeCount()));
    psn->SetAttribute("Min", 1);
    psn->SetAttribute("Max", 100);
    psn->SetEditor("SpinCtrl");

    if (_model.GetLayerSizeCount() > 1) {
        for (int i = 0; i < _model.GetLayerSizeCount(); ++i) {
            wxString id = wxString::Format("Layer%d", i);
            wxString nm = wxString::Format("Layer %d", i + 1);
            if (i == 0)
                nm = "Inside";
            else if (i == _model.GetLayerSizeCount() - 1)
                nm = "Outside";

            wxPGProperty* pls = grid->AppendIn(psn, new wxUIntProperty(nm, id, _model.GetLayerSize(i)));
            pls->SetAttribute("Min", 1);
            pls->SetAttribute("Max", 1000);
            pls->SetEditor("SpinCtrl");
        }
    }
}

bool ModelPropertyAdapter::HandleLayerSizePropertyChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if ("Layers" == event.GetPropertyName()) {
        _model.SetLayerSizeCount(event.GetValue().GetLong());
        _model.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Model::HandleLayerSizePropertyChange::Layers");
        _model.IncrementChangeCount();
        _model.OnLayerSizesChange(true);
        return true;
    } else if (event.GetPropertyName().StartsWith("Layers.Layer")) {
        int layer = wxAtoi(event.GetPropertyName().AfterLast('r'));
        _model.SetLayerSize(layer, event.GetValue().GetLong());
        _model.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "Model::HandleLayerSizePropertyChange::Layer");
        _model.IncrementChangeCount();
        _model.OnLayerSizesChange(false);
        return true;
    }
    return false;
}

void ModelPropertyAdapter::DisableUnusedProperties(wxPropertyGridInterface* grid) {
}


int ModelPropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    

    auto caps = _model.GetControllerCaps();

    _model.modelManager.GetXLightsFrame()->AddTraceMessage("Model::OnPropertyGridChange : " + event.GetPropertyName() + " : " + (event.GetValue().GetType() == "string" ? event.GetValue().GetString() : "N/A") + " : " + (event.GetValue().GetType() == "long" ? std::to_string(event.GetValue().GetLong()) : "N/A"));

    if (HandleLayerSizePropertyChange(grid, event)) {
        return 0;
    }

    if (event.GetPropertyName() == "ModelPixelSize") {
        _model.pixelSize = event.GetValue().GetLong();
        _model.AddASAPWork(OutputModelManager::WORK_VISUAL_CHANGE, "Model::OnPropertyGridChange::ModelPixelSize");
        _model.IncrementChangeCount();
        return 0;
    } else if (event.GetPropertyName() == "ModelPixelStyle") {
        _model._pixelStyle = (Model::PIXEL_STYLE)event.GetValue().GetLong();
        _model.IncrementChangeCount();
        _model.AddASAPWork(OutputModelManager::WORK_VISUAL_CHANGE, "Model::OnPropertyGridChange::ModelPixelStyle");
        return 0;
    } else if (event.GetPropertyName() == "ModelPixelTransparency") {
        _model.transparency = event.GetValue().GetLong();
        _model.IncrementChangeCount();
        _model.AddASAPWork(OutputModelManager::WORK_VISUAL_CHANGE, "Model::OnPropertyGridChange::ModelPixelTransparency");
        return 0;
    } else if (event.GetPropertyName() == "ModelPixelBlackTransparency") {
        _model.blackTransparency = event.GetValue().GetLong();
        _model.IncrementChangeCount();
        _model.AddASAPWork(OutputModelManager::WORK_VISUAL_CHANGE, "Model::OnPropertyGridChange::ModelPixelBlackTransparency");
        return 0;
    } else if (event.GetPropertyName() == "LowDefinition") {
        _model._lowDefFactor = event.GetValue().GetLong();
        _model.IncrementChangeCount();
        _model.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "Model::OnPropertyGridChange::LowDefinition");
        return 0;
    } else if (event.GetPropertyName() == "ModelTagColour") {
        wxColour wxc;
        wxc << event.GetProperty()->GetValue();
        _model._modelTagColour = wxColourToXlColor(wxc);
        _model._modelTagColourValid = true;
        _model._modelTagColourString = std::string(_model._modelTagColour);
        _model.IncrementChangeCount();
        _model.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelTagColour");
        return 0;
    } else if (event.GetPropertyName() == "ModelStrandNodeNames") {
        _model.IncrementChangeCount();
        _model.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Model::OnPropertyGridChange::ModelStrandNames");
        return 0;
    } else if (event.GetPropertyName() == "ModelDimmingCurves") {
        _model.IncrementChangeCount();
        _model.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Model::OnPropertyGridChange::ModelDimmingCurves");
        return 0;
    } else if (event.GetPropertyName() == "ModelChain") {
        std::string modelChain = event.GetValue().GetString();
        if (modelChain == "Beginning") {
            modelChain = "";
        }
        _model.SetModelChain(modelChain);
        if (modelChain != "") {
            _model._hasIndivChans = false;
            AdjustStringProperties(grid, _model.GetNumStrings());
            if (grid->GetPropertyByName("ModelStartChannel") != nullptr) {
                grid->GetPropertyByName("ModelStartChannel")->SetValue(_model.ModelStartChannel);
                grid->GetPropertyByName("ModelStartChannel")->Enable(false);
            } else if (grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel") != nullptr) {
                grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel")->Enable(false);
                grid->GetPropertyByName("ModelIndividualStartChannels")->Enable(false);
                grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel")->SetValue(_model.ModelStartChannel);
            }
        } else {
            _model.SetStartChannel("1");
            if (grid->GetPropertyByName("ModelStartChannel") != nullptr) {
                grid->GetPropertyByName("ModelStartChannel")->Enable();
            } else if (grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel") != nullptr) {
                grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel")->Enable();
                grid->GetPropertyByName("ModelIndividualStartChannels")->Enable();
            }
        }
        UpdateControllerProperties(grid);
        _model.IncrementChangeCount();
        _model.AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID |
                    OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
                    OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "Model::OnPropertyGridChange::ModelChain");
        return 0;
    } else if (event.GetPropertyName() == "ShadowModelFor") {
        if (_model.GetShadowModelFor() != OTHERMODELLIST[event.GetValue().GetInteger()]) {
            _model.SetShadowModelFor(OTHERMODELLIST[event.GetValue().GetInteger()]);
        }
        _model.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::Controller");
        return 0;
    } else if (event.GetPropertyName() == "Controller") {
        if (_model.GetControllerName() != CONTROLLERS[event.GetValue().GetInteger()]) {
            _model.SetControllerName(CONTROLLERS[event.GetValue().GetInteger()]);
            if (_model.GetControllerPort() != 0 && _model.IsPixelProtocol()) {
                _model.SetModelChain(">" + _model.modelManager.GetLastModelOnPort(CONTROLLERS[event.GetValue().GetInteger()], _model.GetControllerPort(), _model.GetName(), _model.GetControllerProtocol()));
            } else {
                _model.SetModelChain("");
            }
        }
        if (_model.GetControllerName() == "") {
            _model.SetModelChain("");
            if (grid->GetPropertyByName("ModelStartChannel") != nullptr) {
                grid->GetPropertyByName("ModelStartChannel")->Enable();
            } else if (grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel") != nullptr) {
                grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel")->Enable();
                grid->GetPropertyByName("ModelIndividualStartChannels")->Enable();
            }
        } else {
            _model._hasIndivChans = false;
            AdjustStringProperties(grid, _model.GetNumStrings());
            if (grid->GetPropertyByName("ModelStartChannel") != nullptr) {
                grid->GetPropertyByName("ModelStartChannel")->SetValue(_model.ModelStartChannel);
                grid->GetPropertyByName("ModelStartChannel")->Enable(false);
            } else if (grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel") != nullptr) {
                grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel")->Enable(false);
                grid->GetPropertyByName("ModelIndividualStartChannels")->Enable(false);
                grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel")->SetValue(_model.ModelStartChannel);
            }
        }
        if (grid->GetPropertyByName("ModelChain") != nullptr) {
            grid->GetPropertyByName("ModelChain")->Enable(_model.GetControllerName() != "" && _model._controller != 0);
        }
        UpdateControllerProperties(grid);
        _model.AddASAPWork(OutputModelManager::WORK_CONTROLLER_CONFIG_CHANGE |
                    OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Model::OnPropertyGridChange::Controller");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPort") {
        bool protocolChanged = false;
        if (_model.GetControllerPort() != event.GetValue().GetLong()) {
            _model.SetControllerPort(event.GetValue().GetLong());

            if (_model.GetControllerPort(1) > 0 && _model.GetControllerProtocol() == "") {
                if (caps == nullptr) {
                    _model.SetControllerProtocol(GetAllPixelTypes()[1]);
                } else {
                    if (caps->GetPixelProtocols().size() > 0) {
                        _model.SetControllerProtocol(caps->GetPixelProtocols().front());
                    } else if (caps->GetSerialProtocols().size() > 0) {
                        _model.SetControllerProtocol(caps->GetSerialProtocols().front());
                    } else if (caps->SupportsVirtualMatrix()) {
                        _model.SetControllerProtocol("Virtual Matrix");
                    } else if (caps->SupportsLEDPanelMatrix()) {
                        _model.SetControllerProtocol("LED Panel Matrix");
                    } else if (caps->SupportsPWM()) {
                        _model.SetControllerProtocol("PWM");
                    }
                }
                protocolChanged = true;
            }

            if (_model.GetControllerName() != "" && _model._controller != 0) {
                if (grid->GetPropertyByName("ModelStartChannel") != nullptr) {
                    grid->GetPropertyByName("ModelStartChannel")->SetValue(_model.ModelStartChannel);
                } else if (grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel") != nullptr) {
                    grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel")->SetValue(_model.ModelStartChannel);
                }
                if (_model.IsPixelProtocol()) {
                    _model.SetModelChain(">" + _model.modelManager.GetLastModelOnPort(_model.GetControllerName(), event.GetValue().GetLong(), _model.GetName(), _model.GetControllerProtocol()));
                } else {
                    _model.SetModelChain("Beginning");
                }
            }
        }

        if (grid->GetPropertyByName("ModelChain") != nullptr) {
            grid->GetPropertyByName("ModelChain")->Enable(_model.GetControllerName() != "" && _model.GetControllerProtocol() != "" && _model.GetControllerPort() != 0 && _model._controller != 0);
        }

        UpdateControllerProperties(grid);
        if (protocolChanged) {
            _model.AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Model::OnPropertyGridChange::ModelControllerConnectionPort");
        }
        _model.AddASAPWork(OutputModelManager::WORK_CONTROLLER_CONFIG_CHANGE |
                    OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Model::OnPropertyGridChange::ModelControllerConnectionPort");
        return 0;
    } else if (event.GetPropertyName() == "Active") {
        _model.SetActive(event.GetValue().GetBool());
        _model.IncrementChangeCount();
        return 0;
    } else if (event.GetPropertyName() == "UseSmartRemote") {
        auto usr = event.GetValue().GetBool();
        if (!usr) {
            _model.ClearControllerProperty(CtrlProps::USE_SMART_REMOTE);
            _model.SetSmartRemote(0);
        } else {
            _model.SetControllerProperty(CtrlProps::USE_SMART_REMOTE);
            _model.SetSmartRemote(1);
        }
        return 0;
    } else if (event.GetPropertyName() == "SmartRemote") {
        int sr = wxAtoi(event.GetValue().GetString());
        _model.SetSmartRemote(sr + 1);
        return 0;
    } else if (event.GetPropertyName() == "CascadeOnPort") {
        _model.SetSRCascadeOnPort(event.GetValue().GetBool());
        return 0;
    } else if (event.GetPropertyName() == "MaxCascadeRemotes") {
        _model.SetSRMaxCascade(event.GetValue().GetLong());
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionSpeed") {
        int sel = -1;
        std::vector<std::string> cs;
        _model.GetSerialProtocolSpeeds(_model.GetControllerProtocol(), cs, sel);
        _model.SetControllerSerialProtocolSpeed((int)std::strtol(cs[event.GetValue().GetLong()].c_str(), nullptr, 10));
        return 0;
    } else if (event.GetPropertyName() == "SmartRemoteType") {
        _model.SetSmartRemoteType(_model.GetSmartRemoteTypeName(wxAtoi(event.GetValue().GetString())));
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionProtocol") {
        std::vector<std::string> cp;
        int idx;
        _model.GetControllerProtocols(cp, idx);
        std::string oldProtocol = _model.GetControllerProtocol();

        if (event.GetValue().GetLong() >= (long)cp.size()) {
            spdlog::critical("Protocol being set is not in the controller protocols which has {} protocols.", (int)cp.size());
            return 0;
        }

        _model.SetControllerProtocol(cp[event.GetValue().GetLong()]);

        if (_model.GetControllerName() != "" && _model._controller != 0) {
            if (grid->GetPropertyByName("ModelStartChannel") != nullptr) {
                grid->GetPropertyByName("ModelStartChannel")->SetValue(_model.ModelStartChannel);
            } else if (grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel") != nullptr) {
                grid->GetPropertyByName("ModelIndividualStartChannels.ModelStartChannel")->SetValue(_model.ModelStartChannel);
            }
        }
        if (grid->GetPropertyByName("ModelChain") != nullptr) {
            grid->GetPropertyByName("ModelChain")->Enable(_model.GetControllerName() != "" && _model.GetControllerProtocol() != "" && _model.GetControllerPort() != 0 && _model._controller != 0);
        }

        UpdateControllerProperties(grid);
        std::string newProtocol = _model.GetControllerProtocol();

        if (!IsPixelProtocol(newProtocol)) {
            if (_model._controllerConnection.GetDMXChannel() == -1) {
                _model._controllerConnection.SetDMXChannel(1);
            }
        }
        if (
            (IsSerialProtocol(newProtocol) && IsPixelProtocol(oldProtocol)) ||
            (IsSerialProtocol(oldProtocol) && IsPixelProtocol(newProtocol)) ||
            (oldProtocol == "" && newProtocol != "") ||
            (newProtocol == "" && oldProtocol != "")) {
            _model.AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Model::OnPropertyGridChange::ModelControllerConnectionProtocol");
        }
        _model.AddASAPWork(OutputModelManager::WORK_CONTROLLER_CONFIG_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionProtocol");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionDMXChannel") {
        _model.SetControllerDMXChannel((int)event.GetValue().GetLong());
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetBrightness") {
        wxPGProperty* prop = grid->GetFirstChild(event.GetProperty());
        grid->EnableProperty(prop, event.GetValue().GetBool());
        if (event.GetValue().GetBool()) {
            _model.SetControllerProperty(CtrlProps::BRIGHTNESS_ACTIVE);
            prop->SetValueFromInt(_model.GetControllerBrightness());
            grid->Expand(event.GetProperty());
        } else {
            _model.ClearControllerProperty(CtrlProps::BRIGHTNESS_ACTIVE);
            grid->Collapse(event.GetProperty());
        }
        _model.AddASAPWork(OutputModelManager::WORK_CONTROLLER_CONFIG_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetBrightness");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetBrightness.ModelControllerConnectionPixelBrightness") {
        _model.SetControllerBrightness((int)event.GetValue().GetLong());
        _model.AddASAPWork(OutputModelManager::WORK_CONTROLLER_CONFIG_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelBrightness");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetGamma") {
        wxPGProperty* prop = grid->GetFirstChild(event.GetProperty());
        grid->EnableProperty(prop, event.GetValue().GetBool());
        if (event.GetValue().GetBool()) {
            _model.SetControllerProperty(CtrlProps::GAMMA_ACTIVE);
            prop->SetValue(_model.GetControllerGamma());
            grid->Expand(event.GetProperty());
        } else {
           _model.ClearControllerProperty(CtrlProps::GAMMA_ACTIVE);
           grid->Collapse(event.GetProperty());
        }
        _model.AddASAPWork(OutputModelManager::WORK_CONTROLLER_CONFIG_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetGamma");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetGamma.ModelControllerConnectionPixelGamma") {
        _model.SetControllerGamma((float)event.GetValue().GetDouble());
        _model.AddASAPWork(OutputModelManager::WORK_CONTROLLER_CONFIG_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelGamma");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetDirection") {
        wxPGProperty* prop = grid->GetFirstChild(event.GetProperty());
        grid->EnableProperty(prop, event.GetValue().GetBool());
        if (event.GetValue().GetBool()) {
            _model.SetControllerProperty(CtrlProps::REVERSE_ACTIVE);
            prop->SetValueFromInt(_model.GetControllerReverse());
            grid->Expand(event.GetProperty());
        } else {
            _model.ClearControllerProperty(CtrlProps::REVERSE_ACTIVE);
            grid->Collapse(event.GetProperty());
        }
        _model.AddASAPWork(OutputModelManager::WORK_CONTROLLER_CONFIG_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetDirection");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetDirection.ModelControllerConnectionPixelDirection") {
        _model.SetControllerReverse((int)event.GetValue().GetLong());
        _model.AddASAPWork(OutputModelManager::WORK_CONTROLLER_CONFIG_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelDirection");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetColorOrder") {
        wxPGProperty* prop = grid->GetFirstChild(event.GetProperty());
        grid->EnableProperty(prop, event.GetValue().GetBool());
        if (event.GetValue().GetBool()) {
            _model.SetControllerProperty(CtrlProps::COLOR_ORDER_ACTIVE);
            prop->SetValueFromString(_model.GetControllerColorOrder());
            grid->Expand(event.GetProperty());
        } else {
            _model.ClearControllerProperty(CtrlProps::COLOR_ORDER_ACTIVE);
            grid->Collapse(event.GetProperty());
        }
        _model.AddASAPWork(OutputModelManager::WORK_CONTROLLER_CONFIG_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetColorOrder");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetColorOrder.ModelControllerConnectionPixelColorOrder") {
        _model.SetControllerColorOrder(Model::CONTROLLER_COLORORDER[event.GetValue().GetLong()]);
        _model.AddASAPWork(OutputModelManager::WORK_CONTROLLER_CONFIG_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelColorOrder");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetNullNodes") {
        wxPGProperty* prop = grid->GetFirstChild(event.GetProperty());
        grid->EnableProperty(prop, event.GetValue().GetBool());
        if (event.GetValue().GetBool()) {
            _model.SetControllerProperty(CtrlProps::START_NULLS_ACTIVE);
            prop->SetValueFromInt(_model.GetControllerStartNulls());
            grid->Expand(event.GetProperty());
        } else {
            _model.ClearControllerProperty(CtrlProps::START_NULLS_ACTIVE);
            grid->Collapse(event.GetProperty());
        }
        _model.AddASAPWork(OutputModelManager::WORK_CONTROLLER_CONFIG_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetNullNodes");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetEndNullNodes") {
        wxPGProperty* prop = grid->GetFirstChild(event.GetProperty());
        grid->EnableProperty(prop, event.GetValue().GetBool());
        if (event.GetValue().GetBool()) {
            _model.SetControllerProperty(CtrlProps::END_NULLS_ACTIVE);
            prop->SetValueFromInt(_model.GetControllerEndNulls());
            grid->Expand(event.GetProperty());
        } else {
            _model.ClearControllerProperty(CtrlProps::END_NULLS_ACTIVE);
            grid->Collapse(event.GetProperty());
        }
        _model.AddASAPWork(OutputModelManager::WORK_CONTROLLER_CONFIG_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetEndNullNodes");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetNullNodes.ModelControllerConnectionPixelNullNodes") {
        _model.SetControllerStartNulls((int)event.GetValue().GetLong());
        _model.AddASAPWork(OutputModelManager::WORK_CONTROLLER_CONFIG_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelNullNodes");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetEndNullNodes.ModelControllerConnectionPixelEndNullNodes") {
        _model.SetControllerEndNulls((int)event.GetValue().GetLong());
        _model.AddASAPWork(OutputModelManager::WORK_CONTROLLER_CONFIG_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelEndNullNodes");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetGroupCount") {
        wxPGProperty* prop = grid->GetFirstChild(event.GetProperty());
        grid->EnableProperty(prop, event.GetValue().GetBool());
        if (event.GetValue().GetBool()) {
            _model.SetControllerProperty(CtrlProps::GROUP_COUNT_ACTIVE);
            prop->SetValueFromInt(_model.GetControllerGroupCount());
            grid->Expand(event.GetProperty());
        } else {
            _model.ClearControllerProperty(CtrlProps::GROUP_COUNT_ACTIVE);
            grid->Collapse(event.GetProperty());
        }
        _model.AddASAPWork(OutputModelManager::WORK_CONTROLLER_CONFIG_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetGroupCount");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetGroupCount.ModelControllerConnectionPixelGroupCount") {
        _model.SetControllerGroupCount((int)event.GetValue().GetLong());
        _model.AddASAPWork(OutputModelManager::WORK_CONTROLLER_CONFIG_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelGroupCount");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetZigZag") {
        wxPGProperty* prop = grid->GetFirstChild(event.GetProperty());
        grid->EnableProperty(prop, event.GetValue().GetBool());
        if (event.GetValue().GetBool()) {
            _model.SetControllerProperty(CtrlProps::ZIG_ZAG_ACTIVE);
            prop->SetValueFromInt(_model.GetControllerZigZag());
            grid->Expand(event.GetProperty());
        } else {
            _model.ClearControllerProperty(CtrlProps::ZIG_ZAG_ACTIVE);
            grid->Collapse(event.GetProperty());
        }
        _model.AddASAPWork(OutputModelManager::WORK_CONTROLLER_CONFIG_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetZigZag");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetZigZag.ModelControllerConnectionPixelZigZag") {
        _model.SetControllerZigZag((int)event.GetValue().GetLong());
        _model.AddASAPWork(OutputModelManager::WORK_CONTROLLER_CONFIG_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelZigZag");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetTs") {
        wxPGProperty* prop = grid->GetFirstChild(event.GetProperty());
        grid->EnableProperty(prop, event.GetValue().GetBool());
        if (event.GetValue().GetBool()) {
            _model.SetControllerProperty(CtrlProps::TS_ACTIVE);
            prop->SetValueFromInt(_model.GetSmartTs());
            grid->Expand(event.GetProperty());
        } else {
            _model.ClearControllerProperty(CtrlProps::TS_ACTIVE);
            grid->Collapse(event.GetProperty());
        }
        _model.AddASAPWork(OutputModelManager::WORK_CONTROLLER_CONFIG_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelSetTs");
        return 0;
    } else if (event.GetPropertyName() == "ModelControllerConnectionPixelSetTs.ModelControllerConnectionPixelTs") {
        _model.SetSmartRemoteTs((int)event.GetValue().GetLong());
        _model.AddASAPWork(OutputModelManager::WORK_CONTROLLER_CONFIG_CHANGE, "Model::OnPropertyGridChange::ModelControllerConnectionPixelTs");
        return 0;
    } else if (event.GetPropertyName() == "SubModels") {
        for (auto& it : _model.GetSubModels()) {
            it->IncrementChangeCount();
        }
        _model.IncrementChangeCount();
        wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
        wxPostEvent(_model.modelManager.GetXLightsFrame(), eventForceRefresh);
        _model.AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS |
                    OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
                    OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::SubModels");

        return 0;
    } else if (event.GetPropertyName() == "Description") {
        _model.description = event.GetValue().GetString();
        _model.SetDescription(_model.description);
        _model.IncrementChangeCount();
        _model.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::Description");
        return 0;
    } else if (event.GetPropertyName() == "ModelFaces") {
        _model.IncrementChangeCount();
        _model.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelFaces");
        return 0;
    } else if (event.GetPropertyName() == "ModelStates") {
        _model.IncrementChangeCount();
        _model.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelStates");
        return 0;
    } else if (event.GetPropertyName() == "Aliases") {
        _model.IncrementChangeCount();
        _model.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::Aliases");
        return 0;
    } else if (event.GetPropertyName().StartsWith("SuperStringColours")) {
        _model.IncrementChangeCount();
        _model.SetSuperStringColours(event.GetValue().GetLong());
        return 0;
    } else if (event.GetPropertyName().StartsWith("SuperStringColour")) {
        int index = wxAtoi(event.GetPropertyName().substr(17));
        wxColor wc;
        wc << event.GetValue();
        _model.IncrementChangeCount();
        _model.SetSuperStringColour(index, wxColourToXlColor(wc));
        return 0;
    } else if (event.GetPropertyName() == "ModelStringColor" || event.GetPropertyName() == "ModelStringType" || event.GetPropertyName() == "ModelRGBWHandling") {
        wxPGProperty* p2 = grid->GetPropertyByName("ModelStringType");
        int i = p2->GetValue().GetLong();
        if (NODE_TYPES[i] == "Single Color" || NODE_TYPES[i] == "Single Color Intensity" || NODE_TYPES[i] == "Node Single Color") {
            wxPGProperty* p = grid->GetPropertyByName("ModelStringColor");
            xlColor c;
            wxString tp = GetColorString(p, c);
            if (NODE_TYPES[i] == "Single Color Intensity") {
                tp = "Single Color Intensity";
            } else if (NODE_TYPES[i] == "Node Single Color" && p != nullptr) {
                tp = "Node Single Color";
                wxColor cc;
                cc << p->GetValue();
                c = wxColourToXlColor(cc);
            }
            if (p != nullptr)
                p->Enable();
            if (tp == "Single Color Custom" || tp == "Single Color Intensity" || tp == "Node Single Color") {
                xlColor xc = c;
                _model.SetCustomColor(xc);
            }
            _model.SetStringType(tp);
        } else {
            _model.SetStringType(NODE_TYPES[i]);
            _model.AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Model::OnPropertyGridChange::ModelStringType");
        }
        if (_model.GetNodeChannelCount(_model.GetStringType()) > 3) {
            p2 = grid->GetPropertyByName("ModelRGBWHandling");
            _model.SetRGBWHandling(RGBW_HANDLING[p2->GetValue().GetLong()]);
        }
        _model.IncrementChangeCount();
        _model.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML |
                    OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Model::OnPropertyGridChange::ModelStringType");
        if (event.GetPropertyName() == "ModelStringType") {
            _model.AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID |
                        OutputModelManager::WORK_RELOAD_MODELLIST |
                        OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                        OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS |
                        OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::ModelStringType");
        }
        return 0;
    } else if (event.GetPropertyName() == "ModelStartChannel" || event.GetPropertyName() == "ModelIndividualStartChannels.ModelStartChannel") {
        wxString val = event.GetValue().GetString();

        if ((val.StartsWith("@") || val.StartsWith("#") || val.StartsWith(">") || val.StartsWith("!")) && !val.Contains(":")) {
            val = val + ":1";
            event.GetProperty()->SetValue(val);
        }

        _model.SetStartChannel(val);
        _model.SetControllerName("");
        if (_model._hasIndivChans) {
            _model.SetIndividualStartChannel(0, val);
        }
        _model.IncrementChangeCount();
        _model.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::ModelStartChannel");
        return 0;
    } else if (event.GetPropertyName() == "ModelIndividualStartChannels") {
        int c = _model.GetNumStrings();
        _model._hasIndivChans = event.GetValue().GetBool();
        if (_model._hasIndivChans) {
            _model._indivStartChannels.resize(c);
            for (int x = 0; x < c; ++x) {
                _model.SetIndividualStartChannel(x, _model.ComputeStringStartChannel(x));
            }
        } else {
            _model._indivStartChannels.clear();
        }
        _model.IncrementChangeCount();
        _model.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                    OutputModelManager::WORK_RELOAD_PROPERTYGRID |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::ModelIndividualStartChannels");
        return 0;
    } else if (event.GetPropertyName().StartsWith("ModelIndividualStartChannels.")) {
        wxString str = event.GetPropertyName();
        str = str.SubString(str.Find(".") + 1, str.length());

        wxString val = event.GetValue().GetString();
        if ((val.StartsWith("@") || val.StartsWith("#") || val.StartsWith(">") || val.StartsWith("!")) && !val.Contains(":")) {
            val = val + ":1";
            event.GetProperty()->SetValue(val);
        }
        std::string text = str.ToStdString();
        int s = ExtractTrailingInt(text);
        if (s < 1) s = 1;
        if (s > _model.GetNumStrings()) s = _model.GetNumStrings();
        if (s >= 1 && (s - 1) < (int)_model._indivStartChannels.size()) {
            _model._indivStartChannels[s-1] = val;
        }
        _model.IncrementChangeCount();
        _model.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Model::OnPropertyGridChange::ModelIndividualStartChannels2");
        return 0;
    } else if (event.GetPropertyName() == "ModelLayoutGroup") {
        _model.SetLayoutGroup(LAYOUT_GROUPS[event.GetValue().GetLong()]);
        _model.IncrementChangeCount();
        _model.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE |
                    OutputModelManager::WORK_RELOAD_ALLMODELS |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Model::OnPropertyGridChange::ModelLayoutGroup");
        return 0;
    }

    int i = ScreenLocationPropertyHelper::OnPropertyGridChange(_model.GetModelScreenLocation(), grid, event);
    _model.IncrementChangeCount();
    _model.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "Model::OnPropertyGridChange::ModelLayoutGroup");

    return i;
}

void ModelPropertyAdapter::OnPropertyGridChanging(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
}

int ModelPropertyAdapter::OnPropertyGridSelection(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    return 0;
}

void ModelPropertyAdapter::OnPropertyGridItemCollapsed(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
}

void ModelPropertyAdapter::OnPropertyGridItemExpanded(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
}

void ModelPropertyAdapter::HandlePropertyGridRightClick(wxPropertyGridEvent& event, wxMenu& mnu) {
    wxString name = event.GetPropertyName();

    if (name.StartsWith("Layers.")) {
        _model.layerSizeMenu = wxAtoi(event.GetPropertyName().AfterLast('r'));
        if (_model.GetLayerSizeCount() != 1) {
            mnu.Append(Model::ID_LAYERSIZE_DELETE, "Delete");
        }
        mnu.Append(Model::ID_LAYERSIZE_INSERT, "Insert");
    }
}

void ModelPropertyAdapter::HandlePropertyGridContextMenu(wxCommandEvent& event) {
    if (event.GetId() == Model::ID_LAYERSIZE_DELETE) {
        _model.DeleteLayerSize(_model.layerSizeMenu);
        _model.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Model::HandlePropertyGridContextMenu::Delete");
        _model.IncrementChangeCount();
        _model.OnLayerSizesChange(true);
    } else if (event.GetId() == Model::ID_LAYERSIZE_INSERT) {
        _model.InsertLayerSizeBefore(_model.layerSizeMenu);
        _model.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Model::HandlePropertyGridContextMenu::Insert");
        _model.IncrementChangeCount();
        _model.OnLayerSizesChange(true);
    }
}
