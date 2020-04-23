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

//(*Headers(PlayListItemFilePanel)
#include <wx/checkbox.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class PlayListItemFile;

class PlayListItemFilePanel: public wxPanel
{
    PlayListItemFile* _file;

	public:

		PlayListItemFilePanel(wxWindow* parent, PlayListItemFile* file, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemFilePanel();

		//(*Declarations(PlayListItemFilePanel)
		wxCheckBox* CheckBox_Append;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxTextCtrl* TextCtrl_Content;
		wxTextCtrl* TextCtrl_Delay;
		wxTextCtrl* TextCtrl_FName;
		wxTextCtrl* TextCtrl_FileName;
		//*)

	protected:

		//(*Identifiers(PlayListItemFilePanel)
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL3;
		static const long ID_STATICTEXT4;
		static const long ID_TEXTCTRL4;
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_CHECKBOX1;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL2;
		//*)

	private:

		//(*Handlers(PlayListItemFilePanel)
		void OnTextCtrl_DelayText(wxCommandEvent& event);
		void OnTextCtrl_ContentText(wxCommandEvent& event);
		void OnTextCtrl_FNameText(wxCommandEvent& event);
		void OnTextCtrl_FileNameText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

