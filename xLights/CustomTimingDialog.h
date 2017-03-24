#ifndef CUSTOMTIMINGDIALOG_H
#define CUSTOMTIMINGDIALOG_H

//(*Headers(CustomTimingDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class CustomTimingDialog: public wxDialog
{
	public:

		CustomTimingDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~CustomTimingDialog();
        wxString GetTiming() const;

		//(*Declarations(CustomTimingDialog)
		wxButton* Button_Ok;
		wxStaticText* StaticText1;
		wxSpinCtrl* SpinCtrl_Interval;
		wxTextCtrl* TextCtrl_FPS;
		//*)

	protected:

		//(*Identifiers(CustomTimingDialog)
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL_Interval;
		static const long ID_TEXTCTRL_FPS;
		static const long ID_BUTTON_Ok;
		//*)

	private:

		//(*Handlers(CustomTimingDialog)
		void OnSpinCtrl_IntervalChange(wxSpinEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
