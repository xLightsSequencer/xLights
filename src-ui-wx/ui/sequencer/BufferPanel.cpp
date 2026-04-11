/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ui/sequencer/BufferPanel.h"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/config.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include <vector>

#include "models/Model.h"
#include "models/ModelGroup.h"
#include "render/PixelBuffer.h"
#include "ui/effectpanels/EffectPanelManager.h"
#include "ui/effectpanels/EffectPanelUtils.h"
#include "ui/sequencer/SubBufferPanel.h"
#include "xLightsMain.h"
#include "xLightsApp.h"

namespace {
nlohmann::json LoadBufferMetadata() {
    std::string metaDir = EffectPanelManager::GetMetadataDirectory();
    if (metaDir.empty()) return {};
    return JsonEffectPanel::LoadMetadata(metaDir + "/shared/Buffer.json");
}

// Roto-Zoom presets (in legacy display order).
const wxString ROTOZOOM_PRESETS[] = {
    "None - Reset",
    "1 Rev CW",
    "1 Rev CCW",
    "Explode",
    "Collapse",
    "Explode + Spin CW",
    "Shake",
    "Spin CW Accelerate"
};
} // namespace

BufferPanel::BufferPanel(wxWindow* parent, wxWindowID /*id*/,
                         const wxPoint& /*pos*/, const wxSize& /*size*/) :
    JsonEffectPanel(parent, LoadBufferMetadata(), /*deferBuild*/ true) {
    BuildFromJson(metadata_);

    SetName("Buffer");

    // Restore the 'Reset panel when changing effects' preference.
    wxConfigBase* config = wxConfigBase::Get();
    bool reset = true;
    if (config) config->Read("xLightsResetBufferPanel", &reset, true);
    if (_resetBufferPanelCheck) _resetBufferPanelCheck->SetValue(reset);

    // Bind events on the framework-built choices that drive visibility /
    // camera / stagger enable logic.
    if (auto* bufferStyle = GetPropertyInfo("BufferStyle")) {
        if (bufferStyle->choice) {
            bufferStyle->choice->Bind(wxEVT_CHOICE, &BufferPanel::OnBufferStyleChoiceSelect, this);
        }
    }
    if (auto* xform = GetPropertyInfo("BufferTransform")) {
        if (xform->choice) {
            xform->choice->Bind(wxEVT_CHOICE, &BufferPanel::OnBufferTransformSelect, this);
        }
    }

    ValidateWindow();
    SetMinSize(wxSize(50, 50));
}

wxWindow* BufferPanel::CreateCustomControl(wxWindow* parentWin, wxSizer* sizer,
                                            const nlohmann::json& prop, int /*cols*/) {
    std::string id = prop.value("id", "");
    if (id == "ResetPanelRow") return BuildResetPanelRow(parentWin, sizer);
    if (id == "SubBuffer") return BuildSubBufferRow(parentWin, sizer);
    if (id == "RotoZoomPreset") return BuildRotoZoomPresetRow(parentWin, sizer);
    return nullptr;
}

wxWindow* BufferPanel::BuildResetPanelRow(wxWindow* parentWin, wxSizer* sizer) {
    _resetBufferPanelCheck = new wxCheckBox(parentWin, wxNewId(),
                                             "Reset panel when changing effects",
                                             wxDefaultPosition, wxDefaultSize, 0,
                                             wxDefaultValidator,
                                             _T("IDD_CHECKBOX_ResetBufferPanel"));
    _resetBufferPanelCheck->SetValue(true);
    sizer->Add(_resetBufferPanelCheck, 0, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);
    _resetBufferPanelCheck->Bind(wxEVT_CHECKBOX, &BufferPanel::OnResetBufferPanelClick, this);
    return _resetBufferPanelCheck;
}

