#ifndef NEWTIMINGDIALOG_H
#define NEWTIMINGDIALOG_H

//(*Headers(NewTimingDialog)
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/choice.h>
//*)

class NewTimingDialog: public wxDialog
{
	public:

		NewTimingDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~NewTimingDialog();

        wxString GetTiming()
        {
            int selection = Choice_New_Fixed_Timing->GetSelection();
            return Choice_New_Fixed_Timing->GetString(selection);
        }

        void RemoveChoice(const wxString selection);

		//(*Declarations(NewTimingDialog)
		wxStaticText* StaticText1;
		wxChoice* Choice_New_Fixed_Timing;
		//*)

	protected:

		//(*Identifiers(NewTimingDialog)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE_New_Fixed_Timing;
		//*)

	private:

		//(*Handlers(NewTimingDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
