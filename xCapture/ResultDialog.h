#ifndef RESULTDIALOG_H
#define RESULTDIALOG_H

//(*Headers(ResultDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
//*)

class ResultDialog: public wxDialog
{
	public:

		ResultDialog(wxWindow* parent, wxString log, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ResultDialog();

		//(*Declarations(ResultDialog)
		wxButton* Button_Save;
		wxTextCtrl* TextCtrl_Log;
		//*)

	protected:

		//(*Identifiers(ResultDialog)
		static const long ID_TEXTCTRL1;
		static const long ID_BUTTON1;
		//*)

	private:

		//(*Handlers(ResultDialog)
		void OnButton_SaveClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
