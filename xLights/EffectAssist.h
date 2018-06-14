#ifndef EFFECTASSIST_H
#define EFFECTASSIST_H

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

#endif
