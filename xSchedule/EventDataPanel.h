#ifndef EVENTDATAPANEL_H
#define EVENTDATAPANEL_H

#include "EventPanel.h"

//(*Headers(EventDataPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/panel.h>
#include <wx/choice.h>
//*)

class EventDataPanel: public EventPanel
{
	public:

		EventDataPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EventDataPanel();
        virtual bool ValidateWindow() override;
        virtual void Save(EventBase* event) override;
        virtual void Load(EventBase* event) override;

		//(*Declarations(EventDataPanel)
		wxChoice* Choice_Test;
		wxStaticText* StaticText2;
		wxTextCtrl* TextCtrl_Channel;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxSpinCtrl* SpinCtrl_Value;
		wxStaticText* StaticText4;
		//*)

	protected:

		//(*Identifiers(EventDataPanel)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT4;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE2;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL3;
		//*)

	private:

		//(*Handlers(EventDataPanel)
		void OnTextCtrl_ChannelText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
