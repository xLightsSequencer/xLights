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

//(*Headers(EffectIconPanel)
#include <wx/panel.h>
#include <wx/sizer.h>
//*)

#include "sequencer/DragEffectBitmapButton.h"
class EffectManager;

class EffectIconPanel: public wxPanel
{
	public:

		EffectIconPanel(const EffectManager &manager,
                        wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EffectIconPanel();

		//(*Declarations(EffectIconPanel)
		wxGridSizer* GridSizer1;
		//*)

	protected:

		//(*Identifiers(EffectIconPanel)
		//*)

	private:
        const EffectManager &manager;
		//(*Handlers(EffectIconPanel)
		void OnResize(wxSizeEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
