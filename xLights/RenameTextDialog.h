#ifndef RENAMETEXTDIALOG_H
#define RENAMETEXTDIALOG_H

//(*Headers(RenameTextDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/dialog.h>
//*)

class RenameTextDialog: public wxDialog
{
	public:

		RenameTextDialog(wxWindow* parent,wxWindowID id=wxID_ANY);
		virtual ~RenameTextDialog();

        void SetRenameText(wxString text) { TextCtrl_Rename_Text->SetValue(text); }
        wxString GetRenameText() {return rename_text; }

		//(*Declarations(RenameTextDialog)
		wxTextCtrl* TextCtrl_Rename_Text;
		wxStaticText* StaticText_Rename_Label;
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

#endif
