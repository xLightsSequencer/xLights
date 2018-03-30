#ifndef EVENTSERIALPANEL_H
#define EVENTSERIALPANEL_H

#include "EventPanel.h"

//(*Headers(EventSerialPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/panel.h>
#include <wx/choice.h>
//*)

class EventSerialPanel: public EventPanel
{
	public:

		EventSerialPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EventSerialPanel();
        virtual bool ValidateWindow() override;
        virtual void Save(EventBase* event) override;
        virtual void Load(EventBase* event) override;

		//(*Declarations(EventSerialPanel)
		wxChoice* Choice_Test;
		wxSpinCtrl* SpinCtrl_Channel;
		wxChoice* Choice_Protocol;
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

		//(*Identifiers(EventSerialPanel)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
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

		//(*Handlers(EventSerialPanel)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
