/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "BulkEditModelPropertiesDialog.h"

#include <algorithm>

#include <wx/propgrid/propgrid.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include "ModelPropertyAdapter.h"
#include "ModelPropertyManager.h"
#include "models/BoxedScreenLocation.h"
#include "models/DisplayAsType.h"
#include "models/Model.h"
#include "models/OutputModelManager.h"
#include "shared/utils/xlPropertyGrid.h"
#include "xLightsMain.h"

BulkEditModelPropertiesDialog::BulkEditModelPropertiesDialog(wxWindow* parent, xLightsFrame* xlights, const std::vector<Model*>& models) :
    wxDialog(parent, wxID_ANY, "Edit Properties", wxDefaultPosition, wxSize(420, 480), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    _xlights(xlights),
    _models(models),
    _primary(models.empty() ? nullptr : models.front())
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    if (_primary != nullptr) {
        SetTitle(wxString::Format("Edit Properties - %d %s Models", (int)_models.size(), wxString(DisplayAsTypeToString(_primary->GetDisplayAs()))));
        sizer->Add(new wxStaticText(this, wxID_ANY,
                       wxString::Format("Changing a value below applies it to all %d selected models.", (int)_models.size())),
                   0, wxEXPAND | wxALL, 8);
    }

    _grid = new xlPropertyGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxPG_SPLITTER_AUTO_CENTER | wxPG_DEFAULT_STYLE);
    sizer->Add(_grid, 1, wxEXPAND | wxLEFT | wxRIGHT, 8);

    if (_primary != nullptr) {
        _adapter = ModelPropertyManager::CreateAdapter(*_primary);
        _adapter->AddTypeProperties(_grid, xlights->GetOutputManager());
    }

    _grid->Bind(wxEVT_PG_CHANGING, &BulkEditModelPropertiesDialog::OnPropertyGridChanging, this);
    _grid->Bind(wxEVT_PG_CHANGED, &BulkEditModelPropertiesDialog::OnPropertyGridChanged, this);

    sizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL, 8);

    SetSizer(sizer);
    Layout();
}

void BulkEditModelPropertiesDialog::OnPropertyGridChanging(wxPropertyGridEvent& event)
{
    if (_adapter) {
        _adapter->OnPropertyGridChanging(_grid, event);
    }
}

void BulkEditModelPropertiesDialog::OnPropertyGridChanged(wxPropertyGridEvent& event)
{
    if (_adapter == nullptr || _primary == nullptr) {
        return;
    }

    // Edit the primary model live, exactly as the main Layout property grid
    // does for a single-model edit (same adapter call, same
    // save/restore-dimensions bracketing around GRIDCHANGE_SUPPRESS_HOLDSIZE).
    // The rest of the selection gets this same change replayed in
    // ShowAndApply() once the dialog is accepted.
    _primary->SaveDisplayDimensions();
    int i = _adapter->OnPropertyGridChange(_grid, event);
    if ((i & GRIDCHANGE_SUPPRESS_HOLDSIZE) == 0 &&
        (dynamic_cast<ModelWithScreenLocation<BoxedScreenLocation>*>(_primary) != nullptr ||
            dynamic_cast<ModelWithScreenLocation<ThreePointScreenLocation>*>(_primary) != nullptr)) {
        _xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML,
            "BulkEditModelPropertiesDialog::OnPropertyGridChanged", _primary);
        _primary->RestoreDisplayDimensions();
    }
    _adapter->UpdateTypeProperties(_grid);

    const wxString name = event.GetPropertyName();
    if (std::find(_changedProperties.begin(), _changedProperties.end(), name) == _changedProperties.end()) {
        _changedProperties.push_back(name);
    }
}

bool BulkEditModelPropertiesDialog::ShowAndApply()
{
    if (_primary == nullptr || _models.size() < 2) {
        return false;
    }

    if (ShowModal() != wxID_OK || _changedProperties.empty()) {
        return false;
    }

    for (Model* m : _models) {
        if (m == _primary) {
            continue;
        }

        // Flush before moving to the next model: AddASAPWork's single-model
        // "mailbox" slots (OutputModelManager::_modelToModelFromXml /
        // _selectedModel) assert if pointed at a second model while still
        // holding an unflushed first one. Queuing repeatedly for the SAME
        // model (the inner loop below) is fine; it's only the handoff
        // between models that needs a flush first.
        _xlights->DoASAPWork();

        std::unique_ptr<ModelPropertyAdapter> adapter = ModelPropertyManager::CreateAdapter(*m);
        for (const wxString& propName : _changedProperties) {
            wxPGProperty* prop = _grid->GetPropertyByName(propName);
            if (prop == nullptr) {
                continue;
            }

            wxPropertyGridEvent event;
            event.SetPropertyGrid(_grid);
            event.SetProperty(prop);
            event.SetPropertyValue(prop->GetValue());

            m->SaveDisplayDimensions();
            int i = adapter->OnPropertyGridChange(_grid, event);
            if ((i & GRIDCHANGE_SUPPRESS_HOLDSIZE) == 0 &&
                (dynamic_cast<ModelWithScreenLocation<BoxedScreenLocation>*>(m) != nullptr ||
                    dynamic_cast<ModelWithScreenLocation<ThreePointScreenLocation>*>(m) != nullptr)) {
                _xlights->GetOutputModelManager()->AddImmediateWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML,
                    "BulkEditModelPropertiesDialog::ShowAndApply", m);
                m->RestoreDisplayDimensions();
            }
        }
    }

    _xlights->DoASAPWork();
    return true;
}
