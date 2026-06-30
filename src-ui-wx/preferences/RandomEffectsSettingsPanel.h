#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

//(*Headers(RandomEffectsSettingsPanel)
#include <wx/panel.h>
//*)

class wxListBox;
class wxCommandEvent;
class xLightsFrame;

class RandomEffectsSettingsPanel: public wxPanel
{
	public:

		RandomEffectsSettingsPanel(wxWindow* parent, xLightsFrame *f, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~RandomEffectsSettingsPanel();

		//(*Declarations(RandomEffectsSettingsPanel)
		//*)

        virtual bool TransferDataFromWindow() override;
        virtual bool TransferDataToWindow() override;

	protected:

		//(*Identifiers(RandomEffectsSettingsPanel)
		//*)

	private:
        xLightsFrame* frame;
        wxListBox* _availableList = nullptr; // effects NOT used by Generate Random
        wxListBox* _usedList = nullptr;      // effects used by Generate Random

        // Move the selected rows from one list to the other (both kept sorted).
        void MoveSelected(wxListBox* from, wxListBox* to);
        // Mirror the original checkbox behaviour: write changes back immediately
        // on platforms where the preferences editor applies as-you-go.
        void ApplyIfImmediate();

		//(*Handlers(RandomEffectsSettingsPanel)
		//*)
        void OnAdd(wxCommandEvent& event);
        void OnRemove(wxCommandEvent& event);
        void OnAvailableDClick(wxCommandEvent& event);
        void OnUsedDClick(wxCommandEvent& event);

		DECLARE_EVENT_TABLE()
};
