#include "SketchPanel.h"
#include "SketchPathDialog.h"

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/utils.h>

BEGIN_EVENT_TABLE(SketchPanel, wxPanel)
END_EVENT_TABLE()

const long SketchPanel::ID_TEXTCTRL_Sketch = wxNewId();

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
    auto textCtrl = new wxTextCtrl(this, ID_TEXTCTRL_Sketch);
    //textCtrl->Disable();
    textCtrl->SetEditable(false);
    textCtrl->AppendText("SKETCH DEFINITION GOES HERE!");
    auto defineSketchBtn = new wxButton(this, wxNewId(), "...", wxDefaultPosition, wxDLG_UNIT(this, wxSize(16, -1)));
    sketchDefSizer->Add(label, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);
    sketchDefSizer->Add(textCtrl, 1, wxALL | wxEXPAND, 2);
    sketchDefSizer->Add(defineSketchBtn, 1, wxALL, 2);

    mainSizer->Add(sketchDefSizer, 1, wxALL | wxEXPAND, 2);

	SetSizer(mainSizer);
    mainSizer->Fit(this);
    mainSizer->SetSizeHints(this);

	Connect(defineSketchBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchPanel::OnButton_DefineSketch);
}

void SketchPanel::ValidateWindow()
{

}

void SketchPanel::OnButton_DefineSketch(wxCommandEvent& /*event*/)
{
    SketchPathDialog dlg(this);

    dlg.ShowModal();
}
