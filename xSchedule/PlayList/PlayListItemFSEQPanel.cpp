#include "PlayListItemFSEQPanel.h"
#include "PlayListItemFSEQ.h"
#include "PlayListDialog.h"

//(*InternalHeaders(PlayListItemFSEQPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemFSEQPanel)
const long PlayListItemFSEQPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemFSEQPanel::ID_FILEPICKERCTRL1 = wxNewId();
const long PlayListItemFSEQPanel::ID_CHECKBOX1 = wxNewId();
const long PlayListItemFSEQPanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemFSEQPanel::ID_FILEPICKERCTRL2 = wxNewId();
const long PlayListItemFSEQPanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemFSEQPanel::ID_TEXTCTRL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemFSEQPanel,wxPanel)
	//(*EventTable(PlayListItemFSEQPanel)
	//*)
END_EVENT_TABLE()

PlayListItemFSEQPanel::PlayListItemFSEQPanel(wxWindow* parent, PlayListItemFSEQ* fseq, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _fseq = fseq;

	//(*Initialize(PlayListItemFSEQPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("FSEQ File:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FilePickerCtrl_FSEQFile = new wxFilePickerCtrl(this, ID_FILEPICKERCTRL1, wxEmptyString, _("FSEQ File"), _T("*.fseq"), wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL1"));
	FlexGridSizer1->Add(FilePickerCtrl_FSEQFile, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_OverrideAudio = new wxCheckBox(this, ID_CHECKBOX1, _("Override Audio"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_OverrideAudio->SetValue(false);
	FlexGridSizer1->Add(CheckBox_OverrideAudio, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Audio File:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FilePickerCtrl_AudioFile = new wxFilePickerCtrl(this, ID_FILEPICKERCTRL2, wxEmptyString, _("Audio File"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL2"));
	FlexGridSizer1->Add(FilePickerCtrl_AudioFile, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Delay:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Delay = new wxTextCtrl(this, ID_TEXTCTRL1, _("0.000"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Delay, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)

    FilePickerCtrl_FSEQFile->SetFileName(wxFileName(fseq->GetFSEQFile()));
    FilePickerCtrl_AudioFile->SetFileName(wxFileName(fseq->GetAudioFile()));
    CheckBox_OverrideAudio->SetValue(fseq->GetOverrideAudio());
    TextCtrl_Delay->SetValue(wxString::Format(wxT("%.3f"), (float)fseq->GetDelay() / 1000.0));

    ValidateWindow();
}

PlayListItemFSEQPanel::~PlayListItemFSEQPanel()
{
	//(*Destroy(PlayListItemFSEQPanel)
	//*)
    _fseq->SetFSEQFile(FilePickerCtrl_FSEQFile->GetFileName().GetFullPath().ToStdString());
    _fseq->SetAudioFile(FilePickerCtrl_AudioFile->GetFileName().GetFullPath().ToStdString());
    _fseq->SetOverrideAudio(CheckBox_OverrideAudio->GetValue());
    _fseq->SetDelay(wxAtof(TextCtrl_Delay->GetValue()) * 1000);
}


void PlayListItemFSEQPanel::OnTextCtrl_DelayText(wxCommandEvent& event)
{
}

void PlayListItemFSEQPanel::OnFilePickerCtrl1FileChanged(wxFileDirPickerEvent& event)
{
    _fseq->SetFSEQFile(FilePickerCtrl_FSEQFile->GetFileName().GetFullPath().ToStdString());
    ((PlayListDialog*)GetParent()->GetParent()->GetParent()->GetParent())->UpdateTree();
}

void PlayListItemFSEQPanel::OnFilePickerCtrl2FileChanged(wxFileDirPickerEvent& event)
{
}

void PlayListItemFSEQPanel::OnCheckBox_OverrideAudioClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void PlayListItemFSEQPanel::ValidateWindow()
{
    if (CheckBox_OverrideAudio->GetValue())
    {
        FilePickerCtrl_AudioFile->Enable(true);
    }
    else
    {
        FilePickerCtrl_AudioFile->Enable(false);
    }
}