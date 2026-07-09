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

#include <wx/panel.h>

class wxCheckBox;
class wxCommandEvent;
class xLightsFrame;

class CheckSequenceSettingsPanel: public wxPanel
{
	public:

		CheckSequenceSettingsPanel(wxWindow* parent, xLightsFrame *f, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~CheckSequenceSettingsPanel();

        virtual bool TransferDataFromWindow() override;
        virtual bool TransferDataToWindow() override;

	private:
        xLightsFrame *frame;

        wxCheckBox* CheckBox_CustomSizeCheck = nullptr;
        wxCheckBox* CheckBox_DisableSketch = nullptr;
        wxCheckBox* CheckBox_DupNodeMG = nullptr;
        wxCheckBox* CheckBox_DupUniv = nullptr;
        wxCheckBox* CheckBox_NonContigChOnPort = nullptr;
        wxCheckBox* CheckBox_PreviewGroup = nullptr;
        wxCheckBox* CheckBox_TransTime = nullptr;

        void OnChanged(wxCommandEvent& event);
};
