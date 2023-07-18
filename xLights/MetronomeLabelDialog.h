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

//(*Headers(MetronomeLabelDialog)
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
		wxSpinCtrl* SpinCtrlRepeating;
		wxSpinCtrl* SpinCtrlTiming;
		wxStaticText* StaticTextDialogText;
		wxStaticText* StaticTextRepeating;
		wxStaticText* StaticTextTiming;
		//*)

		[[nodiscard]] int GetTiming() const { return SpinCtrlTiming->GetValue(); }
		[[nodiscard]] int GetTagCount() const { return SpinCtrlRepeating->GetValue(); } 

	protected:

		//(*Identifiers(MetronomeLabelDialog)
		static const long ID_STATICTEXT_DIALOGTEXT;
		static const long ID_STATICTEXT_TIMING;
		static const long ID_SPINCTRL_TIMING;
		static const long ID_STATICTEXT_REPEATING;
		static const long ID_SPINCTRL_REPEATING;
		//*)

	private:

		//(*Handlers(MetronomeLabelDialog)
		//*)

		DECLARE_EVENT_TABLE()
};
