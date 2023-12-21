#include "SketchPanel.h"
#include "BulkEditControls.h"
#include "assist/SketchAssistPanel.h"
#include "../xLightsMain.h"

#include <wx/filepicker.h>
#include <wx/hyperlink.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/utils.h>

namespace
{
    const char effectInfoText[] =
        "The sketch effect allows you to trace out a sketch over an image. That sketch "
        "is then drawn over some percentage of the effect. In the remaining percentage, the entire sketch "
        "is rendered. The sketch is defined within the Effect Assist panel.\n\n"
        "Optionally, motion can be enabled, which enables drawing over the full duration of the effect "
        "but renders only a percentage of the effect in order to create simple 'motion graphics' elements. "
        "For more info, see the link below.";

    const char demoVideoURL[] = "https://vimeo.com/696352082";
    const char demoVideoURL2[] = "https://vimeo.com/698053599";

    const wxString imgSelect("Select an image file");
    const wxString imgFilters("*.jpg;*.gif;*.png;*.bmp;*.jpeg;*.webp");

    SketchAssistPanel* getSketchAssistPanel(wxWindow* win)
    {
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
            auto w = wxWindow::FindWindowByName("ID_PANEL_SKETCH_ASSIST", pane.window);
            if (w != nullptr)
                return dynamic_cast<SketchAssistPanel*>(w);
        }
        return nullptr;
    }
}

BEGIN_EVENT_TABLE(SketchPanel, wxPanel)
END_EVENT_TABLE()

const long SketchPanel::ID_TEXTCTRL_SketchDef = wxNewId();
const long SketchPanel::ID_FILEPICKER_SketchBackground = wxNewId();
const long SketchPanel::ID_SLIDER_SketchBackgroundOpacity = wxNewId();
const long SketchPanel::ID_SLIDER_DrawPercentage = wxNewId();
const long SketchPanel::ID_TEXTCTRL_DrawPercentage = wxNewId();
const long SketchPanel::ID_VALUECURVE_DrawPercentage = wxNewId();
const long SketchPanel::ID_SLIDER_Thickness = wxNewId();
const long SketchPanel::ID_VALUECURVE_Thickness = wxNewId();
const long SketchPanel::ID_TEXTCTRL_Thickness = wxNewId();
const long SketchPanel::ID_CHECKBOX_MotionEnabled = wxNewId();
const long SketchPanel::ID_SLIDER_MotionPercentage = wxNewId();
const long SketchPanel::ID_VALUECURVE_MotionPercentage = wxNewId();
const long SketchPanel::ID_TEXTCTRL_MotionPercentage = wxNewId();

