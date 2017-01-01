#include "PlayListItemVideoPanel.h"
#include "PlayListItemVideo.h"
#include "VideoWindowPositionDialog.h"
#include "PlayListDialog.h"

//(*InternalHeaders(PlayListItemVideoPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemVideoPanel)
const long PlayListItemVideoPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemVideoPanel::ID_FILEPICKERCTRL1 = wxNewId();
const long PlayListItemVideoPanel::ID_BUTTON1 = wxNewId();
const long PlayListItemVideoPanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemVideoPanel::ID_TEXTCTRL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemVideoPanel,wxPanel)
	//(*EventTable(PlayListItemVideoPanel)
	//*)
END_EVENT_TABLE()

PlayListItemVideoPanel::PlayListItemVideoPanel(wxWindow* parent, PlayListItemVideo* video,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _video = video;

	//(*Initialize(PlayListItemVideoPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Video File:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FilePickerCtrl_VideoFile = new wxFilePickerCtrl(this, ID_FILEPICKERCTRL1, wxEmptyString, _("Video File"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL1"));
	FlexGridSizer1->Add(FilePickerCtrl_VideoFile, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_PositionWindow = new wxButton(this, ID_BUTTON1, _("Position Window"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer1->Add(Button_PositionWindow, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Delay:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Delay = new wxTextCtrl(this, ID_TEXTCTRL1, _("0.000"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Delay, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PlayListItemVideoPanel::OnButton_PositionWindowClick);
	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemVideoPanel::OnTextCtrl_DelayText);
	//*)

    FilePickerCtrl_VideoFile->SetFileName(wxFileName(video->GetVideoFile()));
    TextCtrl_Delay->SetValue(wxString::Format(wxT("%.3f"), (float)video->GetDelay() / 1000.0));
}

PlayListItemVideoPanel::~PlayListItemVideoPanel()
{
	//(*Destroy(PlayListItemVideoPanel)
	//*)
    _video->SetVideoFile(FilePickerCtrl_VideoFile->GetFileName().GetFullPath().ToStdString());
    _video->SetDelay(wxAtof(TextCtrl_Delay->GetValue()) * 1000);
}


void PlayListItemVideoPanel::OnButton_PositionWindowClick(wxCommandEvent& event)
{
    VideoWindowPositionDialog dlg(this, wxID_ANY, _video->GetPosition(), _video->GetSize());

    dlg.ShowModal();

    _video->SetLocation(dlg.GetPosition(), dlg.GetSize());
}

void PlayListItemVideoPanel::OnFilePickerCtrl_VideoFileFileChanged(wxFileDirPickerEvent& event)
{
    _video->SetVideoFile(FilePickerCtrl_VideoFile->GetFileName().GetFullPath().ToStdString());
    ((PlayListDialog*)GetParent()->GetParent()->GetParent()->GetParent())->UpdateTree();
}

void PlayListItemVideoPanel::OnTextCtrl_DelayText(wxCommandEvent& event)
{
}
