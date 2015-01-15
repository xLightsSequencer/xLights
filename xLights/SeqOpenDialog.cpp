#include "SeqOpenDialog.h"

//(*InternalHeaders(SeqOpenDialog)
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

//(*IdInit(SeqOpenDialog)
const long SeqOpenDialog::ID_RADIOBUTTON1 = wxNewId();
const long SeqOpenDialog::ID_CHOICE1 = wxNewId();
const long SeqOpenDialog::ID_RADIOBUTTON4 = wxNewId();
const long SeqOpenDialog::ID_CHOICE3 = wxNewId();
const long SeqOpenDialog::ID_CHOICE2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(SeqOpenDialog,wxDialog)
    //(*EventTable(SeqOpenDialog)
    //*)
END_EVENT_TABLE()

SeqOpenDialog::SeqOpenDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    //(*Initialize(SeqOpenDialog)
    wxStaticText* StaticText2;
    wxFlexGridSizer* FlexGridSizer2;
    wxStaticText* StaticText1;
    wxStaticText* StaticText3;
    wxFlexGridSizer* FlexGridSizer1;
    wxStaticText* StaticText4;
    wxStdDialogButtonSizer* StdDialogButtonSizer1;

    Create(parent, wxID_ANY, _("Open Sequence"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
    SetClientSize(wxSize(298,262));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    RadioButtonBinary = new wxRadioButton(this, ID_RADIOBUTTON1, _("xLights Rendered Sequence"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP, wxDefaultValidator, _T("ID_RADIOBUTTON1"));
    RadioButtonBinary->SetValue(true);
    FlexGridSizer2->Add(RadioButtonBinary, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText1 = new wxStaticText(this, wxID_ANY, _("File name"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    ChoiceSeqBinaryFiles = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_SORT, wxDefaultValidator, _T("ID_CHOICE1"));
    FlexGridSizer2->Add(ChoiceSeqBinaryFiles, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonXML = new wxRadioButton(this, ID_RADIOBUTTON4, _("xLights XML Sequence"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON4"));
    FlexGridSizer2->Add(RadioButtonXML, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText3 = new wxStaticText(this, wxID_ANY, _("(new fseq file will be created)"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText4 = new wxStaticText(this, wxID_ANY, _("File name"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer2->Add(StaticText4, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    ChoiceSeqXMLFiles = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
    FlexGridSizer2->Add(ChoiceSeqXMLFiles, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText2 = new wxStaticText(this, wxID_ANY, _("Timing"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    SeqChoiceTiming = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
    SeqChoiceTiming->Append(_("25 ms"));
    SeqChoiceTiming->SetSelection( SeqChoiceTiming->Append(_("50 ms")) );
    SeqChoiceTiming->Append(_("100 ms"));
    FlexGridSizer2->Add(SeqChoiceTiming, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
    StdDialogButtonSizer1->Realize();
    FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    SetSizer(FlexGridSizer1);
    Layout();

    Connect(ID_RADIOBUTTON1,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&SeqOpenDialog::OnRadioButtonBinarySelect);
    Connect(ID_RADIOBUTTON4,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&SeqOpenDialog::OnRadioButtonXMLSelect);
    //*)
}

SeqOpenDialog::~SeqOpenDialog()
{
    //(*Destroy(SeqOpenDialog)
    //*)
}


void SeqOpenDialog::OnRadioButtonBinarySelect(wxCommandEvent& event)
{
    RadioButtonBinary->SetValue(true);
    RadioButtonXML->SetValue(false);
    ChoiceSeqBinaryFiles->Enable();
    ChoiceSeqXMLFiles->Disable();
}

void SeqOpenDialog::OnRadioButtonXMLSelect(wxCommandEvent& event)
{
    RadioButtonBinary->SetValue(false);
    RadioButtonXML->SetValue(true);
    ChoiceSeqBinaryFiles->Disable();
    ChoiceSeqXMLFiles->Enable();
}
