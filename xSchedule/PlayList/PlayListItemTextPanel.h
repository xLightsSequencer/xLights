#ifndef PLAYLISTITEMTEXTPANEL_H
#define PLAYLISTITEMTEXTPANEL_H

//(*Headers(PlayListItemTextPanel)
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/fontpicker.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class PlayListItemText;

class PlayListItemTextPanel: public wxPanel
{
    PlayListItemText* _text;
    void ValidateWindow();

    public:

		PlayListItemTextPanel(wxWindow* parent, PlayListItemText* text,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemTextPanel();

		//(*Declarations(PlayListItemTextPanel)
		wxCheckBox* CheckBox_RenderWhenBlank;
		wxChoice* Choice_BlendMode;
		wxChoice* Choice_Matrices;
		wxChoice* Choice_Movement;
		wxChoice* Choice_Orientation;
		wxChoice* Choice_Type;
		wxFontPickerCtrl* FontPickerCtrl1;
		wxSpinCtrl* SpinCtrl_Priority;
		wxSpinCtrl* SpinCtrl_Speed;
		wxSpinCtrl* SpinCtrl_X;
		wxSpinCtrl* SpinCtrl_Y;
		wxStaticText* StaticText10;
		wxStaticText* StaticText11;
		wxStaticText* StaticText12;
		wxStaticText* StaticText13;
		wxStaticText* StaticText14;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		wxStaticText* StaticText8;
		wxStaticText* StaticText9;
		wxStaticText* StaticText_Text;
		wxTextCtrl* TextCtrl_Delay;
		wxTextCtrl* TextCtrl_Duration;
		wxTextCtrl* TextCtrl_Format;
		wxTextCtrl* TextCtrl_Name;
		wxTextCtrl* TextCtrl_Text;
		//*)

	protected:

		//(*Identifiers(PlayListItemTextPanel)
		static const long ID_STATICTEXT14;
		static const long ID_TEXTCTRL4;
		static const long ID_STATICTEXT11;
		static const long ID_CHOICE5;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT5;
		static const long ID_FONTPICKERCTRL1;
		static const long ID_STATICTEXT7;
		static const long ID_CHOICE3;
		static const long ID_STATICTEXT4;
		static const long ID_CHOICE2;
		static const long ID_STATICTEXT6;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT10;
		static const long ID_CHOICE4;
		static const long ID_CHECKBOX1;
		static const long ID_STATICTEXT12;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT13;
		static const long ID_SPINCTRL3;
		static const long ID_STATICTEXT8;
		static const long ID_SPINCTRL4;
		static const long ID_STATICTEXT9;
		static const long ID_TEXTCTRL3;
		static const long ID_STATICTEXT15;
		static const long ID_TEXTCTRL5;
		//*)

	private:

		//(*Handlers(PlayListItemTextPanel)
		void OnChoice_TypeSelect(wxCommandEvent& event);
		void OnTextCtrl_NameText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
