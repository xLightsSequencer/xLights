#ifndef SUSTAINDIALOG_H
#define SUSTAINDIALOG_H

//(*Headers(SustainDialog)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class OutputManager;

class SustainDialog: public wxDialog
{
    std::string& _startChannel;
    size_t& _channels;
    std::string& _description;
    bool& _enabled;
    OutputManager* _outputManager;

    void ValidateWindow();

	public:

		SustainDialog(wxWindow* parent, OutputManager* outputManager, std::string& startChannel, size_t& channels, std::string& description, bool& enabled, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~SustainDialog();

		//(*Declarations(SustainDialog)
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxCheckBox* CheckBox_Enabled;
		wxSpinCtrl* SpinCtrl_Channels;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText4;
		wxStaticText* StaticText_StartChannel;
		wxTextCtrl* TextCtrl_Description;
		wxTextCtrl* TextCtrl_StartChannel;
		//*)

	protected:

		//(*Identifiers(SustainDialog)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT5;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT4;
		static const long ID_TEXTCTRL1;
		static const long ID_CHECKBOX1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(SustainDialog)
		void OnTextCtrl_StartChannelText(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
