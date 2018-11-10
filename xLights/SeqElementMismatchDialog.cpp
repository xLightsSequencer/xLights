#include "SeqElementMismatchDialog.h"

//(*InternalHeaders(SeqElementMismatchDialog)
#include <wx/button.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(SeqElementMismatchDialog)
const long SeqElementMismatchDialog::ID_STATICTEXT1 = wxNewId();
const long SeqElementMismatchDialog::ID_RADIOBUTTON2 = wxNewId();
const long SeqElementMismatchDialog::ID_RADIOBUTTON1 = wxNewId();
const long SeqElementMismatchDialog::ID_RADIOBUTTON3 = wxNewId();
const long SeqElementMismatchDialog::ID_CHOICE1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(SeqElementMismatchDialog,wxDialog)
    //(*EventTable(SeqElementMismatchDialog)
    //*)
END_EVENT_TABLE()

SeqElementMismatchDialog::SeqElementMismatchDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    //(*Initialize(SeqElementMismatchDialog)
    wxFlexGridSizer* FlexGridSizer1;
    wxStaticText* StaticText1;
    wxStdDialogButtonSizer* StdDialogButtonSizer1;

    Create(parent, id, _("Sequence Element Mismatch"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
    SetClientSize(wxDefaultSize);
    Move(wxDefaultPosition);
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(0);
    StaticTextMessage = new wxStaticText(this, ID_STATICTEXT1, _("Message"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer1->Add(StaticTextMessage, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 10);
    StaticText1 = new wxStaticText(this, wxID_ANY, _("What action do you want to take\?"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonDelete = new wxRadioButton(this, ID_RADIOBUTTON2, _("Delete this element from the sequence"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON2"));
    RadioButtonDelete->SetValue(true);
    FlexGridSizer1->Add(RadioButtonDelete, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonMap = new wxRadioButton(this, ID_RADIOBUTTON1, _("Map effects to other elements"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON1"));
    FlexGridSizer1->Add(RadioButtonMap, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonRename = new wxRadioButton(this, ID_RADIOBUTTON3, _("Rename this element to:"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON3"));
    FlexGridSizer1->Add(RadioButtonRename, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    ChoiceModels = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_SORT, wxDefaultValidator, _T("ID_CHOICE1"));
    FlexGridSizer1->Add(ChoiceModels, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
    StdDialogButtonSizer1->Realize();
    FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SeqElementMismatchDialog::OnChoiceModelsSelect);
    //*)
    SetEscapeId(wxID_CANCEL);
}

SeqElementMismatchDialog::~SeqElementMismatchDialog()
{
    //(*Destroy(SeqElementMismatchDialog)
    //*)
}
/*
void SeqElementMismatchDialog::SetMessage(const wxString& message)
{
    StaticTextMessage=message;
}

*/

void SeqElementMismatchDialog::OnChoiceModelsSelect(wxCommandEvent& event)
{
    RadioButtonRename->SetValue(true);
}
