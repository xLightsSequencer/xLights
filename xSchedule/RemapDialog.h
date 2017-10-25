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

class RemapDialog: public wxDialog
{
    size_t& _from;
    size_t& _to;
    size_t& _channels;
    std::string& _description;
    bool& _enabled;

	public:

		RemapDialog(wxWindow* parent, size_t& startChannel, size_t& to, size_t& channels, std::string& description, bool& enabled, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~RemapDialog();

		//(*Declarations(RemapDialog)
		wxCheckBox* CheckBox_Enabled;
		wxButton* Button_Ok;
		wxSpinCtrl* SpinCtrl_ToChannel;
		wxTextCtrl* TextCtrl_Description;
		wxSpinCtrl* SpinCtrl_Channels;
		wxStaticText* StaticText2;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxButton* Button_Cancel;
		wxStaticText* StaticText4;
		wxSpinCtrl* SpinCtrl_FromChannel;
		//*)

	protected:

		//(*Identifiers(RemapDialog)
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL2;
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
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
