#ifndef PIANOPANEL_H
#define PIANOPANEL_H

//(*Headers(PianoPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxChoice;
class wxFlexGridSizer;
class wxSpinEvent;
class wxBitmapButton;
class wxSlider;
class wxSpinCtrl;
class wxStaticText;
class wxCheckBox;
//*)

#include <wx/progdlg.h>
#include "../BulkEditControls.h"

class MidiFile;

class PianoPanel: public wxPanel
{
public:

		PianoPanel(wxWindow* parent);
		virtual ~PianoPanel();
        void SetTimingTracks(wxCommandEvent& event);

		//(*Declarations(PianoPanel)
		BulkEditChoice* Choice_Piano_MIDITrack_APPLYLAST;
		BulkEditCheckBox* CheckBox_Piano_ShowSharps;
		BulkEditSpinCtrl* SpinCtrl_Piano_EndMIDI;
		wxBitmapButton* BitmapButton_Piano_ShowSharps;
		BulkEditSlider* Slider_Piano_XOffset;
		BulkEditSpinCtrl* SpinCtrl_Piano_StartMIDI;
		BulkEditTextCtrl* TextCtrl_Piano_XOffset;
		wxStaticText* StaticText_Piano_NumKeys;
		wxStaticText* StaticText1;
		BulkEditSlider* Slider_Piano_Scale;
		wxStaticText* StaticText8;
		wxStaticText* StaticText7;
		BulkEditValueCurveButton* BitmapButton_Piano_ScaleVC;
		BulkEditChoice* Choice_Piano_Type;
		wxStaticText* StaticText2;
		BulkEditTextCtrl* TextCtrl_Piano_Scale;
		wxStaticText* StaticText9;
		//*)

	protected:

		//(*Identifiers(PianoPanel)
		static const long ID_STATICTEXT_Piano_Type;
		static const long ID_CHOICE_Piano_Type;
		static const long ID_STATICTEXT_Piano_StartMIDI;
		static const long ID_SPINCTRL_Piano_StartMIDI;
		static const long ID_STATICTEXT_Piano_EndMIDI;
		static const long ID_SPINCTRL_Piano_EndMIDI;
		static const long ID_CHECKBOX_Piano_ShowSharps;
		static const long ID_BITMAPBUTTON_Piano_ShowSharps;
		static const long ID_STATICTEXT_Piano_MIDITrack_APPLYLAST;
		static const long ID_CHOICE_Piano_MIDITrack_APPLYLAST;
		static const long ID_STATICTEXT_Piano_Scale;
		static const long ID_SLIDER_Piano_Scale;
		static const long ID_VALUECURVE_Piano_Scale;
		static const long IDD_TEXTCTRL_Piano_Scale;
		static const long ID_STATICTEXT_Piano_XOffset;
		static const long ID_SLIDER_Piano_XOffset;
		static const long IDD_TEXTCTRL_Piano_XOffset;
		//*)

	public:

		//(*Handlers(PianoPanel)
		void OnLockButtonClick(wxCommandEvent& event);
		void OnSpinCtrl_Piano_StartMIDIChange(wxSpinEvent& event);
		void OnSpinCtrl_Piano_EndMIDIChange(wxSpinEvent& event);
        void OnVCButtonClick(wxCommandEvent& event);
        void OnVCChanged(wxCommandEvent& event);
        //*)

		DECLARE_EVENT_TABLE()
        void ValidateWindow();
};

#endif
