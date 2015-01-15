#include "NewSequenceDialog.h"

//(*InternalHeaders(NewSequenceDialog)
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

//(*IdInit(NewSequenceDialog)
const long NewSequenceDialog::ID_RADIOBUTTON4 = wxNewId();
const long NewSequenceDialog::ID_STATICTEXT4 = wxNewId();
const long NewSequenceDialog::ID_CHOICE3 = wxNewId();
const long NewSequenceDialog::ID_RADIOBUTTON2 = wxNewId();
const long NewSequenceDialog::ID_STATICTEXT2 = wxNewId();
const long NewSequenceDialog::ID_CHOICE2 = wxNewId();
const long NewSequenceDialog::ID_RADIOBOX_Timing_Choice = wxNewId();
const long NewSequenceDialog::ID_RADIOBUTTON3 = wxNewId();
const long NewSequenceDialog::ID_STATICTEXT5 = wxNewId();
const long NewSequenceDialog::ID_SpinCtrlDuration_Float = wxNewId();
const long NewSequenceDialog::ID_CHOICE1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(NewSequenceDialog,wxDialog)
	//(*EventTable(NewSequenceDialog)
	//*)
END_EVENT_TABLE()

NewSequenceDialog::NewSequenceDialog(wxWindow* parent,wxWindowID id)
{
	//(*Initialize(NewSequenceDialog)
	wxFlexGridSizer* FlexGridSizer2;
	wxStaticText* StaticText1;
	wxFlexGridSizer* FlexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, wxID_ANY, _("New Sequence"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	SetClientSize(wxSize(298,262));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	RadioButtonLor = new wxRadioButton(this, ID_RADIOBUTTON4, _("LOR Sequence"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON4"));
	FlexGridSizer2->Add(RadioButtonLor, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("File name"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer2->Add(StaticText4, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	ChoiceLorFiles = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
	FlexGridSizer2->Add(ChoiceLorFiles, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	RadioButtonNewMusic = new wxRadioButton(this, ID_RADIOBUTTON2, _("New Musical Sequence"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON2"));
	RadioButtonNewMusic->SetValue(true);
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
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Duration (sec)"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer2->Add(StaticText5, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrlDuration_Float = new wxTextCtrl(this, ID_SpinCtrlDuration_Float, _("10.0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SpinCtrlDuration_Float"));
	FlexGridSizer2->Add(SpinCtrlDuration_Float, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(this, wxID_ANY, _("New Sequence Timing"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	NewSequenceTiming = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	NewSequenceTiming->Append(_("25 ms"));
	NewSequenceTiming->SetSelection( NewSequenceTiming->Append(_("50 ms")) );
	NewSequenceTiming->Append(_("100 ms"));
	FlexGridSizer2->Add(NewSequenceTiming, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	SetSizer(FlexGridSizer1);
	Layout();

	Connect(ID_RADIOBUTTON4,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&NewSequenceDialog::OnRadioButtonLorSelect);
	Connect(ID_CHOICE3,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&NewSequenceDialog::OnChoiceLorFilesSelect);
	Connect(ID_RADIOBUTTON2,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&NewSequenceDialog::OnRadioButtonNewMusicSelect);
	Connect(ID_CHOICE2,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&NewSequenceDialog::OnChoiceMediaFilesSelect);
	Connect(ID_RADIOBOX_Timing_Choice,wxEVT_COMMAND_RADIOBOX_SELECTED,(wxObjectEventFunction)&NewSequenceDialog::OnRadioBox1Select1);
	Connect(ID_RADIOBUTTON3,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&NewSequenceDialog::OnRadioButtonNewAnimSelect);
	//*)
}

NewSequenceDialog::~NewSequenceDialog()
{
	//(*Destroy(NewSequenceDialog)
	//*)
}



void NewSequenceDialog::OnRadioBox1Select1(wxCommandEvent& event)
{
    
}
void NewSequenceDialog::OnChoiceSeqFilesSelect(wxCommandEvent& event)
{
    
}

void NewSequenceDialog::OnChoiceMediaFilesSelect(wxCommandEvent& event)
{
    RadioButtonNewMusic->SetValue(true);
    
    
}

void NewSequenceDialog::OnRadioButtonXlightsSelect(wxCommandEvent& event)
{
    RadioBoxTimingChoice->Disable();
}

void NewSequenceDialog::OnRadioButtonNewMusicSelect(wxCommandEvent& event)
{
    RadioBoxTimingChoice->Enable();
}

void NewSequenceDialog::OnRadioButtonNewAnimSelect(wxCommandEvent& event)
{
    RadioBoxTimingChoice->Disable();
}

void NewSequenceDialog::OnRadioButtonLorSelect(wxCommandEvent& event)
{
    RadioBoxTimingChoice->Disable();
}

void NewSequenceDialog::OnChoiceLorFilesSelect(wxCommandEvent& event)
{
    RadioButtonLor->SetValue(true);
}
