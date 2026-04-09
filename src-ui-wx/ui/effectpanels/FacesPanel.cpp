/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "FacesPanel.h"

#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "ui/shared/controls/BulkEditControls.h"
#include "models/Model.h"
#include "models/ModelGroup.h"
#include "models/SubModel.h"
#include "render/SequenceElements.h"

FacesPanel::FacesPanel(wxWindow* parent, const nlohmann::json& metadata)
    : JsonEffectPanel(parent, metadata, /*deferBuild*/ true) {
    BuildFromJson(metadata);
    ValidateWindow();
}

wxWindow* FacesPanel::CreateCustomControl(wxWindow* parentWin, wxSizer* sizer,
                                           const nlohmann::json& prop, int cols) {
    std::string id = prop.value("id", "");

    if (id == "Faces_MouthMovements") {
        return BuildMouthMovements(parentWin, sizer);
    }
    if (id == "Faces_TransparentBlackRow") {
        return BuildTransparentBlackRow(parentWin, sizer);
    }
    return nullptr;
}

wxWindow* FacesPanel::BuildMouthMovements(wxWindow* parentWin, wxSizer* sizer) {
    // 2-column grid: radio button on the left, paired choice on the right.
    // Mirrors the legacy layout of the Mouth Movements static box.
    auto* grid = new wxFlexGridSizer(0, 2, 0, 0);
    grid->AddGrowableCol(1);

    // --- Phoneme row ---
    _radioPhoneme = new wxRadioButton(parentWin, wxNewId(), _("Phoneme"),
                                       wxDefaultPosition, wxDefaultSize,
                                       wxRB_GROUP, wxDefaultValidator,
                                       _T("IDD_RADIOBUTTON_Faces_Phoneme"));
    _radioPhoneme->SetValue(true);
    grid->Add(_radioPhoneme, 0, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

    _phonemeChoice = new BulkEditChoice(parentWin, wxNewId(),
                                         wxDefaultPosition, wxDefaultSize,
                                         0, nullptr, 0, wxDefaultValidator,
                                         _T("ID_CHOICE_Faces_Phoneme"));
    _phonemeChoice->Append("AI");
    _phonemeChoice->Append("E");
    _phonemeChoice->Append("FV");
    _phonemeChoice->Append("L");
    _phonemeChoice->Append("MBP");
    _phonemeChoice->Append("O");
    _phonemeChoice->Append("U");
    _phonemeChoice->Append("WQ");
    _phonemeChoice->Append("etc");
    _phonemeChoice->Append("rest");
    _phonemeChoice->Append("(off)");
    _phonemeChoice->SetSelection(0);
    grid->Add(_phonemeChoice, 1, wxALL | wxEXPAND, 5);

    // --- Timing Track row ---
    _radioTiming = new wxRadioButton(parentWin, wxNewId(), _("Timing Track"),
                                      wxDefaultPosition, wxDefaultSize,
                                      0, wxDefaultValidator,
                                      _T("IDD_RADIOBUTTON_Faces_TimingTrack"));
    grid->Add(_radioTiming, 0, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

    _timingTrackChoice = new BulkEditChoice(parentWin, wxNewId(),
                                             wxDefaultPosition, wxDefaultSize,
                                             0, nullptr, 0, wxDefaultValidator,
                                             _T("ID_CHOICE_Faces_TimingTrack"));
    grid->Add(_timingTrackChoice, 1, wxALL | wxEXPAND, 5);

    sizer->Add(grid, 1, wxALL | wxEXPAND, 0);

    // Either radio toggles the enabled state of both paired choices.
    _radioPhoneme->Bind(wxEVT_RADIOBUTTON, [this](wxCommandEvent& e) {
        UpdateMouthMovementEnable();
        e.Skip();
    });
    _radioTiming->Bind(wxEVT_RADIOBUTTON, [this](wxCommandEvent& e) {
        UpdateMouthMovementEnable();
        e.Skip();
    });

    return _phonemeChoice;
}

wxWindow* FacesPanel::BuildTransparentBlackRow(wxWindow* parentWin, wxSizer* sizer) {
    // Single 1-col slot in the parent — build a horizontal 3-col sub-grid
    // holding the checkbox, the brightness-threshold slider, and the text
    // buddy so they read as a single visually-linked row (matches legacy).
    auto* row = new wxFlexGridSizer(0, 3, 0, 0);
    row->AddGrowableCol(1);

    _transparentBlackCheck = new BulkEditCheckBox(parentWin, wxNewId(),
                                                   _("Transparent Black"),
                                                   wxDefaultPosition, wxDefaultSize,
                                                   0, wxDefaultValidator,
                                                   _T("ID_CHECKBOX_Faces_TransparentBlack"));
    _transparentBlackCheck->SetValue(false);
    row->Add(_transparentBlackCheck, 0, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

    auto* slider = new BulkEditSlider(parentWin, wxNewId(), 0, 0, 300,
                                       wxDefaultPosition, wxDefaultSize,
                                       0, wxDefaultValidator,
                                       _T("IDD_SLIDER_Faces_TransparentBlack"));
    row->Add(slider, 1, wxALL | wxEXPAND, 5);

    auto* textCtrl = new BulkEditTextCtrl(parentWin, wxNewId(), _T("0"),
                                           wxDefaultPosition,
                                           wxDLG_UNIT(parentWin, wxSize(20, -1)),
                                           wxTE_RIGHT, wxDefaultValidator,
                                           _T("ID_TEXTCTRL_Faces_TransparentBlack"));
    textCtrl->SetMaxLength(5);
    row->Add(textCtrl, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    sizer->Add(row, 1, wxALL | wxEXPAND, 0);
    return _transparentBlackCheck;
}

void FacesPanel::UpdateMouthMovementEnable() {
    if (_radioPhoneme == nullptr || _radioTiming == nullptr) return;
    bool phonemeMode = _radioPhoneme->GetValue();
    if (_phonemeChoice) _phonemeChoice->Enable(phonemeMode);
    if (_timingTrackChoice) _timingTrackChoice->Enable(!phonemeMode);
}

void FacesPanel::ValidateWindow() {
    // First run the JSON-driven visibility rules (Outline → UseState etc.).
    JsonEffectPanel::ValidateWindow();
    // Then re-apply the mouth-movement enable state so it survives the base
    // class running through visibility rules on the surrounding controls.
    UpdateMouthMovementEnable();
}

void FacesPanel::SetDefaultParameters() {
    JsonEffectPanel::SetDefaultParameters();
    if (_radioPhoneme) _radioPhoneme->SetValue(true);
    if (_radioTiming) _radioTiming->SetValue(false);
    if (_phonemeChoice) _phonemeChoice->SetStringSelection("AI");
    if (_timingTrackChoice) {
        if (_timingTrackChoice->GetCount() > 0) _timingTrackChoice->SetSelection(0);
    }
    if (_transparentBlackCheck) _transparentBlackCheck->SetValue(false);
    UpdateMouthMovementEnable();
}

void FacesPanel::PopulateLyricTimingTracks() {
    if (_timingTrackChoice == nullptr) return;

    // Only lyric timing tracks are usable for Faces. The legacy panel filtered
    // with GetEffectLayerCount() == 3 (phrase / word / phoneme layers), which
    // is narrower than the default "timingTracks" source.
    wxString selection = _timingTrackChoice->GetStringSelection();
    _timingTrackChoice->Clear();
    if (mSequenceElements != nullptr) {
        for (size_t i = 0; i < mSequenceElements->GetElementCount(); i++) {
            Element* e = mSequenceElements->GetElement(i);
            if (e->GetType() == ElementType::ELEMENT_TYPE_TIMING && e->GetEffectLayerCount() == 3) {
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

void FacesPanel::SetPanelStatus(Model* cls) {
    // Base populates face-definition (dynamicOptions:"faces") and state choices
    // (dynamicOptions:"states"), plus non-lyric timing tracks for anything with
    // dynamicOptions:"timingTracks" — Faces has none of those.
    JsonEffectPanel::SetPanelStatus(cls);

    // Append the "Default" (and optional "Rendered") special entries to the
    // face-definition choice, matching legacy panel behavior.
    auto* faceChoice = dynamic_cast<wxChoice*>(
        wxWindow::FindWindowByName("ID_CHOICE_Faces_FaceDefinition", this));
    if (faceChoice != nullptr) {
        wxString selection = faceChoice->GetStringSelection();

        // Submodel falls back to its parent model (matches old behavior).
        Model* m = cls;
        if (cls != nullptr) {
            if (cls->GetDisplayAs() == DisplayAsType::ModelGroup) {
                m = static_cast<ModelGroup*>(cls)->GetFirstModel();
            } else if (cls->GetDisplayAs() == DisplayAsType::SubModel) {
                m = static_cast<SubModel*>(cls)->GetParent();
            }
        }

        bool addRender = true;
        if (m != nullptr) {
            for (const auto& it : m->GetFaceInfo()) {
                auto typeIt = it.second.find("Type");
                const std::string& type = typeIt != it.second.end() ? typeIt->second : std::string();
                if (type == "Coro" || type == "SingleNode" || type == "NodeRange") {
                    addRender = false;
                    break;
                }
            }
        }

        bool hasDefault = false;
        bool hasRendered = false;
        for (unsigned int i = 0; i < faceChoice->GetCount(); i++) {
            wxString s = faceChoice->GetString(i);
            if (s == "Default") hasDefault = true;
            if (s == "Rendered") hasRendered = true;
        }
        if (!hasDefault) faceChoice->Append("Default");
        if (addRender && !hasRendered) faceChoice->Append("Rendered");

        if (!selection.empty()) {
            faceChoice->SetStringSelection(selection);
        }
        if (faceChoice->GetSelection() == wxNOT_FOUND && faceChoice->GetCount() > 0) {
            faceChoice->SetSelection(0);
        }
    }

    // Lyric timing tracks for the Mouth Movements timing-track choice.
    PopulateLyricTimingTracks();
    UpdateMouthMovementEnable();
}
