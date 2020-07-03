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

//(*Headers(RandomEffectsSettingsPanel)
#include <wx/panel.h>
class wxFlexGridSizer;
class wxStaticText;
//*)

class xLightsFrame;
class RandomEffectsSettingsPanel: public wxPanel
{
	public:

		RandomEffectsSettingsPanel(wxWindow* parent, xLightsFrame *f, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~RandomEffectsSettingsPanel();

		//(*Declarations(RandomEffectsSettingsPanel)
		wxFlexGridSizer* EffectsGridSizer;
		wxFlexGridSizer* MainSizer;
		//*)
    
        virtual bool TransferDataFromWindow() override;
        virtual bool TransferDataToWindow() override;

	protected:

		//(*Identifiers(RandomEffectsSettingsPanel)
		//*)

	private:
        xLightsFrame* frame;
    
		//(*Handlers(RandomEffectsSettingsPanel)
		void OnEffectCheckBoxClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
