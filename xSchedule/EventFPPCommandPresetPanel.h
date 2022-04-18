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

//(*Headers(EventFPPCommandPresetPanel)
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class EventFPPCommandPresetPanel: public EventPanel
{
	public:

		EventFPPCommandPresetPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EventFPPCommandPresetPanel();
        virtual bool ValidateWindow() override { return true; }
        virtual void Save(EventBase* event) override;
        virtual void Load(EventBase* event) override;

		//(*Declarations(EventFPPCommandPresetPanel)
		wxStaticText* StaticText1;
		wxTextCtrl* TextCtrl_PresetName;
		//*)

	protected:

		//(*Identifiers(EventFPPCommandPresetPanel)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL_PRESET_NAME;
		//*)

	private:

		//(*Handlers(EventFPPCommandPresetPanel)
		//*)

		DECLARE_EVENT_TABLE()
};
