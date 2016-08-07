#ifndef NOTEIMPORTDIALOG_H
#define NOTEIMPORTDIALOG_H

//(*Headers(NoteImportDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/slider.h>
#include <wx/filedlg.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
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
		wxTextCtrl* TextCtrl_Piano_File;
		wxStaticText* StaticText6;
		wxSlider* Slider_Piano_MIDI_Speed;
		wxStaticText* StaticText8;
		wxTextCtrl* TextCtrl_Piano_MIDI_Speed;
		wxChoice* Choice_Piano_Notes_Source;
		wxChoice* Choice_Piano_MIDITrack_APPLYLAST;
		wxStaticText* StaticText1;
		wxFileDialog* FileDialog1;
		wxStaticText* StaticText3;
		wxTextCtrl* TextCtrl_TimingName;
		wxButton* ButtonCancel;
		wxStaticText* StaticText5;
		wxButton* ButtonOk;
		wxSlider* Slider_Piano_MIDI_Start;
		wxStaticText* StaticText4;
		wxButton* Button_Piano_File;
		wxTextCtrl* TextCtrl_Piano_MIDI_Start;
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
		void OnSpinCtrl_Piano_StartMIDIChange(wxSpinEvent& event);
		void OnSpinCtrl_Piano_EndMIDIChange(wxSpinEvent& event);
		void OnButton_Piano_FileClick(wxCommandEvent& event);
		void OnTextCtrl_Piano_FileText(wxCommandEvent& event);
		void UpdateLinkedTextCtrl(wxScrollEvent& event);
		void UpdateLinkedSlider(wxCommandEvent& event);
		void OnButtonOkClick(wxCommandEvent& event);
		void OnButtonCancelClick(wxCommandEvent& event);
		void OnTextCtrl_TimingNameText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
