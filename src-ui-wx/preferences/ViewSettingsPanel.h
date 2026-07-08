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

class ViewSettingsPanel: public wxPanel
{
	public:

		ViewSettingsPanel(wxWindow* parent, xLightsFrame *f, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ViewSettingsPanel();

        virtual bool TransferDataFromWindow() override;
        virtual bool TransferDataToWindow() override;

	private:
        xLightsFrame *frame;

        wxCheckBox* CheckBox_BaseShowFolder = nullptr;
        wxCheckBox* CheckBox_DisableKeyAcceleration = nullptr;
        wxCheckBox* CheckBox_PresetPreview = nullptr;
        wxCheckBox* CheckBox_ZoomMethod = nullptr;
        wxCheckBox* HousePreviewCheckBox = nullptr;
        wxCheckBox* PlayControlsCheckBox = nullptr;
        wxChoice* Choice_PaletteSize = nullptr;
        wxChoice* Choice_TimelineZooming = nullptr;
        wxChoice* CrosshairSizeChoice = nullptr;
        wxChoice* EffectAssistChoice = nullptr;
        wxChoice* ModelHandleSizeChoice = nullptr;
        wxChoice* ToolIconSizeChoice = nullptr;

        void OnChanged(wxCommandEvent& event);
};
