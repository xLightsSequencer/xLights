#include "SeqOpenDialog.h"

//(*InternalHeaders(SeqOpenDialog)
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

//(*IdInit(SeqOpenDialog)
const long SeqOpenDialog::ID_RADIOBUTTON1 = wxNewId();
const long SeqOpenDialog::ID_STATICTEXT1 = wxNewId();
const long SeqOpenDialog::ID_CHOICE1 = wxNewId();
const long SeqOpenDialog::ID_RADIOBUTTON2 = wxNewId();
const long SeqOpenDialog::ID_STATICTEXT2 = wxNewId();
const long SeqOpenDialog::ID_CHOICE2 = wxNewId();
const long SeqOpenDialog::ID_RADIOBUTTON3 = wxNewId();
const long SeqOpenDialog::ID_STATICTEXT3 = wxNewId();
const long SeqOpenDialog::ID_SPINCTRL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(SeqOpenDialog,wxDialog)
    //(*EventTable(SeqOpenDialog)
    //*)
END_EVENT_TABLE()

SeqOpenDialog::SeqOpenDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    //(*Initialize(SeqOpenDialog)
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer1;
    wxStdDialogButtonSizer* StdDialogButtonSizer1;

    Create(parent, id, _("Open Sequence"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
    SetClientSize(wxDefaultSize);
    Move(wxDefaultPosition);
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    RadioButtonXlights = new wxRadioButton(this, ID_RADIOBUTTON1, _("xLights Sequence"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON1"));
    RadioButtonXlights->SetValue(true);
    FlexGridSizer2->Add(RadioButtonXlights, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("File name"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    ChoiceSeqFiles = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_SORT, wxDefaultValidator, _T("ID_CHOICE1"));
    FlexGridSizer2->Add(ChoiceSeqFiles, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonNewMusic = new wxRadioButton(this, ID_RADIOBUTTON2, _("New Musical Sequence"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON2"));
    FlexGridSizer2->Add(RadioButtonNewMusic, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Media file"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    ChoiceMediaFiles = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_SORT, wxDefaultValidator, _T("ID_CHOICE2"));
    FlexGridSizer2->Add(ChoiceMediaFiles, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonNewAnim = new wxRadioButton(this, ID_RADIOBUTTON3, _("New animation sequence"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON3"));
    FlexGridSizer2->Add(RadioButtonNewAnim, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Duration (sec)"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrlDuration = new wxSpinCtrl(this, ID_SPINCTRL1, _T("10"), wxDefaultPosition, wxDefaultSize, 0, 1, 300, 10, _T("ID_SPINCTRL1"));
    SpinCtrlDuration->SetValue(_T("10"));
    FlexGridSizer2->Add(SpinCtrlDuration, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
    StdDialogButtonSizer1->Realize();
    FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SeqOpenDialog::OnChoiceSeqFilesSelect);
    Connect(ID_CHOICE2,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SeqOpenDialog::OnChoiceMediaFilesSelect);
    //*)
}

SeqOpenDialog::~SeqOpenDialog()
{
    //(*Destroy(SeqOpenDialog)
    //*)
}


void SeqOpenDialog::OnChoiceSeqFilesSelect(wxCommandEvent& event)
{
    RadioButtonXlights->SetValue(true);
}

void SeqOpenDialog::OnChoiceMediaFilesSelect(wxCommandEvent& event)
{
    RadioButtonNewMusic->SetValue(true);
}
