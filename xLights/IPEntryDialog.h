#ifndef IPENTRYDIALOG_H
#define IPENTRYDIALOG_H

//(*Headers(IPEntryDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class IPEntryDialog: public wxDialog
{
    void ValidateWindow();
    
	public:

		IPEntryDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~IPEntryDialog();

		//(*Declarations(IPEntryDialog)
		wxButton* Button_Ok;
		wxTextCtrl* TextCtrl_IPAddress;
		wxStaticText* StaticText1;
		wxButton* Button_Cancel;
		//*)

	protected:

		//(*Identifiers(IPEntryDialog)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(IPEntryDialog)
		void OnTextCtrl_IPAddressText(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
