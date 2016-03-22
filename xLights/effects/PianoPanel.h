#ifndef PIANOPANEL_H
#define PIANOPANEL_H

#include <wx/progdlg.h>

//(*Headers(PianoPanel)
#include <wx/panel.h>
class wxSpinEvent;
class wxFileDialog;
class wxCheckBox;
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxSpinCtrl;
class wxButton;
class wxChoice;
//*)

#include "../AudioManager.h"
class MidiFile;

class PianoPanel: public wxPanel
{
	public:

		PianoPanel(wxWindow* parent);
		virtual ~PianoPanel();
		void SetAudio(AudioManager* audio) { _media = audio; };

		//(*Declarations(PianoPanel)
		wxStaticText* StaticText9;
		wxBitmapButton* BitmapButton_Piano_MIDITrack_APPLYLAST;
		wxBitmapButton* BitmapButton_Piano_Type;
		wxTextCtrl* TextCtrl_Piano_File;
		wxTextCtrl* TextCtrl_Piano_Scale;
		wxStaticText* StaticText2;
		wxSlider* Slider_Piano_Scale;
		wxStaticText* StaticText_Piano_NumKeys;
		wxCheckBox* CheckBox_Piano_ShowSharps;
		wxStaticText* StaticText6;
		wxBitmapButton* BitmapButton_Piano_StartMIDI;
		wxSlider* Slider_Piano_MIDI_Speed;
		wxStaticText* StaticText8;
		wxSpinCtrl* SpinCtrl_Piano_StartMIDI;
		wxBitmapButton* BitmapButton_Piano_EndMIDI;
		wxTextCtrl* TextCtrl_Piano_MIDI_Speed;
		wxChoice* Choice_Piano_Notes_Source;
		wxBitmapButton* BitmapButton_Piano_MIDI_Speed;
		wxChoice* Choice_Piano_MIDITrack_APPLYLAST;
		wxStaticText* StaticText1;
		wxFileDialog* FileDialog1;
		wxStaticText* StaticText_Piano_NumRows;
		wxBitmapButton* BitmapButton_Piano_MIDI_Start;
		wxStaticText* StaticText3;
		wxBitmapButton* BitmapButton1;
		wxSpinCtrl* SpinCtrl_Piano_EndMIDI;
		wxStaticText* StaticText5;
		wxStaticText* StaticText7;
		wxBitmapButton* BitmapButton_Piano_Notes_Source;
		wxChoice* Choice_Piano_Type;
		wxSlider* Slider_Piano_MIDI_Start;
		wxStaticText* StaticText4;
		wxButton* Button_Piano_File;
		wxTextCtrl* TextCtrl_Piano_MIDI_Start;
		wxBitmapButton* BitmapButton_Piano_ShowSharps;
		//*)

	protected:

		//(*Identifiers(PianoPanel)
		static const long ID_STATICTEXT3;
		static const long ID_CHOICE_Piano_Notes_Source;
		static const long ID_BITMAPBUTTON_Piano_Notes_Source;
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE_Piano_Type;
		static const long ID_BITMAPBUTTON_Piano_Type;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL_Piano_StartMIDI;
		static const long ID_BITMAPBUTTON_Piano_StartMIDI;
		static const long ID_STATICTEXT_Piano_NumKeys;
		static const long ID_SPINCTRL_Piano_EndMIDI;
		static const long ID_BITMAPBUTTON_Piano_EndMIDI;
		static const long ID_STATICTEXT_Piano_NumRows;
		static const long ID_CHECKBOX_Piano_ShowSharps;
		static const long ID_BITMAPBUTTON_Piano_ShowSharps;
		static const long ID_STATICTEXT7;
		static const long IDD_SLIDER_Piano_Scale;
		static const long ID_TEXTCTRL_Piano_Scale;
		static const long ID_BITMAPBUTTON1;
		static const long ID_STATICTEXT4;
		static const long ID_TEXTCTRL_Piano_File;
		static const long ID_BUTTON_Piano_File;
		static const long ID_STATICTEXT8;
		static const long ID_CHOICE_Piano_MIDITrack_APPLYLAST;
		static const long ID_STATICTEXT9;
		static const long ID_BITMAPBUTTON_Piano_MIDITrack_APPLYLAST;
		static const long ID_STATICTEXT5;
		static const long IDD_SLIDER_Piano_MIDI_Start;
		static const long ID_TEXTCTRL_Piano_MIDI_Start;
		static const long ID_BITMAPBUTTON_Piano_MIDI_Start;
		static const long ID_STATICTEXT6;
		static const long IDD_SLIDER_Piano_MIDI_Speed;
		static const long ID_TEXTCTRL_Piano_MIDI_Speed;
		static const long ID_BITMAPBUTTON_Piano_MIDI_Speed;
		//*)
		AudioManager* _media;

	public:

		//(*Handlers(PianoPanel)
		void UpdateLinkedSliderFloat(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
		void UpdateLinkedTextCtrl360(wxScrollEvent& event);
		void UpdateLinkedSlider360(wxCommandEvent& event);
		void UpdateLinkedTextCtrl(wxScrollEvent& event);
		void UpdateLinkedSlider(wxCommandEvent& event);
		void OnLockButtonClick(wxCommandEvent& event);
		void OnPiano_StyleSelect(wxCommandEvent& event);
		void OnSlider_Piano_NumKeysCmdScroll(wxScrollEvent& event);
		void OnTextCtrl_Piano_FilenameText(wxCommandEvent& event);
		void OnButton_Piano_FilenameClick(wxCommandEvent& event);
		void OnSlider_Piano_KeyWidthCmdScroll(wxScrollEvent& event);
		void OnButton_Piano_CueFilenameClick(wxCommandEvent& event);
		void OnButton_Piano_MapFilenameClick(wxCommandEvent& event);
		void OnButton_Piano_ShapeFilenameClick(wxCommandEvent& event);
		void OnTextCtrl_Piano_CueFilenameText(wxCommandEvent& event);
		void OnTextCtrl_Piano_MapFilenameText(wxCommandEvent& event);
		void OnTextCtrl_Piano_ShapeFilenameText(wxCommandEvent& event);
		void OnSlider_Piano_NumRowsCmdScroll(wxScrollEvent& event);
		void OnSpinCtrl_Piano_StartMIDIChange(wxSpinEvent& event);
		void OnSpinCtrl_Piano_EndMIDIChange(wxSpinEvent& event);
		void OnChoice_Piano_Notes_SourceSelect(wxCommandEvent& event);
		void OnTextCtrl_Piano_FileText(wxCommandEvent& event);
		void OnButton_Piano_FileClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
		void ValidateWindow();
		void MIDIExtraValidateWindow();
		void AudacityExtraValidateWindow();
		bool MIDITrackContainsNotes(int track, MidiFile* midifile);
};

#endif
