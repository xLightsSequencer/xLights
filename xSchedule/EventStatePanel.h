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

#include "EventPanel.h"

//(*Headers(EventStatePanel)
#include <wx/choice.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
//*)

class OutputManager;

class EventStatePanel: public EventPanel
{
	public:

		EventStatePanel(wxWindow* parent, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EventStatePanel();
        virtual bool ValidateWindow() override { return true; }
        virtual void Save(EventBase* event) override;
        virtual void Load(EventBase* event) override;

		//(*Declarations(EventStatePanel)
		wxChoice* Choice_States;
		wxStaticText* StaticText2;
		//*)

	protected:

		//(*Identifiers(EventStatePanel)
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE1;
		//*)

	private:

		//(*Handlers(EventStatePanel)
		//*)

		DECLARE_EVENT_TABLE()
};
