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

//(*Headers(SplashDialog)
#include <wx/dialog.h>
//*)

#include <wx/bmpbndl.h>

class SplashDialog: public wxDialog
{
	public:

		SplashDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~SplashDialog();

		//(*Declarations(SplashDialog)
		//*)

	protected:

		//(*Identifiers(SplashDialog)
		//*)

	private:

		//(*Handlers(SplashDialog)
		void OnPaint(wxPaintEvent& event);
		//*)

		DECLARE_EVENT_TABLE()

        wxBitmapBundle _image;
};
