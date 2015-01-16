#ifndef TOPEFFECTSPANEL_H
#define TOPEFFECTSPANEL_H
#include "sequencer/DragEffectBitmapButton.h"


//(*Headers(TopEffectsPanel)
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
//*)

wxDECLARE_EVENT(EVT_WINDOW_RESIZED, wxCommandEvent);

class TopEffectsPanel: public wxPanel
{
	public:

		TopEffectsPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~TopEffectsPanel();

		void SetDragIconBuffer(const char** buffer);

		//(*Declarations(TopEffectsPanel)
		wxButton* Button1;
		DragEffectBitmapButton* BitmapButtonSelectedEffect;
		wxPanel* Panel_EffectContainer;
		//*)


	protected:

		//(*Identifiers(TopEffectsPanel)
		static const long ID_BUTTON1;
		static const long ID_BITMAPBUTTON_SelectedEffect;
		static const long ID_PANEL1;
		//*)

	private:

		//(*Handlers(TopEffectsPanel)
		void OnResize(wxSizeEvent& event);
		void OnBitmapButtonSelectedEffectClick(wxCommandEvent& event);
		void OnLeftDown(wxMouseEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
