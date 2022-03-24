#include "SketchPanel.h"
#include "BulkEditControls.h"
#include "SketchPathDialog.h"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/utils.h>

BEGIN_EVENT_TABLE(SketchPanel, wxPanel)
END_EVENT_TABLE()

const long SketchPanel::ID_TEXTCTRL_SketchDef = wxNewId();
const long SketchPanel::ID_CHECKBOX_MotionEnabled = wxNewId();
const long SketchPanel::ID_SLIDER_MotionPercentage = wxNewId();

SketchPanel::SketchPanel(wxWindow* parent, wxWindowID id /*=wxID_ANY*/, const wxPoint& pos /*= wxDefaultPositi=on*/, const wxSize& size /*= wxDefaultSize*/) :
    xlEffectPanel(parent)
{
    Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));

    auto mainSizer = new wxFlexGridSizer(2, 1, 0, 0);
    mainSizer->AddGrowableRow(1);
    mainSizer->AddGrowableCol(0);

    auto sketchDefSizer = new wxFlexGridSizer(2, 3, 0, 0);
    sketchDefSizer->AddGrowableCol(1);

    auto label = new wxStaticText(this, wxID_ANY, "Sketch:");
    TextCtrl_SketchDef = new BulkEditTextCtrl(this, ID_TEXTCTRL_SketchDef, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(this, wxSize(20, -1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_SketchDef"));
    TextCtrl_SketchDef->SetEditable(false);
    auto defineSketchBtn = new wxButton(this, wxNewId(), "...", wxDefaultPosition, wxDLG_UNIT(this, wxSize(16, -1)));
    sketchDefSizer->Add(label, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);
    sketchDefSizer->Add(TextCtrl_SketchDef, 1, wxALL | wxEXPAND, 2);
    sketchDefSizer->Add(defineSketchBtn, 1, wxALL, 2);

    auto label2 = new wxStaticText(this, wxID_ANY, "Motion:");
    sketchDefSizer->Add(label2, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);
    auto motionControlsSizer = new wxFlexGridSizer(1, 2, 0, 0);
    motionControlsSizer->AddGrowableCol(1);
    CheckBox_MotionEnabled = new BulkEditCheckBox(this, ID_CHECKBOX_MotionEnabled, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_MotionEnabled"));
    Slider_MotionPercentage = new BulkEditSlider(this, ID_SLIDER_MotionPercentage, 100, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_MotionPercentage"));
    motionControlsSizer->Add(CheckBox_MotionEnabled, 1, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    motionControlsSizer->Add(Slider_MotionPercentage, 1, wxALL | wxEXPAND, 2);
    sketchDefSizer->Add(motionControlsSizer, 1, wxALL | wxEXPAND, 2);
    auto motionTextCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(this, wxSize(20, -1)), 0);
    sketchDefSizer->Add(motionTextCtrl, 1);

    mainSizer->Add(sketchDefSizer, 1, wxALL | wxEXPAND, 2);

	SetSizer(mainSizer);
    mainSizer->Fit(this);
    mainSizer->SetSizeHints(this);

    SetName("ID_PANEL_SKETCH"); // necessary?

	Connect(defineSketchBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchPanel::OnButton_DefineSketch);
    //Connect(ID_CHECKBOX_MotionEnabled, wxEVT_CHECKBOX, (wxObjectEventFunction)&SketchPanel::OnCheckBox_ToggleMotion);

    Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&SketchPanel::OnValidateWindow, 0, this);
}

void SketchPanel::ValidateWindow()
{

}

void SketchPanel::OnButton_DefineSketch(wxCommandEvent& /*event*/)
{
    SketchPathDialog dlg(this);

    if ( dlg.ShowModal() == wxID_OK ) {
        std::string sketchDef(dlg.sketchDefString());
        TextCtrl_SketchDef->SetValue(sketchDef);
    }
}
