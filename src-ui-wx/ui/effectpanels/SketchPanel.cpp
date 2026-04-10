/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SketchPanel.h"

#include <wx/filepicker.h>
#include <wx/hyperlink.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "assist/AssistPanel.h"
#include "assist/SketchAssistPanel.h"
#include "effects/SketchEffect.h"
#include "effects/SketchEffectDrawing.h"
#include "ui/shared/controls/MediaPickerCtrl.h"
#include "xLightsMain.h"

namespace {
constexpr const char EFFECT_INFO_TEXT[] =
    "The sketch effect allows you to trace out a sketch over an image. That sketch "
    "is then drawn over some percentage of the effect. In the remaining percentage, the entire sketch "
    "is rendered. The sketch is defined within the Effect Assist panel.\n\n"
    "Optionally, motion can be enabled, which enables drawing over the full duration of the effect "
    "but renders only a percentage of the effect in order to create simple 'motion graphics' elements. "
    "For more info, see the link below.";

constexpr const char DEMO_VIDEO_URL[] = "https://vimeo.com/696352082";
constexpr const char DEMO_VIDEO_URL_2[] = "https://vimeo.com/698053599";

// Walk parents until we find the xLightsFrame, then look up the SketchAssistPanel
// inside the EffectAssist AUI pane. Returns nullptr if assist panel isn't open.
SketchAssistPanel* getSketchAssistPanel(wxWindow* win) {
    xLightsFrame* frame = nullptr;
    for (wxWindow* w = win->GetParent(); w != nullptr; w = w->GetParent()) {
        xLightsFrame* f = dynamic_cast<xLightsFrame*>(w);
        if (f != nullptr) {
            frame = f;
            break;
        }
    }
    if (frame != nullptr) {
        auto pane(frame->m_mgr->GetPane("EffectAssist"));
        auto* w = wxWindow::FindWindowByName("ID_PANEL_SKETCH_ASSIST", pane.window);
        if (w != nullptr) {
            return dynamic_cast<SketchAssistPanel*>(w);
        }
    }
    return nullptr;
}
} // namespace

SketchPanel::SketchPanel(wxWindow* parent, const nlohmann::json& metadata)
    : JsonEffectPanel(parent, metadata, /*deferBuild*/ true) {
    BuildFromJson(metadata);

    // The opacity slider is a regular framework-built slider; cache its
    // pointer so the file-picker / opacity handlers don't need to re-walk
    // the window tree on every update.
    _opacitySlider = dynamic_cast<wxSlider*>(
        wxWindow::FindWindowByName("ID_SLIDER_SketchBackgroundOpacity", this));
    if (_opacitySlider) {
        _opacitySlider->Bind(wxEVT_SLIDER, &SketchPanel::OnOpacitySliderChanged, this);
    }

    ValidateWindow();
}

wxWindow* SketchPanel::CreateCustomControl(wxWindow* parentWin, wxSizer* sizer,
                                            const nlohmann::json& prop, int cols) {
    std::string id = prop.value("id", "");
    if (id == "Sketch_Info") {
        return BuildInfoBlock(parentWin, sizer);
    }
    if (id == "Sketch_DefRow") {
        return BuildSketchDefRow(parentWin, sizer);
    }
    if (id == "Sketch_BackgroundRow") {
        return BuildBackgroundRow(parentWin, sizer);
    }
    return nullptr;
}

wxWindow* SketchPanel::BuildInfoBlock(wxWindow* parentWin, wxSizer* sizer) {
    // Vertical block: descriptive text + two demo video hyperlinks. None of
    // this is part of the saved effect; it's purely informational.
    auto* block = new wxFlexGridSizer(0, 1, 0, 0);
    block->AddGrowableCol(0);

    auto* descText = new wxTextCtrl(parentWin, wxID_ANY, EFFECT_INFO_TEXT,
                                     wxDefaultPosition, wxDefaultSize,
                                     wxTE_NO_VSCROLL | wxTE_MULTILINE | wxTE_READONLY | wxBORDER_NONE);
    descText->SetEditable(false);
    block->Add(descText, 1, wxALL | wxEXPAND, 0);

    auto* link1 = new wxHyperlinkCtrl(parentWin, wxID_ANY, "Sketch Effect Demo", DEMO_VIDEO_URL);
    block->Add(link1, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 0);

    auto* link2 = new wxHyperlinkCtrl(parentWin, wxID_ANY, "Sketch Tracing Demo", DEMO_VIDEO_URL_2);
    block->Add(link2, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 0);

    sizer->Add(block, 1, wxALL | wxEXPAND, 2);
    return descText;
}

wxWindow* SketchPanel::BuildSketchDefRow(wxWindow* parentWin, wxSizer* sizer) {
    // Two-column row: "Sketch:" label + read-only text control holding the
    // sketch definition string. The control name matches legacy so old
    // sequences round-trip and SettingsMap.Get("TEXTCTRL_SketchDef") works.
    auto* row = new wxFlexGridSizer(0, 2, 0, 0);
    row->AddGrowableCol(1);

    auto* label = new wxStaticText(parentWin, wxID_ANY, "Sketch:");
    row->Add(label, 0, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);

    _sketchDefText = new BulkEditTextCtrl(parentWin, wxNewId(), wxEmptyString,
                                           wxDefaultPosition,
                                           wxDLG_UNIT(parentWin, wxSize(20, -1)), 0,
                                           wxDefaultValidator, _T("ID_TEXTCTRL_SketchDef"));
    _sketchDefText->SetEditable(false);
    row->Add(_sketchDefText, 1, wxALL | wxEXPAND, 2);

    sizer->Add(row, 1, wxALL | wxEXPAND, 2);
    return _sketchDefText;
}

