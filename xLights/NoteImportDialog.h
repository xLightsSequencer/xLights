#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

//(*Headers(NoteImportDialog)
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/filedlg.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class MidiFile;
class SequenceElements;

class NoteImportDialog: public wxDialog
{
    SequenceElements& _sequenceElements;

    bool IsTimingUnique(wxString name) const;
    void ValidateWindow();
    void MIDIExtraValidateWindow();
    void MusicXMLExtraValidateWindow();
    void AudacityExtraValidateWindow();
    bool MIDITrackContainsNotes(int track, MidiFile* midifile) const;

	public:

    static std::vector<float> Parse(wxString& l);
        NoteImportDialog(wxWindow* parent, SequenceElements& sequenceElements, bool mediaPresent, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~NoteImportDialog();

		//(*Declarations(NoteImportDialog)
		wxButton* ButtonCancel;
		wxButton* ButtonOk;
		wxButton* Button_Piano_File;
		wxChoice* Choice_Piano_MIDITrack_APPLYLAST;
		wxChoice* Choice_Piano_Notes_Source;
		wxFileDialog* FileDialog1;
		wxSlider* Slider_Piano_MIDI_Speed;
		wxSlider* Slider_Piano_MIDI_Start;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText8;
		wxTextCtrl* TextCtrl_Piano_File;
		wxTextCtrl* TextCtrl_Piano_MIDI_Speed;
		wxTextCtrl* TextCtrl_Piano_MIDI_Start;
		wxTextCtrl* TextCtrl_TimingName;
		//*)

	protected:

		//(*Identifiers(NoteImportDialog)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT3;
		static const long ID_CHOICE_Piano_Notes_Source;
		static const long ID_STATICTEXT4;
		static const long ID_TEXTCTRL_Piano_File;
		static const long ID_BUTTON_Piano_File;
		static const long ID_STATICTEXT8;
		static const long ID_CHOICE_Piano_MIDITrack_APPLYLAST;
		static const long ID_STATICTEXT5;
		static const long ID_SLIDER_Piano_MIDI_Start;
		static const long IDD_TEXTCTRL_Piano_MIDI_Start;
		static const long ID_STATICTEXT6;
		static const long ID_SLIDER_Piano_MIDI_Speed;
		static const long IDD_TEXTCTRL_Piano_MIDI_Speed;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(NoteImportDialog)
		void OnChoice_Piano_Notes_SourceSelect(wxCommandEvent& event);
		void OnButton_Piano_FileClick(wxCommandEvent& event);
		void OnTextCtrl_Piano_FileText(wxCommandEvent& event);
		void OnButtonOkClick(wxCommandEvent& event);
		void OnButtonCancelClick(wxCommandEvent& event);
		void OnTextCtrl_TimingNameText(wxCommandEvent& event);
		void OnSlider_Piano_MIDI_SpeedCmdSliderUpdated(wxScrollEvent& event);
		void OnTextCtrl_Piano_MIDI_SpeedText(wxCommandEvent& event);
		void OnSlider_Piano_MIDI_StartCmdSliderUpdated(wxScrollEvent& event);
		void OnTextCtrl_Piano_MIDI_StartText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

