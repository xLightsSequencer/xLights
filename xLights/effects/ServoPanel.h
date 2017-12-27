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

#include "../BulkEditControls.h"

class ServoPanel: public wxPanel
{
	public:

		ServoPanel(wxWindow* parent);
		virtual ~ServoPanel();

		//(*Declarations(ServoPanel)
		wxFlexGridSizer* FlexGridSizer_Main;
		BulkEditValueCurveButton* ValueCurve_Servo;
		wxStaticText* Label_DMX1;
		BulkEditSlider* Slider_Servo;
		wxStaticText* StaticText1;
		BulkEditChoice* Choice_Channel;
		BulkEditCheckBox* CheckBox_16bit;
		//*)

	protected:

		//(*Identifiers(ServoPanel)
		static const long ID_STATICTEXT_Channel;
		static const long ID_CHOICE_Channel;
		static const long ID_CHECKBOX_16bit;
		static const long ID_STATICTEXT_Servo;
		static const long IDD_SLIDER_Servo;
		static const long ID_VALUECURVE_Servo;
		static const long ID_TEXTCTRL_Servo;
		//*)

	private:

		//(*Handlers(ServoPanel)
        void OnLockButtonClick(wxCommandEvent& event);
        void OnVCButtonClick(wxCommandEvent& event);
        void OnVCChanged(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
