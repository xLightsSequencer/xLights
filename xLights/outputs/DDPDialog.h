#ifndef DDPDIALOG_H
#define DDPDIALOG_H

//(*Headers(DDPDialog)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class DDPOutput;
class OutputManager;

class DDPDialog: public wxDialog
{
    void ValidateWindow();
    DDPOutput* _DDP;
    OutputManager* _outputManager;

	public:

		DDPDialog(wxWindow* parent, DDPOutput* DDP, OutputManager* outputManager, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~DDPDialog();

		//(*Declarations(DDPDialog)
		wxButton* ButtonCancel;
		wxButton* ButtonOk;
		wxCheckBox* CheckBox1;
		wxCheckBox* CheckBoxKeepChannels;
		wxSpinCtrl* SpinCtrl1;
		wxSpinCtrl* SpinCtrlChannels;
		wxSpinCtrl* SpinCtrl_ChannelsPerPacket;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText7;
		wxTextCtrl* TextCtrlDescription;
		wxTextCtrl* TextCtrlIPAddress;
		//*)

	protected:

		//(*Identifiers(DDPDialog)
		static const long ID_STATICTEXT1;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT5;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT7;
		static const long ID_SPINCTRL4;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL_ChannelsPerPacket;
		static const long ID_CHECKBOX_KEEPCHANNELS;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL2;
		static const long ID_CHECKBOX1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(DDPDialog)
		void OnTextCtrlIPAddressText(wxCommandEvent& event);
		void OnButtonOkClick(wxCommandEvent& event);
		void OnButtonCancelClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
