#include "MatrixDialog.h"
#include "xScheduleMain.h"
#include "ScheduleManager.h"
#include "../xLights/outputs/OutputManager.h"

//(*InternalHeaders(MatrixDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(MatrixDialog)
const long MatrixDialog::ID_STATICTEXT7 = wxNewId();
const long MatrixDialog::ID_TEXTCTRL1 = wxNewId();
const long MatrixDialog::ID_STATICTEXT1 = wxNewId();
const long MatrixDialog::ID_SPINCTRL1 = wxNewId();
const long MatrixDialog::ID_STATICTEXT4 = wxNewId();
const long MatrixDialog::ID_SPINCTRL2 = wxNewId();
const long MatrixDialog::ID_STATICTEXT2 = wxNewId();
const long MatrixDialog::ID_SPINCTRL3 = wxNewId();
const long MatrixDialog::ID_STATICTEXT3 = wxNewId();
const long MatrixDialog::ID_TEXTCTRL2 = wxNewId();
const long MatrixDialog::ID_STATICTEXT8 = wxNewId();
const long MatrixDialog::ID_STATICTEXT5 = wxNewId();
const long MatrixDialog::ID_CHOICE1 = wxNewId();
const long MatrixDialog::ID_STATICTEXT6 = wxNewId();
const long MatrixDialog::ID_CHOICE2 = wxNewId();
const long MatrixDialog::ID_BUTTON1 = wxNewId();
const long MatrixDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(MatrixDialog,wxDialog)
	//(*EventTable(MatrixDialog)
	//*)
END_EVENT_TABLE()

MatrixDialog::MatrixDialog(wxWindow* parent, OutputManager* outputManager, std::string& name, std::string& orientation, std::string& startingLocation, int& stringLength, int& strings, int& strandsPerString, std::string& startChannel, wxWindowID id,const wxPoint& pos,const wxSize& size) : _name(name), _orientation(orientation), _startChannel(startChannel), _startingLocation(startingLocation), _strandsPerString(strandsPerString), _strings(strings), _stringLength(stringLength)
{
    _outputManager = outputManager;

	//(*Initialize(MatrixDialog)
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;

	Create(parent, id, _("Matrix Configuration"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer1->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Name = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Name, 1, wxALL|wxEXPAND, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Strings:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Strings = new wxSpinCtrl(this, ID_SPINCTRL1, _T("16"), wxDefaultPosition, wxDefaultSize, 0, 1, 500, 16, _T("ID_SPINCTRL1"));
	SpinCtrl_Strings->SetValue(_T("16"));
	FlexGridSizer1->Add(SpinCtrl_Strings, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("String Length:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_StringLength = new wxSpinCtrl(this, ID_SPINCTRL2, _T("150"), wxDefaultPosition, wxDefaultSize, 0, 1, 65535, 150, _T("ID_SPINCTRL2"));
	SpinCtrl_StringLength->SetValue(_T("150"));
	FlexGridSizer1->Add(SpinCtrl_StringLength, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Strands Per String:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_StrandsPerString = new wxSpinCtrl(this, ID_SPINCTRL3, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 8192, 1, _T("ID_SPINCTRL3"));
	SpinCtrl_StrandsPerString->SetValue(_T("1"));
	FlexGridSizer1->Add(SpinCtrl_StrandsPerString, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Start Channel:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	TextCtrl_StartChannel = new wxTextCtrl(this, ID_TEXTCTRL2, _("1"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer2->Add(TextCtrl_StartChannel, 1, wxALL|wxEXPAND, 5);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("1"), wxDefaultPosition, wxSize(60,-1), 0, _T("ID_STATICTEXT8"));
	FlexGridSizer2->Add(StaticText8, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Start Location:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer1->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_StartLocation = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	Choice_StartLocation->SetSelection( Choice_StartLocation->Append(_("Bottom Left")) );
	Choice_StartLocation->Append(_("Bottom Right"));
	Choice_StartLocation->Append(_("Top Left"));
	Choice_StartLocation->Append(_("Top Right"));
	FlexGridSizer1->Add(Choice_StartLocation, 1, wxALL|wxEXPAND, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Orientation:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer1->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Orientation = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	Choice_Orientation->SetSelection( Choice_Orientation->Append(_("Vertical")) );
	Choice_Orientation->Append(_("Horizontal"));
	FlexGridSizer1->Add(Choice_Orientation, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	Button_Ok->SetDefault();
	BoxSizer1->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	BoxSizer1->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(BoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&MatrixDialog::OnTextCtrl_StartChannelText);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MatrixDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MatrixDialog::OnButton_CancelClick);
	//*)

    TextCtrl_Name->SetValue(_name);
    Choice_Orientation->SetStringSelection(_orientation);
    Choice_StartLocation->SetStringSelection(_startingLocation);
    TextCtrl_StartChannel->SetValue(_startChannel);
    SpinCtrl_StrandsPerString->SetValue(_strandsPerString);
    SpinCtrl_StringLength->SetValue(_stringLength);
    SpinCtrl_Strings->SetValue(_strings);
}

MatrixDialog::~MatrixDialog()
{
	//(*Destroy(MatrixDialog)
	//*)
}

void MatrixDialog::OnButton_OkClick(wxCommandEvent& event)
{
    _name = TextCtrl_Name->GetValue().ToStdString();
    _orientation = Choice_Orientation->GetStringSelection().ToStdString();
    _startingLocation = Choice_StartLocation->GetStringSelection().ToStdString();
    _startChannel = TextCtrl_StartChannel->GetValue();
    _strandsPerString = SpinCtrl_StrandsPerString->GetValue();
    _stringLength = SpinCtrl_StringLength->GetValue();
    _strings = SpinCtrl_Strings->GetValue();

    EndDialog(wxID_OK);
}

void MatrixDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void MatrixDialog::OnTextCtrl_StartChannelText(wxCommandEvent& event)
{
    long sc = _outputManager->DecodeStartChannel(TextCtrl_StartChannel->GetValue().ToStdString());
    if (sc == 0 || sc > xScheduleFrame::GetScheduleManager()->GetTotalChannels())
    {
        StaticText8->SetLabel("Invalid");
    }
    else
    {
        StaticText8->SetLabel(wxString::Format("%ld", (long)sc));
    }
    ValidateWindow();
}

void MatrixDialog::ValidateWindow()
{
    if (StaticText8->GetLabel() == "Invalid")
    {
        Button_Ok->Enable(false);
    }
    else
    {
        Button_Ok->Enable(true);
    }
}