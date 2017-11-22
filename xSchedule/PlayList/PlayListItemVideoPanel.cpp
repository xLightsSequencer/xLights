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
const long PlayListItemVideoPanel::ID_CHECKBOX1 = wxNewId();
const long PlayListItemVideoPanel::ID_CHECKBOX2 = wxNewId();
const long PlayListItemVideoPanel::ID_CHECKBOX3 = wxNewId();
const long PlayListItemVideoPanel::ID_CHECKBOX4 = wxNewId();
const long PlayListItemVideoPanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemVideoPanel::ID_SPINCTRL1 = wxNewId();
const long PlayListItemVideoPanel::ID_STATICTEXT4 = wxNewId();
const long PlayListItemVideoPanel::ID_SPINCTRL2 = wxNewId();
const long PlayListItemVideoPanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemVideoPanel::ID_TEXTCTRL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemVideoPanel,wxPanel)
	//(*EventTable(PlayListItemVideoPanel)
	//*)
END_EVENT_TABLE()

class VideoFilePickerCtrl : public wxFilePickerCtrl
{
#define VIDEOFILES "Video Files|*.avi;*.mp4;*.mkv;*.mov;*.asf;*.flv;*.mpg;*.mpeg;*.m4v|All Files|*.*"

public:
    VideoFilePickerCtrl(wxWindow *parent,
        wxWindowID id,
        const wxString& path = wxEmptyString,
        const wxString& message = wxFileSelectorPromptStr,
        const wxString& wildcard = wxFileSelectorDefaultWildcardStr,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxFLP_DEFAULT_STYLE,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxFilePickerCtrlNameStr) :
        wxFilePickerCtrl(parent, id, path, message, VIDEOFILES, pos, size, style, validator, name)
    {}
    virtual ~VideoFilePickerCtrl() {}
};

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
	FilePickerCtrl_VideoFile = new VideoFilePickerCtrl(this, ID_FILEPICKERCTRL1, wxEmptyString, _("Video File"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL1"));
	FlexGridSizer1->Add(FilePickerCtrl_VideoFile, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_PositionWindow = new wxButton(this, ID_BUTTON1, _("Position Window"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer1->Add(Button_PositionWindow, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Topmost = new wxCheckBox(this, ID_CHECKBOX1, _("Topmost Window"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_Topmost->SetValue(true);
	FlexGridSizer1->Add(CheckBox_Topmost, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_SuppressVirtualMatrix = new wxCheckBox(this, ID_CHECKBOX2, _("Suppress Virtual Matrix"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	CheckBox_SuppressVirtualMatrix->SetValue(false);
	FlexGridSizer1->Add(CheckBox_SuppressVirtualMatrix, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_CacheVideo = new wxCheckBox(this, ID_CHECKBOX3, _("Cache Video"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
	CheckBox_CacheVideo->SetValue(false);
	FlexGridSizer1->Add(CheckBox_CacheVideo, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_LoopVideo = new wxCheckBox(this, ID_CHECKBOX4, _("Loop Video"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX4"));
	CheckBox_LoopVideo->SetValue(false);
	FlexGridSizer1->Add(CheckBox_LoopVideo, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Fade In MS:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_FadeIn = new wxSpinCtrl(this, ID_SPINCTRL1, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 10000, 0, _T("ID_SPINCTRL1"));
	SpinCtrl_FadeIn->SetValue(_T("0"));
	FlexGridSizer1->Add(SpinCtrl_FadeIn, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Fade Out MS:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_FadeOut = new wxSpinCtrl(this, ID_SPINCTRL2, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 10000, 0, _T("ID_SPINCTRL2"));
	SpinCtrl_FadeOut->SetValue(_T("0"));
	FlexGridSizer1->Add(SpinCtrl_FadeOut, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Delay:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Delay = new wxTextCtrl(this, ID_TEXTCTRL1, _("0.000"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Delay, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_FILEPICKERCTRL1,wxEVT_COMMAND_FILEPICKER_CHANGED,(wxObjectEventFunction)&PlayListItemVideoPanel::OnFilePickerCtrl_VideoFileFileChanged);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PlayListItemVideoPanel::OnButton_PositionWindowClick);
	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemVideoPanel::OnTextCtrl_DelayText);
	//*)

    FilePickerCtrl_VideoFile->SetFileName(wxFileName(video->GetVideoFile()));
    TextCtrl_Delay->SetValue(wxString::Format(wxT("%.3f"), (float)video->GetDelay() / 1000.0));
    CheckBox_Topmost->SetValue(video->GetTopMost());
    CheckBox_CacheVideo->SetValue(video->GetCacheVideo());
    CheckBox_LoopVideo->SetValue(video->GetLoopVideo());
    CheckBox_SuppressVirtualMatrix->SetValue(video->GetSuppressVirtualMatrix());
    SpinCtrl_FadeIn->SetValue(video->GetFadeInMS());
    SpinCtrl_FadeOut->SetValue(video->GetFadeOutMS());
}

PlayListItemVideoPanel::~PlayListItemVideoPanel()
{
	//(*Destroy(PlayListItemVideoPanel)
	//*)
    _video->SetVideoFile(FilePickerCtrl_VideoFile->GetFileName().GetFullPath().ToStdString());
    _video->SetDelay(wxAtof(TextCtrl_Delay->GetValue()) * 1000);
    _video->SetTopmost(CheckBox_Topmost->GetValue());
    _video->SetCacheVideo(CheckBox_CacheVideo->GetValue());
    _video->SetLoopVideo(CheckBox_LoopVideo->GetValue());
    _video->SetSuppressVirtualMatrix(CheckBox_SuppressVirtualMatrix->GetValue());
    _video->SetFadeInMS(SpinCtrl_FadeIn->GetValue());
    _video->SetFadeOutMS(SpinCtrl_FadeOut->GetValue());
}

void PlayListItemVideoPanel::OnButton_PositionWindowClick(wxCommandEvent& event)
{
    VideoWindowPositionDialog dlg(this, wxID_ANY, _video->GetPosition(), _video->GetSize());

    dlg.ShowModal();

    if (dlg.IsFullScreen())
    {
        _video->SetLocation(dlg.GetClientAreaOrigin(), dlg.GetClientSize());
    }
    else
    {
        _video->SetLocation(dlg.GetPosition(), dlg.GetSize());
    }
}

void PlayListItemVideoPanel::OnFilePickerCtrl_VideoFileFileChanged(wxFileDirPickerEvent& event)
{
    _video->SetVideoFile(FilePickerCtrl_VideoFile->GetFileName().GetFullPath().ToStdString());
    ((PlayListDialog*)GetParent()->GetParent()->GetParent()->GetParent())->UpdateTree();
}

void PlayListItemVideoPanel::OnTextCtrl_DelayText(wxCommandEvent& event)
{
}
