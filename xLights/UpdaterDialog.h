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

//(*Headers(UpdaterDialog)
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/stattext.h>
//*)

class UpdaterDialog: public wxDialog
{
	public:

		UpdaterDialog(wxWindow* parent,wxWindowID id=wxID_ANY);
		virtual ~UpdaterDialog();
		wxString urlVersion = "";
		wxString downloadUrl = "";
		bool force = false;

		//(*Declarations(UpdaterDialog)
		wxButton* ButtonUpDownload;
		wxStaticText* StaticTextUpdateLabel;
		wxButton* ButtonUpdateIgnore;
		wxButton* ButtonUpdateSkip;
		//*)

	protected:

		//(*Identifiers(UpdaterDialog)
		static const long ID_UPDATELABEL;
		static const long ID_BUTTONUPDOWN;
		static const long ID_BUTTON2;
		static const long ID_BUTTON3;
		//*)

	private:

		//(*Handlers(UpdaterDialog)
		void OnButtonDownloadNewRelease(wxCommandEvent& event);
		void OnButtonUpdateSkipClick(wxCommandEvent& event);
		void OnButtonUpdateIgnoreClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
