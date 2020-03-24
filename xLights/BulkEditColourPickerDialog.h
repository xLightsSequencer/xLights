#ifndef BULKEDITCOLOURPICKERDIALOG_H
#define BULKEDITCOLOURPICKERDIALOG_H

//(*Headers(BulkEditColourPickerDialog)
#include <wx/button.h>
#include <wx/clrpicker.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
//*)

class BulkEditColourPickerDialog: public wxDialog
{
	public:

		BulkEditColourPickerDialog(wxWindow* parent, const std::string& label, wxColour value = *wxBLACK, wxWindowID id = wxID_ANY);
		virtual ~BulkEditColourPickerDialog();
		wxColour GetValue() const;

		//(*Declarations(BulkEditColourPickerDialog)
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxColourPickerCtrl* ColourPickerCtrl1;
		wxStaticText* StaticText1;
		//*)

	protected:

		//(*Identifiers(BulkEditColourPickerDialog)
		static const long ID_STATICTEXT1;
		static const long ID_COLOURPICKERCTRL1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(BulkEditColourPickerDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
