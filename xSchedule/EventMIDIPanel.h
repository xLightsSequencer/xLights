#ifndef EVENTMIDIPANEL_H
#define EVENTMIDIPANEL_H

#include "EventPanel.h"

//(*Headers(EventMIDIPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/panel.h>
#include <wx/choice.h>
//*)

class EventMIDIPanel: public EventPanel
{
	public:

		EventMIDIPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EventMIDIPanel();
        virtual bool ValidateWindow() override;
        virtual void Save(EventBase* event) override;
        virtual void Load(EventBase* event) override;

		//(*Declarations(EventMIDIPanel)
		wxChoice* Choice_Status;
		wxStaticText* StaticText2;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxChoice* Choice_Data1;
		wxChoice* Choice_Channel;
		wxChoice* Choice_Devices;
		wxStaticText* StaticText4;
		//*)

	protected:

		//(*Identifiers(EventMIDIPanel)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE2;
		static const long ID_STATICTEXT4;
		static const long ID_CHOICE4;
		static const long ID_STATICTEXT3;
		static const long ID_CHOICE3;
		//*)

	private:

		//(*Handlers(EventMIDIPanel)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
