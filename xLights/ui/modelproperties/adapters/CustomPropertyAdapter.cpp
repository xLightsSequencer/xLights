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
#include <wx/stopwatch.h>

#include "CustomPropertyAdapter.h"
#include "../../../models/CustomModel.h"
#include "../../../models/ModelManager.h"
#include "../../../OutputModelManager.h"
#include "../../../CustomModelDialog.h"
#include "../../../ExternalHooks.h"
#include "../../../xLightsMain.h"

#include <log.h>

static const std::string CLICK_TO_EDIT("--Click To Edit--");

class CustomModelDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    CustomModelDialogAdapter(CustomModel* model, OutputManager* om) :
        wxPGEditorDialogAdapter(), m_model(model), _outputManager(om)
    {
    }
    virtual bool DoShowDialog(wxPropertyGrid* propGrid,
        wxPGProperty* WXUNUSED(property)) override
    {
        m_model->SaveDisplayDimensions();
        auto oldAutoSave = m_model->GetModelManager().GetXLightsFrame()->_suspendAutoSave;
        m_model->GetModelManager().GetXLightsFrame()->_suspendAutoSave = true;
        CustomModelDialog dlg(propGrid, _outputManager);
        dlg.Setup(m_model);
        bool res = false;
        if (dlg.ShowModal() == wxID_OK) {
            dlg.Save(m_model);
            m_model->RestoreDisplayDimensions();
            wxVariant v(CLICK_TO_EDIT);
            SetValue(v);
            res = true;
        } else {
            m_model->RestoreDisplayDimensions();
            wxVariant v(CLICK_TO_EDIT);
            SetValue(v);
            m_model->GetModelManager().GetXLightsFrame()->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "CustomModel::CancelCustomData");
        }
        m_model->GetModelManager().GetXLightsFrame()->_suspendAutoSave = oldAutoSave;
        return res;
    }
protected:
    CustomModel* m_model = nullptr;
    OutputManager* _outputManager = nullptr;
};

class CustomModelProperty : public wxStringProperty
{
public:
    CustomModelProperty(CustomModel* m,
        OutputManager* om,
        const wxString& label,
        const wxString& name,
        const wxString& value) :
        wxStringProperty(label, name, value), m_model(m), _outputManager(om)
    {
    }
    virtual const wxPGEditor* DoGetEditorClass() const override
    {
        return wxPGEditor_TextCtrlAndButton;
    }
    virtual wxPGEditorDialogAdapter* GetEditorDialog() const override
    {
        return new CustomModelDialogAdapter(m_model, _outputManager);
    }
protected:
    CustomModel* m_model = nullptr;
    OutputManager* _outputManager = nullptr;
};

CustomPropertyAdapter::CustomPropertyAdapter(Model& model)
    : ModelPropertyAdapter(model), _custom(static_cast<CustomModel&>(model)) {}

void CustomPropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    

    wxPGProperty* p = grid->Append(new CustomModelProperty(&_custom, outputManager, "Model Data", "CustomData", CLICK_TO_EDIT));
    grid->LimitPropertyEditing(p);

    p = grid->Append(new wxUIntProperty("# Strings", "CustomModelStrings", _custom.GetNumStrings()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("This is typically the number of connections from the prop to your controller.");

    if (_custom.GetNumStrings() == 1) {
        // cant set start node
    } else {
        std::string nm = _custom.StartNodeAttrName(0);

        p = grid->Append(new wxStringProperty("Start Nodes", "ModelIndividualStartNodes", ""));
        wxPGProperty* psn = grid->AppendIn(p, new wxUIntProperty(nm, nm, _custom.HasIndivStartNodes() ? _custom.GetIndivStartNode(0) : 1));
        psn->SetAttribute("Min", 1);
        psn->SetAttribute("Max", (int)_custom.GetNodeCount());
        psn->SetEditor("SpinCtrl");

        if (_custom.HasIndivStartNodes()) {
            int c = _custom.GetNumStrings();
            for (int x = 0; x < c; ++x) {
                nm = _custom.StartNodeAttrName(x);
                int v = _custom.GetIndivStartNode(x);
                if (v < 1) v = _custom.ComputeStringStartNode(x);
                if (v > _custom.NodesPerString()) v = _custom.NodesPerString();
                if (x == 0) {
                    psn->SetValue(v);
                } else {
                    grid->AppendIn(p, new wxUIntProperty(nm, nm, v));
                }
            }
        } else {
            psn->Enable(false);
        }
    }

    wxStopWatch sw;
    p = grid->Append(new wxImageFileProperty("Background Image", "CustomBkgImage", _custom.GetCustomBackground()));

    if (sw.Time() > 500)
        spdlog::debug("        Adding background image property ({}) to model {} really slow: {}ms", (const char*)_custom.GetCustomBackground().c_str(), (const char*)_custom.GetName().c_str(), sw.Time());

    p->SetAttribute(wxPG_FILE_WILDCARD, "Image files|*.png;*.bmp;*.jpg;*.gif;*.jpeg"
                                        ";*.webp"
                                        "|All files (*.*)|*.*");
}

int CustomPropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if ("CustomData" == event.GetPropertyName()) {
        if (grid->GetPropertyByName("CustomBkgImage")->GetValue() != _custom.GetCustomBackground()) {
            grid->GetPropertyByName("CustomBkgImage")->SetValue(wxVariant(_custom.GetCustomBackground()));
        }
        _custom.IncrementChangeCount();
        _custom.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "CustomModel::OnPropertyGridChange::CustomData");
        return 0;
    } else if ("CustomBkgImage" == event.GetPropertyName()) {
        _custom.SetCustomBackground(event.GetValue().GetString().ToStdString());
        ObtainAccessToURL(_custom.GetCustomBackground());
        _custom.IncrementChangeCount();
        _custom.AddASAPWork(OutputModelManager::WORK_VISUAL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CustomModel::OnPropertyGridChange::CustomBkgImage");
        return 0;
    } else if ("CustomModelStrings" == event.GetPropertyName()) {
        int old_string_count = _custom.GetNumStrings();
        int new_string_count = event.GetValue().GetInteger();
        _custom.SetNumStrings(new_string_count);
        _custom.SetHasIndivStartNodes(new_string_count > 1);
        if (new_string_count != old_string_count && _custom.HasIndivStartNodes()) {
            _custom.SetIndivStartNodesCount(new_string_count);
            for (int x = 0; x < new_string_count; x++) {
                _custom.SetIndivStartNode(x, _custom.ComputeStringStartNode(x));
            }
        }
        if (new_string_count != old_string_count && _custom.HasIndividualStartChannels()) {
            _custom.SetIndivStartChannelCount(new_string_count);
            for (int x = 0; x < new_string_count; x++) {
                _custom.SetIndividualStartChannel(x, _custom.ComputeStringStartChannel(x));
            }
        }
        _custom.IncrementChangeCount();
        _custom.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_RELOAD_PROPERTYGRID, "CustomModel::OnPropertyGridChange::CustomModelStrings");
        return 0;
    } else if (event.GetPropertyName().StartsWith("ModelIndividualStartNodes.NodeStart")) {
        wxString s = event.GetPropertyName().substr(strlen("ModelIndividualStartNodes.NodeStart"));
        int string = wxAtoi(s) - 1;
        int value = event.GetValue().GetInteger();
        if (value < 1) value = 1;
        if (value > _custom.NodesPerString()) value = _custom.NodesPerString();
        _custom.SetIndivStartNode(string, value);
        _custom.IncrementChangeCount();
        _custom.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS, "CustomModel::OnPropertyGridChange::ModelIndividualStartNodes2");
        return 0;
    } else if (event.GetPropertyName() == "ModelIndividualStartChannels") {
        bool hasIndivChans = event.GetValue().GetBool();
        _custom.SetHasIndividualStartChannels(hasIndivChans);
        if (hasIndivChans && _custom.HasIndivStartNodes() && _custom.GetIndivStartNodesCount() > 0) {
            int c = _custom.GetNumStrings();
            _custom.SetIndivStartChannelCount(c);
            int32_t modelStartChannel = _custom.GetNumberFromChannelString(_custom.GetModelStartChannel());
            int chanPerNode = _custom.GetNodeChannelCount(_custom.GetStringType());
            for (int x = 0; x < c; ++x) {
                if (x == 0) {
                    _custom.SetIndividualStartChannel(x, _custom.GetModelStartChannel());
                } else {
                    int node = _custom.GetIndivStartNode(x);
                    int32_t startChannel = modelStartChannel + (node - 1) * chanPerNode;
                    _custom.SetIndividualStartChannel(x, std::to_string(startChannel));
                }
            }
        } else if (hasIndivChans) {
            int c = _custom.GetNumStrings();
            _custom.SetIndivStartChannelCount(c);
            for (int x = 0; x < c; ++x) {
                _custom.SetIndividualStartChannel(x, _custom.ComputeStringStartChannel(x));
            }
        } else {
            _custom.SetIndivStartChannelCount(0);
        }
        _custom.IncrementChangeCount();
        _custom.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_RELOAD_PROPERTYGRID |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS, "CustomModel::OnPropertyGridChange::ModelIndividualStartChannels");
        return 0;
    }
    return ModelPropertyAdapter::OnPropertyGridChange(grid, event);
}
