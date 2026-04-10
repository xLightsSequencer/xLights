/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "MorphPanel.h"

#include <wx/button.h>
#include <wx/choice.h>
#include <wx/choicdlg.h>
#include <wx/event.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "ui/shared/controls/BulkEditControls.h"
#include "assist/AssistPanel.h"
#include "assist/xlGridCanvasMorph.h"
#include "effects/MorphEffect.h"
#include "../../xLightsApp.h"
#include "../../xLightsMain.h"
#include "../../ui/sequencer/MainSequencer.h"
#include "ui/shared/utils/wxUtilities.h"
#include "UtilFunctions.h"

namespace {
// Quick Set preset table shared between the inline panel handler and the
// MorphQuickSet bulk-edit popup. Value order matches:
// Start_X1, Start_X2, End_X1, End_X2, Start_Y1, Start_Y2, End_Y1, End_Y2.
struct MorphPreset { const char* name; const char* values[8]; };
static const MorphPreset MORPH_PRESETS[] = {
    {"Full Sweep Up",     {"0","100","0","100","0","0","100","100"}},
    {"Full Sweep Down",   {"0","100","0","100","100","100","0","0"}},
    {"Full Sweep Left",   {"100","100","0","0","0","100","0","100"}},
    {"Full Sweep Right",  {"0","0","100","100","0","100","0","100"}},
    {"Single Sweep Up",   {"0","0","0","0","0","0","100","100"}},
    {"Single Sweep Down", {"0","0","0","0","100","100","0","0"}},
    {"Single Sweep Left", {"100","100","0","0","0","0","0","0"}},
    {"Single Sweep Right",{"0","0","100","100","0","0","0","0"}},
};

// MorphQuickSet is a BulkEditChoice subclass that provides the bulk-edit popup
// for the Quick Set preset list. On bulk-edit it applies the preset across
// all selected Morph effects in addition to the local panel.
class MorphQuickSet : public BulkEditChoice {
public:
    MorphQuickSet(wxWindow* parent, wxWindowID id,
                  const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize,
                  int n = 0, const wxString choices[] = nullptr,
                  long style = 0,
                  const wxValidator& validator = wxDefaultValidator,
                  const wxString& name = wxChoiceNameStr) :
        BulkEditChoice(parent, id, pos, size, n, choices, style, validator, name) {}
    ~MorphQuickSet() override = default;

    void ApplyEffectSetting(const std::string& id, const std::string& value) {
        xLightsApp::GetFrame()->GetMainSequencer()->ApplyEffectSettingToSelected("Morph", id, value, nullptr, "");
    }

    void OnChoicePopup(wxCommandEvent& event) override {
        if (event.GetId() != ID_CHOICE_BULKEDIT) return;

        std::string label = "Bulk Edit";
        wxStaticText* l = GetSettingLabelControl(GetParent(), GetName().ToStdString(), "CHOICE");
        if (l != nullptr) {
            label = l->GetLabel();
        }

        wxArrayString choices;
        for (size_t i = 0; i < GetCount(); i++) {
            choices.push_back(GetString(i));
        }

        wxSingleChoiceDialog dlg(GetParent(), "", label, choices);
        dlg.SetSelection(GetSelection());
        OptimiseDialogPosition(&dlg);
        if (dlg.ShowModal() != wxID_OK) return;

        SetSelection(dlg.GetSelection());
        std::string value = GetString(dlg.GetSelection()).ToStdString();

        for (const auto& p : MORPH_PRESETS) {
            if (value != p.name) continue;
            ApplyEffectSetting("E_SLIDER_Morph_Start_X1", p.values[0]);
            ApplyEffectSetting("E_SLIDER_Morph_Start_X2", p.values[1]);
            ApplyEffectSetting("E_SLIDER_Morph_End_X1",   p.values[2]);
            ApplyEffectSetting("E_SLIDER_Morph_End_X2",   p.values[3]);
            ApplyEffectSetting("E_SLIDER_Morph_Start_Y1", p.values[4]);
            ApplyEffectSetting("E_SLIDER_Morph_Start_Y2", p.values[5]);
            ApplyEffectSetting("E_SLIDER_Morph_End_Y1",   p.values[6]);
            ApplyEffectSetting("E_SLIDER_Morph_End_Y2",   p.values[7]);
            break;
        }

        wxCommandEvent e(wxEVT_COMMAND_CHOICE_SELECTED, GetId());
        e.SetEventObject(this);
        e.SetString(dlg.GetStringSelection());
        e.SetInt(dlg.GetSelection());
        wxPostEvent(GetParent(), e);
    }
};
} // namespace

