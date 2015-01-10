#ifndef TOPEFFECTSPANEL_H
#define TOPEFFECTSPANEL_H


//(*Headers(TopEffectsPanel)
#include <wx/sizer.h>
#include <wx/panel.h>
//*)

wxDECLARE_EVENT(EVT_WINDOW_RESIZED, wxCommandEvent);

class TopEffectsPanel: public wxPanel
{
	public:

		TopEffectsPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~TopEffectsPanel();

		//(*Declarations(TopEffectsPanel)
		//*)

	protected:

		//(*Identifiers(TopEffectsPanel)
		//*)

	private:

		//(*Handlers(TopEffectsPanel)
		void OnResize(wxSizeEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
