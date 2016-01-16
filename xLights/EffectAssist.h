#ifndef EFFECTASSIST_H
#define EFFECTASSIST_H

//(*Headers(EffectAssist)
#include <wx/sizer.h>
#include <wx/panel.h>
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

	protected:

		//(*Identifiers(EffectAssist)
		//*)

        AssistPanel* mAssistPanel;

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
