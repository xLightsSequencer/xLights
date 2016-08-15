#ifndef SEVENSEGMENTDIALOG_H
#define SEVENSEGMENTDIALOG_H

//(*Headers(SevenSegmentDialog)
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/statbmp.h>
#include <wx/dialog.h>
//*)

class SevenSegmentDialog: public wxDialog
{
	public:

		SevenSegmentDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~SevenSegmentDialog();

		//(*Declarations(SevenSegmentDialog)
		wxCheckBox* CheckBox_Thousands;
		wxStaticBitmap* StaticBitmap1;
		wxCheckBox* CheckBox_Hundreds;
		wxCheckBox* CheckBox_Tens;
		wxCheckBox* CheckBox_Ones;
		wxCheckBox* CheckBox_Decimal;
		wxCheckBox* CheckBox_Colon;
		//*)

	protected:

		//(*Identifiers(SevenSegmentDialog)
		static const long ID_CHECKBOX1;
		static const long ID_CHECKBOX2;
		static const long ID_CHECKBOX4;
		static const long ID_CHECKBOX3;
		static const long ID_CHECKBOX5;
		static const long ID_CHECKBOX6;
		static const long ID_STATICBITMAP1;
		//*)

	private:

		//(*Handlers(SevenSegmentDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
