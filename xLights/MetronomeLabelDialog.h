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

//(*Headers(MetronomeLabelDialog)
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
//*)

class MetronomeLabelDialog: public wxDialog
{
	public:

		MetronomeLabelDialog(int base_timing, wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~MetronomeLabelDialog();

		//(*Declarations(MetronomeLabelDialog)
		wxCheckBox* CheckBox_RandomTagsEnabled;
		wxCheckBox* CheckBox_RandomTiming;
		wxSpinCtrl* SpinCtrlRepeating;
		wxSpinCtrl* SpinCtrlTiming;
		wxSpinCtrl* SpinCtrl_MinTiming;
		wxStaticText* StaticTextDialogText;
		wxStaticText* StaticTextRepeating;
		wxStaticText* StaticTextTiming;
		wxStaticText* StaticText_MinTiming;
		//*)

		[[nodiscard]] int GetTiming() const { return SpinCtrlTiming->GetValue(); }
		[[nodiscard]] int GetTagCount() const { return SpinCtrlRepeating->GetValue(); } 

	protected:

		//(*Identifiers(MetronomeLabelDialog)
		static const wxWindowID ID_STATICTEXT_DIALOGTEXT;
		static const wxWindowID ID_STATICTEXT_TIMING;
		static const wxWindowID ID_SPINCTRL_TIMING;
		static const wxWindowID ID_STATICTEXT_REPEATING;
		static const wxWindowID ID_SPINCTRL_REPEATING;
		static const wxWindowID ID_CHECKBOX_RANDOMTIMING;
		static const wxWindowID ID_STATICTEXT_MINTIMING;
		static const wxWindowID ID_SPINCTRL1;
		static const wxWindowID ID_CHECKBOX_RANDOMTAGS;
		//*)

	private:

		//(*Handlers(MetronomeLabelDialog)
		void OnCheckBox_RandomTimingClick(wxCommandEvent& event);
		void OnCheckBox_RandomTagsEnabledClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
