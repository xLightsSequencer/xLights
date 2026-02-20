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

//(*Headers(ResizeImageDialog)
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
//*)

class ResizeImageDialog: public wxDialog
{
	public:

		ResizeImageDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ResizeImageDialog();

		//(*Declarations(ResizeImageDialog)
		wxChoice* ResizeChoice;
		wxSpinCtrl* HeightSpinCtrl;
		wxSpinCtrl* WidthSpinCtrl;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		//*)

	protected:

		//(*Identifiers(ResizeImageDialog)
		static const wxWindowID ID_STATICTEXT1;
		static const wxWindowID ID_SPINCTRL1;
		static const wxWindowID ID_STATICTEXT2;
		static const wxWindowID ID_SPINCTRL2;
		static const wxWindowID ID_STATICTEXT3;
		static const wxWindowID ID_CHOICE1;
		//*)

	private:

		//(*Handlers(ResizeImageDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

