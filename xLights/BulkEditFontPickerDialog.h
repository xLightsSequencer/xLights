#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

 //(*Headers(BulkEditFontPickerDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/fontpicker.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
//*)

class BulkEditFontPickerDialog: public wxDialog
{
	public:

		BulkEditFontPickerDialog(wxWindow* parent, const std::string& label, const std::string& value, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~BulkEditFontPickerDialog();
        std::string GetValue() const;

		//(*Declarations(BulkEditFontPickerDialog)
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxFontPickerCtrl* FontPickerCtrl1;
		wxStaticText* StaticText_Label;
		//*)

	protected:

		//(*Identifiers(BulkEditFontPickerDialog)
		static const long ID_STATICTEXT_BulkEdit;
		static const long ID_FONTPICKERCTRL1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(BulkEditFontPickerDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
