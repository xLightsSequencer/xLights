#include "BulkEditFontPickerDialog.h"

//(*InternalHeaders(BulkEditFontPickerDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(BulkEditFontPickerDialog)
const long BulkEditFontPickerDialog::ID_STATICTEXT_BulkEdit = wxNewId();
const long BulkEditFontPickerDialog::ID_FONTPICKERCTRL1 = wxNewId();
const long BulkEditFontPickerDialog::ID_BUTTON1 = wxNewId();
const long BulkEditFontPickerDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(BulkEditFontPickerDialog,wxDialog)
	//(*EventTable(BulkEditFontPickerDialog)
	//*)
END_EVENT_TABLE()

BulkEditFontPickerDialog::BulkEditFontPickerDialog(wxWindow* parent, const std::string& label, const std::string& value, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
	//(*Initialize(BulkEditFontPickerDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;

	Create(parent, id, _("Select font"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText_Label = new wxStaticText(this, ID_STATICTEXT_BulkEdit, _("Label:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_BulkEdit"));
	FlexGridSizer1->Add(StaticText_Label, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FontPickerCtrl1 = new wxFontPickerCtrl(this, ID_FONTPICKERCTRL1, wxNullFont, wxDefaultPosition, wxDefaultSize, wxFNTP_FONTDESC_AS_LABEL, wxDefaultValidator, _T("ID_FONTPICKERCTRL1"));
	FlexGridSizer1->Add(FontPickerCtrl1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	Button_Ok->SetDefault();
	FlexGridSizer2->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer2->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BulkEditFontPickerDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BulkEditFontPickerDialog::OnButton_CancelClick);
	//*)

    StaticText_Label->SetLabel(label);

    wxFont oldfont;
    oldfont.SetNativeFontInfoUserDesc(value);
    FontPickerCtrl1->SetSelectedFont(oldfont);
}

BulkEditFontPickerDialog::~BulkEditFontPickerDialog()
{
	//(*Destroy(BulkEditFontPickerDialog)
	//*)
}

std::string BulkEditFontPickerDialog::GetValue() const
{
    wxFont f = FontPickerCtrl1->GetSelectedFont();
    if (f.IsOk()) {
        wxString FontDesc = f.GetNativeFontInfoUserDesc();
        FontDesc.Replace(" unknown-90", "");
        return FontDesc.ToStdString();
    }
    return "";
}

void BulkEditFontPickerDialog::OnButton_OkClick(wxCommandEvent& event)
{
    EndDialog(wxID_OK);
}

void BulkEditFontPickerDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}
