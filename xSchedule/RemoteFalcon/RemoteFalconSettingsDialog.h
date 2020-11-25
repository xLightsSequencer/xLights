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
#include <wx/checkbox.h>
#include <wx/checklst.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class RemoteFalconOptions;

class RemoteFalconSettingsDialog: public wxDialog
{
    RemoteFalconOptions* _options;
	std::vector<int> _plids;
	std::list<std::pair<std::string, int>> _playlists;

	public:

		RemoteFalconSettingsDialog(wxWindow* parent, RemoteFalconOptions* options, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~RemoteFalconSettingsDialog();

		//(*Declarations(RemoteFalconSettingsDialog)
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxCheckBox* CheckBox_ClearQueue;
		wxCheckBox* CheckBox_ImmediatelyInterrupt;
		wxCheckBox* CheckBox_SendEnableDisable;
		wxCheckListBox* CheckListBox_Playlists;
		wxChoice* Choice_Playlists;
		wxSpinCtrl* SpinCtrl_LeadTime;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText_Token;
		wxTextCtrl* TextCtrl_Token;
		//*)

	protected:

		//(*Identifiers(RemoteFalconSettingsDialog)
		static const long ID_STATICTEXT5;
		static const long ID_TEXTCTRL3;
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		static const long ID_CHECKBOX1;
		static const long ID_CHECKBOX2;
		static const long ID_CHECKBOX3;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT3;
		static const long ID_CHECKLISTBOX1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

		static const long ID_MNU_SELECTALL;
		static const long ID_MNU_SELECTNONE;
		static const long ID_MNU_SELECTHIGH;
		static const long ID_MNU_DESELECTHIGH;

	private:

		//(*Handlers(RemoteFalconSettingsDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnTextCtrl_TokenText(wxCommandEvent& event);
		void OnChoice_PlaylistsSelect(wxCommandEvent& event);
		void OnCheckListBox_PlaylistsToggled(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()

		void OnPreviewRightDown(wxMouseEvent& event);
		void OnPopupCommand(wxCommandEvent& event);

        void ValidateWindow();
};
