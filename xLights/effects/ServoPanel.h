#ifndef SERVOPANEL_H
#define SERVOPANEL_H

//(*Headers(ServoPanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxCheckBox;
class wxChoice;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include "../BulkEditControls.h"

class ServoPanel: public wxPanel
{
	public:

		ServoPanel(wxWindow* parent);
		virtual ~ServoPanel();

		//(*Declarations(ServoPanel)
		BulkEditCheckBox* CheckBox_16bit;
		BulkEditChoice* Choice_Channel;
		BulkEditSlider* Slider_Servo;
		BulkEditValueCurveButton* ValueCurve_Servo;
		wxFlexGridSizer* FlexGridSizer_Main;
		wxStaticText* Label_DMX1;
		wxStaticText* StaticText1;
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
