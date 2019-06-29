#ifndef EventMQTTPANEL_H
#define EventMQTTPANEL_H

#include "EventPanel.h"

//(*Headers(EventMQTTPanel)
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class EventMQTTPanel: public EventPanel
{
	public:

		EventMQTTPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EventMQTTPanel();
        virtual bool ValidateWindow() override;
        virtual void Save(EventBase* event) override;
        virtual void Load(EventBase* event) override;

		//(*Declarations(EventMQTTPanel)
		wxSpinCtrl* SpinCtrl_Port;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxTextCtrl* TextCtrl_IP;
		wxTextCtrl* TextCtrl_Password;
		wxTextCtrl* TextCtrl_Path;
		wxTextCtrl* TextCtrl_Username;
		//*)

	protected:

		//(*Identifiers(EventMQTTPanel)
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT4;
		static const long ID_TEXTCTRL3;
		static const long ID_STATICTEXT5;
		static const long ID_TEXTCTRL4;
		//*)

	private:

		//(*Handlers(EventMQTTPanel)
		void OnTextCtrl_PathText(wxCommandEvent& event);
		void OnTextCtrl_IPText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
