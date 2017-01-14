#ifndef PLAYLISTITEMALLOFFPANEL_H
#define PLAYLISTITEMALLOFFPANEL_H

//(*Headers(PlayListItemAllOffPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/panel.h>
#include <wx/choice.h>
//*)

class PlayListItemAllOff;

class PlayListItemAllOffPanel: public wxPanel
{
    PlayListItemAllOff* _alloff;

	public:

		PlayListItemAllOffPanel(wxWindow* parent, PlayListItemAllOff* alloff, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemAllOffPanel();

		//(*Declarations(PlayListItemAllOffPanel)
		wxChoice* Choice_BlendMode;
		wxStaticText* StaticText2;
		wxTextCtrl* TextCtrl_AllOffDuration;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxTextCtrl* TextCtrl_Delay;
		wxSpinCtrl* SpinCtrl_Priority;
		wxStaticText* StaticText5;
		wxStaticText* StaticText4;
		wxSpinCtrl* SpinCtrl_AllSet;
		//*)

	protected:

		//(*Identifiers(PlayListItemAllOffPanel)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT4;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT5;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL2;
		//*)

	private:

		//(*Handlers(PlayListItemAllOffPanel)
		void OnTextCtrl_AllOffDurationText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
