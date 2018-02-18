#ifndef SEQUENCEVIDEOPANEL_H
#define SEQUENCEVIDEOPANEL_H

//(*Headers(SequenceVideoPanel)
#include <wx/panel.h>
//*)

class SequenceVideoPanel: public wxPanel
{
	public:

		SequenceVideoPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~SequenceVideoPanel();

		//(*Declarations(SequenceVideoPanel)
		//*)

	protected:

		//(*Identifiers(SequenceVideoPanel)
		//*)

	private:

		//(*Handlers(SequenceVideoPanel)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
