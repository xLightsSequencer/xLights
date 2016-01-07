#ifndef EFFECTICONPANEL_H
#define EFFECTICONPANEL_H

//(*Headers(EffectIconPanel)
#include <wx/sizer.h>
#include <wx/panel.h>
//*)

#include "DragEffectBitmapButton.h"
class EffectManager;

class EffectIconPanel: public wxPanel
{
	public:

		EffectIconPanel(const EffectManager &manager,
                        wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EffectIconPanel();

		//(*Declarations(EffectIconPanel)
		//*)

	protected:

		//(*Identifiers(EffectIconPanel)
		//*)

	private:

		//(*Handlers(EffectIconPanel)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
