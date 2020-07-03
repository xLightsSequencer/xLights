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

//(*Headers(EventLorPanel)
#include <wx/choice.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
//*)

class EventLorPanel: public EventPanel
{
	public:

		EventLorPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EventLorPanel();
        virtual bool ValidateWindow() override;
        virtual void Save(EventBase* event) override;
        virtual void Load(EventBase* event) override;

		//(*Declarations(EventLorPanel)
		wxChoice* Choice_CommPort;
		wxChoice* Choice_Speed;
		wxChoice* Choice_Test;
		wxChoice* Choice_UnitId;
		wxSpinCtrl* SpinCtrl_Channel;
		wxSpinCtrl* SpinCtrl_Value;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		//*)

	protected:

		//(*Identifiers(EventLorPanel)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE_UNITID;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE2;
		static const long ID_STATICTEXT3;
		static const long ID_CHOICE3;
		static const long ID_STATICTEXT5;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT6;
		static const long ID_CHOICE4;
		static const long ID_STATICTEXT7;
		static const long ID_SPINCTRL2;
		//*)

	private:

		//(*Handlers(EventLorPanel)
		//*)

		DECLARE_EVENT_TABLE()
};

