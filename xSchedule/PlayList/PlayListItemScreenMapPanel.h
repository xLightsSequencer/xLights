#ifndef PLAYLISTITEMSCREENMAPPANEL_H
#define PLAYLISTITEMSCREENMAPPANEL_H

//(*Headers(PlayListItemScreenMapPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/panel.h>
#include <wx/choice.h>
//*)

class PlayListItemScreenMap;
class OutlineWindow;

class PlayListItemScreenMapPanel: public wxPanel
{
    PlayListItemScreenMap* _screenMap;
    OutlineWindow* _outlineWindow;
    void ValidateWindow();

    public:

		PlayListItemScreenMapPanel(wxWindow* parent, PlayListItemScreenMap* screenMap, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemScreenMapPanel();

		//(*Declarations(PlayListItemScreenMapPanel)
		wxStaticText* StaticText10;
		wxStaticText* StaticText9;
		wxSpinCtrl* SpinCtrl_Y;
		wxSpinCtrl* SpinCtrl_X;
		wxCheckBox* CheckBox_Rescale;
		wxChoice* Choice_BlendMode;
		wxTextCtrl* TextCtrl_Name;
		wxStaticText* StaticText13;
		wxStaticText* StaticText2;
		wxChoice* Choice_Matrices;
		wxStaticText* StaticText8;
		wxStaticText* StaticText11;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxSpinCtrl* SpinCtrl_Height;
		wxSpinCtrl* SpinCtrl_Priority;
		wxStaticText* StaticText12;
		wxChoice* Choice_Quality;
		wxTextCtrl* TextCtrl_Duration;
		wxStaticText* StaticText4;
		wxSpinCtrl* SpinCtrl_Width;
		//*)

	protected:

		//(*Identifiers(PlayListItemScreenMapPanel)
		static const long ID_STATICTEXT14;
		static const long ID_TEXTCTRL4;
		static const long ID_STATICTEXT11;
		static const long ID_CHOICE5;
		static const long ID_STATICTEXT10;
		static const long ID_CHOICE4;
		static const long ID_CHECKBOX1;
		static const long ID_STATICTEXT3;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT12;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT13;
		static const long ID_SPINCTRL3;
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL5;
		static const long ID_STATICTEXT8;
		static const long ID_SPINCTRL4;
		static const long ID_STATICTEXT9;
		static const long ID_TEXTCTRL3;
		//*)

	private:

		//(*Handlers(PlayListItemScreenMapPanel)
		void OnTextCtrl_NameText(wxCommandEvent& event);
		void OnCheckBox_RescaleClick(wxCommandEvent& event);
		void OnSpinCtrl_PosChange(wxSpinEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
