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

 //(*Headers(PlayListPanel)
#include <wx/checkbox.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class PlayList;

class PlayListPanel: public wxPanel
{
    PlayList* _playlist;

	public:

		PlayListPanel(wxWindow* parent, PlayList* pl, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListPanel();

		//(*Declarations(PlayListPanel)
		wxCheckBox* CheckBox_AlwaysShuffle;
		wxCheckBox* CheckBox_FirstOnce;
		wxCheckBox* CheckBox_LastStepOnce;
		wxStaticText* StaticText1;
		wxTextCtrl* TextCtrl_PlayListName;
		//*)

	protected:

		//(*Identifiers(PlayListPanel)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_CHECKBOX1;
		static const long ID_CHECKBOX2;
		static const long ID_CHECKBOX3;
		//*)

	private:

		//(*Handlers(PlayListPanel)
		void OnTextCtrl_PlayListNameText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

