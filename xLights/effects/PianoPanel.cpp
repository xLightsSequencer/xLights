#include "PianoPanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"
#include "PianoEffect.h"
#include <wx/textfile.h>

//(*InternalHeaders(PianoPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/bitmap.h>
#include <wx/spinctrl.h>
#include <wx/slider.h>
#include <wx/filedlg.h>
#include <wx/choice.h>
#include <wx/bmpbuttn.h>
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/image.h>
#include <wx/string.h>
//*)


#include "wx/filedlg.h"
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/filepicker.h>

#include "../MIDI/MidiFile.h"
#include <log4cpp/Category.hh>
#include "../xLightsMain.h"

//(*IdInit(PianoPanel)
const long PianoPanel::ID_STATICTEXT3 = wxNewId();
const long PianoPanel::ID_CHOICE_Piano_Notes_Source = wxNewId();
const long PianoPanel::ID_BITMAPBUTTON_Piano_Notes_Source = wxNewId();
const long PianoPanel::ID_STATICTEXT1 = wxNewId();
const long PianoPanel::ID_CHOICE_Piano_Type = wxNewId();
const long PianoPanel::ID_BITMAPBUTTON_Piano_Type = wxNewId();
const long PianoPanel::ID_STATICTEXT2 = wxNewId();
const long PianoPanel::ID_SPINCTRL_Piano_StartMIDI = wxNewId();
const long PianoPanel::ID_BITMAPBUTTON_Piano_StartMIDI = wxNewId();
const long PianoPanel::ID_STATICTEXT_Piano_NumKeys = wxNewId();
const long PianoPanel::ID_SPINCTRL_Piano_EndMIDI = wxNewId();
const long PianoPanel::ID_BITMAPBUTTON_Piano_EndMIDI = wxNewId();
const long PianoPanel::ID_STATICTEXT_Piano_NumRows = wxNewId();
const long PianoPanel::ID_CHECKBOX_Piano_ShowSharps = wxNewId();
const long PianoPanel::ID_BITMAPBUTTON_Piano_ShowSharps = wxNewId();
const long PianoPanel::ID_STATICTEXT7 = wxNewId();
const long PianoPanel::IDD_SLIDER_Piano_Scale = wxNewId();
const long PianoPanel::ID_TEXTCTRL_Piano_Scale = wxNewId();
const long PianoPanel::ID_BITMAPBUTTON1 = wxNewId();
const long PianoPanel::ID_STATICTEXT4 = wxNewId();
const long PianoPanel::ID_TEXTCTRL_Piano_File = wxNewId();
const long PianoPanel::ID_BUTTON_Piano_File = wxNewId();
const long PianoPanel::ID_STATICTEXT8 = wxNewId();
const long PianoPanel::ID_CHOICE_Piano_MIDITrack_APPLYLAST = wxNewId();
const long PianoPanel::ID_STATICTEXT9 = wxNewId();
const long PianoPanel::ID_BITMAPBUTTON_Piano_MIDITrack_APPLYLAST = wxNewId();
const long PianoPanel::ID_STATICTEXT5 = wxNewId();
const long PianoPanel::IDD_SLIDER_Piano_MIDI_Start = wxNewId();
const long PianoPanel::ID_TEXTCTRL_Piano_MIDI_Start = wxNewId();
const long PianoPanel::ID_BITMAPBUTTON_Piano_MIDI_Start = wxNewId();
const long PianoPanel::ID_STATICTEXT6 = wxNewId();
const long PianoPanel::IDD_SLIDER_Piano_MIDI_Speed = wxNewId();
const long PianoPanel::ID_TEXTCTRL_Piano_MIDI_Speed = wxNewId();
const long PianoPanel::ID_BITMAPBUTTON_Piano_MIDI_Speed = wxNewId();
//*)

BEGIN_EVENT_TABLE(PianoPanel,wxPanel)
	//(*EventTable(PianoPanel)
	//*)
