#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

//(*Headers(OptionsDialog)
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class OptionsDialog: public wxDialog
{
	public:

		OptionsDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~OptionsDialog();

		//(*Declarations(OptionsDialog)
		wxButton* Button_Ok;
		wxCheckBox* CheckBox_RunOnMachineStartup;
		wxButton* Button_Cancel;
		wxCheckBox* CheckBox_StartOnLaunch;
		wxCheckBox* CheckBox_SendOffWhenNotRunning;
		//*)

	protected:

		//(*Identifiers(OptionsDialog)
		static const long ID_CHECKBOX1;
		static const long ID_CHECKBOX2;
		static const long ID_CHECKBOX3;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(OptionsDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
