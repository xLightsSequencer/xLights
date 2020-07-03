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

//(*Headers(SelectTimingsDialog)
#include <wx/sizer.h>
#include <wx/checklst.h>
#include <wx/dialog.h>
//*)

class SelectTimingsDialog: public wxDialog
{
	public:

		SelectTimingsDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~SelectTimingsDialog();

		//(*Declarations(SelectTimingsDialog)
		wxCheckListBox* CheckListBox_Timings;
		//*)

	protected:

		//(*Identifiers(SelectTimingsDialog)
		static const long ID_CHECKLISTBOX1;
		//*)

	private:

		//(*Handlers(SelectTimingsDialog)
		//*)

		DECLARE_EVENT_TABLE()
};
