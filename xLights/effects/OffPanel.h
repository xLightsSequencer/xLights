#ifndef OFFPANEL_H
#define OFFPANEL_H

//(*Headers(OffPanel)
#include <wx/panel.h>
class wxFlexGridSizer;
class wxStaticText;
//*)

class OffPanel: public wxPanel
{
	public:

		OffPanel(wxWindow* parent);
		virtual ~OffPanel();

        virtual std::string GetEffectString() { return "";};

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