MorphPanel::MorphPanel(wxWindow* parent, const nlohmann::json& metadata)
    : JsonEffectPanel(parent, metadata, /*deferBuild*/ true) {
    BuildFromJson(metadata);

    CacheCoordControls();
    ApplyStartLimits();
}

wxWindow* MorphPanel::CreateCustomControl(wxWindow* parentWin, wxSizer* sizer,
                                           const nlohmann::json& prop, int cols) {
    std::string id = prop.value("id", "");
    if (id == "Morph_QuickSet") {
        return BuildQuickSet(parentWin, sizer, cols);
    }
    if (id == "Morph_Swap") {
        return BuildSwapButton(parentWin, sizer, cols);
    }
    return nullptr;
}

wxWindow* MorphPanel::BuildQuickSet(wxWindow* parentWin, wxSizer* sizer, int cols) {
    // Quick Set writes preset values into the eight X/Y text controls.
    // Not serialized — the selection is reset to the placeholder after use.
    // Using IDD_ prefix on the name keeps the serializer from saving it.
    auto* label = new wxStaticText(parentWin, wxID_ANY, "Quick Set");
    sizer->Add(label, 1, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);

    _quickSetChoice = new MorphQuickSet(parentWin, wxNewId(),
                                         wxDefaultPosition, wxDefaultSize,
                                         0, nullptr, 0, wxDefaultValidator,
                                         _T("IDD_CHOICE_Morph_QuickSet"));
    _quickSetChoice->Append("Morph Quickset Options");
    _quickSetChoice->Append("Full Sweep Up");
    _quickSetChoice->Append("Full Sweep Down");
    _quickSetChoice->Append("Full Sweep Right");
    _quickSetChoice->Append("Full Sweep Left");
    _quickSetChoice->Append("Single Sweep Up");
    _quickSetChoice->Append("Single Sweep Down");
    _quickSetChoice->Append("Single Sweep Right");
    _quickSetChoice->Append("Single Sweep Left");
    _quickSetChoice->SetSelection(0);
    sizer->Add(_quickSetChoice, 1, wxALL | wxEXPAND, 5);

    _quickSetChoice->Bind(wxEVT_CHOICE, &MorphPanel::OnQuickSetSelect, this);

    if (cols >= 3) sizer->Add(-1, -1, 1, wxALL, 1);
    if (cols >= 4) sizer->Add(-1, -1, 1, wxALL, 1);

    return _quickSetChoice;
}

wxWindow* MorphPanel::BuildSwapButton(wxWindow* parentWin, wxSizer* sizer, int cols) {
    // Swap exchanges Start/End X/Y text control values (and active value curves).
    // Not serialized — purely an in-panel action button.
    sizer->Add(-1, -1, 1, wxALL, 1);
    _swapButton = new wxButton(parentWin, wxNewId(), "Swap Start and End");
    sizer->Add(_swapButton, 1, wxALL | wxEXPAND, 5);
    _swapButton->Bind(wxEVT_BUTTON, &MorphPanel::OnSwapClick, this);

    if (cols >= 3) sizer->Add(-1, -1, 1, wxALL, 1);
    if (cols >= 4) sizer->Add(-1, -1, 1, wxALL, 1);

    return _swapButton;
}

