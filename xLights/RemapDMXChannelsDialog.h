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

//(*Headers(RemapDMXChannelsDialog)
#include <wx/dialog.h>
#include <wx/grid.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
//*)

class RemapDMXChannelsDialog: public wxDialog
{
	public:

		RemapDMXChannelsDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
		virtual ~RemapDMXChannelsDialog();

		//(*Declarations(RemapDMXChannelsDialog)
		wxGrid* Grid1;
		wxScrolledWindow* ScrolledWindow1;
		//*)

	protected:

		//(*Identifiers(RemapDMXChannelsDialog)
		static const long ID_GRID1;
		static const long ID_SCROLLEDWINDOW1;
		//*)

	private:

		//(*Handlers(RemapDMXChannelsDialog)
		//*)

		DECLARE_EVENT_TABLE()
};
