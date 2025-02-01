#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

//(*Headers(ServoPanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxButton;
class wxCheckBox;
class wxChoice;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
class wxToggleButton;
//*)


#include "../BulkEditControls.h"
#include "EffectPanelUtils.h"

class ServoPanel: public xlEffectPanel
{
	public:

		ServoPanel(wxWindow* parent);
		virtual ~ServoPanel();
		virtual void ValidateWindow() override;

		//(*Declarations(ServoPanel)
		BulkEditCheckBox* CheckBox_16bit;
		BulkEditCheckBox* CheckBox_Timing_Track;
		BulkEditChoice* Choice_Channel;
		BulkEditChoice* Choice_Servo_TimingTrack;
		BulkEditSliderF1* SliderEndValue;
		BulkEditSliderF1* Slider_Servo;
		BulkEditTextCtrlF1* TextCtrl_EndValue;
		BulkEditTextCtrlF1* TextCtrl_Servo;
		BulkEditValueCurveButton* ValueCurve_Servo;
		LinkedToggleButton* EndLinkedButton;
		LinkedToggleButton* StartLinkedButton;
		wxButton* EqualButton;
		wxButton* SwapButton;
		wxCheckBox* SyncCheckBox;
		wxFlexGridSizer* FlexGridSizer_Main;
		wxStaticText* EndDMXLabel;
		wxStaticText* Label_DMX1;
		wxStaticText* StaticText1;
		//*)

	protected:

		//(*Identifiers(ServoPanel)
		static const wxWindowID ID_STATICTEXT_Channel;
		static const wxWindowID ID_CHOICE_Channel;
		static const wxWindowID ID_CHECKBOX_16bit;
		static const wxWindowID ID_CHECKBOX_Timing_Track;
		static const wxWindowID ID_CHOICE_Servo_TimingTrack;
		static const wxWindowID ID_TOGGLEBUTTON_Start;
		static const wxWindowID ID_STATICTEXT_Servo;
		static const wxWindowID IDD_SLIDER_Servo;
		static const wxWindowID ID_VALUECURVE_Servo;
		static const wxWindowID ID_TEXTCTRL_Servo;
		static const wxWindowID ID_TOGGLEBUTTON_End;
		static const wxWindowID ID_STATICTEXT1;
		static const wxWindowID IDD_SLIDER_EndValue;
		static const wxWindowID ID_TEXTCTRL_EndValue;
		static const wxWindowID IDD_CHECKBOX_Sync;
		static const wxWindowID ID_BUTTON1;
		static const wxWindowID IDD_SwapButton;
		//*)

	private:

		//(*Handlers(ServoPanel)
		void OnCheckBox1Click(wxCommandEvent& event);
		void OnCheckBox_Timing_TrackClick(wxCommandEvent& event);
		void OnSwapButtonClick(wxCommandEvent& event);
		void OnEqualButtonClick(wxCommandEvent& event);
		void OnSyncCheckBoxClick(wxCommandEvent& event);
		void StartValueUpdated(wxCommandEvent& event);
		void EndValueUpdated(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