wxWindow* BufferPanel::BuildSubBufferRow(wxWindow* parentWin, wxSizer* sizer) {
    // The SubBufferPanel is a live-preview rectangle editor that already
    // exposes xlCustomControl semantics with hardcoded name "ID_CUSTOM_SubBuffer",
    // so the ApplySetting restore path finds it automatically.
    auto* panelParent = dynamic_cast<wxPanel*>(parentWin);
    if (panelParent == nullptr) panelParent = reinterpret_cast<wxPanel*>(parentWin);
    subBufferPanel = new SubBufferPanel(panelParent, /*usevc*/ true, wxID_ANY,
                                         wxDefaultPosition, wxDefaultSize, 0);
    // Match the legacy 30dlg-unit minimum so the natural content size stays
    // small — the surrounding growable-row logic stretches it to fill the
    // tab page when there's space, and it collapses to the minimum when the
    // user resizes the panel smaller without forcing a perpetual scrollbar.
    subBufferPanel->SetMinSize(wxDLG_UNIT(parentWin, wxSize(30, 30)));

    Connect(subBufferPanel->GetId(), SUBBUFFER_RANGE_CHANGED,
            (wxObjectEventFunction)&BufferPanel::HandleCommandChange);

    sizer->Add(subBufferPanel, 1, wxALL | wxEXPAND, 2);
    return subBufferPanel;
}

wxWindow* BufferPanel::BuildRotoZoomPresetRow(wxWindow* parentWin, wxSizer* sizer) {
    auto* row = new wxFlexGridSizer(0, 2, 0, 0);
    row->AddGrowableCol(1);

    auto* label = new wxStaticText(parentWin, wxID_ANY, "Preset");
    row->Add(label, 0, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);

    _rotoZoomPresetChoice = new BulkEditChoice(parentWin, wxNewId(),
                                                wxDefaultPosition, wxDefaultSize,
                                                0, nullptr, 0, wxDefaultValidator,
                                                _T("IDD_CHOICE_RotoZoomPreset"));
    for (const auto& p : ROTOZOOM_PRESETS) {
        _rotoZoomPresetChoice->Append(p);
    }
    _rotoZoomPresetChoice->SetStringSelection("None - Reset");
    row->Add(_rotoZoomPresetChoice, 1, wxALL | wxEXPAND, 2);
    _rotoZoomPresetChoice->Bind(wxEVT_CHOICE, &BufferPanel::OnPresetSelect, this);

    sizer->Add(row, 1, wxALL | wxEXPAND, 2);
    return _rotoZoomPresetChoice;
}

void BufferPanel::OnResetBufferPanelClick(wxCommandEvent& /*event*/) {
    wxConfigBase* config = wxConfigBase::Get();
    if (config && _resetBufferPanelCheck) {
        config->Write("xLightsResetBufferPanel", _resetBufferPanelCheck->IsChecked());
    }
}

void BufferPanel::OnBufferTransformSelect(wxCommandEvent& event) {
    ValidateWindow();
    // Must Skip so HandleCommandChange on the parent fires and the save
    // timer starts — otherwise the user's choice is silently discarded.
    event.Skip();
}

bool BufferPanel::CanRenderBufferUseCamera(const std::string& rb) {
    return rb == "Per Preview" || rb == "Per Model Per Preview";
}

void BufferPanel::OnBufferStyleChoiceSelect(wxCommandEvent& event) {
    auto* bsInfo = GetPropertyInfo("BufferStyle");
    auto* camInfo = GetPropertyInfo("PerPreviewCamera");
    if (!bsInfo || !camInfo || !bsInfo->choice || !camInfo->choice) {
        event.Skip();
        return;
    }

    std::string bs = bsInfo->choice->GetStringSelection().ToStdString();
    if (CanRenderBufferUseCamera(bs)) {
        wxString currentCamera = camInfo->choice->GetStringSelection();

        camInfo->choice->Clear();
        camInfo->choice->Append("2D");
        xLightsFrame* frame = xLightsApp::GetFrame();
        if (frame) {
            for (int i = 0; i < frame->viewpoint_mgr.GetNum3DCameras(); ++i) {
                camInfo->choice->Append(frame->viewpoint_mgr.GetCamera3D(i)->GetName());
            }
        }

        if (camInfo->choice->FindString(currentCamera) != wxNOT_FOUND && !currentCamera.empty()) {
            camInfo->choice->SetStringSelection(currentCamera);
        } else {
            camInfo->choice->SetStringSelection(_defaultCamera);
        }
    } else {
        camInfo->choice->SetStringSelection("2D");
    }

    ValidateWindow();
    // Must Skip so HandleCommandChange on the parent fires and the save
    // timer starts — otherwise the user's choice is silently discarded.
    event.Skip();
}

