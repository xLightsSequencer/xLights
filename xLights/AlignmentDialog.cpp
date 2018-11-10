#include "AlignmentDialog.h"

//(*InternalHeaders(AlignmentDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(AlignmentDialog)
const long AlignmentDialog::ID_RADIOBUTTON1 = wxNewId();
const long AlignmentDialog::ID_RADIOBUTTON2 = wxNewId();
const long AlignmentDialog::ID_RADIOBUTTON3 = wxNewId();
const long AlignmentDialog::ID_RADIOBUTTON4 = wxNewId();
const long AlignmentDialog::ID_RADIOBUTTON5 = wxNewId();
const long AlignmentDialog::ID_RADIOBUTTON6 = wxNewId();
const long AlignmentDialog::ID_RADIOBUTTON7 = wxNewId();
const long AlignmentDialog::ID_RADIOBUTTON8 = wxNewId();
const long AlignmentDialog::ID_RADIOBUTTON9 = wxNewId();
const long AlignmentDialog::ID_BUTTON2 = wxNewId();
const long AlignmentDialog::ID_BUTTON1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(AlignmentDialog,wxDialog)
	//(*EventTable(AlignmentDialog)
	//*)
END_EVENT_TABLE()

AlignmentDialog::AlignmentDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(AlignmentDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxGridSizer* GridSizer1;

	Create(parent, id, _("Select alignment point"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxSIMPLE_BORDER, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	GridSizer1 = new wxGridSizer(3, 3, 0, 0);
	RadioButton_TL = new wxRadioButton(this, ID_RADIOBUTTON1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON1"));
	GridSizer1->Add(RadioButton_TL, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	RadioButton_TC = new wxRadioButton(this, ID_RADIOBUTTON2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON2"));
	GridSizer1->Add(RadioButton_TC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	RadioButton_TR = new wxRadioButton(this, ID_RADIOBUTTON3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON3"));
	GridSizer1->Add(RadioButton_TR, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	RadioButton_ML = new wxRadioButton(this, ID_RADIOBUTTON4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON4"));
	GridSizer1->Add(RadioButton_ML, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	RadioButton_MC = new wxRadioButton(this, ID_RADIOBUTTON5, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON5"));
	RadioButton_MC->SetValue(true);
	GridSizer1->Add(RadioButton_MC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	RadioButton_MR = new wxRadioButton(this, ID_RADIOBUTTON6, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON6"));
	GridSizer1->Add(RadioButton_MR, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	RadioButton_BL = new wxRadioButton(this, ID_RADIOBUTTON7, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON7"));
	GridSizer1->Add(RadioButton_BL, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	RadioButton_BC = new wxRadioButton(this, ID_RADIOBUTTON8, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON8"));
	GridSizer1->Add(RadioButton_BC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	RadioButton_BR = new wxRadioButton(this, ID_RADIOBUTTON9, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON9"));
	GridSizer1->Add(RadioButton_BR, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(GridSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON2, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer2->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON1, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer2->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&AlignmentDialog::OnButton_OkClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&AlignmentDialog::OnButton_CancelClick);
	//*)

    Button_Ok->SetDefault();
    SetEscapeId(ID_BUTTON1);
}

AlignmentDialog::~AlignmentDialog()
{
	//(*Destroy(AlignmentDialog)
	//*)
}

AlignmentDialog::Alignment AlignmentDialog::GetX() const
{
    if (RadioButton_BL->GetValue() == 1 ||
        RadioButton_ML->GetValue() == 1 ||
        RadioButton_TL->GetValue() == 1)
    {
        return Alignment::LEFT;
    }
    else if (RadioButton_BC->GetValue() == 1 ||
            RadioButton_MC->GetValue() == 1 ||
            RadioButton_TC->GetValue() == 1)
    {
        return Alignment::CENTRE;
    }
    return Alignment::RIGHT;
}

AlignmentDialog::Alignment AlignmentDialog::GetY() const
{
    if (RadioButton_TL->GetValue() == 1 ||
        RadioButton_TC->GetValue() == 1 ||
        RadioButton_TR->GetValue() == 1)
    {
        return Alignment::TOP;
    }
    else if (RadioButton_ML->GetValue() == 1 ||
            RadioButton_MC->GetValue() == 1 ||
            RadioButton_MR->GetValue() == 1)
    {
        return Alignment::MIDDLE;
    }
    return Alignment::BOTTOM;
}

void AlignmentDialog::OnButton_OkClick(wxCommandEvent& event)
{
    EndDialog(wxID_OK);
}

void AlignmentDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}