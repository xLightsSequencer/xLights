#include "ResultDialog.h"

//(*InternalHeaders(ResultDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/filedlg.h>
#include <wx/file.h>
#include <wx/msgdlg.h>

//(*IdInit(ResultDialog)
const long ResultDialog::ID_TEXTCTRL1 = wxNewId();
const long ResultDialog::ID_BUTTON1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ResultDialog,wxDialog)
	//(*EventTable(ResultDialog)
	//*)
END_EVENT_TABLE()

ResultDialog::ResultDialog(wxWindow* parent, wxString log, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(ResultDialog)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	TextCtrl_Log = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxVSCROLL|wxHSCROLL, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Log, 1, wxALL|wxEXPAND, 5);
	Button_Save = new wxButton(this, ID_BUTTON1, _("Save Log"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer1->Add(Button_Save, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ResultDialog::OnButton_SaveClick);
	//*)

    TextCtrl_Log->SetValue(log);

    SetSize(400, 400);
    Layout();
}

ResultDialog::~ResultDialog()
{
	//(*Destroy(ResultDialog)
	//*)
}


void ResultDialog::OnButton_SaveClick(wxCommandEvent& event)
{
    wxFileDialog dlg(this, _("Save log"), "", "",
        "LOG (*.log)|*.log|Text (*.txt)|*.txt", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxFile f;

        if (f.Create(dlg.GetDirectory() + "/" + dlg.GetFilename(), true))
        {
            f.Write(TextCtrl_Log->GetValue());
            f.Close();
        }
        else
        {
            wxMessageBox("Error writing log file.");
        }
    }
}
