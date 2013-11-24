#ifndef EXPORTMODELSELECT_H
#define EXPORTMODELSELECT_H

//(*Headers(ExportModelSelect)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/dialog.h>
//*)

class ExportModelSelect: public wxDialog
{
	public:

		ExportModelSelect(wxWindow* parent,wxWindowID id=wxID_ANY);
		virtual ~ExportModelSelect();

		//(*Declarations(ExportModelSelect)
		wxChoice* ModelChoice;
		wxStaticText* ChoiceLabel;
		//*)

	protected:

		//(*Identifiers(ExportModelSelect)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		//*)

	private:

		//(*Handlers(ExportModelSelect)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
