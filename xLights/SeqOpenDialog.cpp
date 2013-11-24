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
const long SeqOpenDialog::ID_RADIOBUTTON4 = wxNewId();
const long SeqOpenDialog::ID_STATICTEXT4 = wxNewId();
const long SeqOpenDialog::ID_CHOICE3 = wxNewId();
const long SeqOpenDialog::ID_RADIOBUTTON2 = wxNewId();
const long SeqOpenDialog::ID_STATICTEXT2 = wxNewId();
const long SeqOpenDialog::ID_CHOICE2 = wxNewId();
const long SeqOpenDialog::ID_RADIOBOX_Timing_Choice = wxNewId();
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

    Create(parent, wxID_ANY, _("Open Sequence"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
    SetClientSize(wxSize(298,262));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    RadioButtonXlights = new wxRadioButton(this, ID_RADIOBUTTON1, _("xLights Sequence"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP, wxDefaultValidator, _T("ID_RADIOBUTTON1"));
    RadioButtonXlights->SetValue(true);
    FlexGridSizer2->Add(RadioButtonXlights, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("File name"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    ChoiceSeqFiles = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_SORT, wxDefaultValidator, _T("ID_CHOICE1"));
    FlexGridSizer2->Add(ChoiceSeqFiles, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonLor = new wxRadioButton(this, ID_RADIOBUTTON4, _("LOR Sequence"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON4"));
    FlexGridSizer2->Add(RadioButtonLor, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("File name"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer2->Add(StaticText4, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    ChoiceLorFiles = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
    FlexGridSizer2->Add(ChoiceLorFiles, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonNewMusic = new wxRadioButton(this, ID_RADIOBUTTON2, _("New Musical Sequence"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON2"));
    FlexGridSizer2->Add(RadioButtonNewMusic, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Media file"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    ChoiceMediaFiles = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_SORT, wxDefaultValidator, _T("ID_CHOICE2"));
    FlexGridSizer2->Add(ChoiceMediaFiles, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    wxString __wxRadioBoxChoices_1[3] =
    {
    	_("None"),
    	_("Audacity"),
    	_("Xlights XML")
    };
    RadioBoxTimingChoice = new wxRadioBox(this, ID_RADIOBOX_Timing_Choice, _("Timing Import"), wxDefaultPosition, wxDefaultSize, 3, __wxRadioBoxChoices_1, 1, wxRA_SPECIFY_ROWS, wxDefaultValidator, _T("ID_RADIOBOX_Timing_Choice"));
    RadioBoxTimingChoice->Disable();
    FlexGridSizer2->Add(RadioBoxTimingChoice, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
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
    SetSizer(FlexGridSizer1);
    Layout();

    Connect(ID_RADIOBUTTON1,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&SeqOpenDialog::OnRadioButtonXlightsSelect);
    Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SeqOpenDialog::OnChoiceSeqFilesSelect);
    Connect(ID_RADIOBUTTON4,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&SeqOpenDialog::OnRadioButtonLorSelect);
    Connect(ID_CHOICE3,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SeqOpenDialog::OnChoiceLorFilesSelect);
    Connect(ID_RADIOBUTTON2,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&SeqOpenDialog::OnRadioButtonNewMusicSelect);
    Connect(ID_CHOICE2,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SeqOpenDialog::OnChoiceMediaFilesSelect);
    Connect(ID_RADIOBOX_Timing_Choice,wxEVT_COMMAND_RADIOBOX_SELECTED,(wxObjectEventFunction)&SeqOpenDialog::OnRadioBox1Select1);
    Connect(ID_RADIOBUTTON3,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&SeqOpenDialog::OnRadioButtonNewAnimSelect);
    //*)
}

SeqOpenDialog::~SeqOpenDialog()
{
    //(*Destroy(SeqOpenDialog)
    //*)
}

void SeqOpenDialog::OnRadioBox1Select1(wxCommandEvent& event)
{

}
void SeqOpenDialog::OnChoiceSeqFilesSelect(wxCommandEvent& event)
{
    RadioButtonXlights->SetValue(true);

}

void SeqOpenDialog::OnChoiceMediaFilesSelect(wxCommandEvent& event)
{
    RadioButtonNewMusic->SetValue(true);


}

void SeqOpenDialog::OnRadioButtonXlightsSelect(wxCommandEvent& event)
{
    RadioBoxTimingChoice->Disable();
}

void SeqOpenDialog::OnRadioButtonNewMusicSelect(wxCommandEvent& event)
{
    RadioBoxTimingChoice->Enable();
}

void SeqOpenDialog::OnRadioButtonNewAnimSelect(wxCommandEvent& event)
{
    RadioBoxTimingChoice->Disable();
}

void SeqOpenDialog::OnRadioButtonLorSelect(wxCommandEvent& event)
{
    RadioBoxTimingChoice->Disable();
}

void SeqOpenDialog::OnChoiceLorFilesSelect(wxCommandEvent& event)
{
    RadioButtonLor->SetValue(true);
}
