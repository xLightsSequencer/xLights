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

//(*Headers(EffectAssist)
#include <wx/panel.h>
#include <wx/sizer.h>
//*)

#include "xlGridCanvas.h"
#include "effects/assist/AssistPanel.h"

class xLightsFrame;


class EffectAssist: public wxPanel
{
	public:

		EffectAssist(wxWindow* parent,wxWindowID id=wxID_ANY);
		virtual ~EffectAssist();

		//(*Declarations(EffectAssist)
		wxFlexGridSizer* FlexGridSizer1;
		//*)

        void SetPanel(AssistPanel* panel_);
        void ForceRefresh();

        AssistPanel * GetDefaultAssistPanel();
	protected:

		//(*Identifiers(EffectAssist)
		//*)

        AssistPanel* mAssistPanel;
        AssistPanel* defaultAssistPanel;

	private:
        void AdjustClientSizes(wxSize s);
        wxSize mSize;

		//(*Handlers(EffectAssist)
		void OnResize(wxSizeEvent& event);
		//*)

        xLightsFrame* mxLightsParent;

		DECLARE_EVENT_TABLE()

};