wxWindow* SketchPanel::BuildBackgroundRow(wxWindow* parentWin, wxSizer* sizer) {
    // Two-column row: "Background:" label + MediaPickerCtrl. We keep a
    // hidden xlSketchFilePickerCtrl with the legacy name so the file path
    // serializes as E_FILEPICKER_SketchBackground (the framework's default
    // filepicker would name it ID_FILEPICKERCTRL_, breaking back-compat).
    auto* row = new wxFlexGridSizer(0, 2, 0, 0);
    row->AddGrowableCol(1);

    auto* label = new wxStaticText(parentWin, wxID_ANY, "Background:");
    row->Add(label, 0, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);

    _hiddenFilePicker = new xlSketchFilePickerCtrl(parentWin, wxNewId(),
                                                    wxEmptyString,
                                                    "Select an image file",
                                                    "*.jpg;*.gif;*.png;*.bmp;*.jpeg;*.webp",
                                                    wxDefaultPosition, wxDefaultSize,
                                                    wxFLP_FILE_MUST_EXIST | wxFLP_OPEN | wxFLP_USE_TEXTCTRL,
                                                    wxDefaultValidator,
                                                    _T("ID_FILEPICKER_SketchBackground"));
    _hiddenFilePicker->Hide();

    _mediaPicker = new MediaPickerCtrl(parentWin, wxID_ANY, MediaType::Image);
    _mediaPicker->SetLinkedPicker(_hiddenFilePicker);
    row->Add(_mediaPicker, 1, wxALL | wxEXPAND, 2);

    sizer->Add(row, 1, wxALL | wxEXPAND, 2);

    // When the underlying file picker reports a change (which the media
    // picker forwards to it), refresh the assist panel background image.
    _hiddenFilePicker->Bind(wxEVT_FILEPICKER_CHANGED, &SketchPanel::OnFilePickerChanged, this);

    return _mediaPicker;
}

void SketchPanel::OnFilePickerChanged(wxCommandEvent& event) {
    UpdateSketchAssistFromControls();
    event.Skip();
}

void SketchPanel::OnOpacitySliderChanged(wxCommandEvent& event) {
    UpdateSketchAssistFromControls();
    event.Skip();
}

void SketchPanel::UpdateSketchAssistFromControls() {
    SketchAssistPanel* panel = getSketchAssistPanel(this);
    if (panel == nullptr || _hiddenFilePicker == nullptr || _opacitySlider == nullptr) {
        return;
    }
    panel->UpdateSketchBackground(_hiddenFilePicker->GetFileName().GetFullPath(),
                                  _opacitySlider->GetValue());
}

void SketchPanel::UpdateSketchAssistBackground() const {
    if (_sketchAssistPanel == nullptr || _hiddenFilePicker == nullptr ||
        _opacitySlider == nullptr) {
        return;
    }
    _sketchAssistPanel->UpdateSketchBackground(_hiddenFilePicker->GetFileName().GetFullPath(),
                                               _opacitySlider->GetValue());
}

AssistPanel* SketchPanel::GetAssistPanel(wxWindow* parent, xLightsFrame* /*xl_frame*/) {
    // The assist panel writes back into our sketch def text, file picker,
    // and opacity slider via this lambda whenever the user finishes editing
    // a sketch in the canvas.
    auto callback = [this](const std::string& sketchDef, const std::string& picPath, unsigned char alpha) {
        if (_sketchDefText) {
            _sketchDefText->SetValue(sketchDef);
        }
        if (_hiddenFilePicker) {
            _hiddenFilePicker->SetFileName(wxFileName(picPath));
        }
        if (_opacitySlider) {
            SetSliderValue(_opacitySlider, alpha);
        }
    };

    auto* assistPanel = new AssistPanel(parent);

    auto* sketchAssistPanel = new SketchAssistPanel(assistPanel->GetCanvasParent());
    if (_sketchDefText) {
        sketchAssistPanel->SetSketchDef(_sketchDefText->GetValue().ToStdString());
    }
    sketchAssistPanel->SetSketchUpdateCallback(callback);
    assistPanel->AddPanel(sketchAssistPanel, wxALL | wxEXPAND);

    _sketchAssistPanel = sketchAssistPanel;
    UpdateSketchAssistBackground();

    return assistPanel;
}

void SketchPanel::ValidateWindow() {
    // Defer to the JSON visibility rules for Motion enable/disable, then
    // refresh the file picker tooltip with the current filename (legacy
    // behavior — useful when the path string is too long to read at a glance).
    JsonEffectPanel::ValidateWindow();
    if (_hiddenFilePicker) {
        _hiddenFilePicker->SetToolTip(wxFileName(_hiddenFilePicker->GetFileName()).GetFullName());
    }
    if (_mediaPicker && _hiddenFilePicker) {
        _mediaPicker->SetToolTip(wxFileName(_hiddenFilePicker->GetFileName()).GetFullName());
    }
}

void SketchPanel::SetDefaultParameters() {
    JsonEffectPanel::SetDefaultParameters();

    // The sketch def text needs the project's default sketch string (a
    // single straight line by default). The hidden file picker is reset to
    // empty since there's no default background image.
    if (_sketchDefText) {
        SetTextValue(_sketchDefText, SketchEffectSketch::DefaultSketchString());
    }
    if (_hiddenFilePicker) {
        _hiddenFilePicker->SetFileName(wxFileName());
    }
    if (_opacitySlider) {
        SetSliderValue(_opacitySlider, 0x30);
    }
}
