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

    auto sizer1 = new wxFlexGridSizer(2, 1, 0, 0);
    sizer1->AddGrowableRow(1);
    sizer1->AddGrowableCol(0);

    auto sizer2 = new wxFlexGridSizer(1, 3, 0, 0);
    sizer2->AddGrowableCol(1);
    auto label = new wxStaticText(this, wxID_ANY, "Sketch:");
    auto textCtrl = new wxTextCtrl(this, ID_TEXTCTRL_Sketch);
    //textCtrl->Disable();
    textCtrl->SetEditable(false);
    textCtrl->AppendText("SKETCH DEFINITION GOES HERE!");
    auto defineSketchBtn = new wxButton(this, wxNewId(), "...", wxDefaultPosition, wxDLG_UNIT(this, wxSize(16, -1)));
    sizer2->Add(label, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);
    sizer2->Add(textCtrl, 1, wxALL | wxEXPAND, 2);
    sizer2->Add(defineSketchBtn, 1, wxALL, 2);

    sizer1->Add(sizer2, 1, wxALL | wxEXPAND, 2);

	SetSizer(sizer1);
    sizer1->Fit(this);
    sizer1->SetSizeHints(this);

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
