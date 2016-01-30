#ifndef OPTIONCHOOSER_H
#define OPTIONCHOOSER_H

//(*Headers(OptionChooser)
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/listbox.h>
//*)

class OptionChooser: public wxDialog
{
	public:

		OptionChooser(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~OptionChooser();

		void SetInstructionText(const wxString& text);
		void SetOptions(const wxArrayString& options);
		void GetSelectedOptions(wxArrayString& options);

		//(*Declarations(OptionChooser)
		wxStaticText* StaticText_Option_Select;
		wxListBox* ListBox_Options;
		//*)

	protected:

		//(*Identifiers(OptionChooser)
		static const long ID_STATICTEXT_Option_Select;
		static const long ID_LISTBOX_Options;
		//*)

	private:

		//(*Handlers(OptionChooser)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
