/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ConvertLogDialog.h"

//(*InternalHeaders(ConvertLogDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "./utils/spdlog_macros.h"

//(*IdInit(ConvertLogDialog)
const long ConvertLogDialog::ID_TEXTCTRL_LOG = wxNewId();
//*)

BEGIN_EVENT_TABLE(ConvertLogDialog,wxDialog)
	//(*EventTable(ConvertLogDialog)
	//*)
END_EVENT_TABLE()

void ConvertLogDialog::AppendConvertStatus(const wxString &msg, bool flushBuffer) {
    if (flushBuffer && !msgBuffer.IsEmpty()) {
        msgBuffer.append(msg);
        TextCtrlLog->AppendText(msgBuffer);
        msgBuffer.Clear();
    }
    else if (flushBuffer) {
        TextCtrlLog->AppendText(msg);
    }
    else {
        msgBuffer.append(msg);
        if (msgBuffer.size() > 10000) {
            TextCtrlLog->AppendText(msgBuffer);
            msgBuffer.Clear();
        }
    }
}

void ConvertLogDialog::Done()
{
    AppendConvertStatus("\nDONE!!!\n");
}

ConvertLogDialog::ConvertLogDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(ConvertLogDialog)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, _("Conversion Log"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	TextCtrlLog = new wxTextCtrl(this, ID_TEXTCTRL_LOG, wxEmptyString, wxDefaultPosition, wxSize(584,401), wxTE_MULTILINE|wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_LOG"));
	FlexGridSizer1->Add(TextCtrlLog, 1, wxALL|wxEXPAND, 2);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)
}

ConvertLogDialog::~ConvertLogDialog()
{
	//(*Destroy(ConvertLogDialog)
	//*)
}

void ConvertLogDialog::OnButtonCloseClick(wxCommandEvent& event)
{
    EndDialog(wxID_CLOSE);
}
