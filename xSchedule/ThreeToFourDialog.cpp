#include "ThreeToFourDialog.h"
#include "xScheduleMain.h"
#include "ScheduleManager.h"
#include "../xLights/outputs/OutputManager.h"

//(*InternalHeaders(ThreeToFourDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(ThreeToFourDialog)
const long ThreeToFourDialog::ID_STATICTEXT1 = wxNewId();
const long ThreeToFourDialog::ID_TEXTCTRL2 = wxNewId();
const long ThreeToFourDialog::ID_STATICTEXT5 = wxNewId();
const long ThreeToFourDialog::ID_STATICTEXT2 = wxNewId();
const long ThreeToFourDialog::ID_SPINCTRL2 = wxNewId();
const long ThreeToFourDialog::ID_STATICTEXT3 = wxNewId();
const long ThreeToFourDialog::ID_CHOICE1 = wxNewId();
const long ThreeToFourDialog::ID_STATICTEXT4 = wxNewId();
const long ThreeToFourDialog::ID_TEXTCTRL1 = wxNewId();
const long ThreeToFourDialog::ID_CHECKBOX1 = wxNewId();
const long ThreeToFourDialog::ID_BUTTON1 = wxNewId();
const long ThreeToFourDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ThreeToFourDialog,wxDialog)
	//(*EventTable(ThreeToFourDialog)
	//*)
END_EVENT_TABLE()

ThreeToFourDialog::ThreeToFourDialog(wxWindow* parent, OutputManager* outputManager, std::string& startChannel, size_t& channels, std::string& colourOrder, std::string& description, bool& enabled, wxWindowID id,const wxPoint& pos,const wxSize& size) : _startChannel(startChannel), _nodes(channels), _colourOrder(colourOrder), _description(description), _enabled(enabled)
{
    _outputManager = outputManager;
    
    //(*Initialize(ThreeToFourDialog)
	wxFlexGridSizer* FlexGridSizer2;
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Start Channel:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	TextCtrl_StartChannel = new wxTextCtrl(this, ID_TEXTCTRL2, _("1"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer2->Add(TextCtrl_StartChannel, 1, wxALL|wxEXPAND, 5);
	StaticText_StartChannel = new wxStaticText(this, ID_STATICTEXT5, _("1"), wxDefaultPosition, wxSize(60,-1), 0, _T("ID_STATICTEXT5"));
	FlexGridSizer2->Add(StaticText_StartChannel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Nodes:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Nodes = new wxSpinCtrl(this, ID_SPINCTRL2, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 100, 1, _T("ID_SPINCTRL2"));
	SpinCtrl_Nodes->SetValue(_T("1"));
	FlexGridSizer1->Add(SpinCtrl_Nodes, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Order:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_ColourOrder = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	Choice_ColourOrder->SetSelection( Choice_ColourOrder->Append(_("RGBW")) );
	Choice_ColourOrder->Append(_("WRGB"));
	FlexGridSizer1->Add(Choice_ColourOrder, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Description:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Description = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Description, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Enabled = new wxCheckBox(this, ID_CHECKBOX1, _("Enabled"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_Enabled->SetValue(false);
	FlexGridSizer1->Add(CheckBox_Enabled, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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

	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ThreeToFourDialog::OnTextCtrl_StartChannelText);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ThreeToFourDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ThreeToFourDialog::OnButton_CancelClick);
	//*)

    long chs = xScheduleFrame::GetScheduleManager()->GetTotalChannels();
    SpinCtrl_Nodes->SetRange(1, chs / 3);

    TextCtrl_StartChannel->SetValue(_startChannel);
    SpinCtrl_Nodes->SetValue(_nodes);
    Choice_ColourOrder->SetStringSelection(_colourOrder);
    TextCtrl_Description->SetValue(_description);
    CheckBox_Enabled->SetValue(_enabled);

    SetEscapeId(Button_Cancel->GetId());
    SetAffirmativeId(Button_Ok->GetId());
    ValidateWindow();
}

ThreeToFourDialog::~ThreeToFourDialog()
{
	//(*Destroy(ThreeToFourDialog)
	//*)
}

void ThreeToFourDialog::ValidateWindow()
{
    long sc = _outputManager->DecodeStartChannel(TextCtrl_StartChannel->GetValue().ToStdString());
    StaticText_StartChannel->SetLabel(wxString::Format("%ld", (long)sc));
    if (sc == 0 || sc > xScheduleFrame::GetScheduleManager()->GetTotalChannels())
    {
        Button_Ok->Enable(false);
    }
    else
    {
        Button_Ok->Enable(true);
    }
}

void ThreeToFourDialog::OnTextCtrl_StartChannelText(wxCommandEvent& event)
{
    ValidateWindow();
}

void ThreeToFourDialog::OnButton_OkClick(wxCommandEvent& event)
{
    _startChannel = TextCtrl_StartChannel->GetValue();
    _nodes = SpinCtrl_Nodes->GetValue();
    _colourOrder = Choice_ColourOrder->GetStringSelection().ToStdString();
    _description = TextCtrl_Description->GetValue().ToStdString();
    _enabled = CheckBox_Enabled->IsChecked();

    EndDialog(wxID_OK);
}

void ThreeToFourDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}
