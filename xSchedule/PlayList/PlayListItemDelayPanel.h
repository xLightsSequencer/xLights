#ifndef PLAYLISTITEMDELAYPANEL_H
#define PLAYLISTITEMDELAYPANEL_H

//(*Headers(PlayListItemDelayPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/panel.h>
//*)

class PlayListItemDelay;

class PlayListItemDelayPanel: public wxPanel
{
    PlayListItemDelay* _Delay;

	public:

		PlayListItemDelayPanel(wxWindow* parent, PlayListItemDelay* Delay, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemDelayPanel();

		//(*Declarations(PlayListItemDelayPanel)
		wxStaticText* StaticText1;
		wxTextCtrl* TextCtrl_DelayDuration;
		//*)

	protected:

		//(*Identifiers(PlayListItemDelayPanel)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		//*)

	private:

		//(*Handlers(PlayListItemDelayPanel)
		void OnTextCtrl_DelayDurationText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
