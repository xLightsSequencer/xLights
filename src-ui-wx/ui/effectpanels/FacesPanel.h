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

#include "JsonEffectPanel.h"

class Model;
class wxRadioButton;
class wxChoice;
class wxCheckBox;

class FacesPanel : public JsonEffectPanel {
public:
    FacesPanel(wxWindow* parent, const nlohmann::json& metadata);
    ~FacesPanel() override = default;

    void SetPanelStatus(Model* cls) override;
    void SetDefaultParameters() override;
    void ValidateWindow() override;

protected:
    wxWindow* CreateCustomControl(wxWindow* parentWin, wxSizer* sizer,
                                   const nlohmann::json& prop, int cols) override;

private:
    wxWindow* BuildMouthMovements(wxWindow* parentWin, wxSizer* sizer);
    wxWindow* BuildTransparentBlackRow(wxWindow* parentWin, wxSizer* sizer);
    void PopulateLyricTimingTracks();
    void UpdateMouthMovementEnable();

    // Cached pointers to custom-created controls (not stored in properties_).
    wxRadioButton* _radioPhoneme = nullptr;
    wxRadioButton* _radioTiming = nullptr;
    wxChoice* _phonemeChoice = nullptr;
    wxChoice* _timingTrackChoice = nullptr;
    wxCheckBox* _transparentBlackCheck = nullptr;
};
