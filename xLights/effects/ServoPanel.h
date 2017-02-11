#ifndef SERVOPANEL_H
#define SERVOPANEL_H

//(*Headers(ServoPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxChoice;
//*)

#include "../ValueCurveButton.h"

class ServoPanel: public wxPanel
{
	public:

		ServoPanel(wxWindow* parent);
		virtual ~ServoPanel();

		//(*Declarations(ServoPanel)
		wxFlexGridSizer* FlexGridSizer_Main;
		wxCheckBox* CheckBox_16bit;
		ValueCurveButton* ValueCurve_Servo;
		wxStaticText* StaticText1;
		wxChoice* Choice_Channel;
		wxSlider* Slider_Servo;
		//*)

	protected:

		//(*Identifiers(ServoPanel)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE_Channel;
		static const long ID_CHECKBOX_16bit;
		static const long IDD_SLIDER_Servo;
		static const long ID_VALUECURVE_Servo;
		static const long ID_TEXTCTRL_Servo;
		//*)

	private:

		//(*Handlers(ServoPanel)
        void UpdateLinkedSliderFloat(wxCommandEvent& event);
        void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
        void UpdateLinkedSliderFloat2(wxCommandEvent& event);
        void UpdateLinkedTextCtrlFloat2(wxScrollEvent& event);
        void UpdateLinkedTextCtrl360(wxScrollEvent& event);
        void UpdateLinkedSlider360(wxCommandEvent& event);
        void UpdateLinkedTextCtrl(wxScrollEvent& event);
        void UpdateLinkedSlider(wxCommandEvent& event);
        void OnLockButtonClick(wxCommandEvent& event);
        void UpdateLinkedTextCtrlVC(wxScrollEvent& event);
        void UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event);
        void OnVCButtonClick(wxCommandEvent& event);
        void OnVCChanged(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
