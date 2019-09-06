#ifndef EMAILDIALOG_H
#define EMAILDIALOG_H

//(*Headers(EmailDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class EmailDialog: public wxDialog
{
    void ValidateWindow();

	public:

		EmailDialog(wxWindow* parent, const std::string& email, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EmailDialog();
        std::string GetEmail() const;

		//(*Declarations(EmailDialog)
		wxButton* Button_No;
		wxButton* Button_Ok;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxTextCtrl* TextCtrl_Email;
		//*)

	protected:

		//(*Identifiers(EmailDialog)
		static const long ID_STATICTEXT1;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(EmailDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_NoClick(wxCommandEvent& event);
		void OnTextCtrl_EmailText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
#endif