#ifndef OFFPANEL_H
#define OFFPANEL_H

//(*Headers(OffPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxFlexGridSizer;
class wxStaticText;
//*)

#include "../BulkEditControls.h"

class OffPanel: public wxPanel
{
	public:

		OffPanel(wxWindow* parent);
		virtual ~OffPanel();

		//(*Declarations(OffPanel)
		BulkEditCheckBox* CheckBox_Transparent;
		//*)

	protected:

		//(*Identifiers(OffPanel)
		static const long ID_CHECKBOX_Off_Transparent;
		//*)

	private:

		//(*Handlers(OffPanel)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
