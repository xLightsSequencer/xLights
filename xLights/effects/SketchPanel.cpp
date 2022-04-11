#include "SketchPanel.h"
#include "BulkEditControls.h"

//#include <wx/button.h>
#include <wx/hyperlink.h>
#include <wx/sizer.h>
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
}

BEGIN_EVENT_TABLE(SketchPanel, wxPanel)
END_EVENT_TABLE()

const long SketchPanel::ID_TEXTCTRL_SketchDef = wxNewId();
const long SketchPanel::ID_SLIDER_DrawPercentage = wxNewId();
const long SketchPanel::ID_TEXTCTRL_DrawPercentage = wxNewId();
const long SketchPanel::ID_SLIDER_Thickness = wxNewId();
const long SketchPanel::ID_VALUECURVE_Thickness = wxNewId();
const long SketchPanel::ID_TEXTCTRL_Thickness = wxNewId();
const long SketchPanel::ID_CHECKBOX_MotionEnabled = wxNewId();
const long SketchPanel::ID_SLIDER_MotionPercentage = wxNewId();
const long SketchPanel::ID_TEXTCTRL_MotionPercentage = wxNewId();

SketchPanel::SketchPanel(wxWindow* parent, wxWindowID id /*=wxID_ANY*/, const wxPoint& pos /*= wxDefaultPositi=on*/, const wxSize& size /*= wxDefaultSize*/) :
    xlEffectPanel(parent)
{
    Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));

    auto mainSizer = new wxFlexGridSizer(3, 1, 5, 0);
    mainSizer->AddGrowableRow(2);
    mainSizer->AddGrowableCol(0);

    // Effect Info
    auto effectInfoSizer = new wxFlexGridSizer(3, 1, 0, 0);
    effectInfoSizer->AddGrowableCol(0);

    auto effectDescText = new wxTextCtrl(this, wxID_ANY, effectInfoText,
                                         wxDefaultPosition, wxDefaultSize,
                                         wxTE_NO_VSCROLL | wxTE_MULTILINE | wxTE_READONLY | wxBORDER_NONE);
    effectDescText->SetEditable(false);

    auto effectDescLink = new wxHyperlinkCtrl(this, wxID_ANY, "Sketch Effect Demo", demoVideoURL);
    auto effectMoreLink = new wxHyperlinkCtrl(this, wxID_ANY, "Sketch Tracing Demo", demoVideoURL2);

    effectInfoSizer->Add(effectDescText, 1, wxALL | wxEXPAND, 0);
    effectInfoSizer->Add(effectDescLink, 1, wxALL | wxALIGN_CENTER_HORIZONTAL, 0);
    effectInfoSizer->Add(effectMoreLink, 1, wxALL | wxALIGN_CENTER_HORIZONTAL, 0);

    // Sketch Definition
    auto sketchDefSizer = new wxFlexGridSizer(4, 3, 0, 0);
    sketchDefSizer->AddGrowableCol(1);

    // Sketch
    auto label = new wxStaticText(this, wxID_ANY, "Sketch:");
    TextCtrl_SketchDef = new BulkEditTextCtrl(this, ID_TEXTCTRL_SketchDef, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(this, wxSize(20, -1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_SketchDef"));
    TextCtrl_SketchDef->SetEditable(false);
    //auto defineSketchBtn = new wxButton(this, wxNewId(), "...", wxDefaultPosition, wxDLG_UNIT(this, wxSize(16, -1)));

    sketchDefSizer->Add(label, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);
    sketchDefSizer->Add(TextCtrl_SketchDef, 1, wxALL | wxEXPAND, 2);
    //sketchDefSizer->Add(defineSketchBtn, 1, wxALL | wxALIGN_CENTER_HORIZONTAL, 2);
    sketchDefSizer->AddStretchSpacer();

    // Draw Percentage
    auto label2 = new wxStaticText(this, wxID_ANY, "Draw Percentage:");
    Slider_DrawPercentage = new BulkEditSlider(this, ID_SLIDER_DrawPercentage, 40, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DrawPercentage"));
    TextCtrl_DrawPercentage = new BulkEditTextCtrl(this, ID_TEXTCTRL_DrawPercentage, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(this, wxSize(20, -1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_DrawPercentage"));

    sketchDefSizer->Add(label2, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);
    sketchDefSizer->Add(Slider_DrawPercentage, 1, wxALL | wxEXPAND, 2);
    sketchDefSizer->Add(TextCtrl_DrawPercentage, 1, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    // Thickness
    auto label3 = new wxStaticText(this, wxID_ANY, "Thickness:");
    Slider_Thickness = new BulkEditSlider(this, ID_SLIDER_Thickness, 1, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Thickness"));
    BitmapButton_Thickness = new BulkEditValueCurveButton(this, ID_VALUECURVE_Thickness, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW | wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Thickness"));
    TextCtrl_Thickness = new BulkEditTextCtrl(this, ID_TEXTCTRL_Thickness, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(this, wxSize(20, -1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Thickness"));

    auto thicknessControlsSizer = new wxFlexGridSizer(1, 2, 0, 0);
    thicknessControlsSizer->AddGrowableCol(0);
    thicknessControlsSizer->Add(Slider_Thickness, 1, wxALL | wxEXPAND, 2);
    thicknessControlsSizer->Add(BitmapButton_Thickness, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 2);

    sketchDefSizer->Add(label3, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);
    sketchDefSizer->Add(thicknessControlsSizer, 1, wxALL | wxEXPAND, 0);
    sketchDefSizer->Add(TextCtrl_Thickness, 1, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    // Motion
    auto label4 = new wxStaticText(this, wxID_ANY, "Motion:");
    CheckBox_MotionEnabled = new BulkEditCheckBox(this, ID_CHECKBOX_MotionEnabled, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_MotionEnabled"));
    Slider_MotionPercentage = new BulkEditSlider(this, ID_SLIDER_MotionPercentage, 100, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_MotionPercentage"));
    TextCtrl_MotionPercentage = new BulkEditTextCtrl(this, ID_TEXTCTRL_MotionPercentage, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(this, wxSize(20, -1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_MotionPercentage"));

    auto motionControlsSizer = new wxFlexGridSizer(1, 2, 0, 0);
    motionControlsSizer->AddGrowableCol(1);
    motionControlsSizer->Add(CheckBox_MotionEnabled, 1, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    motionControlsSizer->Add(Slider_MotionPercentage, 1, wxALL | wxEXPAND, 2);

    sketchDefSizer->Add(label4, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);
    sketchDefSizer->Add(motionControlsSizer, 1, wxALL | wxEXPAND, 0);
    sketchDefSizer->Add(TextCtrl_MotionPercentage, 1, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    // Etc
    mainSizer->Add(effectInfoSizer, 1, wxALL | wxEXPAND, 2);
    mainSizer->Add(sketchDefSizer, 1, wxALL | wxEXPAND, 2);

	SetSizer(mainSizer);
    mainSizer->Fit(this);
    mainSizer->SetSizeHints(this);

    SetName("ID_PANEL_SKETCH");

	//Connect(defineSketchBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchPanel::OnButton_DefineSketch);

    Connect(ID_VALUECURVE_Thickness, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchPanel::OnVCButtonClick);

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&SketchPanel::OnVCChanged, 0, this);
    Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&SketchPanel::OnValidateWindow, 0, this);

    BitmapButton_Thickness->SetLimits(1, 10);
}

void SketchPanel::ValidateWindow()
{

}

//void SketchPanel::OnButton_DefineSketch(wxCommandEvent& /*event*/)
//{
//    SketchPathDialog dlg(this);
//    dlg.setSketch(TextCtrl_SketchDef->GetValue());
//
//    if ( dlg.ShowModal() == wxID_OK ) {
//        std::string sketchDef(dlg.sketchDefString());
//        TextCtrl_SketchDef->SetValue(sketchDef);
//    }
//}
