#include "SeqParmsDialog.h"

//(*InternalHeaders(SeqParmsDialog)
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

//(*IdInit(SeqParmsDialog)
const long SeqParmsDialog::ID_CHECKBOX_All = wxNewId();
const long SeqParmsDialog::ID_STATICTEXT_FILENAME = wxNewId();
const long SeqParmsDialog::ID_CHECKLISTBOX1 = wxNewId();
//*)
    StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Models to Include"));
    CheckListBox1 = new wxCheckListBox(this, ID_CHECKLISTBOX1, wxDefaultPosition, wxSize(325,158), 0, 0, wxLB_EXTENDED|wxLB_SORT, wxDefaultValidator, _T("ID_CHECKLISTBOX1"));
    StaticBoxSizer1->Add(CheckListBox1, 1, wxALL|wxEXPAND|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    FlexGridSizer2->Add(StaticBoxSizer1, 1, wxALL|wxEXPAND|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_CHECKBOX_All,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&SeqParmsDialog::OnCheckBox_AllClick);
    //*)
}

SeqParmsDialog::~SeqParmsDialog()
{
    //(*Destroy(SeqParmsDialog)
    //*)
}

void SeqParmsDialog::OnCheckBox_AllClick(wxCommandEvent& event)
{
    wxArrayInt ignored;
    int numsel = Che
//(*IdInit(SeqParmsDialog)
const long SeqParmsDialog::ID_CHECKBOX_All = wxNewId();
const long SeqParmsDialog::ID_STATICTEXT_FILENAME = wxNewId();
const long SeqParmsDialog::ID_CHECKLISTBOX1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(SeqParmsDialog,wxDialog)
    //(*EventTable(SeqParmsDialog)
    //*)
END_EVENT_TABLE()

SeqParmsDialog::SeqParmsDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    //(*Initialize(SeqParmsDialog)
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer2;
    wxStaticBoxSizer* StaticBoxSizer1;
    wxFlexGridSizer* FlexGridSizer1;
    wxStdDialogButtonSizer* StdDialogButtonSizer1;

    Create(parent, wxID_ANY, _("RGB Sequence Display Elements"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
    StdDialogButtonSizer1->Realize();
    FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
    CheckBox_All = new wxCheckBox(this, ID_CHECKBOX_All, _("All"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_All"));
    CheckBox_All->SetValue(false);
    FlexGridSizer3->Add(CheckBox_All, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_Filename = new wxStaticText(this, ID_STATICTEXT_FILENAME, _("Filename"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_FILENAME"));
    FlexGridSizer3->Add(StaticText_Filename, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);ckListBox1->GetSelections(ignored);
    for (int i = 0; i < CheckListBox1->GetCount(); ++i

BEGIN_EVENT_TABLE(SeqParmsDialog,wxDialog)
    //(*EventTable(SeqParmsDialog)
    //*)
END_EVENT_TABLE()

SeqParmsDialog::SeqParmsDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    //(*Initialize(SeqParmsDialog)
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer2;
    wxStaticBoxSizer* StaticBoxSizer1;
    wxFlexGridSizer* FlexGridSizer1;
    wxStdDialogButtonSizer* StdDialogButtonSizer1;

    Create(parent, wxID_ANY, _("RGB Sequence Display Elements"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
    StdDialogButtonSizer1->Realize();
    FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
    CheckBox_All = new wxCheckBox(this, ID_CHECKBOX_All, _("All"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_All"));
    CheckBox_All->SetValue(false);
    FlexGridSizer3->Add(CheckBox_All, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_Filename = new wxStaticText(this, ID_STATICTEXT_FILENAME, _("Filename"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_FILENAME"));
    FlexGridSizer3->Add(StaticText_Filename, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);)
        CheckListBox1->Check(i, CheckBox_All->GetValue()); //numsel < CheckListBox1->GetCount()); //turn all on or off
//TODO: update label to tell what will happen next time? (in case it's not obvious)
}