END_EVENT_TABLE()

PianoPanel::PianoPanel(wxWindow* parent)
{
	_media = NULL;

	//(*Initialize(PianoPanel)
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer27;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer42;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer42 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer42->AddGrowableCol(0);
	FlexGridSizer27 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer27->AddGrowableCol(1);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Notes Source"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer27->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Piano_Notes_Source = new wxChoice(this, ID_CHOICE_Piano_Notes_Source, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Piano_Notes_Source"));
	Choice_Piano_Notes_Source->Append(_("Polyphonic Transcription"));
	Choice_Piano_Notes_Source->SetSelection( Choice_Piano_Notes_Source->Append(_("Audacity Timing File")) );
	Choice_Piano_Notes_Source->Append(_("MIDI File"));
	FlexGridSizer27->Add(Choice_Piano_Notes_Source, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Piano_Notes_Source = new wxBitmapButton(this, ID_BITMAPBUTTON_Piano_Notes_Source, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Piano_Notes_Source"));
	BitmapButton_Piano_Notes_Source->SetDefault();
	FlexGridSizer27->Add(BitmapButton_Piano_Notes_Source, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer27->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Piano_Type = new wxChoice(this, ID_CHOICE_Piano_Type, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Piano_Type"));
	Choice_Piano_Type->SetSelection( Choice_Piano_Type->Append(_("True Piano")) );
	Choice_Piano_Type->Append(_("Bars"));
	FlexGridSizer27->Add(Choice_Piano_Type, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Piano_Type = new wxBitmapButton(this, ID_BITMAPBUTTON_Piano_Type, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Piano_Type"));
	BitmapButton_Piano_Type->SetDefault();
	FlexGridSizer27->Add(BitmapButton_Piano_Type, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Start MIDI Channel [0-127]"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer27->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	SpinCtrl_Piano_StartMIDI = new wxSpinCtrl(this, ID_SPINCTRL_Piano_StartMIDI, _T("60"), wxDefaultPosition, wxDefaultSize, 0, 0, 127, 60, _T("ID_SPINCTRL_Piano_StartMIDI"));
	SpinCtrl_Piano_StartMIDI->SetValue(_T("60"));
	FlexGridSizer27->Add(SpinCtrl_Piano_StartMIDI, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Piano_StartMIDI = new wxBitmapButton(this, ID_BITMAPBUTTON_Piano_StartMIDI, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Piano_StartMIDI"));
	BitmapButton_Piano_StartMIDI->SetDefault();
	FlexGridSizer27->Add(BitmapButton_Piano_StartMIDI, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText_Piano_NumKeys = new wxStaticText(this, ID_STATICTEXT_Piano_NumKeys, _("End MIDI Channel"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Piano_NumKeys"));
	FlexGridSizer27->Add(StaticText_Piano_NumKeys, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	SpinCtrl_Piano_EndMIDI = new wxSpinCtrl(this, ID_SPINCTRL_Piano_EndMIDI, _T("72"), wxDefaultPosition, wxDefaultSize, 0, 0, 127, 72, _T("ID_SPINCTRL_Piano_EndMIDI"));
	SpinCtrl_Piano_EndMIDI->SetValue(_T("72"));
	FlexGridSizer27->Add(SpinCtrl_Piano_EndMIDI, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Piano_EndMIDI = new wxBitmapButton(this, ID_BITMAPBUTTON_Piano_EndMIDI, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Piano_EndMIDI"));
	BitmapButton_Piano_EndMIDI->SetDefault();
	FlexGridSizer27->Add(BitmapButton_Piano_EndMIDI, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText_Piano_NumRows = new wxStaticText(this, ID_STATICTEXT_Piano_NumRows, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Piano_NumRows"));
	FlexGridSizer27->Add(StaticText_Piano_NumRows, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	CheckBox_Piano_ShowSharps = new wxCheckBox(this, ID_CHECKBOX_Piano_ShowSharps, _("Show Sharps and Flats"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Piano_ShowSharps"));
	CheckBox_Piano_ShowSharps->SetValue(true);
	FlexGridSizer27->Add(CheckBox_Piano_ShowSharps, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Piano_ShowSharps = new wxBitmapButton(this, ID_BITMAPBUTTON_Piano_ShowSharps, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Piano_ShowSharps"));
	BitmapButton_Piano_ShowSharps->SetDefault();
	FlexGridSizer27->Add(BitmapButton_Piano_ShowSharps, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer42->Add(FlexGridSizer27, 0, wxEXPAND, 2);
	FlexGridSizer3 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer3->AddGrowableCol(1);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Scale"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer3->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Piano_Scale = new wxSlider(this, IDD_SLIDER_Piano_Scale, 100, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Piano_Scale"));
	FlexGridSizer3->Add(Slider_Piano_Scale, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Piano_Scale = new wxTextCtrl(this, ID_TEXTCTRL_Piano_Scale, _("100"), wxDefaultPosition, wxSize(44,24), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Piano_Scale"));
	FlexGridSizer3->Add(TextCtrl_Piano_Scale, 1, wxALL, 2);
	BitmapButton1 = new wxBitmapButton(this, ID_BITMAPBUTTON1, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON1"));
	BitmapButton1->SetDefault();
	FlexGridSizer3->Add(BitmapButton1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer42->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer1 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("File"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Piano_File = new wxTextCtrl(this, ID_TEXTCTRL_Piano_File, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_Piano_File"));
	TextCtrl_Piano_File->SetToolTip(_("The file should be a Polyphonic Transcription file consisting of 3 floating point numbers per line. #1 is the start time in seconds. #2 the end time. #3 the MIDI channel number."));
	FlexGridSizer1->Add(TextCtrl_Piano_File, 1, wxALL|wxEXPAND, 2);
	Button_Piano_File = new wxButton(this, ID_BUTTON_Piano_File, _("..."), wxDefaultPosition, wxSize(32,28), 0, wxDefaultValidator, _T("ID_BUTTON_Piano_File"));
	FlexGridSizer1->Add(Button_Piano_File, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer42->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer2 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("Track"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer2->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Piano_MIDITrack_APPLYLAST = new wxChoice(this, ID_CHOICE_Piano_MIDITrack_APPLYLAST, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Piano_MIDITrack_APPLYLAST"));
	FlexGridSizer2->Add(Choice_Piano_MIDITrack_APPLYLAST, 1, wxALL|wxEXPAND, 2);
	StaticText9 = new wxStaticText(this, ID_STATICTEXT9, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
	FlexGridSizer2->Add(StaticText9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Piano_MIDITrack_APPLYLAST = new wxBitmapButton(this, ID_BITMAPBUTTON_Piano_MIDITrack_APPLYLAST, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Piano_MIDITrack_APPLYLAST"));
	BitmapButton_Piano_MIDITrack_APPLYLAST->SetDefault();
	FlexGridSizer2->Add(BitmapButton_Piano_MIDITrack_APPLYLAST, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("MIDI Start Time Adjust"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer2->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Piano_MIDI_Start = new wxSlider(this, IDD_SLIDER_Piano_MIDI_Start, 0, -1000, 1000, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Piano_MIDI_Start"));
	FlexGridSizer2->Add(Slider_Piano_MIDI_Start, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Piano_MIDI_Start = new wxTextCtrl(this, ID_TEXTCTRL_Piano_MIDI_Start, _("0"), wxDefaultPosition, wxSize(44,24), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Piano_MIDI_Start"));
	FlexGridSizer2->Add(TextCtrl_Piano_MIDI_Start, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Piano_MIDI_Start = new wxBitmapButton(this, ID_BITMAPBUTTON_Piano_MIDI_Start, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Piano_MIDI_Start"));
	BitmapButton_Piano_MIDI_Start->SetDefault();
	FlexGridSizer2->Add(BitmapButton_Piano_MIDI_Start, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("MIDI Speed Adjust"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer2->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Piano_MIDI_Speed = new wxSlider(this, IDD_SLIDER_Piano_MIDI_Speed, 0, -100, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Piano_MIDI_Speed"));
	FlexGridSizer2->Add(Slider_Piano_MIDI_Speed, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Piano_MIDI_Speed = new wxTextCtrl(this, ID_TEXTCTRL_Piano_MIDI_Speed, _("0"), wxDefaultPosition, wxSize(44,24), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Piano_MIDI_Speed"));
	FlexGridSizer2->Add(TextCtrl_Piano_MIDI_Speed, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Piano_MIDI_Speed = new wxBitmapButton(this, ID_BITMAPBUTTON_Piano_MIDI_Speed, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Piano_MIDI_Speed"));
	BitmapButton_Piano_MIDI_Speed->SetDefault();
	FlexGridSizer2->Add(BitmapButton_Piano_MIDI_Speed, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer42->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 2);
	SetSizer(FlexGridSizer42);
	FileDialog1 = new wxFileDialog(this, _("Select Polyphonic Transcription file"), wxEmptyString, wxEmptyString, _("Text Files|*.txt"), wxFD_DEFAULT_STYLE, wxDefaultPosition, wxDefaultSize, _T("wxFileDialog"));
	FlexGridSizer42->Fit(this);
	FlexGridSizer42->SetSizeHints(this);

	Connect(ID_CHOICE_Piano_Notes_Source,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&PianoPanel::OnChoice_Piano_Notes_SourceSelect);
	Connect(ID_BITMAPBUTTON_Piano_Notes_Source,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_Piano_Type,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_Piano_StartMIDI,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_Piano_EndMIDI,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_Piano_ShowSharps,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnLockButtonClick);
	Connect(IDD_SLIDER_Piano_Scale,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&PianoPanel::UpdateLinkedTextCtrl);
	Connect(ID_TEXTCTRL_Piano_Scale,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PianoPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnLockButtonClick);
	Connect(ID_TEXTCTRL_Piano_File,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PianoPanel::OnTextCtrl_Piano_FileText);
	Connect(ID_BUTTON_Piano_File,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnButton_Piano_FileClick);
	Connect(ID_BITMAPBUTTON_Piano_MIDITrack_APPLYLAST,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnLockButtonClick);
	Connect(IDD_SLIDER_Piano_MIDI_Start,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&PianoPanel::UpdateLinkedTextCtrl);
	Connect(ID_TEXTCTRL_Piano_MIDI_Start,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PianoPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_Piano_MIDI_Start,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnLockButtonClick);
	Connect(IDD_SLIDER_Piano_MIDI_Speed,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&PianoPanel::UpdateLinkedTextCtrl);
	Connect(ID_TEXTCTRL_Piano_MIDI_Speed,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PianoPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_Piano_MIDI_Speed,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnLockButtonClick);
	//*)
    SetName("ID_PANEL_PIANO");

	ValidateWindow();
	// this is slow so we do it outside of the commonly called ValidateWindow
	MIDIExtraValidateWindow();
	AudacityExtraValidateWindow();
}

PianoPanel::~PianoPanel()
{
	//(*Destroy(PianoPanel)
	//*)
}

PANEL_EVENT_HANDLERS(PianoPanel)
void PianoPanel::OnSpinCtrl_Piano_StartMIDIChange(wxSpinEvent& event)
{
	int start = SpinCtrl_Piano_StartMIDI->GetValue();
	int end = SpinCtrl_Piano_EndMIDI->GetValue();

	if (start > end)
	{
		SpinCtrl_Piano_EndMIDI->SetValue(start);
	}
}

void PianoPanel::OnSpinCtrl_Piano_EndMIDIChange(wxSpinEvent& event)
{
	int start = SpinCtrl_Piano_StartMIDI->GetValue();
	int end = SpinCtrl_Piano_EndMIDI->GetValue();

	if (end < start)
	{
		SpinCtrl_Piano_StartMIDI->SetValue(end);
	}
}

void Progress(wxProgressDialog* pd, int p)
{
	if (pd != NULL)
	{
		pd->Update(p);
	}
}

void PianoPanel::OnChoice_Piano_Notes_SourceSelect(wxCommandEvent& event)
{
	wxString notes = Choice_Piano_Notes_Source->GetStringSelection();
	if (notes == "Polyphonic Transcription")
	{
		if (_media != NULL)
		{
			try
			{
				if (!_media->IsPolyphonicTranscriptionDone())
				{
					wxProgressDialog pd("Processing Audio", "");
					_media->DoPolyphonicTranscription(&pd, &Progress);
				}
			}
			catch (...)
			{
				log4cpp::Category& logger = log4cpp::Category::getRoot();
				logger.warn("Exception caught processing Polyphonic Transcription");
			}
		}
	}

	ValidateWindow();

	// this is slow so we do it outside of the commonly called ValidateWindow
	MIDIExtraValidateWindow();
	AudacityExtraValidateWindow();
}

void PianoPanel::AudacityExtraValidateWindow()
{
	wxString notes = Choice_Piano_Notes_Source->GetStringSelection();
	if (notes == "Audacity Timing File")
	{
		wxString file = TextCtrl_Piano_File->GetValue();
		wxTextFile f(file);

		if (file != "" && wxFile::Exists(file))
		{
			if (f.Open())
			{
				wxString l = f.GetNextLine();
				std::vector<float> components = PianoEffect::Parse(l);
				if (components.size() != 3)
				{
					TextCtrl_Piano_File->SetBackgroundColour(*wxRED);
				}
			}
		}
		else
		{
			TextCtrl_Piano_File->SetBackgroundColour(*wxRED);
		}
	}
}

bool PianoPanel::MIDITrackContainsNotes(int track, MidiFile* midifile)
{
	for (int i = 0; i < midifile->getNumEvents(track); i++)
	{
		MidiEvent e = midifile->getEvent(track, i);
		if (e.isNote())
		{
			return true;
		}
	}

	return false;
}

void PianoPanel::MIDIExtraValidateWindow()
{
	wxString notes = Choice_Piano_Notes_Source->GetStringSelection();
	if (notes == "MIDI File")
	{
		wxString file = TextCtrl_Piano_File->GetValue();
		MidiFile midifile;
		if (file == "" || !wxFile::Exists(file) || midifile.read(file) == 0)
		{
			// invalid midi file
			TextCtrl_Piano_File->SetBackgroundColour(*wxRED);
			Choice_Piano_MIDITrack_APPLYLAST->Enable(false);
			BitmapButton_Piano_MIDITrack_APPLYLAST->Enable(false);
		}
		else
		{
			std::string selection = std::string(Choice_Piano_MIDITrack_APPLYLAST->GetStringSelection().c_str());
			int tracks = midifile.getNumTracks();
			Choice_Piano_MIDITrack_APPLYLAST->Clear();
			Choice_Piano_MIDITrack_APPLYLAST->Append("All");
			if (selection == "All")
			{
				Choice_Piano_MIDITrack_APPLYLAST->Select(0);
			}
			for (int i = 1; i <= tracks; i++)
			{
				if (MIDITrackContainsNotes(i-1, &midifile))
				{
					std::string n = std::string(wxString::Format("%i", i).c_str());
					Choice_Piano_MIDITrack_APPLYLAST->Append(n);
					if (selection == n)
					{
						Choice_Piano_MIDITrack_APPLYLAST->Select(i);
					}
				}
			}
			if (Choice_Piano_MIDITrack_APPLYLAST->GetStringSelection() == "")
			{
				Choice_Piano_MIDITrack_APPLYLAST->Select(0);
			}
			Choice_Piano_MIDITrack_APPLYLAST->Enable(true);
			BitmapButton_Piano_MIDITrack_APPLYLAST->Enable(true);
		}
	}
}

void PianoPanel::ValidateWindow()
{
	wxString notes = Choice_Piano_Notes_Source->GetStringSelection();
	if (notes == "Polyphonic Transcription")
	{
		TextCtrl_Piano_File->Enable(false);
		Button_Piano_File->Enable(false);
		Slider_Piano_MIDI_Speed->Enable(false);
		Slider_Piano_MIDI_Start->Enable(false);
		TextCtrl_Piano_MIDI_Speed->Enable(false);
		TextCtrl_Piano_MIDI_Start->Enable(false);
		BitmapButton_Piano_MIDI_Speed->Enable(false);
		BitmapButton_Piano_MIDI_Start->Enable(false);
		Choice_Piano_MIDITrack_APPLYLAST->Enable(false);
		BitmapButton_Piano_MIDITrack_APPLYLAST->Enable(false);
	}
	else if (notes == "Audacity Timing File")
	{
		TextCtrl_Piano_File->Enable(true);
		Button_Piano_File->Enable(true);
		Slider_Piano_MIDI_Speed->Enable(false);
		Slider_Piano_MIDI_Start->Enable(false);
		TextCtrl_Piano_MIDI_Speed->Enable(false);
		TextCtrl_Piano_MIDI_Start->Enable(false);
		BitmapButton_Piano_MIDI_Speed->Enable(false);
		BitmapButton_Piano_MIDI_Start->Enable(false);
		Choice_Piano_MIDITrack_APPLYLAST->Enable(false);
		BitmapButton_Piano_MIDITrack_APPLYLAST->Enable(false);
	}
	else if (notes == "MIDI File")
	{
		TextCtrl_Piano_File->Enable(true);
		Button_Piano_File->Enable(true);
		Slider_Piano_MIDI_Speed->Enable(true);
		Slider_Piano_MIDI_Start->Enable(true);
		TextCtrl_Piano_MIDI_Speed->Enable(true);
		TextCtrl_Piano_MIDI_Start->Enable(true);
		BitmapButton_Piano_MIDI_Speed->Enable(true);
		BitmapButton_Piano_MIDI_Start->Enable(true);
	}

	wxString file = TextCtrl_Piano_File->GetValue();
	if (file == "" || wxFile::Exists(file))
	{
		TextCtrl_Piano_File->SetBackgroundColour(*wxWHITE);
	}
	else
	{
		TextCtrl_Piano_File->SetBackgroundColour(*wxRED);
	}
}
void PianoPanel::OnTextCtrl_Piano_FileText(wxCommandEvent& event)
{
    ValidateWindow();
	// this is slow so we do it outside of the commonly called ValidateWindow
	MIDIExtraValidateWindow();
	AudacityExtraValidateWindow();
}

void PianoPanel::OnButton_Piano_FileClick(wxCommandEvent& event)
{
	wxString notes = Choice_Piano_Notes_Source->GetStringSelection();
	if (notes == "Audacity Timing File")
	{
		FileDialog1->SetWildcard("Text Files|*.txt|All Files|*.*");
	}
	else if (notes == "MIDI File")
	{
		FileDialog1->SetWildcard("MIDI Files|*.mid;*.midi|All Files|*.*");
	}
	if (FileDialog1->ShowModal() == wxID_OK)
	{
		TextCtrl_Piano_File->SetValue(FileDialog1->GetDirectory() +"/"+ FileDialog1->GetFilename());
		ValidateWindow();
		// this is slow so we do it outside of the commonly called ValidateWindow
		MIDIExtraValidateWindow();
		AudacityExtraValidateWindow();
	}
}
