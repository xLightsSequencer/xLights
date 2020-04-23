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

 //(*Headers(PlayListItemProjectorPanel)
#include <wx/choice.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class PlayListItemProjector;

class PlayListItemProjectorPanel: public wxPanel
{
    PlayListItemProjector* _Projector;
    void ValidateWindow();
    bool HasCommPort(wxString protocol);
    bool HasIP(wxString protocol);
    bool HasParameter(wxString protocol);
    bool HasPassword(wxString protocol);
    void PopulateCommands(wxString protocol);
    void GetConfig(wxString protocol, wxString command, int& baudRate, int& charBits, std::string& parity, int& stopBits, int& port, std::string& ipProtocol, std::string& commandData);

	public:

		PlayListItemProjectorPanel(wxWindow* parent, PlayListItemProjector* Projector, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemProjectorPanel();

		//(*Declarations(PlayListItemProjectorPanel)
		wxChoice* Choice_CommPort;
		wxChoice* Choice_Command;
		wxChoice* Choice_Parameter;
		wxChoice* Choice_ProjectorProtocol;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		wxTextCtrl* TextCtrl_Delay;
		wxTextCtrl* TextCtrl_IPAddress;
		wxTextCtrl* TextCtrl_Password;
		//*)

	protected:

		//(*Identifiers(PlayListItemProjectorPanel)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE2;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE3;
		static const long ID_STATICTEXT7;
		static const long ID_CHOICE4;
		static const long ID_STATICTEXT5;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT4;
		static const long ID_TEXTCTRL3;
		static const long ID_STATICTEXT6;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL1;
		//*)

	private:

		//(*Handlers(PlayListItemProjectorPanel)
		void OnTextCtrl_DelayText(wxCommandEvent& event);
		void OnChoice_ProjectorProtocolSelect(wxCommandEvent& event);
		void OnTextCtrl_IPAddressText(wxCommandEvent& event);
		void OnChoice_CommPortSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

