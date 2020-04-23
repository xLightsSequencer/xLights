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

//(*Headers(EventPingPanel)
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
//*)

class OutputManager;

class EventPingPanel: public EventPanel
{
	public:

		EventPingPanel(wxWindow* parent, OutputManager* outputManager, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EventPingPanel();
        virtual bool ValidateWindow() override { return true; }
        virtual void Save(EventBase* event) override;
        virtual void Load(EventBase* event) override;

		//(*Declarations(EventPingPanel)
		wxCheckBox* CheckBox_FireOnce;
		wxChoice* Choice_IPs;
		wxSpinCtrl* SpinCtrl_PingFailures;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		//*)

	protected:

		//(*Identifiers(EventPingPanel)
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL1;
		static const long ID_CHECKBOX1;
		//*)

	private:

		//(*Handlers(EventPingPanel)
		//*)

		DECLARE_EVENT_TABLE()
};
