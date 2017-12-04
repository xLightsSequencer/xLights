#ifndef REMAPDIALOG_H
#define REMAPDIALOG_H

//(*Headers(RemapDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class OutputManager;

class RemapDialog: public wxDialog
{
    std::string& _from;
    std::string& _to;
    size_t& _channels;
    std::string& _description;
    bool& _enabled;
    OutputManager* _outputManager;

    void ValidateWindow();

    public:

		RemapDialog(wxWindow* parent, OutputManager* outputManager, std::string& startChannel, std::string& to, size_t& channels, std::string& description, bool& enabled, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~RemapDialog();

		//(*Declarations(RemapDialog)
		wxCheckBox* CheckBox_Enabled;
		wxButton* Button_Ok;
		wxTextCtrl* TextCtrl_Description;
		wxSpinCtrl* SpinCtrl_Channels;
		wxStaticText* StaticText2;
		wxStaticText* StaticText_From;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxTextCtrl* TextCtrl_FromChannel;
		wxButton* Button_Cancel;
		wxStaticText* StaticText4;
		wxTextCtrl* TextCtrl_ToChannel;
		wxStaticText* StaticText_To;
		//*)

	protected:

		//(*Identifiers(RemapDialog)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT5;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL3;
		static const long ID_STATICTEXT6;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL3;
		static const long ID_STATICTEXT4;
		static const long ID_TEXTCTRL1;
		static const long ID_CHECKBOX1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(RemapDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnTextCtrl_FromChannelText(wxCommandEvent& event);
		void OnTextCtrl_ToChannelText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
