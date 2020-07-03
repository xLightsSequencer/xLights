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

 //(*Headers(PlayListItemSerialPanel)
#include <wx/choice.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class PlayListItemSerial;

class PlayListItemSerialPanel: public wxPanel
{
    PlayListItemSerial* _serial;

	public:

		PlayListItemSerialPanel(wxWindow* parent, PlayListItemSerial* process, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemSerialPanel();

		//(*Declarations(PlayListItemSerialPanel)
		wxChoice* Choice_CommPort;
		wxChoice* Choice_Configuration;
		wxChoice* Choice_Speed;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxTextCtrl* TextCtrl_Data;
		wxTextCtrl* TextCtrl_Delay;
		wxTextCtrl* TextCtrl_SerialName;
		//*)

	protected:

		//(*Identifiers(PlayListItemSerialPanel)
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL3;
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE3;
		static const long ID_STATICTEXT4;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT6;
		static const long ID_CHOICE2;
		static const long ID_STATICTEXT5;
		static const long ID_TEXTCTRL4;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL2;
		//*)

	private:

		//(*Handlers(PlayListItemSerialPanel)
		void OnTextCtrl_DelayText(wxCommandEvent& event);
		void OnTextCtrl_SerialNameText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

