/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "StatePanel.h"

#include <wx/choice.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>

#include "ui/shared/controls/BulkEditControls.h"
#include "ui/shared/utils/wxUtilities.h"
#include "effects/StateEffect.h"
#include "models/Model.h"
#include "models/ModelGroup.h"
#include "render/SequenceElements.h"

StatePanel::StatePanel(wxWindow* parent, const nlohmann::json& metadata)
    : JsonEffectPanel(parent, metadata, /*deferBuild*/ true) {
    BuildFromJson(metadata);

    // State_TimingTrack is a custom control (not a JSON property), so we must
    // manually bind the timing-track repopulate event that BuildFromJson would
    // otherwise wire when dynamicOptions:"timingTracks" is present.
    Bind(EVT_SETTIMINGTRACKS, [this](wxCommandEvent& event) {
        PopulateTimingTracks();
        event.Skip();
    });

    // When the state-definition choice changes, repopulate the state-list
    // choice with the states inside the newly selected definition.
    auto* defChoice = dynamic_cast<wxChoice*>(
        wxWindow::FindWindowByName("ID_CHOICE_State_StateDefinition", this));
    if (defChoice) {
        defChoice->Bind(wxEVT_CHOICE, [this](wxCommandEvent& e) {
            PopulateStates();
            e.Skip();
        });
    }

    ValidateWindow();
}

wxWindow* StatePanel::CreateCustomControl(wxWindow* parentWin, wxSizer* sizer,
                                           const nlohmann::json& prop, int /*cols*/) {
    std::string id = prop.value("id", "");
    if (id != "State_StateSource") return nullptr;

    // 2-column grid: radio button on the left, paired choice on the right.
    auto* grid = new wxFlexGridSizer(0, 2, 0, 0);
    grid->AddGrowableCol(1);

    // --- State row ---
    _radioState = new wxRadioButton(parentWin, wxNewId(), _("State"),
                                     wxDefaultPosition, wxDefaultSize,
                                     wxRB_GROUP, wxDefaultValidator,
                                     _T("IDD_RADIOBUTTON_State_State"));
    _radioState->SetValue(true);
    grid->Add(_radioState, 0, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

    _stateChoice = new BulkEditChoice(parentWin, wxNewId(),
                                       wxDefaultPosition, wxDefaultSize,
                                       0, nullptr, 0, wxDefaultValidator,
                                       _T("ID_CHOICE_State_State"));
    _stateChoice->Append("<ALL>");
    grid->Add(_stateChoice, 1, wxALL | wxEXPAND, 5);

    // --- Timing Track row ---
    _radioTiming = new wxRadioButton(parentWin, wxNewId(), _("Timing Track"),
                                      wxDefaultPosition, wxDefaultSize,
                                      0, wxDefaultValidator,
                                      _T("IDD_RADIOBUTTON_State_TimingTrack"));
    grid->Add(_radioTiming, 0, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

    _timingTrackChoice = new BulkEditChoice(parentWin, wxNewId(),
                                             wxDefaultPosition, wxDefaultSize,
                                             0, nullptr, 0, wxDefaultValidator,
                                             _T("ID_CHOICE_State_TimingTrack"));
    grid->Add(_timingTrackChoice, 1, wxALL | wxEXPAND, 5);

    sizer->Add(grid, 1, wxALL | wxEXPAND, 0);

    // Either radio toggles the enabled state of both paired choices.
    _radioState->Bind(wxEVT_RADIOBUTTON, [this](wxCommandEvent& e) {
        UpdateStateSourceEnable();
        e.Skip();
    });
    _radioTiming->Bind(wxEVT_RADIOBUTTON, [this](wxCommandEvent& e) {
        UpdateStateSourceEnable();
        e.Skip();
    });

    return _stateChoice;
}

void StatePanel::UpdateStateSourceEnable() {
    if (_radioState == nullptr || _radioTiming == nullptr) return;
    bool stateMode = _radioState->GetValue();
    if (_stateChoice) _stateChoice->Enable(stateMode);
    if (_timingTrackChoice) _timingTrackChoice->Enable(!stateMode);
}

void StatePanel::ValidateWindow() {
    JsonEffectPanel::ValidateWindow();
    UpdateStateSourceEnable();
}

void StatePanel::SetDefaultParameters() {
    JsonEffectPanel::SetDefaultParameters();
    if (_radioState) _radioState->SetValue(true);
    if (_radioTiming) _radioTiming->SetValue(false);
    if (_stateChoice && _stateChoice->GetCount() > 0) _stateChoice->SetSelection(0);
    if (_timingTrackChoice && _timingTrackChoice->GetCount() > 0) _timingTrackChoice->SetSelection(0);
    UpdateStateSourceEnable();
}

void StatePanel::SetRenderableEffect(RenderableEffect* eff) {
    JsonEffectPanel::SetRenderableEffect(eff);
    _effect = dynamic_cast<StateEffect*>(eff);
}

void StatePanel::SetPanelStatus(Model* cls) {
    // Base class populates timing tracks and model-driven dynamicOptions
    // (including State_StateDefinition via dynamicOptions: "states").
    JsonEffectPanel::SetPanelStatus(cls);

    _model = cls;
    PopulateTimingTracks();
    PopulateStates();
    UpdateStateSourceEnable();
}

void StatePanel::PopulateTimingTracks() {
    if (_timingTrackChoice == nullptr) return;
    wxString selection = _timingTrackChoice->GetStringSelection();
    _timingTrackChoice->Clear();
    if (mSequenceElements != nullptr) {
        for (size_t i = 0; i < mSequenceElements->GetElementCount(); i++) {
            Element* e = mSequenceElements->GetElement(i);
            if (e->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
                _timingTrackChoice->Append(e->GetName());
            }
        }
    }
    if (!selection.empty()) {
        _timingTrackChoice->SetStringSelection(selection);
    }
    if (_timingTrackChoice->GetSelection() == wxNOT_FOUND && _timingTrackChoice->GetCount() > 0) {
        _timingTrackChoice->SetSelection(0);
    }
}

void StatePanel::PopulateStates() {
    if (_stateChoice == nullptr) return;

    auto* defChoice = dynamic_cast<wxChoice*>(
        wxWindow::FindWindowByName("ID_CHOICE_State_StateDefinition", this));

    wxString selected = _stateChoice->GetStringSelection();
    _stateChoice->Clear();
    _stateChoice->Append("<ALL>");

    if (_effect != nullptr && _model != nullptr && defChoice != nullptr) {
        std::string defName = defChoice->GetStringSelection().ToStdString();
        if (!defName.empty()) {
            std::list<std::string> states = _effect->GetStates(_model, defName);
            for (const auto& s : states) {
                _stateChoice->Append(wxString(s));
            }
        }
    }

    // Restore the prior selection if it still exists; otherwise default to
    // <ALL> so users can see a sensible initial option.
    if (!selected.empty() && _stateChoice->SetStringSelection(selected)) {
        // kept
    } else {
        _stateChoice->SetSelection(0);
    }
}
