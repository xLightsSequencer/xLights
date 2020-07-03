#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "sequencer/DragEffectBitmapButton.h"

//(*Headers(TopEffectsPanel)
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/panel.h>
#include <wx/sizer.h>
//*)

wxDECLARE_EVENT(EVT_WINDOW_RESIZED, wxCommandEvent);
wxDECLARE_EVENT(EVT_EFFECT_UPDATED, wxCommandEvent);
wxDECLARE_EVENT(EVT_EFFECT_RANDOMIZE, wxCommandEvent);


class TopEffectsPanel: public wxPanel
{
	public:

		TopEffectsPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~TopEffectsPanel();

		void SetDragIconBuffer(const wxBitmap &buffer);

		//(*Declarations(TopEffectsPanel)
		DragEffectBitmapButton* BitmapButtonSelectedEffect;
		wxBitmapButton* BitmapButtonRandomize;
		wxButton* ButtonUpdateEffect;
		wxFlexGridSizer* EffectSizer;
		wxFlexGridSizer* MainSizer;
		//*)


	protected:

		//(*Identifiers(TopEffectsPanel)
		static const long ID_BITMAPBUTTON_RANDOMIZE;
		static const long ID_BUTTON_UpdateEffect;
		static const long ID_BITMAPBUTTON_SelectedEffect;
		//*)

	private:

		//(*Handlers(TopEffectsPanel)
		void OnResize(wxSizeEvent& event);
		void OnBitmapButtonSelectedEffectClick(wxCommandEvent& event);
		void OnLeftDown(wxMouseEvent& event);
		void OnButtonUpdateEffectClick(wxCommandEvent& event);
		void OnButtonRandomizeEffectClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
