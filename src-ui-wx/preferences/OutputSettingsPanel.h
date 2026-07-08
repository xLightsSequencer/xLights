#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/panel.h>

class wxCheckBox;
class wxChoice;
class wxCommandEvent;
class xLightsFrame;

class OutputSettingsPanel: public wxPanel
{
	public:

		OutputSettingsPanel(wxWindow* parent, xLightsFrame *f, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~OutputSettingsPanel();

        virtual bool TransferDataFromWindow() override;
        virtual bool TransferDataToWindow() override;

	private:
        xLightsFrame *frame;

        wxCheckBox* FrameSyncCheckBox = nullptr;
        wxChoice* DuplicateSuppressChoice = nullptr;
        wxChoice* ForceLocalIPChoice = nullptr;
        wxChoice* xFadexScheduleChoice = nullptr;

        void OnChanged(wxCommandEvent& event);
};
