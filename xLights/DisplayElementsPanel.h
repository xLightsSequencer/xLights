#ifndef DISPLAYELEMENTSPANEL_H
#define DISPLAYELEMENTSPANEL_H

//(*Headers(DisplayElementsPanel)
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/panel.h>
//*)

#include "wxCheckedListCtrl.h"
#include "sequencer/SequenceElements.h"

class DisplayElementsPanel: public wxPanel
{
	public:

		DisplayElementsPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~DisplayElementsPanel();

		void SetSequenceElements(SequenceElements* elements);
        void Initialize();
		//(*Declarations(DisplayElementsPanel)
		wxCheckedListCtrl* ListCtrlDisplayElements;
		//*)

	protected:

		//(*Identifiers(DisplayElementsPanel)
		static const long ID_LISTCTRL_DISPLAY_ELEMENTS;
		//*)

	private:

        SequenceElements* mSequenceElements;

		//(*Handlers(DisplayElementsPanel)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
