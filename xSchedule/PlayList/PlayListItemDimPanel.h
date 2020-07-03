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

 //(*Headers(PlayListItemDimPanel)
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class OutputManager;
class PlayListItemDim;

class PlayListItemDimPanel: public wxPanel
{
    OutputManager* _outputManager;
    PlayListItemDim* _dim;
    void ValidateWindow();

	public:

		PlayListItemDimPanel(wxWindow* parent, OutputManager* outputManager, PlayListItemDim* dim, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemDimPanel();

		//(*Declarations(PlayListItemDimPanel)
		wxSpinCtrl* SpinCtrl_Channels;
		wxSpinCtrl* SpinCtrl_Dim;
		wxSpinCtrl* SpinCtrl_Priority;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		wxStaticText* StaticText_StartChannel;
		wxTextCtrl* TextCtrl_Duration;
		wxTextCtrl* TextCtrl_Name;
		wxTextCtrl* TextCtrl_StartChannel;
		//*)

	protected:

		//(*Identifiers(PlayListItemDimPanel)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT5;
		static const long ID_TEXTCTRL4;
		static const long ID_STATICTEXT9;
		static const long ID_STATICTEXT6;
		static const long ID_SPINCTRL4;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT7;
		static const long ID_TEXTCTRL2;
		//*)

	private:

		//(*Handlers(PlayListItemDimPanel)
		void OnTextCtrl_NameText(wxCommandEvent& event);
		void OnChoice_ModeSelect(wxCommandEvent& event);
		void OnTextCtrl_StartChannelText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

