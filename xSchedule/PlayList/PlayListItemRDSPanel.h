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

 //(*Headers(PlayListItemRDSPanel)
#include <wx/choice.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class PlayListItemRDS;

class PlayListItemRDSPanel: public wxPanel
{
    PlayListItemRDS* _rds;
    void ValidateWindow();

	public:

		PlayListItemRDSPanel(wxWindow* parent, PlayListItemRDS* rds, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemRDSPanel();

		//(*Declarations(PlayListItemRDSPanel)
		wxChoice* Choice_CommPort;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText5;
		wxTextCtrl* TextCtrl_StationName;
		wxTextCtrl* TextCtrl_Text;
		//*)

	protected:

		//(*Identifiers(PlayListItemRDSPanel)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT5;
		static const long ID_TEXTCTRL2;
		//*)

	private:

		//(*Handlers(PlayListItemRDSPanel)
		void OnChoice_ScrollModeSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