void BufferPanel::UpdateBufferStyles(const Model* model) {
    auto* info = GetPropertyInfo("BufferStyle");
    if (!info || !info->choice) return;

    wxString sel = info->choice->GetStringSelection();
    info->choice->Clear();
    if (model != nullptr) {
        for (const auto& it : model->GetBufferStyles()) {
            info->choice->Append(it);
        }
        _mg = model->GetDisplayAs() == DisplayAsType::ModelGroup;
        sel = model->AdjustBufferStyle(sel.ToStdString());
    } else {
        _mg = false;
    }
    if (info->choice->IsEmpty()) {
        info->choice->Append("Default");
        info->choice->Append("Per Preview");
        info->choice->Append("As Pixel");
    }
    info->choice->SetStringSelection(sel);
}

void BufferPanel::UpdateCamera(const Model* model) {
    auto* info = GetPropertyInfo("PerPreviewCamera");
    if (!info || !info->choice) return;

    _defaultCamera = "2D";
    info->choice->SetStringSelection("2D");
    if (model != nullptr) {
        auto mg = dynamic_cast<const ModelGroup*>(model);
        if (mg != nullptr) {
            info->choice->SetStringSelection(mg->GetDefaultCamera());
            _defaultCamera = mg->GetDefaultCamera();
        }
        _mg = model->GetDisplayAs() == DisplayAsType::ModelGroup;
    } else {
        _mg = false;
    }
}

void BufferPanel::SetDefaultControls(const Model* model, bool optionbased) {
    bool reset = !optionbased || (_resetBufferPanelCheck && _resetBufferPanelCheck->GetValue());
    if (!reset) {
        ValidateWindow();
        return;
    }

    // Framework defaults reset all standard rows (sliders, choices, spin,
    // checkboxes) to their JSON defaults.
    JsonEffectPanel::SetDefaultParameters();

    UpdateBufferStyles(model);
    UpdateCamera(model);

    if (subBufferPanel) subBufferPanel->SetDefaults();
    if (_rotoZoomPresetChoice) _rotoZoomPresetChoice->SetStringSelection("None - Reset");

    ValidateWindow();
}

void BufferPanel::ValidateWindow() {
    JsonEffectPanel::ValidateWindow();

    auto* bsInfo = GetPropertyInfo("BufferStyle");
    auto* camInfo = GetPropertyInfo("PerPreviewCamera");
    auto* staggerInfo = GetPropertyInfo("BufferStagger");
    if (!bsInfo || !bsInfo->choice) return;

    std::string bs = bsInfo->choice->GetStringSelection().ToStdString();

    if (camInfo && camInfo->choice) {
        bool cameraActive = (bs == "Per Preview" || bs == "Per Model Per Preview");
        camInfo->choice->Enable(cameraActive);
    }

    if (staggerInfo && staggerInfo->spinCtrl) {
        // Only some ModelGroup shapes support stagger.
        bool staggerOk = _mg && (bs == "Horizontal Stack" || bs == "Vertical Stack");
        staggerInfo->spinCtrl->Enable(staggerOk);
    }
}

wxString BufferPanel::GetBufferString() {
    // Framework's GetEffectString honours suppressIfDefault on every property.
    // Swap the E_ prefix for B_ to match the legacy Buffer panel output.
    wxString s = JsonEffectPanel::GetEffectString();
    s.Replace(",E_", ",B_");
    if (s.StartsWith("E_")) {
        s = "B_" + s.Mid(2);
    }

    // SubBufferPanel is a custom widget, not in properties_, so append
    // its value explicitly with the legacy B_CUSTOM_SubBuffer key.
    if (subBufferPanel) {
        wxString subB = subBufferPanel->GetValue();
        if (!subB.empty()) {
            if (!s.empty() && !s.EndsWith(",")) s += ",";
            s += "B_CUSTOM_SubBuffer=" + subB + ",";
        }
    }

    return s;
}

