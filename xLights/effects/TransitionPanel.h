#ifndef TransitionPANEL_H
#define TransitionPANEL_H

//(*Headers(TransitionPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxChoice;
//*)

class TransitionPanel: public wxPanel
{
	public:

		TransitionPanel(wxWindow* parent);
		virtual ~TransitionPanel();

		//(*Declarations(TransitionPanel)
		wxStaticText* StaticText10;
		wxBitmapButton* BitmapButton_Transition_Sensitivity;
		wxStaticText* StaticText9;
		wxSlider* Slider_Transition_Adjust;
		wxTextCtrl* TextCtrl_Transition_Adjust;
		wxStaticText* StaticText2;
		wxCheckBox* CheckBox_Transition_Reveal;
		wxStaticText* StaticText6;
		wxBitmapButton* BitmapButton_Transition_Type;
		wxCheckBox* CheckBox_Transition_Reverse;
		wxBitmapButton* BitmapButton_Transition_SlowDownFalls;
		wxStaticText* StaticText3;
		wxStaticText* StaticText5;
		wxBitmapButton* BitmapButton_Transition_TimingTrack;
		wxStaticText* StaticText4;
		wxChoice* Choice_Transition_Type;
		//*)

	protected:

		//(*Identifiers(TransitionPanel)
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE_Transition_Type;
		static const long ID_STATICTEXT5;
		static const long ID_BITMAPBUTTON_CHOICE_Transition_Type;
		static const long ID_STATICTEXT3;
		static const long ID_CHECKBOX_Transition_Reveal;
		static const long ID_STATICTEXT4;
		static const long ID_BITMAPBUTTON_CHOICE_Transition_TimingTrack;
		static const long ID_STATICTEXT6;
		static const long IDD_SLIDER_Transition_Adjust;
		static const long ID_TEXTCTRL_Transition_Adjust;
		static const long ID_BITMAPBUTTON_SLIDER_Transition_Sensitivity;
		static const long ID_STATICTEXT9;
		static const long ID_CHECKBOX_Transition_Reverse;
		static const long ID_STATICTEXT10;
		static const long ID_BITMAPBUTTON_CHECKBOX_Transition_SlowDownFalls;
		//*)

	public:

		//(*Handlers(TransitionPanel)
		void UpdateLinkedSliderFloat(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
		void UpdateLinkedTextCtrl360(wxScrollEvent& event);
		void UpdateLinkedSlider360(wxCommandEvent& event);
		void UpdateLinkedTextCtrl(wxScrollEvent& event);
		void UpdateLinkedSlider(wxCommandEvent& event);
		void OnLockButtonClick(wxCommandEvent& event);
		void OnChoiceTransitionDirectionSelect(wxCommandEvent& event);
		void OnTextCtrl1Text(wxCommandEvent& event);
		void OnChoice_Transition_TypeSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()

	public:

		void ValidateWindow();
};

#endif
