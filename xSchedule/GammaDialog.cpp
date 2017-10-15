#include "GammaDialog.h"
#include "xScheduleMain.h"
#include "ScheduleManager.h"

//(*InternalHeaders(GammaDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(GammaDialog)
const long GammaDialog::ID_STATICTEXT1 = wxNewId();
const long GammaDialog::ID_SPINCTRL1 = wxNewId();
const long GammaDialog::ID_STATICTEXT2 = wxNewId();
const long GammaDialog::ID_SPINCTRL2 = wxNewId();
const long GammaDialog::ID_CHECKBOX1 = wxNewId();
const long GammaDialog::ID_STATICTEXT7 = wxNewId();
const long GammaDialog::ID_TEXTCTRL2 = wxNewId();
const long GammaDialog::ID_STATICTEXT4 = wxNewId();
const long GammaDialog::ID_TEXTCTRL3 = wxNewId();
const long GammaDialog::ID_STATICTEXT5 = wxNewId();
const long GammaDialog::ID_TEXTCTRL4 = wxNewId();
const long GammaDialog::ID_STATICTEXT6 = wxNewId();
const long GammaDialog::ID_TEXTCTRL5 = wxNewId();
const long GammaDialog::ID_STATICTEXT3 = wxNewId();
const long GammaDialog::ID_TEXTCTRL1 = wxNewId();
const long GammaDialog::ID_BUTTON1 = wxNewId();
const long GammaDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(GammaDialog,wxDialog)
	//(*EventTable(GammaDialog)
	//*)
END_EVENT_TABLE()

