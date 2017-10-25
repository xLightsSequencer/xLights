#ifndef ADDREVERSEDIALOG_H
#define ADDREVERSEDIALOG_H

//(*Headers(AddReverseDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class AddReverseDialog: public wxDialog
{
    size_t& _startChannel;
    size_t& _nodes;
    std::string& _description;
    bool& _enabled;

	public:

		AddReverseDialog(wxWindow* parent, size_t& startChannel, size_t& nodes, size_t& ignore, std::string& description, bool& enabled, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~AddReverseDialog();

		//(*Declarations(AddReverseDialog)
		wxCheckBox* CheckBox_Enabled;
		wxButton* Button_Ok;
		wxTextCtrl* TextCtrl_Description;
		wxStaticText* StaticText2;
		wxSpinCtrl* SpinCtrl_Nodes;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxButton* Button_Cancel;
		wxSpinCtrl* SpinCtrl_StartChannel;
		//*)

	protected:

		//(*Identifiers(AddReverseDialog)
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL1;
		static const long ID_CHECKBOX1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(AddReverseDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
