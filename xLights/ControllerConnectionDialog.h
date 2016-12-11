#ifndef CONTROLLERCONNECTIONDIALOG_H
#define CONTROLLERCONNECTIONDIALOG_H

//(*Headers(ControllerConnectionDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class ControllerConnectionDialog: public wxDialog
{
	public:

		ControllerConnectionDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ControllerConnectionDialog();

		//(*Declarations(ControllerConnectionDialog)
		wxButton* Button_Ok;
		wxChoice* Choice_Protocol;
		wxStaticText* StaticText2;
		wxSpinCtrl* SpinCtrl_Port;
		wxStaticText* StaticText1;
		wxButton* Button_Cancel;
		//*)

        void Set(const wxString &controllerconnection);
        std::string Get();

	protected:

		//(*Identifiers(ControllerConnectionDialog)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(ControllerConnectionDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
