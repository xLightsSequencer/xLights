#ifndef PLAYLISTITEMFPPEVENTPANEL_H
#define PLAYLISTITEMFPPEVENTPANEL_H

//(*Headers(PlayListItemFPPEventPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/panel.h>
//*)

class PlayListItemFPPEvent;

class PlayListItemFPPEventPanel: public wxPanel
{
    PlayListItemFPPEvent* _fppevent;

	public:

		PlayListItemFPPEventPanel(wxWindow* parent, PlayListItemFPPEvent* fppevent, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemFPPEventPanel();

		//(*Declarations(PlayListItemFPPEventPanel)
		wxTextCtrl* TextCtrl_FPPEventName;
		wxStaticText* StaticText2;
		wxSpinCtrl* SpinCtrl_Minor;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxTextCtrl* TextCtrl_Delay;
		wxSpinCtrl* SpinCtrl_Major;
		wxStaticText* StaticText4;
		//*)

	protected:

		//(*Identifiers(PlayListItemFPPEventPanel)
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL3;
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL2;
		//*)

	private:

		//(*Handlers(PlayListItemFPPEventPanel)
		void OnTextCtrl_DelayText(wxCommandEvent& event);
		void OnTextCtrl_FPPEventNameText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
