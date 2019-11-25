#include "PlayListItemESEQPanel.h"
#include "PlayListItemESEQ.h"
#include "PlayListDialog.h"

//(*InternalHeaders(PlayListItemESEQPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemESEQPanel)
const long PlayListItemESEQPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemESEQPanel::ID_FILEPICKERCTRL1 = wxNewId();
const long PlayListItemESEQPanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemESEQPanel::ID_CHOICE1 = wxNewId();
const long PlayListItemESEQPanel::ID_STATICTEXT4 = wxNewId();
const long PlayListItemESEQPanel::ID_SPINCTRL1 = wxNewId();
const long PlayListItemESEQPanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemESEQPanel::ID_TEXTCTRL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemESEQPanel,wxPanel)
	//(*EventTable(PlayListItemESEQPanel)
	//*)
END_EVENT_TABLE()

class ESEQFilePickerCtrl : public wxFilePickerCtrl
{
#define ESEQFILES "ESEQ files|*.eseq|All files (*.*)|*.*"

public:
    ESEQFilePickerCtrl(wxWindow *parent,
        wxWindowID id,
        const wxString& path = wxEmptyString,
        const wxString& message = wxFileSelectorPromptStr,
        const wxString& wildcard = wxFileSelectorDefaultWildcardStr,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxFLP_DEFAULT_STYLE,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxFilePickerCtrlNameStr) :
        wxFilePickerCtrl(parent, id, path, message, ESEQFILES, pos, size, style, validator, name)
    {}
    virtual ~ESEQFilePickerCtrl() {}
};

PlayListItemESEQPanel::PlayListItemESEQPanel(wxWindow* parent, PlayListItemESEQ* ESEQ, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _ESEQ = ESEQ;

	//(*Initialize(PlayListItemESEQPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("ESEQ File:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FilePickerCtrl_ESEQFile = new ESEQFilePickerCtrl(this, ID_FILEPICKERCTRL1, wxEmptyString, _("ESEQ File"), _T("*.ESEQ"), wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL1"));
	FlexGridSizer1->Add(FilePickerCtrl_ESEQFile, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Blend Mode:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_BlendMode = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	FlexGridSizer1->Add(Choice_BlendMode, 1, wxALL|wxEXPAND, 5);
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

	Connect(ID_FILEPICKERCTRL1,wxEVT_COMMAND_FILEPICKER_CHANGED,(wxObjectEventFunction)&PlayListItemESEQPanel::OnFilePickerCtrl1FileChanged);
	//*)

    auto bms = ESEQFile::GetBlendModes();
    for (auto it = bms.begin(); it != bms.end(); ++it)
    {
        Choice_BlendMode->AppendString(*it);
    }

    FilePickerCtrl_ESEQFile->SetFileName(wxFileName(ESEQ->GetESEQFileName()));
    TextCtrl_Delay->SetValue(wxString::Format(wxT("%.3f"), (float)ESEQ->GetDelay() / 1000.0));
    Choice_BlendMode->SetSelection(ESEQ->GetBlendMode());
    SpinCtrl_Priority->SetValue(ESEQ->GetPriority());

    ValidateWindow();
}

PlayListItemESEQPanel::~PlayListItemESEQPanel()
{
	//(*Destroy(PlayListItemESEQPanel)
	//*)
    _ESEQ->SetESEQFileName(FilePickerCtrl_ESEQFile->GetFileName().GetFullPath().ToStdString());
    _ESEQ->SetDelay(wxAtof(TextCtrl_Delay->GetValue()) * 1000);
    _ESEQ->SetBlendMode(Choice_BlendMode->GetStringSelection().ToStdString());
    _ESEQ->SetPriority(SpinCtrl_Priority->GetValue());
}


void PlayListItemESEQPanel::OnTextCtrl_DelayText(wxCommandEvent& event)
{
}

void PlayListItemESEQPanel::OnFilePickerCtrl1FileChanged(wxFileDirPickerEvent& event)
{
    _ESEQ->SetESEQFileName(FilePickerCtrl_ESEQFile->GetFileName().GetFullPath().ToStdString());
    wxCommandEvent e(EVT_UPDATEITEMNAME);
    wxPostEvent(GetParent()->GetParent()->GetParent()->GetParent(), e);
}

void PlayListItemESEQPanel::ValidateWindow()
{
}
