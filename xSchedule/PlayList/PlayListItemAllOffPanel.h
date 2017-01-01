#ifndef PLAYLISTITEMALLOFFPANEL_H
#define PLAYLISTITEMALLOFFPANEL_H

//(*Headers(PlayListItemAllOffPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/panel.h>
//*)

class PlayListItemAllOff;

class PlayListItemAllOffPanel: public wxPanel
{
    PlayListItemAllOff* _alloff;

	public:

		PlayListItemAllOffPanel(wxWindow* parent, PlayListItemAllOff* alloff, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemAllOffPanel();

		//(*Declarations(PlayListItemAllOffPanel)
		wxStaticText* StaticText2;
		wxTextCtrl* TextCtrl_AllOffDuration;
		wxStaticText* StaticText1;
		wxTextCtrl* TextCtrl_Delay;
		//*)

	protected:

		//(*Identifiers(PlayListItemAllOffPanel)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
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
