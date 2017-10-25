#ifndef DEADCHANNELDIALOG_H
#define DEADCHANNELDIALOG_H

//(*Headers(DeadChannelDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class DeadChannelDialog: public wxDialog
{
    void ValidateWindow();

	public:
        size_t& _nodeStartChannel;
        size_t& _channel;
        std::string& _description;
        bool& _enabled;

		DeadChannelDialog(wxWindow* parent, size_t& nodeStartChannel, size_t& channel, std::string& description, bool& enabled, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~DeadChannelDialog();

		//(*Declarations(DeadChannelDialog)
		wxCheckBox* CheckBox_Enabled;
		wxButton* Button_Ok;
		wxTextCtrl* TextCtrl_Description;
		wxStaticText* StaticText2;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxChoice* Choice_Channel;
		wxButton* Button_Cancel;
		wxSpinCtrl* SpinCtrl_NodeStartChannel;
		//*)

	protected:

		//(*Identifiers(DeadChannelDialog)
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL1;
		static const long ID_CHECKBOX1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(DeadChannelDialog)
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
