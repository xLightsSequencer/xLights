#ifndef CONVERTLOGDIALOG_H
#define CONVERTLOGDIALOG_H

//(*Headers(ConvertLogDialog)
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/dialog.h>
//*)

class ConvertLogDialog: public wxDialog
{
    wxString msgBuffer;

    public:

        void Done();
        void AppendConvertStatus(const wxString &msg, bool flushBuffer = true);
        ConvertLogDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ConvertLogDialog();

		//(*Declarations(ConvertLogDialog)
		wxTextCtrl* TextCtrlLog;
		//*)

	protected:

		//(*Identifiers(ConvertLogDialog)
		static const long ID_TEXTCTRL_LOG;
		//*)

	private:

		//(*Handlers(ConvertLogDialog)
		void OnButtonCloseClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
