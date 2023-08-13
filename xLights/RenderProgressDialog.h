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
};
