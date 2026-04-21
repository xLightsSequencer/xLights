#include "BulkEditComboDialog.h"
#include "UtilFunctions.h"

//(*InternalHeaders(BulkEditComboDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(BulkEditComboDialog)
const long BulkEditComboDialog::ID_STATICTEXT1 = wxNewId();
const long BulkEditComboDialog::ID_COMBOBOX1 = wxNewId();
const long BulkEditComboDialog::ID_BUTTON1 = wxNewId();
const long BulkEditComboDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(BulkEditComboDialog,wxDialog)
	//(*EventTable(BulkEditComboDialog)
	//*)
END_EVENT_TABLE()

BulkEditComboDialog::BulkEditComboDialog(wxWindow* parent, const std::string& value, const std::string& label, const wxArrayString& choices, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
	//(*Initialize(BulkEditComboDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;

	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText_Combo = new wxStaticText(this, ID_STATICTEXT1, _("Label"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText_Combo, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ComboBox_Combo = new wxComboBox(this, ID_COMBOBOX1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_COMBOBOX1"));
	FlexGridSizer2->Add(ComboBox_Combo, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	Button_Ok->SetDefault();
	FlexGridSizer3->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer3->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_COMBOBOX1,wxEVT_COMMAND_COMBOBOX_SELECTED,(wxObjectEventFunction)&BulkEditComboDialog::OnComboBox_ComboSelected);
	Connect(ID_COMBOBOX1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&BulkEditComboDialog::OnComboBox_ComboTextUpdated);
	Connect(ID_COMBOBOX1,wxEVT_COMMAND_TEXT_ENTER,(wxObjectEventFunction)&BulkEditComboDialog::OnComboBox_ComboTextEnter);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BulkEditComboDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BulkEditComboDialog::OnButton_CancelClick);
	//*)

	SetEscapeId(Button_Cancel->GetId());
    Button_Ok->SetDefault();

	StaticText_Combo->SetLabel(label);

	for (const auto& it: choices) {
        ComboBox_Combo->AppendString(it);
	}
    ComboBox_Combo->SetValue(value);

    Layout();
    Fit();
}

BulkEditComboDialog::~BulkEditComboDialog()
{
	//(*Destroy(BulkEditComboDialog)
	//*)
}

void BulkEditComboDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void BulkEditComboDialog::OnButton_OkClick(wxCommandEvent& event)
{
    EndDialog(wxID_OK);
}

void BulkEditComboDialog::OnComboBox_ComboTextUpdated(wxCommandEvent& event)
{
    ValidateWindow();
}

void BulkEditComboDialog::OnComboBox_ComboSelected(wxCommandEvent& event)
{
    ValidateWindow();
}

void BulkEditComboDialog::OnComboBox_ComboTextEnter(wxCommandEvent& event)
{
    ValidateWindow();
}

void BulkEditComboDialog::ValidateWindow()
{
    if (IsFloat(ComboBox_Combo->GetValue())) {
        Button_Ok->Enable();
	} else {
        Button_Ok->Disable();
    }
}

std::string BulkEditComboDialog::GetValue()
{
    return ComboBox_Combo->GetValue();
}
