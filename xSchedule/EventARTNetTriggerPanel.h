#ifndef EventARTNetTriggerPanel_H
#define EventARTNetTriggerPanel_H

#include "EventPanel.h"

//(*Headers(EventARTNetTriggerPanel)
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
//*)

class EventARTNetTriggerPanel: public EventPanel
{
	public:

		EventARTNetTriggerPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EventARTNetTriggerPanel();
        virtual bool ValidateWindow() override;
        virtual void Save(EventBase* event) override;
        virtual void Load(EventBase* event) override;

		//(*Declarations(EventARTNetTriggerPanel)
		wxSpinCtrl* SpinCtrl_Key;
		wxSpinCtrl* SpinCtrl_OEM;
		wxSpinCtrl* SpinCtrl_SubKey;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		//*)

	protected:

		//(*Identifiers(EventARTNetTriggerPanel)
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL3;
		//*)

	private:

		//(*Handlers(EventARTNetTriggerPanel)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
