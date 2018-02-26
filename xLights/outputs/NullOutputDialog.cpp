#include "NullOutputDialog.h"

//(*InternalHeaders(NullOutputDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "NullOutput.h"
#include "OutputManager.h"

//(*IdInit(NullOutputDialog)
const long NullOutputDialog::ID_STATICTEXT1 = wxNewId();
const long NullOutputDialog::ID_STATICTEXT2 = wxNewId();
const long NullOutputDialog::ID_SPINCTRL2 = wxNewId();
const long NullOutputDialog::ID_SPINCTRL1 = wxNewId();
const long NullOutputDialog::ID_TEXTCTRL_DESCRIPTION = wxNewId();
const long NullOutputDialog::ID_BUTTON1 = wxNewId();
const long NullOutputDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(NullOutputDialog,wxDialog)
	//(*EventTable(NullOutputDialog)
	//*)
END_EVENT_TABLE()

NullOutputDialog::NullOutputDialog(wxWindow* parent, NullOutput* null, OutputManager* outputManager, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _null = null;
    _outputManager = outputManager;

	//(*Initialize(NullOutputDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxStaticText* StaticText2;
	wxStaticText* StaticText3;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Null Output Setup\n\nA Null output reserves a number of channels \nin the file but doesn\'t actually output the\nchannels to any device.  \n\nThis is useful if the range of channels may be\noutput from some other method such as an\nOctoscroller on a BBB.\n\nId if unique amongst all your output universes \nthen you can use #id:startchannel for start \nchannel identification"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT2, _("Id"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer2->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl1 = new wxSpinCtrl(this, ID_SPINCTRL2, _T("64001"), wxDefaultPosition, wxDefaultSize, 0, 1, 65535, 64001, _T("ID_SPINCTRL2"));
	SpinCtrl1->SetValue(_T("64001"));
	FlexGridSizer2->Add(SpinCtrl1, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, wxID_ANY, _("Number of Channels"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	NumChannelsSpinCtrl = new wxSpinCtrl(this, ID_SPINCTRL1, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 512000, 1, _T("ID_SPINCTRL1"));
	NumChannelsSpinCtrl->SetValue(_T("1"));
	FlexGridSizer2->Add(NumChannelsSpinCtrl, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, wxID_ANY, _("Description"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Description = new wxTextCtrl(this, ID_TEXTCTRL_DESCRIPTION, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_DESCRIPTION"));
	TextCtrl_Description->SetMaxLength(64);
	FlexGridSizer2->Add(TextCtrl_Description, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer3->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer3->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL_DESCRIPTION,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&NullOutputDialog::OnTextCtrl_DescriptionText);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NullOutputDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NullOutputDialog::OnButton_CancelClick);
	//*)

    NumChannelsSpinCtrl->SetValue(_null->GetChannels());
    TextCtrl_Description->SetValue(_null->GetDescription());
    SpinCtrl1->SetValue(null->GetId());
    //Choice_Controller->SetStringValue(_null->GetController()->GetId());

    Button_Ok->SetDefault();
    ValidateWindow();
}

NullOutputDialog::~NullOutputDialog()
{
	//(*Destroy(NullOutputDialog)
	//*)
}


void NullOutputDialog::OnTextCtrl_DescriptionText(wxCommandEvent& event)
{
    ValidateWindow();
}

void NullOutputDialog::OnButton_OkClick(wxCommandEvent& event)
{
    _null->SetDescription(TextCtrl_Description->GetValue().ToStdString());
    _null->SetChannels(NumChannelsSpinCtrl->GetValue());
    _null->SetId(SpinCtrl1->GetValue());

    EndDialog(wxID_OK);
}

void NullOutputDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}
