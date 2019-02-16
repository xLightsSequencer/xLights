#ifndef TestMessagesDialog_H
#define TestMessagesDialog_H

//(*Headers(TestMessagesDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class TestMessagesDialog: public wxDialog
{
    void ValidateWindow();

    public:

		TestMessagesDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~TestMessagesDialog();

		//(*Declarations(TestMessagesDialog)
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxStaticText* StaticText1;
		wxTextCtrl* TextCtrl_Messages;
		//*)

	protected:

		//(*Identifiers(TestMessagesDialog)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(TestMessagesDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnTextCtrl_MessagesText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