void BufferPanel::OnPresetSelect(wxCommandEvent& event) {
    // Must Skip so HandleCommandChange on the parent panel fires and the
    // save timer captures all the slider/VC mutations below.
    event.Skip();
    if (!_rotoZoomPresetChoice) return;
    wxString preset = _rotoZoomPresetChoice->GetStringSelection();

    // Helper lookups into the framework-built controls. Int sliders are
    // SLIDER-primary (info->slider + info->buddyText); float sliders with a
    // divisor are TEXTCTRL-primary (info->buddySlider + info->textCtrl).
    auto setIntValue = [this](const char* propId, int value) {
        auto* info = GetPropertyInfo(propId);
        if (info == nullptr) return;
        if (info->slider) info->slider->SetValue(value);
        if (info->buddyText) static_cast<wxTextCtrl*>(info->buddyText)->SetValue(wxString::Format("%d", value));
    };
    auto setFloatValue = [this](const char* propId, double value) {
        auto* info = GetPropertyInfo(propId);
        if (info == nullptr) return;
        int scaled = static_cast<int>(value * info->divisor);
        if (info->buddySlider) static_cast<wxSlider*>(info->buddySlider)->SetValue(scaled);
        if (info->textCtrl) info->textCtrl->SetValue(wxString::Format("%.1f", value));
    };
    auto vcSetRamp = [this](const char* propId, double p1, double p2, bool active = true) {
        auto* info = GetPropertyInfo(propId);
        if (info == nullptr || info->valueCurveBtn == nullptr) return;
        info->valueCurveBtn->GetValue()->SetType("Ramp");
        info->valueCurveBtn->GetValue()->SetParameter1(p1);
        info->valueCurveBtn->GetValue()->SetParameter2(p2);
        info->valueCurveBtn->SetActive(active);
    };
    auto vcClear = [this](const char* propId) {
        auto* info = GetPropertyInfo(propId);
        if (info == nullptr || info->valueCurveBtn == nullptr) return;
        info->valueCurveBtn->GetValue()->SetDefault();
        info->valueCurveBtn->SetActive(false);
    };

    // Reset everything Roto-Zoom related first, then apply preset overrides.
    setFloatValue("Zoom", 1.0);
    setIntValue("PivotPointX", 50);
    setIntValue("PivotPointY", 50);
    setIntValue("Rotation", 0);
    setFloatValue("Rotations", 0.0);
    setIntValue("ZoomQuality", 1);
    setIntValue("XPivot", 50);
    setIntValue("YPivot", 50);
    setIntValue("XRotation", 0);
    setIntValue("YRotation", 0);

    for (const char* p : { "Zoom", "PivotPointX", "PivotPointY", "Rotation", "Rotations",
                           "XPivot", "YPivot", "XRotation", "YRotation" }) {
        vcClear(p);
    }

    if (preset == "None - Reset") {
        // nothing more to do
    } else if (preset == "1 Rev CW") {
        setFloatValue("Rotations", 1.0);
        vcSetRamp("Rotation", 0, 100);
        setIntValue("ZoomQuality", 2);
    } else if (preset == "1 Rev CCW") {
        setFloatValue("Rotations", 1.0);
        vcSetRamp("Rotation", 100, 0);
        setIntValue("ZoomQuality", 2);
    } else if (preset == "Explode") {
        vcSetRamp("Zoom", 0, 10);
    } else if (preset == "Collapse") {
        vcSetRamp("Zoom", 10, 0);
    } else if (preset == "Explode + Spin CW") {
        vcSetRamp("Zoom", 0, 10);
        setFloatValue("Rotations", 1.0);
        vcSetRamp("Rotation", 0, 100);
        setIntValue("ZoomQuality", 2);
    } else if (preset == "Shake") {
        setFloatValue("Rotations", 1.0);
        if (auto* info = GetPropertyInfo("Rotation"); info && info->valueCurveBtn) {
            auto* vc = info->valueCurveBtn->GetValue();
            vc->SetType("Sine");
            vc->SetParameter1(0);
            vc->SetParameter2(10);
            vc->SetParameter3(50);
            vc->SetParameter4(25);
            vc->SetWrap(true);
            info->valueCurveBtn->SetActive(true);
        }
        setIntValue("ZoomQuality", 2);
    } else if (preset == "Spin CW Accelerate") {
        vcSetRamp("Rotation", 0, 100);
        vcSetRamp("Rotations", 0, 10);
        setIntValue("ZoomQuality", 2);
    }

    _rotoZoomPresetChoice->SetStringSelection("None - Reset");
    if (auto* rotOrder = GetPropertyInfo("RZ_RotationOrder"); rotOrder && rotOrder->choice) {
        rotOrder->choice->SetSelection(0);
    }
    ValidateWindow();
}
