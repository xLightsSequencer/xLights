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

//(*Headers(ExportModelSelect)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/dialog.h>
//*)

class ExportModelSelect: public wxDialog
{
	public:

		ExportModelSelect(wxWindow* parent,wxWindowID id=wxID_ANY);
		virtual ~ExportModelSelect();

		//(*Declarations(ExportModelSelect)
		wxChoice* ModelChoice;
		wxStaticText* ChoiceLabel;
		//*)

	protected:

		//(*Identifiers(ExportModelSelect)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		//*)

	private:

		//(*Handlers(ExportModelSelect)
		//*)

		DECLARE_EVENT_TABLE()
};
