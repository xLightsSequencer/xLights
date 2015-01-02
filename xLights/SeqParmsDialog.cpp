#include "SeqParmsDialog.h"

//(*InternalHeaders(SeqParmsDialog)
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

//(*IdInit(SeqParmsDialog)
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
    wxFlexGridSizer* FlexGridSizer2;
    wxStaticBoxSizer* StaticBoxSizer1;
    wxFlexGridSizer* FlexGridSizer1;
    wxStdDialogButtonSizer* StdDialogButtonSizer1;

    Create(parent, id, _("RGB Sequence Display Elements"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
    SetClientSize(wxDefaultSize);
    Move(wxDefaultPosition);
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
    StdDialogButtonSizer1->Realize();
    FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_Filename = new wxStaticText(this, ID_STATICTEXT_FILENAME, _("Filename"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_FILENAME"));
    FlexGridSizer1->Add(StaticText_Filename, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 10);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Models to Include"));
    CheckListBox1 = new wxCheckListBox(this, ID_CHECKLISTBOX1, wxDefaultPosition, wxSize(325,158), 0, 0, wxLB_SORT, wxDefaultValidator, _T("ID_CHECKLISTBOX1"));
    StaticBoxSizer1->Add(CheckListBox1, 1, wxALL|wxEXPAND|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    FlexGridSizer2->Add(StaticBoxSizer1, 1, wxALL|wxEXPAND|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);
    //*)
}

SeqParmsDialog::~SeqParmsDialog()
{
    //(*Destroy(SeqParmsDialog)
    //*)
}
