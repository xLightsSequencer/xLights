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

SketchPanel::SketchPanel(wxWindow* parent, wxWindowID id /*=wxID_ANY*/, const wxPoint& pos /*= wxDefaultPositi=on*/, const wxSize& size /*= wxDefaultSize*/) :
    xlEffectPanel(parent)
{
    Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));

    auto mainSizer = new wxFlexGridSizer(2, 1, 0, 0);
    mainSizer->AddGrowableRow(1);
    mainSizer->AddGrowableCol(0);

    auto sketchDefSizer = new wxFlexGridSizer(1, 3, 0, 0);
    sketchDefSizer->AddGrowableCol(1);
    auto label = new wxStaticText(this, wxID_ANY, "Sketch:");
    TextCtrl_SketchDef = new BulkEditTextCtrl(this, ID_TEXTCTRL_SketchDef, wxString(), wxDefaultPosition, wxDLG_UNIT(this, wxSize(20, -1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_SketchDef"));
    //TextCtrl_SketchDef->Disable();
    TextCtrl_SketchDef->SetEditable(false);
    auto defineSketchBtn = new wxButton(this, wxNewId(), "...", wxDefaultPosition, wxDLG_UNIT(this, wxSize(16, -1)));
    sketchDefSizer->Add(label, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);
    sketchDefSizer->Add(TextCtrl_SketchDef, 1, wxALL | wxEXPAND, 2);
    sketchDefSizer->Add(defineSketchBtn, 1, wxALL, 2);

    mainSizer->Add(sketchDefSizer, 1, wxALL | wxEXPAND, 2);

	SetSizer(mainSizer);
    mainSizer->Fit(this);
    mainSizer->SetSizeHints(this);

    SetName("ID_PANEL_SKETCH"); // necessary?

	Connect(defineSketchBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchPanel::OnButton_DefineSketch);

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
