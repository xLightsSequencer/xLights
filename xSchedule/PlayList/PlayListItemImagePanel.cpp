/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

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
const long PlayListItemImagePanel::ID_STATICTEXT4 = wxNewId();
const long PlayListItemImagePanel::ID_CHECKBOX1 = wxNewId();
const long PlayListItemImagePanel::ID_CHECKBOX2 = wxNewId();
const long PlayListItemImagePanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemImagePanel::ID_TEXTCTRL2 = wxNewId();
const long PlayListItemImagePanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemImagePanel::ID_TEXTCTRL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemImagePanel,wxPanel)
	//(*EventTable(PlayListItemImagePanel)
	//*)
END_EVENT_TABLE()

class ImageFilePickerCtrl : public wxFilePickerCtrl
{
#define IMAGEFILES "Image Files|*.png;*.bmp;*.jpg;*.gif|All Files|*.*"

public:
    ImageFilePickerCtrl(wxWindow *parent,
        wxWindowID id,
        const wxString& path = wxEmptyString,
        const wxString& message = wxFileSelectorPromptStr,
        const wxString& wildcard = wxFileSelectorDefaultWildcardStr,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxFLP_DEFAULT_STYLE,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxFilePickerCtrlNameStr) :
        wxFilePickerCtrl(parent, id, path, message, IMAGEFILES, pos, size, style, validator, name)
    {}
    virtual ~ImageFilePickerCtrl() {}
};

PlayListItemImagePanel::PlayListItemImagePanel(wxWindow* parent, PlayListItemImage* Image,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _image = Image;

	//(*Initialize(PlayListItemImagePanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Image File:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FilePickerCtrl_ImageFile = new ImageFilePickerCtrl(this, ID_FILEPICKERCTRL1, wxEmptyString, _("Image File"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL1"));
	FlexGridSizer1->Add(FilePickerCtrl_ImageFile, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_PositionWindow = new wxButton(this, ID_BUTTON1, _("Position Window"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer1->Add(Button_PositionWindow, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_Position = new wxStaticText(this, ID_STATICTEXT4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText_Position, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Topmost = new wxCheckBox(this, ID_CHECKBOX1, _("Topmost Window"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_Topmost->SetValue(true);
	FlexGridSizer1->Add(CheckBox_Topmost, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_SuppressVirtualMatrix = new wxCheckBox(this, ID_CHECKBOX2, _("Suppress Virtual Matrix"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	CheckBox_SuppressVirtualMatrix->SetValue(false);
	FlexGridSizer1->Add(CheckBox_SuppressVirtualMatrix, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Duration:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Duration = new wxTextCtrl(this, ID_TEXTCTRL2, _("0.000"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer1->Add(TextCtrl_Duration, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Delay:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Delay = new wxTextCtrl(this, ID_TEXTCTRL1, _("0.000"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Delay, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_FILEPICKERCTRL1,wxEVT_COMMAND_FILEPICKER_CHANGED,(wxObjectEventFunction)&PlayListItemImagePanel::OnFilePickerCtrl_ImageFileFileChanged);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PlayListItemImagePanel::OnButton_PositionWindowClick);
	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemImagePanel::OnTextCtrl_DurationText);
	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemImagePanel::OnTextCtrl_DelayText);
	//*)

    FilePickerCtrl_ImageFile->SetFileName(wxFileName(Image->GetImageFile()));
    TextCtrl_Delay->SetValue(wxString::Format(wxT("%.3f"), (float)Image->GetDelay() / 1000.0));
    TextCtrl_Duration->SetValue(wxString::Format(wxT("%.3f"), (float)Image->GetDuration() / 1000.0));
    CheckBox_Topmost->SetValue(Image->GetTopMost());
    CheckBox_SuppressVirtualMatrix->SetValue(Image->GetSuppressVirtualMatrix());

    SetWindowPositionText();
}

PlayListItemImagePanel::~PlayListItemImagePanel()
{
	//(*Destroy(PlayListItemImagePanel)
	//*)
    _image->SetImageFile(FilePickerCtrl_ImageFile->GetFileName().GetFullPath().ToStdString());
    _image->SetDelay(wxAtof(TextCtrl_Delay->GetValue()) * 1000);
    _image->SetDuration(wxAtof(TextCtrl_Duration->GetValue()) * 1000);
    _image->SetTopmost(CheckBox_Topmost->GetValue());
    _image->SetSuppressVirtualMatrix(CheckBox_SuppressVirtualMatrix->GetValue());
}

void PlayListItemImagePanel::OnButton_PositionWindowClick(wxCommandEvent& event)
{
    VideoWindowPositionDialog dlg(this, wxID_ANY, _image->GetPosition(), _image->GetSize());

    if (dlg.ShowModal() == wxID_OK)
    {
        _image->SetLocation(dlg.GetDesiredPosition(), dlg.GetDesiredSize());
    }

    SetWindowPositionText();
}

void PlayListItemImagePanel::OnFilePickerCtrl_ImageFileFileChanged(wxFileDirPickerEvent& event)
{
    _image->SetImageFile(FilePickerCtrl_ImageFile->GetFileName().GetFullPath().ToStdString());
    wxCommandEvent e(EVT_UPDATEITEMNAME);
    wxPostEvent(GetParent()->GetParent()->GetParent()->GetParent(), e);
}

void PlayListItemImagePanel::OnTextCtrl_DelayText(wxCommandEvent& event)
{
    ValidateWindow();
}

void PlayListItemImagePanel::OnTextCtrl_DurationText(wxCommandEvent& event)
{
    ValidateWindow();
}

void PlayListItemImagePanel::ValidateWindow()
{}

void PlayListItemImagePanel::SetWindowPositionText()
{
    StaticText_Position->SetLabel(wxString::Format("X: %d, Y: %d, W: %d, H: %d", _image->GetPosition().x, _image->GetPosition().y, _image->GetSize().GetWidth(), _image->GetSize().GetHeight()));
}
