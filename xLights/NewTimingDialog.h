#ifndef NEWTIMINGDIALOG_H
#define NEWTIMINGDIALOG_H

//(*Headers(NewTimingDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/dialog.h>
//*)

class NewTimingDialog: public wxDialog
{
	public:

		NewTimingDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~NewTimingDialog();

		//(*Declarations(NewTimingDialog)
		wxStaticText* StaticText1;
		wxChoice* Choice1;
		//*)

	protected:

		//(*Identifiers(NewTimingDialog)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		//*)

	private:

		//(*Handlers(NewTimingDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
