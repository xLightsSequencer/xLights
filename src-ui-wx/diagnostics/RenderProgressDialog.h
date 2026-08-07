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

//(*Headers(RenderProgressDialog)
#include <wx/dialog.h>
class wxFlexGridSizer;
class wxScrolledWindow;
class wxStdDialogButtonSizer;
//*)

class RenderProgressDialog: public wxDialog
{
	public:

		RenderProgressDialog(wxWindow* parent);
		virtual ~RenderProgressDialog();

		//(*Declarations(RenderProgressDialog)
		wxFlexGridSizer* scrolledWindowSizer;
		wxScrolledWindow* scrolledWindow;
		wxStdDialogButtonSizer* ButtonSizer;
		//*)

	protected:

		//(*Identifiers(RenderProgressDialog)
		static const long ID_SCROLLEDWINDOW1;
		//*)

	public:

		//(*Handlers(RenderProgressDialog)
		//*)

		DECLARE_EVENT_TABLE()

	private:

		// Re-raises this dialog above _owner when _owner is activated, so
		// clicking the main frame can't bury this modeless dialog behind it
		// (#6404) without resorting to wxSTAY_ON_TOP, which pins it above
		// every application on the desktop, not just xLights (#6795).
		wxWindow* _owner = nullptr;
		void OnOwnerActivate(wxActivateEvent& event);
};
