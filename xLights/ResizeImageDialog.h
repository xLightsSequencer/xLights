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

//(*Headers(ResizeImageDialog)
#include <wx/spinctrl.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/choice.h>
//*)

class ResizeImageDialog: public wxDialog
{
	public:

		ResizeImageDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ResizeImageDialog();

		//(*Declarations(ResizeImageDialog)
		wxSpinCtrl* WidthSpinCtrl;
		wxStaticText* StaticText1;
		wxChoice* ResizeChoice;
		wxStaticText* StaticText3;
		wxSpinCtrl* HeightSpinCtrl;
		wxStaticText* StaticText2;
		//*)

	protected:

		//(*Identifiers(ResizeImageDialog)
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT3;
		static const long ID_CHOICE1;
		//*)

	private:

		//(*Handlers(ResizeImageDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

