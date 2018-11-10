#ifndef PlayListItemMQTTPANEL_H
#define PlayListItemMQTTPANEL_H

//(*Headers(PlayListItemMQTTPanel)
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class PlayListItemMQTT;

class PlayListItemMQTTPanel: public wxPanel
{
    PlayListItemMQTT* _MQTT;
    void ValidateWindow();

	public:

		PlayListItemMQTTPanel(wxWindow* parent, PlayListItemMQTT* MQTT, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemMQTTPanel();

		//(*Declarations(PlayListItemMQTTPanel)
		wxSpinCtrl* SpinCtrl_Port;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		wxStaticText* StaticText8;
		wxStaticText* StaticText9;
		wxTextCtrl* TextCtrl_ClientId;
		wxTextCtrl* TextCtrl_Data;
		wxTextCtrl* TextCtrl_Delay;
		wxTextCtrl* TextCtrl_IPAddress;
		wxTextCtrl* TextCtrl_OSCName;
		wxTextCtrl* TextCtrl_Password;
		wxTextCtrl* TextCtrl_Path;
		wxTextCtrl* TextCtrl_Username;
		//*)

	protected:

		//(*Identifiers(PlayListItemMQTTPanel)
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL3;
		static const long ID_STATICTEXT9;
		static const long ID_TEXTCTRL7;
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT4;
		static const long ID_TEXTCTRL4;
		static const long ID_STATICTEXT5;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT6;
		static const long ID_TEXTCTRL5;
		static const long ID_STATICTEXT7;
		static const long ID_TEXTCTRL10;
		static const long ID_STATICTEXT8;
		static const long ID_TEXTCTRL6;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL2;
		//*)

	private:

		//(*Handlers(PlayListItemMQTTPanel)
		void OnTextCtrl_DelayText(wxCommandEvent& event);
		void OnTextCtrl_RunProcessNameText(wxCommandEvent& event);
		void OnChoice_TypeSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
