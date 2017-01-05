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
	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCAPTION|wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	StaticText_Message = new wxStaticText(this, ID_STATICTEXT1, _("Position and size window where you want the content displayed."), wxPoint(8,16), wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	Button_ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxPoint(144,40), wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));

	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VideoWindowPositionDialog::OnButton_OkClick);
	Connect(wxEVT_SIZE,(wxObjectEventFunction)&VideoWindowPositionDialog::OnResize);
	//*)

    Move(pos);
    SetSize(size);
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

void VideoWindowPositionDialog::OnResize(wxSizeEvent& event)
{
    int msgw, msgh;
    StaticText_Message->GetSize(&msgw, &msgh);

    int winw, winh;
    GetClientSize(&winw, &winh);

    int bw, bh;
    Button_ok->GetSize(&bw, &bh);

    StaticText_Message->Move((winw - msgw) / 2, 5);
    Button_ok->Move((winw - bw) / 2, 5 + msgh + 5);
}
