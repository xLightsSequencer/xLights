#include "PlayListItemFSEQVideoPanel.h"
#include "PlayListItemFSEQVideo.h"
#include "PlayListDialog.h"
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"
#include "VideoWindowPositionDialog.h"
#include "../../xLights/outputs/OutputManager.h"

//(*InternalHeaders(PlayListItemFSEQVideoPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemFSEQVideoPanel)
const long PlayListItemFSEQVideoPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_FILEPICKERCTRL1 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_STATICTEXT8 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_FILEPICKERCTRL3 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_BUTTON1 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_STATICTEXT11 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_CHECKBOX5 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_CHECKBOX6 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_STATICTEXT5 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_CHOICE1 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_CHECKBOX3 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_STATICTEXT6 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_TEXTCTRL2 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_STATICTEXT12 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_STATICTEXT7 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_SPINCTRL3 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_CHECKBOX1 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_FILEPICKERCTRL2 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_CHECKBOX4 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_CHECKBOX7 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_CHECKBOX8 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_CHECKBOX2 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_SLIDER1 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_STATICTEXT4 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_SPINCTRL1 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_STATICTEXT10 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_SPINCTRL4 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_STATICTEXT9 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_SPINCTRL5 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemFSEQVideoPanel::ID_TEXTCTRL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemFSEQVideoPanel,wxPanel)
	//(*EventTable(PlayListItemFSEQVideoPanel)
	//*)
END_EVENT_TABLE()

class FSEQFilePickerCtrl : public wxFilePickerCtrl
{
#define FSEQFILES "FSEQ files|*.fseq|All files (*.*)|*.*"

public:
    FSEQFilePickerCtrl(wxWindow *parent,
        wxWindowID id,
        const wxString& path = wxEmptyString,
        const wxString& message = wxFileSelectorPromptStr,
        const wxString& wildcard = wxFileSelectorDefaultWildcardStr,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxFLP_DEFAULT_STYLE,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxFilePickerCtrlNameStr) :
        wxFilePickerCtrl(parent, id, path, message, FSEQFILES, pos, size, style, validator, name)
    {}
    virtual ~FSEQFilePickerCtrl() {}
};

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

class AudioFilePickerCtrl : public wxFilePickerCtrl
{
#define AUDIOFILES "Audio files|*.mp3;*.ogg;*.m4p;*.mp4;*.avi;*.wma;*.au;*.wav;*.m4a;*.mid;*.mkv;*.mov;*.mpg;*.asf;*.flv;*.mpeg|All Files|*.*"

public:
    AudioFilePickerCtrl(wxWindow *parent,
        wxWindowID id,
        const wxString& path = wxEmptyString,
        const wxString& message = wxFileSelectorPromptStr,
        const wxString& wildcard = wxFileSelectorDefaultWildcardStr,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxFLP_DEFAULT_STYLE,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxFilePickerCtrlNameStr) :
        wxFilePickerCtrl(parent, id, path, message, AUDIOFILES, pos, size, style, validator, name)
    {}
    virtual ~AudioFilePickerCtrl() {}
};

