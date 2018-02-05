#ifndef PLAYLISTITEMTEXTPANEL_H
#define PLAYLISTITEMTEXTPANEL_H

//(*Headers(PlayListItemTextPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/fontpicker.h>
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
		wxStaticText* StaticText10;
		wxStaticText* StaticText9;
		wxCheckBox* CheckBox_RenderWhenBlank;
		wxSpinCtrl* SpinCtrl_Y;
		wxChoice* Choice_Orientation;
		wxSpinCtrl* SpinCtrl_X;
		wxFontPickerCtrl* FontPickerCtrl1;
		wxChoice* Choice_BlendMode;
		wxTextCtrl* TextCtrl_Name;
		wxStaticText* StaticText13;
		wxStaticText* StaticText2;
		wxChoice* Choice_Matrices;
		wxStaticText* StaticText6;
		wxSpinCtrl* SpinCtrl_Speed;
		wxChoice* Choice_Movement;
		wxStaticText* StaticText8;
		wxStaticText* StaticText11;
		wxTextCtrl* TextCtrl_Text;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxTextCtrl* TextCtrl_Format;
		wxSpinCtrl* SpinCtrl_Priority;
		wxStaticText* StaticText5;
		wxStaticText* StaticText7;
		wxStaticText* StaticText_Text;
		wxStaticText* StaticText12;
		wxChoice* Choice_Type;
		wxTextCtrl* TextCtrl_Duration;
		wxStaticText* StaticText4;
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
		//*)

	private:

		//(*Handlers(PlayListItemTextPanel)
		void OnChoice_TypeSelect(wxCommandEvent& event);
		void OnButton_ColorClick(wxCommandEvent& event);
		void OnTextCtrl_NameText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
