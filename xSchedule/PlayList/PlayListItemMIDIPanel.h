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

 //(*Headers(PlayListItemMIDIPanel)
#include <wx/choice.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class PlayListItemMIDI;

class PlayListItemMIDIPanel: public wxPanel
{
    PlayListItemMIDI* _MIDI;

	public:

		PlayListItemMIDIPanel(wxWindow* parent, PlayListItemMIDI* process, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemMIDIPanel();

		//(*Declarations(PlayListItemMIDIPanel)
		wxChoice* Choice_Channel;
		wxChoice* Choice_Data1;
		wxChoice* Choice_Data2;
		wxChoice* Choice_Devices;
		wxChoice* Choice_Status;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		wxTextCtrl* TextCtrl_Delay;
		wxTextCtrl* TextCtrl_MIDIName;
		//*)

	protected:

		//(*Identifiers(PlayListItemMIDIPanel)
		static const long ID_STATICTEXT5;
		static const long ID_TEXTCTRL3;
		static const long ID_STATICTEXT3;
		static const long ID_CHOICE4;
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE3;
		static const long ID_STATICTEXT4;
		static const long ID_CHOICE5;
		static const long ID_STATICTEXT6;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT7;
		static const long ID_CHOICE2;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL2;
		//*)

	private:

		//(*Handlers(PlayListItemMIDIPanel)
		void OnTextCtrl_DelayText(wxCommandEvent& event);
		void OnTextCtrl_MIDINameText(wxCommandEvent& event);
		void OnTextCtrl_MIDINameText1(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

