#ifndef AUTOLABELDIALOG_H
#define AUTOLABELDIALOG_H

//(*Headers(AutoLabelDialog)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
//*)

class AutoLabelDialog: public wxDialog
{
	public:

		AutoLabelDialog(wxWindow* parent,wxWindowID id=wxID_ANY);
		virtual ~AutoLabelDialog();
        uint32_t GetStart() const;
        uint32_t GetEnd() const;
        bool IsOverwrite() const;

		//(*Declarations(AutoLabelDialog)
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxCheckBox* CheckBox_Overwrite;
		wxSpinCtrl* SpinCtrl_Start;
		wxSpinCtrl* SpinCtrl_end;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		//*)

	protected:

		//(*Identifiers(AutoLabelDialog)
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL2;
		static const long ID_CHECKBOX1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(AutoLabelDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
