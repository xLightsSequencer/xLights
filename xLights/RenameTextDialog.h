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

//(*Headers(RenameTextDialog)
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class RenameTextDialog: public wxDialog
{
	public:

		RenameTextDialog(wxWindow* parent,wxWindowID id=wxID_ANY);
		virtual ~RenameTextDialog();

        void SetRenameText(wxString text) { TextCtrl_Rename_Text->SetValue(text); }
        wxString GetRenameText() {return rename_text; }

		//(*Declarations(RenameTextDialog)
		wxStaticText* StaticText_Rename_Label;
		wxTextCtrl* TextCtrl_Rename_Text;
		//*)

	protected:

		//(*Identifiers(RenameTextDialog)
		static const long ID_STATICTEXT_Rename_Label;
		static const long ID_TEXTCTRL_Rename_Text;
		//*)

	private:

		//(*Handlers(RenameTextDialog)
		void OnTextCtrl_Rename_TextText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()

		wxString rename_text;
};

