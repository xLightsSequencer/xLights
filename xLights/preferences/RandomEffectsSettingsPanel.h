#ifndef RANDOMEFFECTSSETTINGSPANEL_H
#define RANDOMEFFECTSSETTINGSPANEL_H

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

#endif
