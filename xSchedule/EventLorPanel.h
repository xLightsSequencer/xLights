#ifndef EVENTLORPANEL_H
#define EVENTLORPANEL_H

#include "EventPanel.h"

//(*Headers(EventLorPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/panel.h>
#include <wx/choice.h>
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
		wxChoice* Choice_UnitId;
		wxChoice* Choice_Test;
		wxSpinCtrl* SpinCtrl_Channel;
		wxStaticText* StaticText2;
		wxStaticText* StaticText6;
		wxChoice* Choice_Speed;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxChoice* Choice_CommPort;
		wxStaticText* StaticText5;
		wxStaticText* StaticText7;
		wxSpinCtrl* SpinCtrl_Value;
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

#endif
