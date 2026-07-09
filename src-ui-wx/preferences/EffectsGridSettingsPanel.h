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
class wxChoice;
class wxCommandEvent;
class xLightsFrame;

class EffectsGridSettingsPanel: public wxPanel
{
	public:

		EffectsGridSettingsPanel(wxWindow* parent,xLightsFrame *f,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EffectsGridSettingsPanel();

        virtual bool TransferDataFromWindow() override;
        virtual bool TransferDataToWindow() override;

	private:
        xLightsFrame *frame;

        wxCheckBox* BellOnRenderCompletion = nullptr;
        wxCheckBox* ColorUpdateWarnCheckBox = nullptr;
        wxCheckBox* GroupEffectIndicator = nullptr;
        wxCheckBox* IconBackgroundsCheckBox = nullptr;
        wxCheckBox* NodeValuesCheckBox = nullptr;
        wxCheckBox* ShowAlternateTimingFormatCheckBox = nullptr;
        wxCheckBox* SmallWaveformCheckBox = nullptr;
        wxCheckBox* SnapToTimingCheckBox = nullptr;
        wxCheckBox* TransistionMarksCheckBox = nullptr;
        wxChoice* DoubleClickChoice = nullptr;
        wxChoice* GridSpacingChoice = nullptr;
        wxChoice* PasteAsChoice = nullptr;

        void OnChanged(wxCommandEvent& event);
};
