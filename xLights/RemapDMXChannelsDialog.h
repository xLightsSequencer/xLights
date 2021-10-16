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
#include <wx/button.h>
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
		wxButton* Button_Cancel;
		wxButton* Button_LoadMapping;
		wxButton* Button_Ok;
		wxButton* Button_SaveMapping;
		wxGrid* Grid1;
		wxScrolledWindow* ScrolledWindow1;
		//*)

	protected:

		//(*Identifiers(RemapDMXChannelsDialog)
		static const long ID_GRID1;
		static const long ID_SCROLLEDWINDOW1;
		static const long ID_BUTTON_OK;
		static const long ID_BUTTON_CANCEL;
		static const long ID_BUTTON_SAVE_MAPPING;
		static const long ID_BUTTON_LOAD_MAPPING;
		//*)

	private:

		//(*Handlers(RemapDMXChannelsDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnButton_SaveMappingClick(wxCommandEvent& event);
		void OnButton_LoadMappingClick(wxCommandEvent& event);
		//*)

		bool IsValidRemapping(const wxString& value);

		DECLARE_EVENT_TABLE()
};
