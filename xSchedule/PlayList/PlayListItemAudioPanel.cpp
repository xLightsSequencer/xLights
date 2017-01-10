#include "PlayListItemAudioPanel.h"
#include "PlayListItemAudio.h"
#include "PlayListDialog.h"

//(*InternalHeaders(PlayListItemAudioPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemAudioPanel)
const long PlayListItemAudioPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemAudioPanel::ID_STATICTEXT5 = wxNewId();
const long PlayListItemAudioPanel::ID_CHOICE1 = wxNewId();
const long PlayListItemAudioPanel::ID_CHECKBOX1 = wxNewId();
const long PlayListItemAudioPanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemAudioPanel::ID_FILEPICKERCTRL2 = wxNewId();
const long PlayListItemAudioPanel::ID_CHECKBOX2 = wxNewId();
const long PlayListItemAudioPanel::ID_SLIDER1 = wxNewId();
const long PlayListItemAudioPanel::ID_STATICTEXT4 = wxNewId();
const long PlayListItemAudioPanel::ID_SPINCTRL1 = wxNewId();
const long PlayListItemAudioPanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemAudioPanel::ID_TEXTCTRL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemAudioPanel,wxPanel)
	//(*EventTable(PlayListItemAudioPanel)
	//*)
END_EVENT_TABLE()

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

PlayListItemAudioPanel::PlayListItemAudioPanel(wxWindow* parent, PlayListItemAudio* audio, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _audio = audio;

	//(*Initialize(PlayListItemAudioPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("FSEQ File:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FilePickerCtrl_FSEQFile = new FSEQFilePickerCtrl(this, ID_FILEPICKERCTRL1, wxEmptyString, _("FSEQ File"), _T("*.fseq"), wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL1"));
	FlexGridSizer1->Add(FilePickerCtrl_FSEQFile, 1, wxALL|wxEXPAND, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Blend Mode:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer1->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_BlendMode = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	FlexGridSizer1->Add(Choice_BlendMode, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_OverrideAudio = new wxCheckBox(this, ID_CHECKBOX1, _("Override Audio"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_OverrideAudio->SetValue(false);
	FlexGridSizer1->Add(CheckBox_OverrideAudio, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Audio File:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FilePickerCtrl_AudioFile = new AudioFilePickerCtrl(this, ID_FILEPICKERCTRL2, wxEmptyString, _("Audio File"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL2"));
	FlexGridSizer1->Add(FilePickerCtrl_AudioFile, 1, wxALL|wxEXPAND, 5);
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
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Delay:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Delay = new wxTextCtrl(this, ID_TEXTCTRL1, _("0.000"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Delay, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_FILEPICKERCTRL1,wxEVT_COMMAND_FILEPICKER_CHANGED,(wxObjectEventFunction)&PlayListItemAudioPanel::OnFilePickerCtrl1FileChanged);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&PlayListItemAudioPanel::OnCheckBox_OverrideAudioClick);
	Connect(ID_FILEPICKERCTRL2,wxEVT_COMMAND_FILEPICKER_CHANGED,(wxObjectEventFunction)&PlayListItemAudioPanel::OnFilePickerCtrl2FileChanged);
	Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&PlayListItemAudioPanel::OnCheckBox_OverrideVolumeClick);
	//*)

    FilePickerCtrl_AudioFile->SetFileName(wxFileName(audio->GetAudioFile()));
    TextCtrl_Delay->SetValue(wxString::Format(wxT("%.3f"), (float)audio->GetDelay() / 1000.0));

    if (audio->GetVolume() != -1)
    {
        CheckBox_OverrideVolume->SetValue(true);
        Slider1->SetValue(audio->GetVolume());
    }
    else
    {
        CheckBox_OverrideVolume->SetValue(false);
    }

    ValidateWindow();
}

PlayListItemAudioPanel::~PlayListItemAudioPanel()
{
	//(*Destroy(PlayListItemAudioPanel)
	//*)
    _audio->SetAudioFile(FilePickerCtrl_AudioFile->GetFileName().GetFullPath().ToStdString());
    _audio->SetDelay(wxAtof(TextCtrl_Delay->GetValue()) * 1000);

    if (CheckBox_OverrideVolume->GetValue())
    {
        _audio->SetVolume(Slider1->GetValue());
    }
    else
    {
        _audio->SetVolume(-1);
    }
}


void PlayListItemAudioPanel::OnTextCtrl_DelayText(wxCommandEvent& event)
{
}

void PlayListItemAudioPanel::OnFilePickerCtrl2FileChanged(wxFileDirPickerEvent& event)
{
   _audio->SetAudioFile(FilePickerCtrl_AudioFile->GetFileName().GetFullPath().ToStdString());
    ((PlayListDialog*)GetParent()->GetParent()->GetParent()->GetParent())->UpdateTree();
}

void PlayListItemAudioPanel::ValidateWindow()
{
    if (CheckBox_OverrideVolume->GetValue())
    {
        Slider1->Enable();
    }
    else
    {
        Slider1->Enable(false);
    }
}
