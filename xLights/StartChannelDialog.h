#ifndef STARTCHANNELDIALOG_H
#define STARTCHANNELDIALOG_H

//(*Headers(StartChannelDialog)
#include <wx/spinctrl.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
//*)

#include <string>

class StartChannelDialog: public wxDialog
{
	public:

		StartChannelDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~StartChannelDialog();

		//(*Declarations(StartChannelDialog)
		wxSpinCtrl* StartChannel;
		wxSpinCtrl* OutputSpin;
		wxStaticText* StaticText2;
		//*)
    
        void Set(const wxString &start);
        std::string Get();

	protected:

		//(*Identifiers(StartChannelDialog)
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL2;
		//*)

	private:

		//(*Handlers(StartChannelDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
