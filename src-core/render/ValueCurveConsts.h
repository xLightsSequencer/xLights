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

// Value curve min/max/divisor constants for buffer, color, and timing panels.
// These are used by RenderableEffect::UpgradeValueCurve() and by the
// corresponding UI panels. Keeping them here avoids core (effects/, render/)
// needing to include ui/ headers.

#include <string>

// --- Buffer constants ---
constexpr int BLUR_MIN = 1;
constexpr int BLUR_MAX = 15;

constexpr int RZ_ROTATION_MIN = 0;
constexpr int RZ_ROTATION_MAX = 100;

constexpr int RZ_ZOOM_MIN = 0;
constexpr int RZ_ZOOM_MAX = 30;
constexpr int RZ_ZOOM_DIVISOR = 10;

constexpr int RZ_ROTATIONS_MIN = 0;
constexpr int RZ_ROTATIONS_MAX = 200;
constexpr int RZ_ROTATIONS_DIVISOR = 10;

constexpr int RZ_PIVOTX_MIN = 0;
constexpr int RZ_PIVOTX_MAX = 100;

constexpr int RZ_PIVOTY_MIN = 0;
constexpr int RZ_PIVOTY_MAX = 100;

constexpr int RZ_XROTATION_MIN = 0;
constexpr int RZ_XROTATION_MAX = 360;

constexpr int RZ_YROTATION_MIN = 0;
constexpr int RZ_YROTATION_MAX = 360;

constexpr int RZ_XPIVOT_MIN = 0;
constexpr int RZ_XPIVOT_MAX = 100;

constexpr int RZ_YPIVOT_MIN = 0;
constexpr int RZ_YPIVOT_MAX = 100;

// --- Sub-buffer constants ---
constexpr int SB_LEFT_BOTTOM_MIN = -100;
constexpr int SB_LEFT_BOTTOM_MAX = 99;

constexpr int SB_RIGHT_TOP_MIN = 1;
constexpr int SB_RIGHT_TOP_MAX = 200;

constexpr int SB_CENTRE_MIN = -100;
constexpr int SB_CENTRE_MAX = 100;

// --- Color constants ---
constexpr int COLORPANEL_BRIGHTNESS_MIN = 0;
constexpr int COLORPANEL_BRIGHTNESS_MAX = 400;

constexpr int COLORPANEL_SPARKLE_MIN = 0;
constexpr int COLORPANEL_SPARKLE_MAX = 200;

constexpr int COLORPANEL_VALUE_MIN = -100;
constexpr int COLORPANEL_VALUE_MAX = 100;

constexpr int COLORPANEL_HUE_MIN = -100;
constexpr int COLORPANEL_HUE_MAX = 100;

constexpr int COLORPANEL_SATURATION_MIN = -100;
constexpr int COLORPANEL_SATURATION_MAX = 100;

// --- Timing constants ---
constexpr int IN_TRANSITION_MIN = 0;
constexpr int IN_TRANSITION_MAX = 100;

constexpr int OUT_TRANSITION_MIN = 0;
constexpr int OUT_TRANSITION_MAX = 100;

// --- Lookup functions ---
// These replace the static methods formerly on BufferPanel, ColorPanel, BlendingPanel.

namespace ValueCurveConsts {

    double GetBufferSettingVCMin(const std::string& name);
    double GetBufferSettingVCMax(const std::string& name);
    int GetBufferSettingVCDivisor(const std::string& name);

    double GetColorSettingVCMin(const std::string& name);
    double GetColorSettingVCMax(const std::string& name);
    int GetColorSettingVCDivisor(const std::string& name);

    double GetTimingSettingVCMin(const std::string& name);
    double GetTimingSettingVCMax(const std::string& name);
    int GetTimingSettingVCDivisor(const std::string& name);

} // namespace ValueCurveConsts
