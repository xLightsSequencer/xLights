#ifndef NEWSEQUENCEDIALOG_H
#define NEWSEQUENCEDIALOG_H

//(*Headers(NewSequenceDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/radiobox.h>
#include <wx/textctrl.h>
#include <wx/radiobut.h>
#include <wx/choice.h>
#include <wx/dialog.h>
//*)

class NewSequenceDialog: public wxDialog
{
	public:

		NewSequenceDialog(wxWindow* parent,wxWindowID id=wxID_ANY);
		virtual ~NewSequenceDialog();

		//(*Declarations(NewSequenceDialog)
		wxChoice* ChoiceMediaFiles;
		wxChoice* NewSequenceTiming;
		wxRadioButton* RadioButtonNewMusic;
		wxStaticText* StaticText2;
		wxTextCtrl* SpinCtrlDuration_Float;
		wxRadioBox* RadioBoxTimingChoice;
		wxStaticText* StaticText5;
		wxRadioButton* RadioButtonLor;
		wxChoice* ChoiceLorFiles;
		wxStaticText* StaticText4;
		wxRadioButton* RadioButtonNewAnim;
		//*)

	protected:

		//(*Identifiers(NewSequenceDialog)
		static const long ID_RADIOBUTTON4;
		static const long ID_STATICTEXT4;
		static const long ID_CHOICE3;
		static const long ID_RADIOBUTTON2;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE2;
		static const long ID_RADIOBOX_Timing_Choice;
		static const long ID_RADIOBUTTON3;
		static const long ID_STATICTEXT5;
		static const long ID_SpinCtrlDuration_Float;
		static const long ID_CHOICE1;
		//*)

	private:

		//(*Handlers(NewSequenceDialog)
    void OnChoiceSeqFilesSelect(wxCommandEvent& event);
    void OnChoiceMediaFilesSelect(wxCommandEvent& event);
    void OnRadioButton1Select(wxCommandEvent& event);
    void OnRadioButton2Select(wxCommandEvent& event);
    void OnRadioBox1Select(wxCommandEvent& event);
    void OnRadioBox1Select1(wxCommandEvent& event);
    void OnRadioButtonXlightsSelect(wxCommandEvent& event);
    void OnRadioButtonNewMusicSelect(wxCommandEvent& event);
    void OnRadioButtonNewAnimSelect(wxCommandEvent& event);
    void OnRadioButtonLorSelect(wxCommandEvent& event);
    void OnChoiceLorFilesSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
