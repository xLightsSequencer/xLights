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

 //(*Headers(PlayListItemARTNetTriggerPanel)
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class PlayListItemARTNetTrigger;

class PlayListItemARTNetTriggerPanel: public wxPanel
{
    PlayListItemARTNetTrigger* _ARTNetTrigger;

	public:

		PlayListItemARTNetTriggerPanel(wxWindow* parent, PlayListItemARTNetTrigger* ARTNetTrigger, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemARTNetTriggerPanel();

		//(*Declarations(PlayListItemARTNetTriggerPanel)
		wxSpinCtrl* SpinCtrl1_SubKey;
		wxSpinCtrl* SpinCtrl_Key;
		wxSpinCtrl* SpinCtrl_OEM;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		wxTextCtrl* TextCtrl_ARTNetTriggerName;
		wxTextCtrl* TextCtrl_Data;
		wxTextCtrl* TextCtrl_Delay;
		wxTextCtrl* TextCtrl_IPAddress;
		//*)

	protected:

		//(*Identifiers(PlayListItemARTNetTriggerPanel)
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL3;
		static const long ID_STATICTEXT5;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT6;
		static const long ID_SPINCTRL3;
		static const long ID_STATICTEXT7;
		static const long ID_TEXTCTRL4;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL2;
		//*)

	private:

		//(*Handlers(PlayListItemARTNetTriggerPanel)
		void OnTextCtrl_DelayText(wxCommandEvent& event);
		void OnTextCtrl_ARTNetTriggerNameText(wxCommandEvent& event);
		void OnTextCtrl_IPAddressText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
