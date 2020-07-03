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

 //(*Headers(PlayListItemPluginPanel)
#include <wx/choice.h>
#include <wx/combobox.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class PlayListItemPlugin;

class PlayListItemPluginPanel: public wxPanel
{
    PlayListItemPlugin* _plugin;

	public:

		PlayListItemPluginPanel(wxWindow* parent, PlayListItemPlugin* plugin, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListItemPluginPanel();
        void ValidateWindow();

		//(*Declarations(PlayListItemPluginPanel)
		wxChoice* Choice_Action;
		wxComboBox* ComboBox_PluginName;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxTextCtrl* TextCtrl_Delay;
		wxTextCtrl* TextCtrl_EventParm;
		wxTextCtrl* TextCtrl_Name;
		//*)

	protected:

		//(*Identifiers(PlayListItemPluginPanel)
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL3;
		static const long ID_STATICTEXT4;
		static const long ID_COMBOBOX1;
		static const long ID_STATICTEXT5;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL4;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL2;
		//*)

	private:

		//(*Handlers(PlayListItemPluginPanel)
		void OnTextCtrl_DelayText(wxCommandEvent& event);
		void OnChoice_ActionSelect(wxCommandEvent& event);
		void OnTextCtrl_NameText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

