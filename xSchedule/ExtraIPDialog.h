#ifndef EXTRAIPDIALOG_H
#define EXTRAIPDIALOG_H

//(*Headers(ExtraIPDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

#include "ScheduleOptions.h"

class ExtraIPDialog: public wxDialog
{
    std::list<ExtraIP*>* _extraIPs;
    ExtraIP* _sel;
    void ValidateWindow();

	public:

		ExtraIPDialog(wxWindow* parent, std::list<ExtraIP*>* extraIPs, ExtraIP* sel,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ExtraIPDialog();

		//(*Declarations(ExtraIPDialog)
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxTextCtrl* TextCtrl_Description;
		wxTextCtrl* TextCtrl_Ip;
		//*)

	protected:

		//(*Identifiers(ExtraIPDialog)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL2;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(ExtraIPDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnTextCtrl_IpText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
