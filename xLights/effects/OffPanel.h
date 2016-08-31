#ifndef OFFPANEL_H
#define OFFPANEL_H

//(*Headers(OffPanel)
#include <wx/panel.h>
class wxStaticText;
class wxFlexGridSizer;
//*)

class OffPanel: public wxPanel
{
	public:

		OffPanel(wxWindow* parent);
		virtual ~OffPanel();

		//(*Declarations(OffPanel)
		//*)

	protected:

		//(*Identifiers(OffPanel)
		//*)

	private:

		//(*Handlers(OffPanel)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
