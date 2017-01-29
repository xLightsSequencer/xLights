#ifndef PIANOPANEL_H
#define PIANOPANEL_H

//(*Headers(PianoPanel)
#include <wx/panel.h>
class wxSpinEvent;
class wxCheckBox;
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxSpinCtrl;
class wxChoice;
//*)

#include <wx/progdlg.h>
#include "../ValueCurveButton.h"

class MidiFile;

class PianoPanel: public wxPanel
{
public:

		PianoPanel(wxWindow* parent);
		virtual ~PianoPanel();
        void SetTimingTracks(wxCommandEvent& event);

		//(*Declarations(PianoPanel)
		wxStaticText* StaticText9;
		wxTextCtrl* TextCtrl_Piano_XOffset;
		wxTextCtrl* TextCtrl_Piano_Scale;
		wxStaticText* StaticText2;
		wxSlider* Slider_Piano_Scale;
		wxStaticText* StaticText_Piano_NumKeys;
		wxCheckBox* CheckBox_Piano_ShowSharps;
		wxBitmapButton* BitmapButton_Piano_StartMIDI;
		wxStaticText* StaticText8;
		wxSpinCtrl* SpinCtrl_Piano_StartMIDI;
		wxBitmapButton* BitmapButton_Piano_EndMIDI;
		wxChoice* Choice_Piano_MIDITrack_APPLYLAST;
		wxStaticText* StaticText1;
		wxBitmapButton* BitmapButton_Piano_XOffset;
		wxSpinCtrl* SpinCtrl_Piano_EndMIDI;
		ValueCurveButton* BitmapButton_Piano_ScaleVC;
		wxStaticText* StaticText7;
		wxBitmapButton* BitmapButton_Piano_scale;
		wxSlider* Slider_Piano_XOffset;
		wxChoice* Choice_Piano_Type;
		wxBitmapButton* BitmapButton_Piano_ShowSharps;
		//*)

	protected:

		//(*Identifiers(PianoPanel)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE_Piano_Type;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL_Piano_StartMIDI;
		static const long ID_BITMAPBUTTON_Piano_StartMIDI;
		static const long ID_STATICTEXT_Piano_NumKeys;
		static const long ID_SPINCTRL_Piano_EndMIDI;
		static const long ID_BITMAPBUTTON_Piano_EndMIDI;
		static const long ID_CHECKBOX_Piano_ShowSharps;
		static const long ID_BITMAPBUTTON_Piano_ShowSharps;
		static const long ID_STATICTEXT8;
		static const long ID_CHOICE_Piano_MIDITrack_APPLYLAST;
		static const long ID_STATICTEXT7;
		static const long ID_SLIDER_Piano_Scale;
		static const long ID_VALUECURVE_Piano_Scale;
		static const long IDD_TEXTCTRL_Piano_Scale;
		static const long ID_BITMAPBUTTON_Piano_Scale;
		static const long ID_STATICTEXT9;
		static const long ID_SLIDER_Piano_XOffset;
		static const long IDD_TEXTCTRL_Piano_XOffset;
		static const long ID_BITMAPBUTTON_Piano_XOffset;
		//*)

	public:

		//(*Handlers(PianoPanel)
		void UpdateLinkedSliderFloat(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
		void UpdateLinkedTextCtrl360(wxScrollEvent& event);
		void UpdateLinkedSlider360(wxCommandEvent& event);
		void UpdateLinkedTextCtrl(wxScrollEvent& event);
		void UpdateLinkedSlider(wxCommandEvent& event);
		void OnLockButtonClick(wxCommandEvent& event);
		void OnSpinCtrl_Piano_StartMIDIChange(wxSpinEvent& event);
		void OnSpinCtrl_Piano_EndMIDIChange(wxSpinEvent& event);
        void UpdateLinkedTextCtrlVC(wxScrollEvent& event);
        void UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event);
        void OnVCButtonClick(wxCommandEvent& event);
        void OnVCChanged(wxCommandEvent& event);
        //*)

		DECLARE_EVENT_TABLE()
        void ValidateWindow();
};

#endif