void MorphPanel::CacheCoordControls() {
    auto findText = [this](const char* name) -> wxTextCtrl* {
        return dynamic_cast<wxTextCtrl*>(wxWindow::FindWindowByName(name, this));
    };
    auto findVC = [this](const char* name) -> BulkEditValueCurveButton* {
        return dynamic_cast<BulkEditValueCurveButton*>(wxWindow::FindWindowByName(name, this));
    };

    _textStartX1 = findText("IDD_TEXTCTRL_Morph_Start_X1");
    _textStartY1 = findText("IDD_TEXTCTRL_Morph_Start_Y1");
    _textStartX2 = findText("IDD_TEXTCTRL_Morph_Start_X2");
    _textStartY2 = findText("IDD_TEXTCTRL_Morph_Start_Y2");
    _textEndX1   = findText("IDD_TEXTCTRL_Morph_End_X1");
    _textEndY1   = findText("IDD_TEXTCTRL_Morph_End_Y1");
    _textEndX2   = findText("IDD_TEXTCTRL_Morph_End_X2");
    _textEndY2   = findText("IDD_TEXTCTRL_Morph_End_Y2");

    _vcStartX1 = findVC("ID_VALUECURVE_Morph_Start_X1");
    _vcStartY1 = findVC("ID_VALUECURVE_Morph_Start_Y1");
    _vcStartX2 = findVC("ID_VALUECURVE_Morph_Start_X2");
    _vcStartY2 = findVC("ID_VALUECURVE_Morph_Start_Y2");
    _vcEndX1   = findVC("ID_VALUECURVE_Morph_End_X1");
    _vcEndY1   = findVC("ID_VALUECURVE_Morph_End_Y1");
    _vcEndX2   = findVC("ID_VALUECURVE_Morph_End_X2");
    _vcEndY2   = findVC("ID_VALUECURVE_Morph_End_Y2");
}

void MorphPanel::ApplyStartLimits() {
    // The framework doesn't know the render-layer limits, so set the value
    // curve spans explicitly to match the legacy panel.
    auto setLimits = [](BulkEditValueCurveButton* b, int lo, int hi) {
        if (b) b->GetValue()->SetLimits(lo, hi);
    };
    setLimits(_vcStartX1, MORPH_X_MIN, MORPH_X_MAX);
    setLimits(_vcStartX2, MORPH_X_MIN, MORPH_X_MAX);
    setLimits(_vcEndX1,   MORPH_X_MIN, MORPH_X_MAX);
    setLimits(_vcEndX2,   MORPH_X_MIN, MORPH_X_MAX);
    setLimits(_vcStartY1, MORPH_Y_MIN, MORPH_Y_MAX);
    setLimits(_vcStartY2, MORPH_Y_MIN, MORPH_Y_MAX);
    setLimits(_vcEndY1,   MORPH_Y_MIN, MORPH_Y_MAX);
    setLimits(_vcEndY2,   MORPH_Y_MIN, MORPH_Y_MAX);

    setLimits(dynamic_cast<BulkEditValueCurveButton*>(
        wxWindow::FindWindowByName("ID_VALUECURVE_MorphStartLength", this)),
        MORPH_STARTLENGTH_MIN, MORPH_STARTLENGTH_MAX);
    setLimits(dynamic_cast<BulkEditValueCurveButton*>(
        wxWindow::FindWindowByName("ID_VALUECURVE_MorphEndLength", this)),
        MORPH_ENDLENGTH_MIN, MORPH_ENDLENGTH_MAX);
    setLimits(dynamic_cast<BulkEditValueCurveButton*>(
        wxWindow::FindWindowByName("ID_VALUECURVE_MorphDuration", this)),
        MORPH_DURATION_MIN, MORPH_DURATION_MAX);
    setLimits(dynamic_cast<BulkEditValueCurveButton*>(
        wxWindow::FindWindowByName("ID_VALUECURVE_MorphAccel", this)),
        MORPH_ACCEL_MIN, MORPH_ACCEL_MAX);
    setLimits(dynamic_cast<BulkEditValueCurveButton*>(
        wxWindow::FindWindowByName("ID_VALUECURVE_Morph_Repeat_Count", this)),
        MORPH_REPEAT_MIN, MORPH_REPEAT_MAX);
    setLimits(dynamic_cast<BulkEditValueCurveButton*>(
        wxWindow::FindWindowByName("ID_VALUECURVE_Morph_Repeat_Skip", this)),
        MORPH_REPEATSKIP_MIN, MORPH_REPEATSKIP_MAX);
    setLimits(dynamic_cast<BulkEditValueCurveButton*>(
        wxWindow::FindWindowByName("ID_VALUECURVE_Morph_Stagger", this)),
        MORPH_STAGGER_MIN, MORPH_STAGGER_MAX);
}

