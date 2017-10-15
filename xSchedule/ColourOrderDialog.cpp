#include "ColourOrderDialog.h"
#include "xScheduleMain.h"
#include "ScheduleManager.h"

//(*InternalHeaders(ColourOrderDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(ColourOrderDialog)
const long ColourOrderDialog::ID_STATICTEXT1 = wxNewId();
const long ColourOrderDialog::ID_SPINCTRL1 = wxNewId();
const long ColourOrderDialog::ID_STATICTEXT2 = wxNewId();
const long ColourOrderDialog::ID_SPINCTRL2 = wxNewId();
const long ColourOrderDialog::ID_STATICTEXT3 = wxNewId();
const long ColourOrderDialog::ID_CHOICE1 = wxNewId();
const long ColourOrderDialog::ID_STATICTEXT4 = wxNewId();
const long ColourOrderDialog::ID_TEXTCTRL1 = wxNewId();
const long ColourOrderDialog::ID_BUTTON1 = wxNewId();
const long ColourOrderDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ColourOrderDialog,wxDialog)
	//(*EventTable(ColourOrderDialog)
	//*)
END_EVENT_TABLE()

void ColourOrderDialog::SetChoiceFromString(wxChoice* choice, std::string value)
{
    int sel = choice->GetSelection();

    choice->SetSelection(-1);
    for (size_t i = 0; i < choice->GetCount(); i++)
    {
        if (choice->GetString(i) == value)
        {
            choice->SetSelection(i);
            return;
        }
    }

    choice->SetSelection(sel);
}

ColourOrderDialog::ColourOrderDialog(wxWindow* parent, size_t& startChannel, size_t& nodes, size_t& colourOrder, std::string& description,wxWindowID id,const wxPoint& pos,const wxSize& size) : _startChannel(startChannel), _nodes(nodes), _colourOrder(colourOrder), _description(description)
{
	//(*Initialize(ColourOrderDialog)
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, _("Reorder Colors"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
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
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Color Order"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice1 = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	Choice1->SetSelection( Choice1->Append(_("132")) );
	Choice1->Append(_("213"));
	Choice1->Append(_("231"));
	Choice1->Append(_("312"));
	Choice1->Append(_("321"));
	FlexGridSizer1->Add(Choice1, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Desription"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Description = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Description, 1, wxALL|wxEXPAND, 5);
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

	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColourOrderDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColourOrderDialog::OnButton_CancelClick);
	//*)

    long chs = xScheduleFrame::GetScheduleManager()->GetTotalChannels();
    SpinCtrl_StartChannel->SetRange(1, chs);
    SpinCtrl_Nodes->SetRange(1, chs / 3);

    SpinCtrl_StartChannel->SetValue(_startChannel);
    SpinCtrl_Nodes->SetValue(_nodes);
    SetChoiceFromString(Choice1, wxString::Format(wxT("%ld"), (long)_colourOrder).ToStdString());
    TextCtrl_Description->SetValue(_description);

    SetEscapeId(Button_Cancel->GetId());
    SetAffirmativeId(Button_Ok->GetId());
}

ColourOrderDialog::~ColourOrderDialog()
{
	//(*Destroy(ColourOrderDialog)
	//*)
}

void ColourOrderDialog::OnButton_OkClick(wxCommandEvent& event)
{
    _startChannel = SpinCtrl_StartChannel->GetValue();
    _nodes = SpinCtrl_Nodes->GetValue();
    _colourOrder = wxAtoi(Choice1->GetStringSelection());
    _description = TextCtrl_Description->GetValue();
    EndDialog(wxID_OK);
}

void ColourOrderDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}
