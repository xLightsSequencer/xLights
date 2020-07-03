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

 //(*Headers(PlayListItemOSCPanel)
#include <wx/choice.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class PlayListItemOSC;

class PlayListItemOSCPanel: public wxPanel
{
    PlayListItemOSC* _osc;
    void ValidateWindow();

	public:

		PlayListItemOSCPanel(wxWindow* parent, PlayListItemOSC* osc, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemOSCPanel();

		//(*Declarations(PlayListItemOSCPanel)
		wxChoice* Choice_P1Type;
		wxChoice* Choice_P2Type;
		wxChoice* Choice_P3Type;
		wxChoice* Choice_P4Type;
		wxChoice* Choice_P5Type;
		wxSpinCtrl* SpinCtrl_Port;
		wxStaticText* StaticText10;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		wxStaticText* StaticText8;
		wxStaticText* StaticText9;
		wxTextCtrl* TextCtrl_Delay;
		wxTextCtrl* TextCtrl_IPAddress;
		wxTextCtrl* TextCtrl_OSCName;
		wxTextCtrl* TextCtrl_P1Value;
		wxTextCtrl* TextCtrl_P2Value;
		wxTextCtrl* TextCtrl_P3Value;
		wxTextCtrl* TextCtrl_P4Value;
		wxTextCtrl* TextCtrl_P5Value;
		wxTextCtrl* TextCtrl_Path;
		//*)

	protected:

		//(*Identifiers(PlayListItemOSCPanel)
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL3;
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT4;
		static const long ID_TEXTCTRL4;
		static const long ID_STATICTEXT5;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT6;
		static const long ID_CHOICE1;
		static const long ID_TEXTCTRL5;
		static const long ID_STATICTEXT7;
		static const long ID_CHOICE2;
		static const long ID_TEXTCTRL6;
		static const long ID_STATICTEXT8;
		static const long ID_CHOICE3;
		static const long ID_TEXTCTRL7;
		static const long ID_STATICTEXT9;
		static const long ID_CHOICE4;
		static const long ID_TEXTCTRL8;
		static const long ID_STATICTEXT10;
		static const long ID_CHOICE5;
		static const long ID_TEXTCTRL9;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL2;
		//*)

	private:

		//(*Handlers(PlayListItemOSCPanel)
		void OnTextCtrl_DelayText(wxCommandEvent& event);
		void OnTextCtrl_RunProcessNameText(wxCommandEvent& event);
		void OnChoice_TypeSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

