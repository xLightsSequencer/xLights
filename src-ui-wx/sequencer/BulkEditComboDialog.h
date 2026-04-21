#ifndef BULKEDITCOMBODIALOG_H
#define BULKEDITCOMBODIALOG_H

//(*Headers(BulkEditComboDialog)
#include <wx/button.h>
#include <wx/combobox.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
//*)

class BulkEditComboDialog: public wxDialog
{
	public:

		BulkEditComboDialog(wxWindow* parent, const std::string& value, const std::string& label, const wxArrayString& choices, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
		virtual ~BulkEditComboDialog();
        std::string GetValue();

		//(*Declarations(BulkEditComboDialog)
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxComboBox* ComboBox_Combo;
		wxStaticText* StaticText_Combo;
		//*)

	protected:

		void ValidateWindow();

		//(*Identifiers(BulkEditComboDialog)
		static const long ID_STATICTEXT1;
		static const long ID_COMBOBOX1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(BulkEditComboDialog)
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		void OnComboBox_ComboTextUpdated(wxCommandEvent& event);
		void OnComboBox_ComboSelected(wxCommandEvent& event);
		void OnComboBox_ComboTextEnter(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
