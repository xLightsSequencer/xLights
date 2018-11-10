#ifndef PLAYLISTITEMRUNCOMMANDPANEL_H
#define PLAYLISTITEMRUNCOMMANDPANEL_H

//(*Headers(PlayListItemRunCommandPanel)
#include <wx/choice.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class PlayListItemRunCommand;

class PlayListItemRunCommandPanel: public wxPanel
{
    PlayListItemRunCommand* _Command;
    void ValidateWindow();

	public:

		PlayListItemRunCommandPanel(wxWindow* parent, PlayListItemRunCommand* Command, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemRunCommandPanel();

		//(*Declarations(PlayListItemRunCommandPanel)
		wxChoice* Choice_Command;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxTextCtrl* TextCtrl_Delay;
		wxTextCtrl* TextCtrl_Parm1;
		wxTextCtrl* TextCtrl_Parm2;
		wxTextCtrl* TextCtrl_Parm3;
		wxTextCtrl* TextCtrl_RunCommandName;
		//*)

	protected:

		//(*Identifiers(PlayListItemRunCommandPanel)
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL3;
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT6;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT5;
		static const long ID_TEXTCTRL4;
		static const long ID_STATICTEXT4;
		static const long ID_TEXTCTRL5;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL2;
		//*)

	private:

		//(*Handlers(PlayListItemRunCommandPanel)
		void OnTextCtrl_DelayText(wxCommandEvent& event);
		void OnTextCtrl_RunCommandNameText(wxCommandEvent& event);
		void OnTextCtrl_Parm1Text(wxCommandEvent& event);
		void OnTextCtrl_Parm2Text(wxCommandEvent& event);
		void OnTextCtrl_Parm3Text(wxCommandEvent& event);
		void OnChoice_CommandSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
