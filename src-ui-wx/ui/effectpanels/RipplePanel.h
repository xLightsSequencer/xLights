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

class MediaPickerCtrl;

class RipplePanel : public JsonEffectPanel {
public:
    RipplePanel(wxWindow* parent, const nlohmann::json& metadata);
    void ValidateWindow() override;
    wxWindow* CreateCustomControl(wxWindow* parentWin, wxSizer* sizer, const nlohmann::json& prop, int cols) override;

private:
    MediaPickerCtrl* _svgPicker = nullptr;
};