SketchPanel::SketchPanel(wxWindow* parent, wxWindowID id /*=wxID_ANY*/, const wxPoint& pos /*= wxDefaultPositi=on*/, const wxSize& size /*= wxDefaultSize*/) :
    xlEffectPanel(parent)
{
    Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));

    // EffectInfo controls
    auto effectDescText = new wxTextCtrl(this, wxID_ANY, effectInfoText,
                                         wxDefaultPosition, wxDefaultSize,
                                         wxTE_NO_VSCROLL | wxTE_MULTILINE | wxTE_READONLY | wxBORDER_NONE);
    effectDescText->SetEditable(false);

    auto effectDescLink = new wxHyperlinkCtrl(this, wxID_ANY, "Sketch Effect Demo", demoVideoURL);
    auto effectMoreLink = new wxHyperlinkCtrl(this, wxID_ANY, "Sketch Tracing Demo", demoVideoURL2);

    // Sketch controls
    auto sketchLabel = new wxStaticText(this, wxID_ANY, "Sketch:");
    TextCtrl_SketchDef = new BulkEditTextCtrl(this, ID_TEXTCTRL_SketchDef, wxEmptyString, wxDefaultPosition,
                                              wxDLG_UNIT(this, wxSize(20, -1)), 0,
                                              wxDefaultValidator, _T("ID_TEXTCTRL_SketchDef"));
    TextCtrl_SketchDef->SetEditable(false);

    auto bgLabel = new wxStaticText(this, wxID_ANY, "Background:");
    FilePicker_SketchBackground = new wxFilePickerCtrl(this, ID_FILEPICKER_SketchBackground,
                                                       wxEmptyString, imgSelect, imgFilters,
                                                       wxDefaultPosition, wxDefaultSize,
                                                       wxFLP_FILE_MUST_EXIST | wxFLP_OPEN | wxFLP_USE_TEXTCTRL,
                                                       wxDefaultValidator, "ID_FILEPICKER_SketchBackground");
    //FilePicker_SketchBackground->GetTextCtrl()->SetEditable(false);

    auto opacityLabel = new wxStaticText(this, wxID_ANY, "Opacity:");
    Slider_SketchBackgroundOpacity = new wxSlider(this, ID_SLIDER_SketchBackgroundOpacity, 0x30, 0x00, 0xff,
                                                  wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "ID_SLIDER_SketchBackgroundOpacity");

    auto separator = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);

    // Settings controls
    //
    // Bulk Edit is weird... apparently the secret sauce is for the
    // TextCtrl id to be "IDD_" instead of "ID_"

    auto drawPctLabel = new wxStaticText(this, wxID_ANY, "Draw Percentage:");
    Slider_DrawPercentage = new BulkEditSlider(this, ID_SLIDER_DrawPercentage,
                                               DrawPercentageDef, DrawPercentageMin, DrawPercentageMax,
                                               wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DrawPercentage"));
    BitmapButton_DrawPercentage = new BulkEditValueCurveButton(this, ID_VALUECURVE_DrawPercentage,
                                                               wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW | wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_DrawPercentage"));
    TextCtrl_DrawPercentage = new BulkEditTextCtrl(this, ID_TEXTCTRL_DrawPercentage,
                                                   wxEmptyString, wxDefaultPosition,
                                                   wxDLG_UNIT(this, wxSize(20, -1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_DrawPercentage"));

    auto thicknessLabel = new wxStaticText(this, wxID_ANY, "Thickness:");
    Slider_Thickness = new BulkEditSlider(this, ID_SLIDER_Thickness,
                                          ThicknessDef, ThicknessMin, ThicknessMax,
                                          wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Thickness"));
    BitmapButton_Thickness = new BulkEditValueCurveButton(this, ID_VALUECURVE_Thickness,
                                                          wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW | wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Thickness"));
    TextCtrl_Thickness = new BulkEditTextCtrl(this, ID_TEXTCTRL_Thickness,
                                              wxEmptyString, wxDefaultPosition,
                                              wxDLG_UNIT(this, wxSize(20, -1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Thickness"));

    auto motionLabel = new wxStaticText(this, wxID_ANY, "Motion:");
    CheckBox_MotionEnabled = new BulkEditCheckBox(this, ID_CHECKBOX_MotionEnabled, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_MotionEnabled"));
    Slider_MotionPercentage = new BulkEditSlider(this, ID_SLIDER_MotionPercentage,
                                                 MotionPercentageDef, MotionPercentageMin, MotionPercentageMax,
                                                 wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_MotionPercentage"));
    BitmapButton_MotionPercentage = new BulkEditValueCurveButton(this, ID_VALUECURVE_MotionPercentage,
                                                                 wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW | wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_MotionPercentage"));

    TextCtrl_MotionPercentage = new BulkEditTextCtrl(this, ID_TEXTCTRL_MotionPercentage,
                                                     wxEmptyString, wxDefaultPosition,
                                                     wxDLG_UNIT(this, wxSize(20, -1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_MotionPercentage"));

    // Sizers - main gets effectInfoSizer, sketchSizer, separator, and settingsSizer
    auto mainSizer = new wxFlexGridSizer(7, 1, 0, 0);
    mainSizer->AddGrowableRow(6);
    mainSizer->AddGrowableCol(0);

    auto effectInfoSizer = new wxFlexGridSizer(3, 1, 0, 0);
    effectInfoSizer->AddGrowableCol(0);
    effectInfoSizer->Add(effectDescText, 1, wxALL | wxEXPAND, 0);
    effectInfoSizer->Add(effectDescLink, 1, wxALL | wxALIGN_CENTER_HORIZONTAL, 0);
    effectInfoSizer->Add(effectMoreLink, 1, wxALL | wxALIGN_CENTER_HORIZONTAL, 0);
    mainSizer->Add(effectInfoSizer, 1, wxALL | wxEXPAND, 2);

    auto sketchSizer = new wxFlexGridSizer(3, 2, 0, 0);
    sketchSizer->AddGrowableCol(1);
    sketchSizer->Add(sketchLabel, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);
    sketchSizer->Add(TextCtrl_SketchDef, 1, wxALL | wxEXPAND, 2);
    sketchSizer->Add(bgLabel, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);
    sketchSizer->Add(FilePicker_SketchBackground, 1, wxALL | wxEXPAND, 2);
    sketchSizer->Add(opacityLabel, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);
    sketchSizer->Add(Slider_SketchBackgroundOpacity, 1, wxALL | wxEXPAND, 2);
    mainSizer->Add(sketchSizer, 1, wxALL | wxEXPAND, 2);

    mainSizer->AddSpacer(5);
    mainSizer->Add(separator, 1, wxALL | wxEXPAND, 2);
    mainSizer->AddSpacer(5);

    auto settingsSizer = new wxFlexGridSizer(3, 5, 0, 0);
    settingsSizer->AddGrowableCol(2);

    settingsSizer->Add(drawPctLabel, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);
    settingsSizer->AddStretchSpacer();
    settingsSizer->Add(Slider_DrawPercentage, 1, wxALL | wxEXPAND, 2);
    settingsSizer->Add(BitmapButton_DrawPercentage, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 2);
    settingsSizer->Add(TextCtrl_DrawPercentage, 1, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    settingsSizer->Add(thicknessLabel, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);
    settingsSizer->AddStretchSpacer();
    settingsSizer->Add(Slider_Thickness, 1, wxALL | wxEXPAND, 2);
    settingsSizer->Add(BitmapButton_Thickness, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 2);
    settingsSizer->Add(TextCtrl_Thickness, 1, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    settingsSizer->Add(motionLabel, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);
    settingsSizer->Add(CheckBox_MotionEnabled, 1, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    settingsSizer->Add(Slider_MotionPercentage, 1, wxALL | wxEXPAND, 2);
    settingsSizer->Add(BitmapButton_MotionPercentage, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 2);
    settingsSizer->Add(TextCtrl_MotionPercentage, 1, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    mainSizer->Add(settingsSizer, 1, wxALL | wxEXPAND, 2);

	SetSizer(mainSizer);
    mainSizer->Fit(this);
    mainSizer->SetSizeHints(this);

    Connect(ID_FILEPICKER_SketchBackground, wxEVT_COMMAND_FILEPICKER_CHANGED, (wxObjectEventFunction)&SketchPanel::OnFilePickerCtrl_FileChanged);
    Connect(ID_SLIDER_SketchBackgroundOpacity, wxEVT_COMMAND_SLIDER_UPDATED, (wxObjectEventFunction)&SketchPanel::OnSlider_BgAlphaChanged);

    Connect(ID_VALUECURVE_DrawPercentage, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchPanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_Thickness, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchPanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_MotionPercentage, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchPanel::OnVCButtonClick);

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&SketchPanel::OnVCChanged, 0, this);
    Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&SketchPanel::OnValidateWindow, 0, this);

    SetName("ID_PANEL_SKETCH");

    Slider_DrawPercentage->SetRange(DrawPercentageMin, DrawPercentageMax);
    Slider_Thickness->SetRange(ThicknessMin, ThicknessMax);
    Slider_MotionPercentage->SetRange(MotionPercentageMin, MotionPercentageMax);

    BitmapButton_DrawPercentage->SetLimits(DrawPercentageMin, DrawPercentageMax);
    BitmapButton_Thickness->SetLimits(ThicknessMin, ThicknessMax);
    BitmapButton_MotionPercentage->SetLimits(MotionPercentageMin, MotionPercentageMax);

    Connect(ID_CHECKBOX_MotionEnabled, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&SketchPanel::OnCheckBox_MotionClick);
}

void SketchPanel::ValidateWindow()
{
    bool motion = CheckBox_MotionEnabled->IsChecked();
    Slider_DrawPercentage->Enable(!motion);
    BitmapButton_DrawPercentage->Enable(!motion);
    TextCtrl_DrawPercentage->Enable(!motion);
    
    Slider_MotionPercentage->Enable(motion);
    BitmapButton_MotionPercentage->Enable(motion);
    TextCtrl_MotionPercentage->Enable(motion);
}

void SketchPanel::OnFilePickerCtrl_FileChanged(wxCommandEvent& event)
{
    updateSketchAssist(getSketchAssistPanel(this));
}

void SketchPanel::OnSlider_BgAlphaChanged(wxCommandEvent& /*event*/)
{
    updateSketchAssist(getSketchAssistPanel(this));
}

void SketchPanel::OnCheckBox_MotionClick(wxCommandEvent& /*event*/)
{
    ValidateWindow();
}

void SketchPanel::updateSketchAssist(SketchAssistPanel* panel)
{
    if (panel == nullptr)
        return;

    wxString path(FilePicker_SketchBackground->GetFileName().GetFullPath());
    int opacity = Slider_SketchBackgroundOpacity->GetValue();

    panel->UpdateSketchBackground(path, opacity);
}
