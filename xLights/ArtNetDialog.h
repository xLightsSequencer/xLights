#ifndef ARTNETDIALOG_H
#define ARTNETDIALOG_H

//(*Headers(ArtNetDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class ArtNetDialog: public wxDialog
{
    void ValidateWindow();

	public:

		ArtNetDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ArtNetDialog();

		//(*Declarations(ArtNetDialog)
		wxSpinCtrl* SpinCtrlSubnet;
		wxStaticText* StaticText2;
		wxStaticText* StaticText6;
		wxTextCtrl* TextCtrlIPAddress;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxTextCtrl* TextCtrlDescription;
		wxButton* ButtonCancel;
		wxSpinCtrl* SpinCtrlUniverse;
		wxStaticText* StaticText5;
		wxSpinCtrl* SpinCtrlNet;
		wxStaticText* StaticText7;
		wxButton* ButtonOk;
		wxStaticText* StaticText4;
		wxSpinCtrl* SpinCtrlChannels;
		//*)

	protected:

		//(*Identifiers(ArtNetDialog)
		static const long ID_STATICTEXT1;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT5;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT6;
		static const long ID_SPINCTRL3;
		static const long ID_STATICTEXT7;
		static const long ID_SPINCTRL4;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL2;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(ArtNetDialog)
		void OnTextCtrlIPAddressText(wxCommandEvent& event);
		void OnButtonOkClick(wxCommandEvent& event);
		void OnButtonCancelClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
