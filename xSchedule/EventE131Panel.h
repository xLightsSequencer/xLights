#ifndef EVENTE131PANEL_H
#define EVENTE131PANEL_H

#include "EventPanel.h"

//(*Headers(EventE131Panel)
#include <wx/choice.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
//*)

class EventE131Panel: public EventPanel
{
	public:

		EventE131Panel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EventE131Panel();
        virtual bool ValidateWindow() override;
        virtual void Save(EventBase* event) override;
        virtual void Load(EventBase* event) override;

		//(*Declarations(EventE131Panel)
		wxChoice* Choice_Test;
		wxSpinCtrl* SpinCtrl_Channel;
		wxSpinCtrl* SpinCtrl_Universe;
		wxSpinCtrl* SpinCtrl_Value;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		//*)

	protected:

		//(*Identifiers(EventE131Panel)
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

		//(*Handlers(EventE131Panel)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
