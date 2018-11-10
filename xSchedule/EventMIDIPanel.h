#ifndef EVENTMIDIPANEL_H
#define EVENTMIDIPANEL_H

#include "EventPanel.h"

//(*Headers(EventMIDIPanel)
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
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
		wxButton* Button_Scan;
		wxChoice* Choice_Channel;
		wxChoice* Choice_Devices;
		wxChoice* Choice_Status;
		wxChoice* Choice_TestData1;
		wxChoice* Choice_TestData2;
		wxSpinCtrl* SpinCtrl_Data1;
		wxSpinCtrl* SpinCtrl_Data2;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
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
		static const long ID_CHOICE6;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT5;
		static const long ID_CHOICE7;
		static const long ID_SPINCTRL2;
		static const long ID_BUTTON1;
		//*)

	private:

		//(*Handlers(EventMIDIPanel)
		void OnButton_ScanClick(wxCommandEvent& event);
		void OnChoice_TestData1Select(wxCommandEvent& event);
		void OnChoice_TestData2Select(wxCommandEvent& event);
		//*)

        void OnMIDIEvent(wxCommandEvent& event);

		DECLARE_EVENT_TABLE()
};

#endif
