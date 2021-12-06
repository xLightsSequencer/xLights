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

 //(*Headers(MagicWordDialog)
 #include <wx/button.h>
 #include <wx/choice.h>
 #include <wx/dialog.h>
 #include <wx/sizer.h>
 #include <wx/stattext.h>
 #include <wx/textctrl.h>
 //*)

#include "../CommandManager.h"

class MagicWord;

class MagicWordDialog: public wxDialog
{
    MagicWord* _magicWord;
	CommandManager _commandManager;

	public:

		MagicWordDialog(wxWindow* parent, MagicWord* magicWord, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~MagicWordDialog();
        MagicWord* GetMagicWord() const { return _magicWord; }
        void ValidateWindow();

		//(*Declarations(MagicWordDialog)
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxChoice* Choice_Command;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxTextCtrl* TextCtrl_Name;
		wxTextCtrl* TextCtrl_P1;
		wxTextCtrl* TextCtrl_P2;
		wxTextCtrl* TextCtrl_P3;
		//*)

	protected:

		//(*Identifiers(MagicWordDialog)
		static const long ID_STATICTEXT5;
		static const long ID_TEXTCTRL4;
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT4;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL3;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(MagicWordDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnChoice_CommandSelect(wxCommandEvent& event);
		void OnTextCtrl_P1Text(wxCommandEvent& event);
		void OnTextCtrl_P2Text(wxCommandEvent& event);
		void OnTextCtrl_P3Text(wxCommandEvent& event);
		void OnTextCtrl_NameText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
