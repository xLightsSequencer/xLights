#ifndef EventOSCPANEL_H
#define EventOSCPANEL_H

#include "EventPanel.h"

//(*Headers(EventOSCPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/panel.h>
#include <wx/choice.h>
//*)

class EventOSCPanel: public EventPanel
{
	public:

		EventOSCPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EventOSCPanel();
        virtual bool ValidateWindow() override;
        virtual void Save(EventBase* event) override;
        virtual void Load(EventBase* event) override;

		//(*Declarations(EventOSCPanel)
		wxTextCtrl* TextCtrl_Path;
		wxStaticText* StaticText2;
		wxChoice* Choice_Parm1Type;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxChoice* Choice_Parm3Type;
		wxChoice* Choice_Parm2Type;
		wxStaticText* StaticText4;
		//*)

	protected:

		//(*Identifiers(EventOSCPanel)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT3;
		static const long ID_CHOICE2;
		static const long ID_STATICTEXT4;
		static const long ID_CHOICE3;
		//*)

	private:

		//(*Handlers(EventOSCPanel)
		void OnTextCtrl_PathText(wxCommandEvent& event);
		void OnChoice_Parm1TypeSelect(wxCommandEvent& event);
		void OnChoice_Parm2TypeSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
