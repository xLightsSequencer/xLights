#ifndef NULLOUTPUTDIALOG_H
#define NULLOUTPUTDIALOG_H

//(*Headers(NullOutputDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/dialog.h>
//*)

class NullOutputDialog: public wxDialog
{
	public:

		NullOutputDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~NullOutputDialog();

		//(*Declarations(NullOutputDialog)
		wxSpinCtrl* NumChannelsSpinCtrl;
		wxStaticText* StaticText1;
		//*)

	protected:

		//(*Identifiers(NullOutputDialog)
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL1;
		//*)

	private:

		//(*Handlers(NullOutputDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
