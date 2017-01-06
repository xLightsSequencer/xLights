#ifndef PROJECTORDETAILSDIALOG_H
#define PROJECTORDETAILSDIALOG_H

//(*Headers(ProjectorDetailsDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class ProjectorDetailsDialog: public wxDialog
{
	public:

		ProjectorDetailsDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ProjectorDetailsDialog();

		//(*Declarations(ProjectorDetailsDialog)
		wxButton* Button_Ok;
		wxTextCtrl* TextCtrl_Name;
		wxStaticText* StaticText2;
		wxTextCtrl* TextCtrl_IPAddress;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxButton* Button_Cancel;
		wxTextCtrl* TextCtrl_Password;
		//*)

	protected:

		//(*Identifiers(ProjectorDetailsDialog)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL3;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(ProjectorDetailsDialog)
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		void OnTextCtrl_IPAddressText(wxCommandEvent& event);
		void OnTextCtrl_NameText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
