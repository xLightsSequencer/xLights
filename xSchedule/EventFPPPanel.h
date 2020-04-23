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

//(*Headers(EventFPPPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/panel.h>
//*)

class EventFPPPanel: public EventPanel
{
	public:

		EventFPPPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EventFPPPanel();
        virtual bool ValidateWindow() override { return true; }
        virtual void Save(EventBase* event) override;
        virtual void Load(EventBase* event) override;

		//(*Declarations(EventFPPPanel)
		wxStaticText* StaticText2;
		wxSpinCtrl* SpinCtrl_Minor;
		wxStaticText* StaticText1;
		wxSpinCtrl* SpinCtrl_Major;
		//*)

	protected:

		//(*Identifiers(EventFPPPanel)
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL2;
		//*)

	private:

		//(*Handlers(EventFPPPanel)
		//*)

		DECLARE_EVENT_TABLE()
};
