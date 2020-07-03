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

//(*Headers(CustomTimingDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/dialog.h>
//*)

class CustomTimingDialog: public wxDialog
{
	public:

		CustomTimingDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~CustomTimingDialog();
        wxString GetTiming() const;
        int GetValue() const;
        void SetTiming(int value);

		//(*Declarations(CustomTimingDialog)
		wxStaticText* StaticText1;
		wxSpinCtrl* SpinCtrl_Interval;
		wxTextCtrl* TextCtrl_FPS;
		//*)

	protected:

		//(*Identifiers(CustomTimingDialog)
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL_Interval;
		static const long ID_TEXTCTRL_FPS;
		//*)

	private:

		//(*Handlers(CustomTimingDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		//*)

        void OnTextChange(wxCommandEvent& event);

		DECLARE_EVENT_TABLE()

		void UpdateFPS();

		bool init;
};
