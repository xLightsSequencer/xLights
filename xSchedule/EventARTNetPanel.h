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

//(*Headers(EventARTNetPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/panel.h>
#include <wx/choice.h>
//*)

class EventARTNetPanel: public EventPanel
{
	public:

		EventARTNetPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EventARTNetPanel();
        virtual bool ValidateWindow() override;
        virtual void Save(EventBase* event) override;
        virtual void Load(EventBase* event) override;

		//(*Declarations(EventARTNetPanel)
		wxChoice* Choice_Test;
		wxSpinCtrl* SpinCtrl_Channel;
		wxStaticText* StaticText2;
		wxSpinCtrl* SpinCtrl_Universe;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxSpinCtrl* SpinCtrl_Value;
		wxStaticText* StaticText4;
		//*)

	protected:

		//(*Identifiers(EventARTNetPanel)
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT4;
		static const long ID_CHOICE2;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL3;
		//*)

	private:

		//(*Handlers(EventARTNetPanel)
		//*)

		DECLARE_EVENT_TABLE()
};
