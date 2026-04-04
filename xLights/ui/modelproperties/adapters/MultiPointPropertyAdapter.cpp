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

#include "MultiPointPropertyAdapter.h"
#include "../../../models/MultiPointModel.h"
#include "../../../models/OutputModelManager.h"

MultiPointPropertyAdapter::MultiPointPropertyAdapter(Model& model)
    : ModelPropertyAdapter(model), _multiPoint(static_cast<MultiPointModel&>(model)) {}

void MultiPointPropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    wxPGProperty* p;
    if (_multiPoint.IsSingleNode()) {
        p = grid->Append(new wxUIntProperty("# Lights", "MultiPointNodes", _multiPoint.GetNumPoints()));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");
        p->Enable(false);
    } else {
        p = grid->Append(new wxUIntProperty("# Nodes", "MultiPointNodes", _multiPoint.GetNumPoints()));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");
        p->Enable(false);
    }

    p = grid->Append(new wxUIntProperty("Strings", "MultiPointStrings", _multiPoint.GetNumStrings()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 48);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("This is typically the number of connections from the prop to your controller.");

    if (_multiPoint.GetNumStrings() == 1) {
        // cant set start node
    } else {
        std::string nm = _multiPoint.StartNodeAttrName(0);

        p = grid->Append(new wxBoolProperty("Indiv Start Nodes", "ModelIndividualStartNodes", _multiPoint.HasIndivStartNodes()));
        p->SetAttribute("UseCheckbox", true);

        wxPGProperty* psn = grid->AppendIn(p, new wxUIntProperty(nm, nm, _multiPoint.HasIndivStartNodes() ? _multiPoint.GetIndivStartNode(0) : 1));
        psn->SetAttribute("Min", 1);
        psn->SetAttribute("Max", (int)_multiPoint.GetNodeCount());
        psn->SetEditor("SpinCtrl");

        if (_multiPoint.HasIndivStartNodes()) {
            int c = _multiPoint.GetNumStrings();
            for (int x = 0; x < c; x++) {
                nm = _multiPoint.StartNodeAttrName(x);
                int v = _multiPoint.GetIndivStartNode(x);
                if (v < 1) v = 1;
                if (v > _multiPoint.NodesPerString()) v = _multiPoint.NodesPerString();
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

    p = grid->Append(new wxFloatProperty("Height", "ModelHeight", _multiPoint.GetModelHeight()));
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");
}

int MultiPointPropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if ("MultiPointNodes" == event.GetPropertyName()) {
        _multiPoint.SetNumPoints((int)event.GetPropertyValue().GetLong());
        wxPGProperty* sp = grid->GetPropertyByLabel("# Nodes");
        if (sp == nullptr) {
            sp = grid->GetPropertyByLabel("# Lights");
        }
        sp->SetValueFromInt((int)event.GetPropertyValue().GetLong());
        _multiPoint.IncrementChangeCount();
        _multiPoint.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS |
                    OutputModelManager::WORK_RELOAD_PROPERTYGRID, "MultiPointModel::OnPropertyGridChange::MultiPointNodes");
        return 0;
    } else if ("MultiPointStrings" == event.GetPropertyName()) {
        int old_string_count = _multiPoint.GetNumStrings();
        int new_string_count = event.GetValue().GetInteger();
        _multiPoint.SetNumStrings(new_string_count);
        if (old_string_count != new_string_count) {
            if (_multiPoint.HasIndivStartNodes()) {
                for (int x = 0; x < new_string_count; x++) {
                    _multiPoint.SetIndivStartNode(x, _multiPoint.ComputeStringStartNode(x));
                }
            }
        }
        _multiPoint.IncrementChangeCount();
        _multiPoint.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_RELOAD_PROPERTYGRID, "MultiPointModel::OnPropertyGridChange::MultiPointStrings");
        return 0;
    } else if (!_multiPoint.GetModelScreenLocation().IsLocked() && !_multiPoint.IsFromBase() && "ModelHeight" == event.GetPropertyName()) {
        float height = event.GetValue().GetDouble();
        if (std::abs(height) < 0.01f) {
            height = height < 0.0f ? -0.01f : 0.01f;
        }
        _multiPoint.SetModelHeight(height);
        _multiPoint.IncrementChangeCount();
        _multiPoint.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "MultiPointModel::OnPropertyGridChange::ModelHeight");
        return 0;
    } else if ((_multiPoint.GetModelScreenLocation().IsLocked() || _multiPoint.IsFromBase()) && "ModelHeight" == event.GetPropertyName()) {
        event.Veto();
        return 0;
    }
    return ModelPropertyAdapter::OnPropertyGridChange(grid, event);
}

int MultiPointPropertyAdapter::OnPropertyGridSelection(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if (event.GetPropertyName().StartsWith("ModelIndividualSegments.")) {
        wxString str = event.GetPropertyName();
        str = str.SubString(str.Find(".") + 1, str.length());
        str = str.SubString(3, str.length());
        int segment = wxAtoi(str) - 1;
        return segment;
    }
    return -1;
}
