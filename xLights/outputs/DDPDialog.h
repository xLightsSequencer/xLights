#ifndef DDPDIALOG_H
#define DDPDIALOG_H

//(*Headers(DDPDialog)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
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
		wxButton* VisualizeButton;
		wxCheckBox* CheckBox1;
		wxCheckBox* CheckBoxKeepChannels;
		wxCheckBox* CheckBox_AutoSize_Output;
		wxCheckBox* CheckBox_Auto_Channels;
		wxChoice* ControllerTypeChoice;
		wxSpinCtrl* SpinCtrl1;
		wxSpinCtrl* SpinCtrlChannels;
		wxSpinCtrl* SpinCtrl_ChannelsPerPacket;
		wxStaticText* StaticText10;
		wxStaticText* StaticText1;
		wxTextCtrl* FPPProxyHost;
		wxTextCtrl* TextCtrlDescription;
		wxTextCtrl* TextCtrlIPAddress;
		//*)

	protected:

		//(*Identifiers(DDPDialog)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_SPINCTRL1;
		static const long ID_SPINCTRL4;
		static const long ID_SPINCTRL_ChannelsPerPacket;
		static const long ID_CHECKBOX_KEEPCHANNELS;
		static const long ID_CHECKBOX1;
		static const long ID_TEXTCTRL2;
		static const long ID_CHOICE1;
		static const long ID_CHECKBOX_AUTO_CHANNELS;
		static const long ID_STATICTEXT2;
		static const long ID_CHECKBOX_AUTO_SIZE_OUTPUT;
		static const long ID_TEXTCTRL3;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_BUTTON3;
		//*)

	private:
        void SaveDDPFields();

		//(*Handlers(DDPDialog)
		void OnTextCtrlIPAddressText(wxCommandEvent& event);
		void OnButtonOkClick(wxCommandEvent& event);
		void OnButtonCancelClick(wxCommandEvent& event);
		void OnButton1Click(wxCommandEvent& event);
		void OnVisualizeButtonClick(wxCommandEvent& event);
		void OnControllerTypeChoiceSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
