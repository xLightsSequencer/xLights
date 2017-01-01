#include "VideoWindowPositionDialog.h"

//(*InternalHeaders(VideoWindowPositionDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(VideoWindowPositionDialog)
const long VideoWindowPositionDialog::ID_STATICTEXT1 = wxNewId();
const long VideoWindowPositionDialog::ID_BUTTON1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(VideoWindowPositionDialog,wxDialog)
	//(*EventTable(VideoWindowPositionDialog)
	//*)
END_EVENT_TABLE()

VideoWindowPositionDialog::VideoWindowPositionDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(VideoWindowPositionDialog)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCAPTION|wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMINIMIZE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Position this window where you want the video displayed and click Ok."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer1->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VideoWindowPositionDialog::OnButton_OkClick);
	//*)
}

VideoWindowPositionDialog::~VideoWindowPositionDialog()
{
	//(*Destroy(VideoWindowPositionDialog)
	//*)
}


void VideoWindowPositionDialog::OnButton_OkClick(wxCommandEvent& event)
{
    EndDialog(wxID_OK);
}
