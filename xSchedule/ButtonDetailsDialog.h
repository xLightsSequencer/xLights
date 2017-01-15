#ifndef BUTTONDETAILSDIALOG_H
#define BUTTONDETAILSDIALOG_H

//(*Headers(ButtonDetailsDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class ButtonDetailsDialog: public wxDialog
{
    void ValidateWindow();

	public:

        std::string& _label;
        std::string& _parameter;
        std::string& _command;
        char& _hotkey;

		ButtonDetailsDialog(wxWindow* parent, std::string& label, std::string& command, std::string& parameter, char& hotkey, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ButtonDetailsDialog();

		//(*Declarations(ButtonDetailsDialog)
		wxChoice* Choice_Command;
		wxButton* Button_Ok;
		wxStaticText* StaticText2;
		wxChoice* Choice_Hotkey;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxTextCtrl* TextCtrl_Parameters;
		wxButton* Button_Cancel;
		wxTextCtrl* TextCtrl_Label;
		wxStaticText* StaticText4;
		//*)

	protected:

		//(*Identifiers(ButtonDetailsDialog)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT4;
		static const long ID_CHOICE2;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(ButtonDetailsDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnTextCtrl_ParametersText(wxCommandEvent& event);
		void OnTextCtrl_LabelText(wxCommandEvent& event);
		void OnChoice_CommandSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
