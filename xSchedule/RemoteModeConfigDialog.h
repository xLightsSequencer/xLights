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

 //(*Headers(RemoteModeConfigDialog)
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
//*)

class RemoteModeConfigDialog: public wxDialog
{
	public:

		RemoteModeConfigDialog(wxWindow* parent, int latency, int jitter, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
		virtual ~RemoteModeConfigDialog();
        int GetLatency() const;
        int GetJitter() const;

		//(*Declarations(RemoteModeConfigDialog)
		wxSpinCtrl* SpinCtrl_AcceptableJitter;
		wxSpinCtrl* SpinCtrl_Adjust;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		//*)

	protected:

		//(*Identifiers(RemoteModeConfigDialog)
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL2;
		//*)

	private:

		//(*Handlers(RemoteModeConfigDialog)
		//*)

		DECLARE_EVENT_TABLE()
};
