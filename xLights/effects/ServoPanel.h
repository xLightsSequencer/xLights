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
		BulkEditCheckBox* CheckBox_Timing_Track;
		BulkEditChoice* Choice_Channel;
		BulkEditChoice* Choice_Servo_TimingTrack;
		BulkEditSliderF1* Slider_Servo;
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
		static const long ID_CHECKBOX_Timing_Track;
		static const long ID_CHOICE_Servo_TimingTrack;
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
		void OnCheckBox1Click(wxCommandEvent& event);
		void OnCheckBox_Timing_TrackClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
