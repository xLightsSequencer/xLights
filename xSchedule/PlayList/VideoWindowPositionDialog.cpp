#include "VideoWindowPositionDialog.h"

//(*InternalHeaders(VideoWindowPositionDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(VideoWindowPositionDialog)
const long VideoWindowPositionDialog::ID_STATICTEXT1 = wxNewId();
const long VideoWindowPositionDialog::ID_BUTTON1 = wxNewId();
const long VideoWindowPositionDialog::ID_STATICTEXT2 = wxNewId();
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
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Position and size window where you want the content displayed."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	Button_ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxPoint(48,40), wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	StaticText_Position = new wxStaticText(this, ID_STATICTEXT2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));

	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VideoWindowPositionDialog::OnButton_OkClick);
	Connect(wxEVT_SIZE,(wxObjectEventFunction)&VideoWindowPositionDialog::OnResize);
	//*)

    Connect(wxEVT_MOVE, (wxObjectEventFunction)&VideoWindowPositionDialog::OnMove);

    Move(pos);
    SetSize(size);

    SetWindowPositionText();
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
    SetWindowPositionText();

    int msgw, msgh;
    StaticText1->GetSize(&msgw, &msgh);

    int pw, ph;
    StaticText_Position->GetSize(&pw, &ph);

    int winw, winh;
    GetClientSize(&winw, &winh);

    int bw, bh;
    Button_ok->GetSize(&bw, &bh);

    StaticText1->Move((winw - msgw) / 2, 5);
    Button_ok->Move((winw - bw) / 2, 5 + msgh + 5);
    StaticText_Position->Move((winw - pw) / 2, 5 + msgh + 5 + bh + 5);
}

void VideoWindowPositionDialog::OnMove(wxMoveEvent& event)
{
    SetWindowPositionText();
}

void VideoWindowPositionDialog::SetWindowPositionText()
{
    StaticText_Position->SetLabel(wxString::Format("X: %d, Y: %d, W: %d, H: %d", GetPosition().x, GetPosition().y, GetSize().GetWidth(), GetSize().GetHeight()));
}

