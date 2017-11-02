#ifndef UPDATERDIALOG_H
#define UPDATERDIALOG_H

//(*Headers(UpdaterDialog)
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/stattext.h>
//*)
#include <wx/config.h>

class UpdaterDialog: public wxDialog
{
	public:

		UpdaterDialog(wxWindow* parent,wxWindowID id=wxID_ANY);
		virtual ~UpdaterDialog();
		wxString urlVersion = "";
		wxString downloadUrl = "";
		bool force = false;
		wxConfigBase* config;

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

#endif
