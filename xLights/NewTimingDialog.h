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

//(*Headers(NewTimingDialog)
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
//*)

class NewTimingDialog: public wxDialog
{
	public:

		NewTimingDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~NewTimingDialog();

        wxString GetTiming()
        {
            int selection = Choice_New_Fixed_Timing->GetSelection();
            return Choice_New_Fixed_Timing->GetString(selection);
        }

        void RemoveChoice(const wxString selection);

		//(*Declarations(NewTimingDialog)
		wxChoice* Choice_New_Fixed_Timing;
		wxStaticText* StaticText1;
		//*)

	protected:

		//(*Identifiers(NewTimingDialog)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE_New_Fixed_Timing;
		//*)

	private:

		//(*Handlers(NewTimingDialog)
		//*)

		DECLARE_EVENT_TABLE()
};
