#include "SketchPanel.h"
#include "BulkEditControls.h"
#include "SketchPathDialog.h"

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/utils.h>

BEGIN_EVENT_TABLE(SketchPanel, wxPanel)
END_EVENT_TABLE()

const long SketchPanel::ID_TEXTCTRL_SketchDef = wxNewId();
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

    auto mainSizer = new wxFlexGridSizer(2, 1, 0, 0);
    mainSizer->AddGrowableRow(1);
    mainSizer->AddGrowableCol(0);

    auto sketchDefSizer = new wxFlexGridSizer(3, 3, 0, 0);
    sketchDefSizer->AddGrowableCol(1);

    auto label = new wxStaticText(this, wxID_ANY, "Sketch:");
    TextCtrl_SketchDef = new BulkEditTextCtrl(this, ID_TEXTCTRL_SketchDef, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(this, wxSize(20, -1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_SketchDef"));
    TextCtrl_SketchDef->SetEditable(false);
    auto defineSketchBtn = new wxButton(this, wxNewId(), "...", wxDefaultPosition, wxDLG_UNIT(this, wxSize(16, -1)));
    sketchDefSizer->Add(label, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);
    sketchDefSizer->Add(TextCtrl_SketchDef, 1, wxALL | wxEXPAND, 2);
    sketchDefSizer->Add(defineSketchBtn, 1, wxALL | wxALIGN_CENTER_HORIZONTAL, 2);

    auto label2 = new wxStaticText(this, wxID_ANY, "Thickness:");
    sketchDefSizer->Add(label2, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);
    auto thicknessControlsSizer = new wxFlexGridSizer(1, 2, 0, 0);
    thicknessControlsSizer->AddGrowableCol(0);
    Slider_Thickness = new BulkEditSlider(this, ID_SLIDER_Thickness, 1, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Thickness"));
    BitmapButton_Thickness = new BulkEditValueCurveButton(this, ID_VALUECURVE_Thickness, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW | wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Thickness"));
    thicknessControlsSizer->Add(Slider_Thickness, 1, wxALL | wxEXPAND, 2);
    thicknessControlsSizer->Add(BitmapButton_Thickness, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 2);
    sketchDefSizer->Add(thicknessControlsSizer, 1, wxALL | wxEXPAND, 0);
    TextCtrl_Thickness = new BulkEditTextCtrl(this, ID_TEXTCTRL_Thickness, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(this, wxSize(20, -1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Thickness"));
    sketchDefSizer->Add(TextCtrl_Thickness, 1, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    auto label3 = new wxStaticText(this, wxID_ANY, "Motion:");
    sketchDefSizer->Add(label3, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);
    auto motionControlsSizer = new wxFlexGridSizer(1, 3, 0, 0);
    motionControlsSizer->AddGrowableCol(1);
    CheckBox_MotionEnabled = new BulkEditCheckBox(this, ID_CHECKBOX_MotionEnabled, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_MotionEnabled"));
    Slider_MotionPercentage = new BulkEditSlider(this, ID_SLIDER_MotionPercentage, 100, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_MotionPercentage"));
    BitmapButton_MotionPercentage = new BulkEditValueCurveButton(this, ID_VALUECURVE_MotionPercentage, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW | wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_MotionPercentage"));
    motionControlsSizer->Add(CheckBox_MotionEnabled, 1, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    motionControlsSizer->Add(Slider_MotionPercentage, 1, wxALL | wxEXPAND, 2);
    motionControlsSizer->Add(BitmapButton_MotionPercentage, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 2);
    sketchDefSizer->Add(motionControlsSizer, 1, wxALL | wxEXPAND, 0);
    TextCtrl_MotionPercentage = new BulkEditTextCtrl(this, ID_TEXTCTRL_MotionPercentage, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(this, wxSize(20, -1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_MotionPercentage"));
    sketchDefSizer->Add(TextCtrl_MotionPercentage, 1, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    mainSizer->Add(sketchDefSizer, 1, wxALL | wxEXPAND, 2);

	SetSizer(mainSizer);
    mainSizer->Fit(this);
    mainSizer->SetSizeHints(this);

    SetName("ID_PANEL_SKETCH"); // necessary?

	Connect(defineSketchBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchPanel::OnButton_DefineSketch);

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&SketchPanel::OnVCChanged, 0, this);
    Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&SketchPanel::OnValidateWindow, 0, this);

    BitmapButton_Thickness->SetLimits(1, 10);
    BitmapButton_MotionPercentage->SetLimits(0, 100);
}

void SketchPanel::ValidateWindow()
{

}

void SketchPanel::OnButton_DefineSketch(wxCommandEvent& /*event*/)
{
    SketchPathDialog dlg(this);
    dlg.setSketch(TextCtrl_SketchDef->GetValue());

    if ( dlg.ShowModal() == wxID_OK ) {
        std::string sketchDef(dlg.sketchDefString());
        TextCtrl_SketchDef->SetValue(sketchDef);
    }
}
