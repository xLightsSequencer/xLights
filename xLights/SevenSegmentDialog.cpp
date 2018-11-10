#include "SevenSegmentDialog.h"

//(*InternalHeaders(SevenSegmentDialog)
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

#include <wx/image.h>
#include "../include/sevensegment.xpm"

//(*IdInit(SevenSegmentDialog)
const long SevenSegmentDialog::ID_CHECKBOX1 = wxNewId();
const long SevenSegmentDialog::ID_CHECKBOX2 = wxNewId();
const long SevenSegmentDialog::ID_CHECKBOX4 = wxNewId();
const long SevenSegmentDialog::ID_CHECKBOX3 = wxNewId();
const long SevenSegmentDialog::ID_CHECKBOX5 = wxNewId();
const long SevenSegmentDialog::ID_CHECKBOX6 = wxNewId();
const long SevenSegmentDialog::ID_STATICBITMAP1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(SevenSegmentDialog,wxDialog)
	//(*EventTable(SevenSegmentDialog)
	//*)
END_EVENT_TABLE()

SevenSegmentDialog::SevenSegmentDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(SevenSegmentDialog)
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, id, _("Select Labels to Include:"), wxDefaultPosition, wxDefaultSize, wxCAPTION, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 6, 0, 0);
	CheckBox_Thousands = new wxCheckBox(this, ID_CHECKBOX1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_Thousands->SetValue(false);
	CheckBox_Thousands->SetToolTip(_("1000\'s"));
	FlexGridSizer2->Add(CheckBox_Thousands, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Hundreds = new wxCheckBox(this, ID_CHECKBOX2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	CheckBox_Hundreds->SetValue(false);
	CheckBox_Hundreds->SetToolTip(_("100\'s"));
	FlexGridSizer2->Add(CheckBox_Hundreds, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Colon = new wxCheckBox(this, ID_CHECKBOX4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX4"));
	CheckBox_Colon->SetValue(false);
	CheckBox_Colon->SetToolTip(_("Colon"));
	FlexGridSizer2->Add(CheckBox_Colon, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Tens = new wxCheckBox(this, ID_CHECKBOX3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
	CheckBox_Tens->SetValue(false);
	CheckBox_Tens->SetToolTip(_("10\'s"));
	FlexGridSizer2->Add(CheckBox_Tens, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Decimal = new wxCheckBox(this, ID_CHECKBOX5, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX5"));
	CheckBox_Decimal->SetValue(false);
	CheckBox_Decimal->SetToolTip(_("Decimal Point"));
	FlexGridSizer2->Add(CheckBox_Decimal, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Ones = new wxCheckBox(this, ID_CHECKBOX6, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX6"));
	CheckBox_Ones->SetValue(false);
	CheckBox_Ones->SetToolTip(_("1\'s"));
	FlexGridSizer2->Add(CheckBox_Ones, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	StaticBitmap_7Segment = new wxStaticBitmap(this, ID_STATICBITMAP1, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER, _T("ID_STATICBITMAP1"));
	FlexGridSizer1->Add(StaticBitmap_7Segment, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)

    wxImage i;
    i.Create(sevensegment);
    wxBitmap ss(i);
    StaticBitmap_7Segment->SetBitmap(ss);

    CheckBox_Thousands->SetLabel("      ");
    CheckBox_Hundreds->SetLabel("   ");
    CheckBox_Colon->SetLabel("");
    CheckBox_Tens->SetLabel("");
    CheckBox_Decimal->SetLabel("");
    CheckBox_Ones->SetLabel("  ");

    FlexGridSizer1->Fit(this);
    SetEscapeId(wxID_CANCEL);
}

SevenSegmentDialog::~SevenSegmentDialog()
{
	//(*Destroy(SevenSegmentDialog)
	//*)
}

