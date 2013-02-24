#ifndef SHOWDATESDIALOG_H
#define SHOWDATESDIALOG_H

//(*Headers(ShowDatesDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/calctrl.h>
#include <wx/statline.h>
#include <wx/dialog.h>
//*)

class ShowDatesDialog: public wxDialog
{
	public:

		ShowDatesDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ShowDatesDialog();

		//(*Declarations(ShowDatesDialog)
		wxStaticText* StaticText2;
		wxStaticText* StaticText1;
		wxCalendarCtrl* CalendarCtrlStart;
		wxStaticLine* StaticLine1;
		wxCalendarCtrl* CalendarCtrlEnd;
		//*)

	protected:

		//(*Identifiers(ShowDatesDialog)
		static const long ID_STATICTEXT1;
		static const long ID_CALENDARCTRL_START;
		static const long ID_STATICLINE1;
		static const long ID_STATICTEXT2;
		static const long ID_CALENDARCTRL_END;
		//*)

	private:

		//(*Handlers(ShowDatesDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
