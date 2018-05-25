#ifndef UNIVERSEENTRYDIALOG_H
#define UNIVERSEENTRYDIALOG_H

//(*Headers(UniverseEntryDialog)
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class UniverseEntryDialog: public wxDialog
{
    void ValidateWindow();

	public:

		UniverseEntryDialog(wxWindow* parent, int start, int end, std::string ipaddress, std::string desc, std::string protocol, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~UniverseEntryDialog();

		//(*Declarations(UniverseEntryDialog)
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxChoice* Choice_Protocol;
		wxSpinCtrl* SpinCtrl_End;
		wxSpinCtrl* SpinCtrl_Start;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxTextCtrl* TextCtrl_Description;
		wxTextCtrl* TextCtrl_IPAddress;
		//*)

	protected:

		//(*Identifiers(UniverseEntryDialog)
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT5;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT4;
		static const long ID_TEXTCTRL2;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(UniverseEntryDialog)
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		void OnSpinCtrl_StartChange(wxSpinEvent& event);
		void OnSpinCtrl_EndChange(wxSpinEvent& event);
		void OnTextCtrl_IPAddressText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
