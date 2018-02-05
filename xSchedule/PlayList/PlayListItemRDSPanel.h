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

class PlayListItemRDS;

class PlayListItemRDSPanel: public wxPanel
{
    PlayListItemRDS* _rds;
    void ValidateWindow();

	public:

		PlayListItemRDSPanel(wxWindow* parent, PlayListItemRDS* rds, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemRDSPanel();

		//(*Declarations(PlayListItemRDSPanel)
		wxSpinCtrl* SpinCtrl_StationDuration;
		wxStaticText* StaticText_StationDuration;
		wxChoice* Choice_SerialSpeed;
		wxSpinCtrl* SpinCtrl_TextDuration;
		wxStaticText* StaticText2;
		wxStaticText* StaticText6;
		wxStaticText* StaticText8;
		wxCheckBox* CheckBox_HighSpeed;
		wxTextCtrl* TextCtrl_Text;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxChoice* Choice_CommPort;
		wxStaticText* StaticText5;
		wxChoice* Choice_ScrollMode;
		wxStaticText* StaticText_TextDuration;
		wxTextCtrl* TextCtrl_StationName;
		wxStaticText* StaticText4;
		wxCheckBox* CheckBox_MRDS;
		//*)

	protected:

		//(*Identifiers(PlayListItemRDSPanel)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT8;
		static const long ID_CHOICE3;
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
		static const long ID_CHECKBOX2;
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
