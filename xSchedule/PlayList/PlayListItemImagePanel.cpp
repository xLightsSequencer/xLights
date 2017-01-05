#include "PlayListItemImagePanel.h"
#include "PlayListItemImage.h"
#include "VideoWindowPositionDialog.h"
#include "PlayListDialog.h"

//(*InternalHeaders(PlayListItemImagePanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemImagePanel)
const long PlayListItemImagePanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemImagePanel::ID_FILEPICKERCTRL1 = wxNewId();
const long PlayListItemImagePanel::ID_BUTTON1 = wxNewId();
const long PlayListItemImagePanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemImagePanel::ID_TEXTCTRL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemImagePanel,wxPanel)
	//(*EventTable(PlayListItemImagePanel)
	//*)
END_EVENT_TABLE()

PlayListItemImagePanel::PlayListItemImagePanel(wxWindow* parent, PlayListItemImage* Image,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _Image = Image;

	//(*Initialize(PlayListItemImagePanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Image File:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FilePickerCtrl_ImageFile = new wxFilePickerCtrl(this, ID_FILEPICKERCTRL1, wxEmptyString, _("Image File"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL1"));
	FlexGridSizer1->Add(FilePickerCtrl_ImageFile, 1, wxALL|wxEXPAND, 5);
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

	Connect(ID_FILEPICKERCTRL1,wxEVT_COMMAND_FILEPICKER_CHANGED,(wxObjectEventFunction)&PlayListItemImagePanel::OnFilePickerCtrl_ImageFileFileChanged);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PlayListItemImagePanel::OnButton_PositionWindowClick);
	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemImagePanel::OnTextCtrl_DelayText);
	//*)

    FilePickerCtrl_ImageFile->SetFileName(wxFileName(Image->GetImageFile()));
    TextCtrl_Delay->SetValue(wxString::Format(wxT("%.3f"), (float)Image->GetDelay() / 1000.0));
}

PlayListItemImagePanel::~PlayListItemImagePanel()
{
	//(*Destroy(PlayListItemImagePanel)
	//*)
    _Image->SetImageFile(FilePickerCtrl_ImageFile->GetFileName().GetFullPath().ToStdString());
    _Image->SetDelay(wxAtof(TextCtrl_Delay->GetValue()) * 1000);
}


void PlayListItemImagePanel::OnButton_PositionWindowClick(wxCommandEvent& event)
{
    VideoWindowPositionDialog dlg(this, wxID_ANY, _Image->GetPosition(), _Image->GetSize());

    dlg.ShowModal();

    if (dlg.IsFullScreen())
    {
        _Image->SetLocation(dlg.GetClientAreaOrigin(), dlg.GetClientSize());
    }
    else
    {
        _Image->SetLocation(dlg.GetPosition(), dlg.GetSize());
    }
}

void PlayListItemImagePanel::OnFilePickerCtrl_ImageFileFileChanged(wxFileDirPickerEvent& event)
{
    _Image->SetImageFile(FilePickerCtrl_ImageFile->GetFileName().GetFullPath().ToStdString());
    ((PlayListDialog*)GetParent()->GetParent()->GetParent()->GetParent())->UpdateTree();
}

void PlayListItemImagePanel::OnTextCtrl_DelayText(wxCommandEvent& event)
{
}
