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

class wxStaticText;

class wxFontPickerCtrl;
class wxSpinCtrl;
class wxChoice;

class ShapePanel : public JsonEffectPanel {
public:
    ShapePanel(wxWindow* parent, const nlohmann::json& metadata);
    void ValidateWindow() override;
    wxWindow* CreateCustomControl(wxWindow* parentWin, wxSizer* sizer, const nlohmann::json& prop, int cols) override;

private:
    wxStaticText* _emojiDisplay = nullptr;
    wxFontPickerCtrl* _fontPicker = nullptr;
    wxSpinCtrl* _charSpin = nullptr;
    wxChoice* _skinToneChoice = nullptr;
};
