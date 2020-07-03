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

 //(*Headers(PlayListItemDelayPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/panel.h>
//*)

class PlayListItemDelay;

class PlayListItemDelayPanel: public wxPanel
{
    PlayListItemDelay* _Delay;

    void ValidateWindow();

	public:

		PlayListItemDelayPanel(wxWindow* parent, PlayListItemDelay* Delay, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemDelayPanel();

		//(*Declarations(PlayListItemDelayPanel)
		wxStaticText* StaticText1;
		wxTextCtrl* TextCtrl_DelayDuration;
		//*)

	protected:

		//(*Identifiers(PlayListItemDelayPanel)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		//*)

	private:

		//(*Handlers(PlayListItemDelayPanel)
		void OnTextCtrl_DelayDurationText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

