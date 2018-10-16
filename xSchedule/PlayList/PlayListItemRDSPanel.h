#ifndef PLAYLISTITEMRDSPANEL_H
#define PLAYLISTITEMRDSPANEL_H

//(*Headers(PlayListItemRDSPanel)
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class PlayListItemRDS;

class PlayListItemRDSPanel: public wxPanel
{
    PlayListItemRDS* _rds;
    void ValidateWindow();

	public:

		PlayListItemRDSPanel(wxWindow* parent, PlayListItemRDS* rds, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemRDSPanel();

		//(*Declarations(PlayListItemRDSPanel)
		wxCheckBox* CheckBox_HighSpeed;
		wxChoice* Choice_CommPort;
		wxChoice* Choice_ScrollMode;
		wxSpinCtrl* SpinCtrl_StationDuration;
		wxSpinCtrl* SpinCtrl_TextDuration;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText_StationDuration;
		wxStaticText* StaticText_TextDuration;
		wxTextCtrl* TextCtrl_StationName;
		wxTextCtrl* TextCtrl_Text;
		//*)

	protected:

		//(*Identifiers(PlayListItemRDSPanel)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT10;
		static const long ID_STATICTEXT4;
		static const long ID_CHOICE2;
		static const long ID_STATICTEXT5;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT6;
		static const long ID_SPINCTRL2;
		static const long ID_CHECKBOX1;
		//*)

	private:

		//(*Handlers(PlayListItemRDSPanel)
		void OnChoice_ScrollModeSelect(wxCommandEvent& event);
		void OnSpinCtrl_StationDurationChange(wxSpinEvent& event);
		void OnSpinCtrl_TextDurationChange(wxSpinEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
