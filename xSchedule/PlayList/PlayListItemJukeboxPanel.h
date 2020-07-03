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

 //(*Headers(PlayListItemJukeboxPanel)
#include <wx/choice.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class PlayListItemJukebox;

class PlayListItemJukeboxPanel: public wxPanel
{
    PlayListItemJukebox* _jukebox;
    void ValidateWindow();

	public:

		PlayListItemJukeboxPanel(wxWindow* parent, PlayListItemJukebox* jukebox, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
		virtual ~PlayListItemJukeboxPanel();

		//(*Declarations(PlayListItemJukeboxPanel)
		wxChoice* Choice_Port;
		wxSpinCtrl* SpinCtrl_Button;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxTextCtrl* TextCtrl_Delay;
		wxTextCtrl* TextCtrl_JukeboxName;
		//*)

	protected:

		//(*Identifiers(PlayListItemJukeboxPanel)
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL3;
		static const long ID_STATICTEXT4;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL2;
		//*)

	private:

		//(*Handlers(PlayListItemJukeboxPanel)
		void OnTextCtrl_DelayText(wxCommandEvent& event);
		void OnTextCtrl_JukeboxNameText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

