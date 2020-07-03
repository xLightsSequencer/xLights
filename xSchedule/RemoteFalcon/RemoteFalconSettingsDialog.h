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

//(*Headers(RemoteFalconSettingsDialog)
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class RemoteFalconOptions;

class RemoteFalconSettingsDialog: public wxDialog
{
    RemoteFalconOptions* _options;
	std::vector<int> _plids;

	public:

		RemoteFalconSettingsDialog(wxWindow* parent, RemoteFalconOptions* options, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~RemoteFalconSettingsDialog();

		//(*Declarations(RemoteFalconSettingsDialog)
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxChoice* Choice_Playlists;
		wxStaticText* StaticText1;
		wxStaticText* StaticText_Token;
		wxTextCtrl* TextCtrl_Token;
		//*)

	protected:

		//(*Identifiers(RemoteFalconSettingsDialog)
		static const long ID_STATICTEXT5;
		static const long ID_TEXTCTRL3;
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(RemoteFalconSettingsDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnTextCtrl_xScheduleIPAddressText(wxCommandEvent& event);
		void OnTextCtrl_TargetMatrixText(wxCommandEvent& event);
		void OnTextCtrl_TwilioSIDText(wxCommandEvent& event);
		void OnTextCtrl_TwilioTokenText(wxCommandEvent& event);
		void OnTextCtrl_TwilioPhoneText(wxCommandEvent& event);
		void OnTextCtrl_UserText(wxCommandEvent& event);
		void OnCheckBox_UsePurgoMalumClick(wxCommandEvent& event);
		void OnChoice_RemoteFalconServiceSelect(wxCommandEvent& event);
		void OnTextCtrl_TokenText(wxCommandEvent& event);
		void OnChoice_PlaylistsSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()

        void ValidateWindow();
};
