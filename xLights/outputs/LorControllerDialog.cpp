#include "LorControllerDialog.h"
#include "LorController.h"

//(*InternalHeaders(LorControllerDialog)
#include <wx/font.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(LorControllerDialog)
const long LorControllerDialog::ID_STATICTEXT1 = wxNewId();
const long LorControllerDialog::ID_STATICTEXT2 = wxNewId();
const long LorControllerDialog::ID_CHOICE_TYPE = wxNewId();
const long LorControllerDialog::ID_STATICTEXT3 = wxNewId();
const long LorControllerDialog::ID_SPINCTRL_CHANNELS = wxNewId();
const long LorControllerDialog::ID_STATICTEXT4 = wxNewId();
const long LorControllerDialog::ID_SPINCTRL_UNITID = wxNewId();
const long LorControllerDialog::ID_STATICTEXT_HEX = wxNewId();
const long LorControllerDialog::ID_STATICTEXT5 = wxNewId();
const long LorControllerDialog::ID_CHOICE_MODE = wxNewId();
const long LorControllerDialog::ID_STATICTEXT6 = wxNewId();
const long LorControllerDialog::ID_TEXTCTRL_DESCRIPTION = wxNewId();
const long LorControllerDialog::ID_BUTTON_OK = wxNewId();
const long LorControllerDialog::ID_BUTTON_CANCEL = wxNewId();
//*)

BEGIN_EVENT_TABLE(LorControllerDialog,wxDialog)
	//(*EventTable(LorControllerDialog)
	//*)
END_EVENT_TABLE()

LorControllerDialog::LorControllerDialog(wxWindow* parent, LorController* controller, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _controller = controller;

	//(*Initialize(LorControllerDialog)
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, _("LOR Controller Definition"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("LOR Controller Definition"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	wxFont StaticText1Font(10,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	StaticText1->SetFont(StaticText1Font);
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Type:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Type = new wxChoice(this, ID_CHOICE_TYPE, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_TYPE"));
	Choice_Type->SetSelection( Choice_Type->Append(_("AC Controller")) );
	Choice_Type->Append(_("RGB Controller"));
	Choice_Type->Append(_("CCR"));
	Choice_Type->Append(_("CCB"));
	FlexGridSizer2->Add(Choice_Type, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("# Channels:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Channels = new wxSpinCtrl(this, ID_SPINCTRL_CHANNELS, _T("16"), wxDefaultPosition, wxDefaultSize, 0, 1, 10000, 16, _T("ID_SPINCTRL_CHANNELS"));
	SpinCtrl_Channels->SetValue(_T("16"));
	FlexGridSizer2->Add(SpinCtrl_Channels, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Unit ID:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer2->Add(StaticText4, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_UnitID = new wxSpinCtrl(this, ID_SPINCTRL_UNITID, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 255, 1, _T("ID_SPINCTRL_UNITID"));
	SpinCtrl_UnitID->SetValue(_T("1"));
	FlexGridSizer2->Add(SpinCtrl_UnitID, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_Hex = new wxStaticText(this, ID_STATICTEXT_HEX, _("Hex"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_HEX"));
	wxFont StaticText_HexFont(12,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	StaticText_Hex->SetFont(StaticText_HexFont);
	FlexGridSizer2->Add(StaticText_Hex, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Addr Mode:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer2->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Mode = new wxChoice(this, ID_CHOICE_MODE, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_MODE"));
	Choice_Mode->SetSelection( Choice_Mode->Append(_("Normal")) );
	Choice_Mode->Append(_("Legacy"));
	Choice_Mode->Append(_("Split"));
	FlexGridSizer2->Add(Choice_Mode, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Description:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer2->Add(StaticText6, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Description = new wxTextCtrl(this, ID_TEXTCTRL_DESCRIPTION, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_DESCRIPTION"));
	FlexGridSizer2->Add(TextCtrl_Description, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON_OK, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_OK"));
	FlexGridSizer5->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CANCEL"));
	FlexGridSizer5->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHOICE_TYPE,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&LorControllerDialog::OnChoice_TypeSelect);
	Connect(ID_SPINCTRL_UNITID,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&LorControllerDialog::OnSpinCtrl_UnitIDChange);
	Connect(ID_CHOICE_MODE,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&LorControllerDialog::OnChoice_ModeSelect);
	Connect(ID_BUTTON_OK,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LorControllerDialog::OnButton_OkClick);
	Connect(ID_BUTTON_CANCEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LorControllerDialog::OnButton_CancelClick);
	//*)

	if( _controller != nullptr ) {
    	Choice_Type->SetStringSelection(_controller->GetType());
    	SpinCtrl_UnitID->SetValue(_controller->GetUnitId());
    	SpinCtrl_Channels->SetValue(_controller->GetNumChannels());
    	Choice_Mode->SetSelection((int)(_controller->GetAddressMode()));
    	TextCtrl_Description->SetValue(_controller->GetDescription());
	}

    ValidateWindow();
}

LorControllerDialog::~LorControllerDialog()
{
	//(*Destroy(LorControllerDialog)
	//*)
}


void LorControllerDialog::OnChoice_TypeSelect(wxCommandEvent& event)
{
}

void LorControllerDialog::OnSpinCtrl_UnitIDChange(wxSpinEvent& event)
{
    ValidateWindow();
}

void LorControllerDialog::OnChoice_ModeSelect(wxCommandEvent& event)
{
}

void LorControllerDialog::OnSpinCtrl_ChannelsChange(wxSpinEvent& event)
{
}

void LorControllerDialog::OnTextCtrl_DescriptionText(wxCommandEvent& event)
{
}

void LorControllerDialog::ValidateWindow()
{
    wxString label = wxString::Format("0x%X", SpinCtrl_UnitID->GetValue());
    StaticText_Hex->SetLabel(label);
}

void LorControllerDialog::OnButton_OkClick(wxCommandEvent& event)
{
    if( _controller == nullptr ) {
        _controller = new LorController();
    }
    _controller->SetType(Choice_Type->GetStringSelection().ToStdString());
    _controller->SetUnitID(SpinCtrl_UnitID->GetValue());
    _controller->SetNumChannels(SpinCtrl_Channels->GetValue());
    _controller->SetMode(LorController::AddressMode(Choice_Mode->GetSelection()));
    _controller->SetDescription(TextCtrl_Description->GetValue().ToStdString());

    EndDialog(wxID_OK);
}

void LorControllerDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}


