#ifndef E131DIALOG_H
#define E131DIALOG_H

//(*Headers(E131Dialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/radiobut.h>
#include <wx/dialog.h>
//*)

class E131Dialog: public wxDialog
{
	public:

		E131Dialog(wxWindow* parent);
		virtual ~E131Dialog();

		//(*Declarations(E131Dialog)
		wxRadioButton* RadioButtonUnicast;
		wxTextCtrl* TextCtrlIpAddr;
		wxStaticText* StaticText2;
		wxStaticText* StaticText6;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxRadioButton* RadioButtonMulticast;
		wxStaticText* StaticText5;
		wxSpinCtrl* SpinCtrl_NumUniv;
		wxSpinCtrl* SpinCtrl_StartUniv;
		wxStaticText* StaticText4;
		wxSpinCtrl* SpinCtrl_LastChannel;
		//*)

	protected:

		//(*Identifiers(E131Dialog)
		static const long ID_STATICTEXT4;
		static const long ID_STATICTEXT5;
		static const long ID_RADIOBUTTON1;
		static const long ID_RADIOBUTTON2;
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL_IP_ADDR;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT6;
		static const long ID_SPINCTRL_LAST_CHANNEL;
		//*)

	private:

		//(*Handlers(E131Dialog)
		void OnRadioButtonUnicastSelect(wxCommandEvent& event);
		void OnRadioButtonMulticastSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
