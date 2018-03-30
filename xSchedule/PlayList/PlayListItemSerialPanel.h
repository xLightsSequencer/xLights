#ifndef PLAYLISTITEMSERIALPANEL_H
#define PLAYLISTITEMSERIALPANEL_H

//(*Headers(PlayListItemSerialPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/panel.h>
#include <wx/choice.h>
//*)

class PlayListItemSerial;

class PlayListItemSerialPanel: public wxPanel
{
    PlayListItemSerial* _serial;

	public:

		PlayListItemSerialPanel(wxWindow* parent, PlayListItemSerial* process, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemSerialPanel();

		//(*Declarations(PlayListItemSerialPanel)
		wxStaticText* StaticText2;
		wxStaticText* StaticText6;
		wxChoice* Choice_Speed;
		wxTextCtrl* TextCtrl_SerialName;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxTextCtrl* TextCtrl_Delay;
		wxChoice* Choice_CommPort;
		wxStaticText* StaticText5;
		wxChoice* Choice_Configuration;
		wxTextCtrl* TextCtrl_Data;
		wxStaticText* StaticText4;
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

#endif