void MorphPanel::OnQuickSetSelect(wxCommandEvent& event) {
    if (_quickSetChoice == nullptr) return;
    wxString value = _quickSetChoice->GetStringSelection();

    for (const auto& p : MORPH_PRESETS) {
        if (value != p.name) continue;
        if (_textStartX1) _textStartX1->SetValue(p.values[0]);
        if (_textStartX2) _textStartX2->SetValue(p.values[1]);
        if (_textEndX1)   _textEndX1->SetValue(p.values[2]);
        if (_textEndX2)   _textEndX2->SetValue(p.values[3]);
        if (_textStartY1) _textStartY1->SetValue(p.values[4]);
        if (_textStartY2) _textStartY2->SetValue(p.values[5]);
        if (_textEndY1)   _textEndY1->SetValue(p.values[6]);
        if (_textEndY2)   _textEndY2->SetValue(p.values[7]);
        break;
    }

    _quickSetChoice->SetSelection(0);
    FireChangeEvent();
    event.Skip();
}

void MorphPanel::SwapTextCtrls(wxTextCtrl* a, wxTextCtrl* b) {
    if (a == nullptr || b == nullptr) return;
    wxString tmp = a->GetValue();
    a->SetValue(b->GetValue());
    b->SetValue(tmp);
}

void MorphPanel::SwapValueCurves(BulkEditValueCurveButton* a, BulkEditValueCurveButton* b) {
    if (a == nullptr || b == nullptr) return;
    if (!a->GetValue()->IsActive() && !b->GetValue()->IsActive()) return;
    std::string tmp = a->GetValue()->Serialise();
    a->SetValue(b->GetValue()->Serialise());
    b->SetValue(tmp);
}

void MorphPanel::OnSwapClick(wxCommandEvent& event) {
    SwapTextCtrls(_textStartX1, _textEndX1);
    SwapValueCurves(_vcStartX1, _vcEndX1);

    SwapTextCtrls(_textStartX2, _textEndX2);
    SwapValueCurves(_vcStartX2, _vcEndX2);

    SwapTextCtrls(_textStartY1, _textEndY1);
    SwapValueCurves(_vcStartY1, _vcEndY1);

    SwapTextCtrls(_textStartY2, _textEndY2);
    SwapValueCurves(_vcStartY2, _vcEndY2);

    FireChangeEvent();
    event.Skip();
}

AssistPanel* MorphPanel::GetAssistPanel(wxWindow* parent, xLightsFrame* xl_frame) {
    AssistPanel* assist_panel = new AssistPanel(parent);
    xlGridCanvas* grid = new xlGridCanvasMorph(assist_panel->GetCanvasParent(), wxNewId(),
                                               wxDefaultPosition, wxDefaultSize,
                                               wxTAB_TRAVERSAL | wxFULL_REPAINT_ON_RESIZE,
                                               _T("MorphGrid"));
    assist_panel->SetGridCanvas(grid);
    return assist_panel;
}