PlayListItemFSEQVideoPanel::PlayListItemFSEQVideoPanel(wxWindow* parent, OutputManager* outputManager, PlayListItemFSEQVideo* fseq, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _outputManager = outputManager;
    _fseq = fseq;

	//(*Initialize(PlayListItemFSEQVideoPanel)
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("FSEQ File:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FilePickerCtrl_FSEQFile = new FSEQFilePickerCtrl(this, ID_FILEPICKERCTRL1, wxEmptyString, _("FSEQ File"), _T("*.fseq"), wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL1"));
	FlexGridSizer1->Add(FilePickerCtrl_FSEQFile, 1, wxALL|wxEXPAND, 5);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("Video File:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer1->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FilePickerCtrl_VideoFile = new VideoFilePickerCtrl(this, ID_FILEPICKERCTRL3, wxEmptyString, _("Video File"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL3"));
	FlexGridSizer1->Add(FilePickerCtrl_VideoFile, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_PositionWindow = new wxButton(this, ID_BUTTON1, _("Position Window"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer1->Add(Button_PositionWindow, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_Position = new wxStaticText(this, ID_STATICTEXT11, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
	FlexGridSizer1->Add(StaticText_Position, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Topmost = new wxCheckBox(this, ID_CHECKBOX5, _("Topmost Window"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX5"));
	CheckBox_Topmost->SetValue(true);
	FlexGridSizer1->Add(CheckBox_Topmost, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_SuppressVirtualMatrix = new wxCheckBox(this, ID_CHECKBOX6, _("Suppress Virtual Matrix"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX6"));
	CheckBox_SuppressVirtualMatrix->SetValue(false);
	FlexGridSizer1->Add(CheckBox_SuppressVirtualMatrix, 1, wxALL|wxEXPAND, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Blend Mode:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer1->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_BlendMode = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	FlexGridSizer1->Add(Choice_BlendMode, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_LimitChannels = new wxCheckBox(this, ID_CHECKBOX3, _("Limit Channels"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
	CheckBox_LimitChannels->SetValue(false);
	FlexGridSizer1->Add(CheckBox_LimitChannels, 1, wxALL|wxEXPAND, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Start Channel"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer1->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	TextCtrl_StartChannel = new wxTextCtrl(this, ID_TEXTCTRL2, _("1"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer2->Add(TextCtrl_StartChannel, 1, wxALL|wxEXPAND, 5);
	StaticText_StartChannel = new wxStaticText(this, ID_STATICTEXT12, _("1"), wxDefaultPosition, wxSize(60,-1), 0, _T("ID_STATICTEXT12"));
	FlexGridSizer2->Add(StaticText_StartChannel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Channels"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer1->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Channels = new wxSpinCtrl(this, ID_SPINCTRL3, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 100, 0, _T("ID_SPINCTRL3"));
	SpinCtrl_Channels->SetValue(_T("0"));
	FlexGridSizer1->Add(SpinCtrl_Channels, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_OverrideAudio = new wxCheckBox(this, ID_CHECKBOX1, _("Override Audio"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_OverrideAudio->SetValue(false);
	FlexGridSizer1->Add(CheckBox_OverrideAudio, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Audio File:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FilePickerCtrl_AudioFile = new AudioFilePickerCtrl(this, ID_FILEPICKERCTRL2, wxEmptyString, _("Audio File"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL2"));
	FlexGridSizer1->Add(FilePickerCtrl_AudioFile, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_FastStartAudio = new wxCheckBox(this, ID_CHECKBOX4, _("Fast start audio"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX4"));
	CheckBox_FastStartAudio->SetValue(false);
	FlexGridSizer1->Add(CheckBox_FastStartAudio, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_CacheVideo = new wxCheckBox(this, ID_CHECKBOX7, _("Cache Video"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX7"));
	CheckBox_CacheVideo->SetValue(false);
	FlexGridSizer1->Add(CheckBox_CacheVideo, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_LoopVideo = new wxCheckBox(this, ID_CHECKBOX8, _("Loop video"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX8"));
	CheckBox_LoopVideo->SetValue(false);
	FlexGridSizer1->Add(CheckBox_LoopVideo, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_OverrideVolume = new wxCheckBox(this, ID_CHECKBOX2, _("Override Volume"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	CheckBox_OverrideVolume->SetValue(false);
	FlexGridSizer1->Add(CheckBox_OverrideVolume, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider1 = new wxSlider(this, ID_SLIDER1, 100, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER1"));
	FlexGridSizer1->Add(Slider1, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Priority:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Priority = new wxSpinCtrl(this, ID_SPINCTRL1, _T("5"), wxDefaultPosition, wxDefaultSize, 0, 1, 10, 5, _T("ID_SPINCTRL1"));
	SpinCtrl_Priority->SetValue(_T("5"));
	FlexGridSizer1->Add(SpinCtrl_Priority, 1, wxALL|wxEXPAND, 5);
	StaticText10 = new wxStaticText(this, ID_STATICTEXT10, _("Fade In MS:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
	FlexGridSizer1->Add(StaticText10, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_FadeIn = new wxSpinCtrl(this, ID_SPINCTRL4, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 10000, 0, _T("ID_SPINCTRL4"));
	SpinCtrl_FadeIn->SetValue(_T("0"));
	FlexGridSizer1->Add(SpinCtrl_FadeIn, 1, wxALL|wxEXPAND, 5);
	StaticText9 = new wxStaticText(this, ID_STATICTEXT9, _("Fade Out MS:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
	FlexGridSizer1->Add(StaticText9, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_FadeOut = new wxSpinCtrl(this, ID_SPINCTRL5, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 10000, 0, _T("ID_SPINCTRL5"));
	SpinCtrl_FadeOut->SetValue(_T("0"));
	FlexGridSizer1->Add(SpinCtrl_FadeOut, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Delay:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Delay = new wxTextCtrl(this, ID_TEXTCTRL1, _("0.000"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Delay, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_FILEPICKERCTRL1,wxEVT_COMMAND_FILEPICKER_CHANGED,(wxObjectEventFunction)&PlayListItemFSEQVideoPanel::OnFilePickerCtrl1FileChanged);
	Connect(ID_FILEPICKERCTRL3,wxEVT_COMMAND_FILEPICKER_CHANGED,(wxObjectEventFunction)&PlayListItemFSEQVideoPanel::OnFilePickerCtrl_VideoFileFileChanged);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PlayListItemFSEQVideoPanel::OnButton_PositionWindowClick);
	Connect(ID_CHECKBOX3,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&PlayListItemFSEQVideoPanel::OnCheckBox_LimitChannelsClick);
	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemFSEQVideoPanel::OnTextCtrl_StartChannelText);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&PlayListItemFSEQVideoPanel::OnCheckBox_OverrideAudioClick);
	Connect(ID_FILEPICKERCTRL2,wxEVT_COMMAND_FILEPICKER_CHANGED,(wxObjectEventFunction)&PlayListItemFSEQVideoPanel::OnFilePickerCtrl2FileChanged);
	Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&PlayListItemFSEQVideoPanel::OnCheckBox_OverrideVolumeClick);
	//*)

    PopulateBlendModes(Choice_BlendMode);
    Choice_BlendMode->SetSelection(0);

    FilePickerCtrl_FSEQFile->SetFileName(wxFileName(fseq->GetFSEQFileName()));
    FilePickerCtrl_AudioFile->SetFileName(wxFileName(fseq->GetAudioFile()));
    CheckBox_OverrideAudio->SetValue(fseq->GetOverrideAudio());
    TextCtrl_Delay->SetValue(wxString::Format(wxT("%.3f"), (float)fseq->GetDelay() / 1000.0));
    Choice_BlendMode->SetSelection(fseq->GetBlendMode());
    SpinCtrl_Priority->SetValue(fseq->GetPriority());
    CheckBox_FastStartAudio->SetValue(fseq->GetFastStartAudio());
    CheckBox_CacheVideo->SetValue(fseq->GetCacheVideo());
    FilePickerCtrl_VideoFile->SetFileName(wxFileName(fseq->GetVideoFile()));
    CheckBox_Topmost->SetValue(fseq->GetTopMost());
    CheckBox_SuppressVirtualMatrix->SetValue(fseq->GetSuppressVirtualMatrix());
    SpinCtrl_FadeIn->SetValue(fseq->GetFadeInMS());
    SpinCtrl_FadeOut->SetValue(fseq->GetFadeOutMS());

    if (fseq->GetVolume() != -1)
    {
        CheckBox_OverrideVolume->SetValue(true);
        Slider1->SetValue(fseq->GetVolume());
    }
    else
    {
        CheckBox_OverrideVolume->SetValue(false);
    }

    long channels = xScheduleFrame::GetScheduleManager()->GetTotalChannels();
    SpinCtrl_Channels->SetRange(1, channels);

    TextCtrl_StartChannel->SetValue(fseq->GetStartChannel());
    int chs = fseq->GetChannels();
    if (chs == 0)
    {
        CheckBox_LimitChannels->SetValue(false);
    }
    else
    {
        CheckBox_LimitChannels->SetValue(true);
        SpinCtrl_Channels->SetValue(fseq->GetChannels());
    }

    if (!CheckBox_OverrideAudio->GetValue())
    {
        std::string f = FSEQFile::GrabAudioFilename(FilePickerCtrl_FSEQFile->GetFileName().GetFullPath().ToStdString());
        FilePickerCtrl_AudioFile->SetFileName(wxFileName(f));
        FilePickerCtrl_AudioFile->SetToolTip(f);
    }

    SetWindowPositionText();

    ValidateWindow();
}

PlayListItemFSEQVideoPanel::~PlayListItemFSEQVideoPanel()
{
	//(*Destroy(PlayListItemFSEQVideoPanel)
	//*)
    _fseq->SetFSEQFileName(FilePickerCtrl_FSEQFile->GetFileName().GetFullPath().ToStdString());
    _fseq->SetAudioFile(FilePickerCtrl_AudioFile->GetFileName().GetFullPath().ToStdString());
    _fseq->SetOverrideAudio(CheckBox_OverrideAudio->GetValue());
    _fseq->SetDelay(wxAtof(TextCtrl_Delay->GetValue()) * 1000);
    _fseq->SetBlendMode(Choice_BlendMode->GetStringSelection().ToStdString());
    _fseq->SetPriority(SpinCtrl_Priority->GetValue());
    _fseq->SetFastStartAudio(CheckBox_FastStartAudio->GetValue());
    _fseq->SetCacheVideo(CheckBox_CacheVideo->GetValue());
    _fseq->SetLoopVideo(CheckBox_LoopVideo->GetValue());
    _fseq->SetLoopVideo(CheckBox_LoopVideo->GetValue());
    _fseq->SetVideoFile(FilePickerCtrl_VideoFile->GetFileName().GetFullPath().ToStdString());
    _fseq->SetTopmost(CheckBox_Topmost->GetValue());
    _fseq->SetSuppressVirtualMatrix(CheckBox_SuppressVirtualMatrix->GetValue());
    _fseq->SetFadeInMS(SpinCtrl_FadeIn->GetValue());
    _fseq->SetFadeOutMS(SpinCtrl_FadeOut->GetValue());

    if (CheckBox_OverrideVolume->GetValue())
    {
        _fseq->SetVolume(Slider1->GetValue());
    }
    else
    {
        _fseq->SetVolume(-1);
    }
    if (CheckBox_LimitChannels->GetValue())
    {
        _fseq->SetStartChannel(TextCtrl_StartChannel->GetValue().ToStdString());
        _fseq->SetChannels(SpinCtrl_Channels->GetValue());
    }
    else
    {
        _fseq->SetStartChannel("1");
        _fseq->SetChannels(0);
    }
}

void PlayListItemFSEQVideoPanel::OnTextCtrl_DelayText(wxCommandEvent& event)
{
}

void PlayListItemFSEQVideoPanel::OnFilePickerCtrl1FileChanged(wxFileDirPickerEvent& event)
{
    _fseq->SetFSEQFileName(FilePickerCtrl_FSEQFile->GetFileName().GetFullPath().ToStdString());
    ((PlayListDialog*)GetParent()->GetParent()->GetParent()->GetParent())->UpdateTree();

    if (!CheckBox_OverrideAudio->GetValue())
    {
        std::string f = FSEQFile::GrabAudioFilename(FilePickerCtrl_FSEQFile->GetFileName().GetFullPath().ToStdString());
        FilePickerCtrl_AudioFile->SetFileName(wxFileName(f));
        FilePickerCtrl_AudioFile->SetToolTip(f);
    }
}

void PlayListItemFSEQVideoPanel::OnFilePickerCtrl2FileChanged(wxFileDirPickerEvent& event)
{
}

void PlayListItemFSEQVideoPanel::OnCheckBox_OverrideAudioClick(wxCommandEvent& event)
{
    if (!CheckBox_OverrideAudio->GetValue())
    {
        std::string f = FSEQFile::GrabAudioFilename(FilePickerCtrl_FSEQFile->GetFileName().GetFullPath().ToStdString());
        FilePickerCtrl_AudioFile->SetFileName(wxFileName(f));
        FilePickerCtrl_AudioFile->SetToolTip(f);
    }

    ValidateWindow();
}

void PlayListItemFSEQVideoPanel::ValidateWindow()
{
    if (CheckBox_OverrideAudio->GetValue())
    {
        FilePickerCtrl_AudioFile->Enable(true);
    }
    else
    {
        FilePickerCtrl_AudioFile->Enable(false);
    }

    if (CheckBox_OverrideVolume->GetValue())
    {
        Slider1->Enable();
    }
    else
    {
        Slider1->Enable(false);
    }

    if (CheckBox_LimitChannels->GetValue())
    {
        TextCtrl_StartChannel->Enable();
        SpinCtrl_Channels->Enable();
    }
    else
    {
        TextCtrl_StartChannel->Enable(false);
        SpinCtrl_Channels->Enable(false);
    }

    if (FilePickerCtrl_AudioFile->GetFileName() == "")
    {
        CheckBox_FastStartAudio->Enable(false);
        CheckBox_FastStartAudio->SetValue(false);
    }
    else
    {
        CheckBox_FastStartAudio->Enable();
    }
    StaticText_StartChannel->SetLabel(wxString::Format("%ld", _outputManager->DecodeStartChannel(TextCtrl_StartChannel->GetValue().ToStdString())));
}

void PlayListItemFSEQVideoPanel::OnCheckBox_OverrideVolumeClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void PlayListItemFSEQVideoPanel::OnCheckBox_LimitChannelsClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void PlayListItemFSEQVideoPanel::OnFilePickerCtrl_VideoFileFileChanged(wxFileDirPickerEvent& event)
{
}

void PlayListItemFSEQVideoPanel::OnButton_PositionWindowClick(wxCommandEvent& event)
{
    VideoWindowPositionDialog dlg(this, wxID_ANY, _fseq->GetPosition(), _fseq->GetSize());

    dlg.ShowModal();

    if (dlg.IsFullScreen())
    {
        _fseq->SetLocation(dlg.GetClientAreaOrigin(), dlg.GetClientSize());
    }
    else
    {
        _fseq->SetLocation(dlg.GetPosition(), dlg.GetSize());
    }

    SetWindowPositionText();
}

void PlayListItemFSEQVideoPanel::SetWindowPositionText()
{
    StaticText_Position->SetLabel(wxString::Format("X: %d, Y: %d, W: %d, H: %d", _fseq->GetPosition().x, _fseq->GetPosition().y, _fseq->GetSize().GetWidth(), _fseq->GetSize().GetHeight()));
}

void PlayListItemFSEQVideoPanel::OnTextCtrl_StartChannelText(wxCommandEvent& event)
{
    ValidateWindow();
}
