/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "PlayListItemAudioPanel.h"
#include "PlayListItemAudio.h"
#include "PlayListDialog.h"
#include "PlayListSimpleDialog.h"

//(*InternalHeaders(PlayListItemAudioPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemAudioPanel)
const long PlayListItemAudioPanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemAudioPanel::ID_FILEPICKERCTRL2 = wxNewId();
const long PlayListItemAudioPanel::ID_CHECKBOX2 = wxNewId();
const long PlayListItemAudioPanel::ID_SLIDER1 = wxNewId();
const long PlayListItemAudioPanel::ID_CHECKBOX1 = wxNewId();
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
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_FastStartAudio = new wxCheckBox(this, ID_CHECKBOX1, _("Fast Start Audio"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_FastStartAudio->SetValue(false);
	FlexGridSizer1->Add(CheckBox_FastStartAudio, 1, wxALL|wxEXPAND, 5);
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

	Connect(ID_FILEPICKERCTRL2,wxEVT_COMMAND_FILEPICKER_CHANGED,(wxObjectEventFunction)&PlayListItemAudioPanel::OnFilePickerCtrl2FileChanged);
	Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&PlayListItemAudioPanel::OnCheckBox_OverrideVolumeClick);
	//*)

    FilePickerCtrl_AudioFile->SetFileName(wxFileName(audio->GetAudioFile()));
    TextCtrl_Delay->SetValue(wxString::Format(wxT("%.3f"), (float)audio->GetDelay() / 1000.0));
    CheckBox_FastStartAudio->SetValue(audio->GetFastStartAudio());

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
    _audio->SetDelay(wxAtof(TextCtrl_Delay->GetValue()) * 1000);
    _audio->SetFastStartAudio(CheckBox_FastStartAudio->GetValue());
    if (CheckBox_OverrideVolume->GetValue())
    {
        _audio->SetVolume(Slider1->GetValue());
    }
    else
    {
        _audio->SetVolume(-1);
    }
    _audio->SetAudioFile(FilePickerCtrl_AudioFile->GetFileName().GetFullPath().ToStdString());
}


void PlayListItemAudioPanel::OnTextCtrl_DelayText(wxCommandEvent& event)
{
}

void PlayListItemAudioPanel::OnFilePickerCtrl2FileChanged(wxFileDirPickerEvent& event)
{
   _audio->SetAudioFile(FilePickerCtrl_AudioFile->GetFileName().GetFullPath().ToStdString());
   wxCommandEvent e(EVT_UPDATEITEMNAME);
   wxPostEvent(GetParent()->GetParent()->GetParent()->GetParent(), e);
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

void PlayListItemAudioPanel::OnCheckBox_OverrideVolumeClick(wxCommandEvent& event)
{
    ValidateWindow();
}