GammaDialog::GammaDialog(wxWindow* parent, size_t& startChannel, size_t& channels, float& gamma, float& gammaR, float& gammaG, float& gammaB, std::string& description, wxWindowID id,const wxPoint& pos,const wxSize& size) : _startChannel(startChannel), _nodes(channels), _gamma(gamma), _gammaR(gammaR), _gammaG(gammaG), _gammaB(gammaB), _description(description)
{
	//(*Initialize(GammaDialog)
	wxFlexGridSizer* FlexGridSizer2;
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
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
	CheckBox_Simple = new wxCheckBox(this, ID_CHECKBOX1, _("Simple"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_Simple->SetValue(false);
	FlexGridSizer1->Add(CheckBox_Simple, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText_Simple = new wxStaticText(this, ID_STATICTEXT7, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer2->Add(StaticText_Simple, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Simple = new wxTextCtrl(this, ID_TEXTCTRL2, _("1.0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer2->Add(TextCtrl_Simple, 1, wxALL|wxEXPAND, 5);
	StaticText_R = new wxStaticText(this, ID_STATICTEXT4, _("R"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer2->Add(StaticText_R, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_R = new wxTextCtrl(this, ID_TEXTCTRL3, _("1.0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	FlexGridSizer2->Add(TextCtrl_R, 1, wxALL|wxEXPAND, 5);
	StaticText_G = new wxStaticText(this, ID_STATICTEXT5, _("G"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer2->Add(StaticText_G, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_G = new wxTextCtrl(this, ID_TEXTCTRL4, _("1.0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL4"));
	FlexGridSizer2->Add(TextCtrl_G, 1, wxALL|wxEXPAND, 5);
	StaticText_B = new wxStaticText(this, ID_STATICTEXT6, _("B"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer2->Add(StaticText_B, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_B = new wxTextCtrl(this, ID_TEXTCTRL5, _("1.0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL5"));
	FlexGridSizer2->Add(TextCtrl_B, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Description:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Description = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Description, 1, wxALL|wxEXPAND, 5);
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

	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&GammaDialog::OnCheckBox_SimpleClick);
	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&GammaDialog::OnTextCtrl_ValidateNumbers);
	Connect(ID_TEXTCTRL3,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&GammaDialog::OnTextCtrl_ValidateNumbers);
	Connect(ID_TEXTCTRL4,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&GammaDialog::OnTextCtrl_ValidateNumbers);
	Connect(ID_TEXTCTRL5,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&GammaDialog::OnTextCtrl_ValidateNumbers);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GammaDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GammaDialog::OnButton_CancelClick);
	//*)

    long chs = xScheduleFrame::GetScheduleManager()->GetTotalChannels();
    SpinCtrl_StartChannel->SetRange(1, chs);
    SpinCtrl_Nodes->SetRange(1, chs / 3);

    SpinCtrl_StartChannel->SetValue(_startChannel);
    SpinCtrl_Nodes->SetValue(_nodes);
    if (_gamma == 0.0)
    {
        CheckBox_Simple->SetValue(false);
        TextCtrl_Simple->SetValue("1.00");
        TextCtrl_R->SetValue(wxString::Format("%.2f", _gammaR));
        TextCtrl_G->SetValue(wxString::Format("%.2f", _gammaG));
        TextCtrl_B->SetValue(wxString::Format("%.2f", _gammaB));
    }
    else
    {
        CheckBox_Simple->SetValue(true);
        TextCtrl_R->SetValue("1.00");
        TextCtrl_G->SetValue("1.00");
        TextCtrl_B->SetValue("1.00");
        TextCtrl_Simple->SetValue(wxString::Format("%.2f", _gamma));
    }
    TextCtrl_Description->SetValue(_description);

    SetEscapeId(Button_Cancel->GetId());
    SetAffirmativeId(Button_Ok->GetId());

    ValidateWindow();
}

GammaDialog::~GammaDialog()
{
	//(*Destroy(GammaDialog)
	//*)
}


void GammaDialog::OnCheckBox_SimpleClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void GammaDialog::OnTextCtrl_ValidateNumbers(wxCommandEvent& event)
{
    ValidateWindow();
}

void GammaDialog::OnButton_OkClick(wxCommandEvent& event)
{
    _startChannel = SpinCtrl_StartChannel->GetValue();
    _nodes = SpinCtrl_Nodes->GetValue();
    if (CheckBox_Simple->GetValue())
    {
        _gamma = wxAtof(TextCtrl_Simple->GetValue());
        _gammaR = 0.0;
        _gammaG = 0.0;
        _gammaB = 0.0;
    }
    else
    {
        _gamma = 0.0;
        _gammaR = wxAtof(TextCtrl_R->GetValue());
        _gammaG = wxAtof(TextCtrl_G->GetValue());
        _gammaB = wxAtof(TextCtrl_B->GetValue());
    }
    _description = TextCtrl_Description->GetValue().ToStdString();

    EndDialog(wxID_OK);
}

void GammaDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void GammaDialog::ValidateWindow()
{
    float simple = wxAtof(TextCtrl_Simple->GetValue());
    float r = wxAtof(TextCtrl_R->GetValue());
    float g = wxAtof(TextCtrl_G->GetValue());
    float b = wxAtof(TextCtrl_B->GetValue());
    if (CheckBox_Simple->GetValue())
    {
        if (simple <= 0 || simple > 50)
        {
            Button_Ok->Enable(false);
        }
        else
        {
            Button_Ok->Enable();
        }
        StaticText_R->Hide();
        StaticText_G->Hide();
        StaticText_B->Hide();
        TextCtrl_R->Hide();
        TextCtrl_G->Hide();
        TextCtrl_B->Hide();
        TextCtrl_Simple->Show();
        StaticText_Simple->Show();
    }
    else
    {
        if (r <= 0.01 || r > 50 ||
            g <= 0.01 || g > 50 ||
            b <= 0.01 || b > 50
            )
        {
            Button_Ok->Enable(false);
        }
        else
        {
            Button_Ok->Enable();
        }
        StaticText_R->Show();
        StaticText_G->Show();
        StaticText_B->Show();
        TextCtrl_R->Show();
        TextCtrl_G->Show();
        TextCtrl_B->Show();
        TextCtrl_Simple->Hide();
        StaticText_Simple->Hide();
    }
    Layout();
}
