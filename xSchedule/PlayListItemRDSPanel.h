#ifndef PLAYLISTITEMRDSPANEL_H
#define PLAYLISTITEMRDSPANEL_H

//(*Headers(PlayListItemRDSPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/panel.h>
#include <wx/choice.h>
//*)

class PlayListItemRDSPanel: public wxPanel
{
	public:

		PlayListItemRDSPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemRDSPanel();

		//(*Declarations(PlayListItemRDSPanel)
		wxSpinCtrl* SpinCtrl_StationDuration;
		wxSpinCtrl* SpinCtrl_LineDuration;
		wxSpinCtrl* SpinCtrl_TextDuration;
		wxStaticText* StaticText2;
		wxStaticText* StaticText6;
		wxCheckBox* CheckBox_HighSpeed;
		wxTextCtrl* TextCtrl_Text;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxChoice* Choice_CommPort;
		wxStaticText* StaticText5;
		wxStaticText* StaticText7;
		wxChoice* Choice_ScrollMode;
		wxTextCtrl* TextCtrl_StationName;
		wxStaticText* StaticText4;
		//*)

	protected:

		//(*Identifiers(PlayListItemRDSPanel)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT4;
		static const long ID_CHOICE2;
		static const long ID_STATICTEXT5;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT6;
		static const long ID_SPINCTRL2;
		static const long ID_CHECKBOX1;
		static const long ID_STATICTEXT7;
		static const long ID_SPINCTRL3;
		//*)

	private:

		//(*Handlers(PlayListItemRDSPanel)
		void OnChoice_ScrollModeSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
