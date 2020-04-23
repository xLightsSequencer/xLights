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

 //(*Headers(PlayListItemFPPEventPanel)
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class PlayListItemFPPEvent;

class PlayListItemFPPEventPanel: public wxPanel
{
    PlayListItemFPPEvent* _fppevent;

	public:

		PlayListItemFPPEventPanel(wxWindow* parent, PlayListItemFPPEvent* fppevent, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemFPPEventPanel();

		//(*Declarations(PlayListItemFPPEventPanel)
		wxSpinCtrl* SpinCtrl_Major;
		wxSpinCtrl* SpinCtrl_Minor;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxTextCtrl* TextCtrl_Delay;
		wxTextCtrl* TextCtrl_FPPEventName;
		wxTextCtrl* TextCtrl_IPAddress;
		//*)

	protected:

		//(*Identifiers(PlayListItemFPPEventPanel)
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL3;
		static const long ID_STATICTEXT5;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL2;
		//*)

	private:

		//(*Handlers(PlayListItemFPPEventPanel)
		void OnTextCtrl_DelayText(wxCommandEvent& event);
		void OnTextCtrl_FPPEventNameText(wxCommandEvent& event);
		void OnTextCtrl_IPAddressText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

