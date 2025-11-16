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

 //(*Headers(PlayListStepPanel)
 #include <wx/checkbox.h>
 #include <wx/dateevt.h>
 #include <wx/panel.h>
 #include <wx/sizer.h>
 #include <wx/stattext.h>
 #include <wx/textctrl.h>
 #include <wx/timectrl.h>
 //*)

class PlayListStep;

class PlayListStepPanel: public wxPanel
{
    PlayListStep* _step;

	void ValidateWindow();

	public:

		PlayListStepPanel(wxWindow* parent, PlayListStep* step, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListStepPanel();

		//(*Declarations(PlayListStepPanel)
		wxCheckBox* CheckBox_EveryStep;
		wxCheckBox* CheckBox_ExcludeFirstStep;
		wxCheckBox* CheckBox_ExcludeFromRandom;
		wxCheckBox* CheckBox_ExcludeLastStep;
		wxCheckBox* CheckBox_TimeCodeOverride;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText_BaseTime;
		wxTextCtrl* TextCtrl_PlayListStepName;
		wxTimePickerCtrl* TimeCodeBaseTime;
		//*)

	protected:

		//(*Identifiers(PlayListStepPanel)
		static const wxWindowID ID_STATICTEXT1;
		static const wxWindowID ID_TEXTCTRL1;
		static const wxWindowID ID_CHECKBOX1;
		static const wxWindowID ID_CHECKBOX2;
		static const wxWindowID ID_STATICTEXT2;
		static const wxWindowID ID_CHECKBOX3;
		static const wxWindowID ID_STATICTEXT3;
		static const wxWindowID ID_CHECKBOX4;
		static const wxWindowID ID_CHECKBOX5;
		static const wxWindowID ID_STATICTEXT4;
		static const wxWindowID ID_TIMEPICKERCTRL1;
		//*)

	private:

		//(*Handlers(PlayListStepPanel)
		void OnTextCtrl_PlayListStepNameText(wxCommandEvent& event);
		void OnCheckBox_EveryStepClick(wxCommandEvent& event);
		void OnCheckBox_ExcludeFirstStepClick(wxCommandEvent& event);
		void OnCheckBox_TimeCodeOverrideClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

