#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

 //(*Headers(PlayListItemScreenMapPanel)
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
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
        void SizeChange(int x, int y, int w, int h);

		//(*Declarations(PlayListItemScreenMapPanel)
		wxCheckBox* CheckBox_Rescale;
		wxChoice* Choice_BlendMode;
		wxChoice* Choice_Matrices;
		wxChoice* Choice_Quality;
		wxSpinCtrl* SpinCtrl_Height;
		wxSpinCtrl* SpinCtrl_Priority;
		wxSpinCtrl* SpinCtrl_Width;
		wxSpinCtrl* SpinCtrl_X;
		wxSpinCtrl* SpinCtrl_Y;
		wxStaticText* StaticText10;
		wxStaticText* StaticText11;
		wxStaticText* StaticText12;
		wxStaticText* StaticText13;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText8;
		wxStaticText* StaticText9;
		wxTextCtrl* TextCtrl_Delay;
		wxTextCtrl* TextCtrl_Duration;
		wxTextCtrl* TextCtrl_Name;
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
		static const long ID_STATICTEXT4;
		static const long ID_TEXTCTRL2;
		//*)

	private:

		//(*Handlers(PlayListItemScreenMapPanel)
		void OnTextCtrl_NameText(wxCommandEvent& event);
		void OnCheckBox_RescaleClick(wxCommandEvent& event);
		void OnSpinCtrl_PosChange(wxSpinEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

