#include "DimWhiteDialog.h"
#include "xScheduleMain.h"
#include "ScheduleManager.h"

//(*InternalHeaders(DimWhiteDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(DimWhiteDialog)
const long DimWhiteDialog::ID_STATICTEXT1 = wxNewId();
const long DimWhiteDialog::ID_SPINCTRL1 = wxNewId();
const long DimWhiteDialog::ID_STATICTEXT2 = wxNewId();
const long DimWhiteDialog::ID_SPINCTRL2 = wxNewId();
const long DimWhiteDialog::ID_STATICTEXT3 = wxNewId();
const long DimWhiteDialog::ID_SPINCTRL3 = wxNewId();
const long DimWhiteDialog::ID_STATICTEXT4 = wxNewId();
const long DimWhiteDialog::ID_TEXTCTRL1 = wxNewId();
const long DimWhiteDialog::ID_CHECKBOX1 = wxNewId();
const long DimWhiteDialog::ID_BUTTON1 = wxNewId();
const long DimWhiteDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(DimWhiteDialog,wxDialog)
	//(*EventTable(DimWhiteDialog)
	//*)
END_EVENT_TABLE()

DimWhiteDialog::DimWhiteDialog(wxWindow* parent, size_t& startChannel, size_t& channels, size_t& dim, std::string& description, bool& enabled, wxWindowID id,const wxPoint& pos,const wxSize& size) : _startChannel(startChannel), _nodes(channels), _dim(dim), _description(description), _enabled(enabled)
{
	//(*Initialize(DimWhiteDialog)
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, _("Dim White"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Start Channel:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_StartChannel = new wxSpinCtrl(this, ID_SPINCTRL1, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 100, 1, _T("ID_SPINCTRL1"));
	SpinCtrl_StartChannel->SetValue(_T("1"));
	FlexGridSizer1->Add(SpinCtrl_StartChannel, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Nodes:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Nodes = new wxSpinCtrl(this, ID_SPINCTRL2, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 100, 1, _T("ID_SPINCTRL2"));
	SpinCtrl_Nodes->SetValue(_T("1"));
	FlexGridSizer1->Add(SpinCtrl_Nodes, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Brightness:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Brightness = new wxSpinCtrl(this, ID_SPINCTRL3, _T("100"), wxDefaultPosition, wxDefaultSize, 0, 0, 100, 100, _T("ID_SPINCTRL3"));
	SpinCtrl_Brightness->SetValue(_T("100"));
	FlexGridSizer1->Add(SpinCtrl_Brightness, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Description:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Description = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Description, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Enabled = new wxCheckBox(this, ID_CHECKBOX1, _("Enabled"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_Enabled->SetValue(false);
	FlexGridSizer1->Add(CheckBox_Enabled, 1, wxALL|wxEXPAND, 5);
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

	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DimWhiteDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DimWhiteDialog::OnButton_CancelClick);
	//*)

    long chs = xScheduleFrame::GetScheduleManager()->GetTotalChannels();
    SpinCtrl_StartChannel->SetRange(1, chs);
    SpinCtrl_Nodes->SetRange(1, chs / 3);

    SpinCtrl_StartChannel->SetValue(_startChannel);
    SpinCtrl_Nodes->SetValue(_nodes);
    SpinCtrl_Brightness->SetValue(_dim);
    TextCtrl_Description->SetValue(_description);
    CheckBox_Enabled->SetValue(_enabled);

    SetEscapeId(Button_Cancel->GetId());
    SetAffirmativeId(Button_Ok->GetId());
}

DimWhiteDialog::~DimWhiteDialog()
{
	//(*Destroy(DimWhiteDialog)
	//*)
}


void DimWhiteDialog::OnButton_OkClick(wxCommandEvent& event)
{
    _startChannel = SpinCtrl_StartChannel->GetValue();
    _nodes = SpinCtrl_Nodes->GetValue();
    _dim = SpinCtrl_Brightness->GetValue();
    _description = TextCtrl_Description->GetValue().ToStdString();
    _enabled = CheckBox_Enabled->IsChecked();

    EndDialog(wxID_OK);
}

void DimWhiteDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}
