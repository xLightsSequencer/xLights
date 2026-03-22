/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "DmxAbilityPropertyHelpers.h"
#include "../../../models/DMX/DmxColorAbility.h"
#include "../../../models/DMX/DmxShutterAbility.h"
#include "../../../models/DMX/DmxDimmerAbility.h"
#include "../../../models/DMX/DmxBeamAbility.h"
#include "../../../models/DMX/DmxPresetAbility.h"
#include "../../../models/DMX/DmxColorAbilityRGB.h"
#include "../../../models/DMX/DmxColorAbilityCMY.h"
#include "../../../models/DMX/DmxColorAbilityWheel.h"
#include "../../../OutputModelManager.h"
#include "../../wxUtilities.h"

namespace DmxAbilityPropertyHelpers {

// ---- Polymorphic Color Dispatch ----

void AddColorProperties(wxPropertyGridInterface* grid, DmxColorAbility* color, bool pwm) {
    if (!color) return;
    switch (color->GetColorType()) {
        case DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_RGBW:
            AddColorRGBProperties(grid, static_cast<DmxColorAbilityRGB&>(*color), pwm);
            break;
        case DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_CMYW:
            AddColorCMYProperties(grid, static_cast<DmxColorAbilityCMY&>(*color), pwm);
            break;
        case DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_WHEEL:
            AddColorWheelProperties(grid, static_cast<DmxColorAbilityWheel&>(*color), pwm);
            break;
        case DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_UNUSED:
            break;
    }
}

int OnColorPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, DmxColorAbility* color, BaseObject* base) {
    if (!color) return -1;
    switch (color->GetColorType()) {
        case DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_RGBW:
            return OnColorRGBPropertyGridChange(grid, event, static_cast<DmxColorAbilityRGB&>(*color), base);
        case DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_CMYW:
            return OnColorCMYPropertyGridChange(grid, event, static_cast<DmxColorAbilityCMY&>(*color), base);
        case DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_WHEEL:
            return OnColorWheelPropertyGridChange(grid, event, static_cast<DmxColorAbilityWheel&>(*color), base);
        case DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_UNUSED:
            break;
    }
    return -1;
}

// ---- Shutter ----

void AddShutterProperties(wxPropertyGridInterface* grid, const DmxShutterAbility& shutter) {
    grid->Append(new wxPropertyCategory("Shutter Properties", "DmxShutterProperties"));

    auto p = grid->Append(new wxUIntProperty("Shutter Channel", "DmxShutterChannel", shutter.GetShutterChannel()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxIntProperty("Shutter Open Threshold", "DmxShutterOpen", shutter.GetShutterThreshold()));
    p->SetAttribute("Min", -255);
    p->SetAttribute("Max", 255);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxIntProperty("Shutter On Value", "DmxShutterOnValue", shutter.GetShutterOnValue()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 255);
    p->SetEditor("SpinCtrl");
}

int OnShutterPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, DmxShutterAbility& shutter, BaseObject* base) {
    if ("DmxShutterChannel" == event.GetPropertyName()) {
        shutter.SetShutterChannel((int)event.GetPropertyValue().GetLong());
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "DmxShutterAbility::OnPropertyGridChange::DMXShutterChannel");
        return 0;
    } else if ("DmxShutterOpen" == event.GetPropertyName()) {
        shutter.SetShutterThreshold((int)event.GetPropertyValue().GetLong());
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "DmxShutterAbility::OnPropertyGridChange::DMXShutterOpen");
        return 0;
    } else if ("DmxShutterOnValue" == event.GetPropertyName()) {
        shutter.SetShutterOnValue((int)event.GetPropertyValue().GetLong());
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "DmxShutterAbility::OnPropertyGridChange::DmxShutterOnValue");
        return 0;
    }
    return -1;
}

// ---- Dimmer ----

void AddDimmerProperties(wxPropertyGridInterface* grid, const DmxDimmerAbility& dimmer) {
    grid->Append(new wxPropertyCategory("Dimmer Properties", "DmxDimmerProperties"));

    auto p = grid->Append(new wxUIntProperty("Dimmer Channel", "MhDimmerChannel", dimmer.GetDimmerChannel()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");
}

int OnDimmerPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, DmxDimmerAbility& dimmer, BaseObject* base) {
    if ("MhDimmerChannel" == event.GetPropertyName()) {
        dimmer.SetDimmerChannel((int)event.GetPropertyValue().GetLong());
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "DmxDimmerAbility::OnPropertyGridChange::DMXDimmerChannel");
        return 0;
    }
    return -1;
}

// ---- Beam ----

void AddBeamProperties(wxPropertyGridInterface* grid, const DmxBeamAbility& beam) {
    grid->Append(new wxPropertyCategory("Beam Properties", "DmxBeamProperties"));

    auto p = grid->Append(new wxFloatProperty("Beam Display Length", "DmxBeamLength", beam.GetBeamLength()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 100);
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxFloatProperty("Beam Display Width", "DmxBeamWidth", beam.GetBeamWidth()));
    p->SetAttribute("Min", 0.01);
    p->SetAttribute("Max", 150);
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");

    if (beam.SupportsOrient()) {
        p = grid->Append(new wxUIntProperty("Beam Orientation", "DmxBeamOrient", beam.GetBeamOrient()));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 360);
        p->SetEditor("SpinCtrl");
    }

    if (beam.SupportsYOffset()) {
        p = grid->Append(new wxFloatProperty("Beam Y Offset", "DmxBeamYOffset", beam.GetBeamYOffset()));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 500);
        p->SetAttribute("Precision", 1);
        p->SetAttribute("Step", 1);
        p->SetEditor("SpinCtrl");
    }
}

int OnBeamPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, DmxBeamAbility& beam, BaseObject* base) {
    if ("DmxBeamLength" == event.GetPropertyName()) {
        beam.SetBeamLength((float)event.GetPropertyValue().GetDouble());
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "DmxBeamAbility::OnBeamPropertyGridChange::DmxBeamLength");
        return 0;
    } else if ("DmxBeamWidth" == event.GetPropertyName()) {
        beam.SetBeamWidth((float)event.GetPropertyValue().GetDouble());
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "DmxBeamAbility::OnBeamPropertyGridChange::DmxBeamWidth");
        return 0;
    } else if ("DmxBeamOrient" == event.GetPropertyName()) {
        beam.SetBeamOrient((int)event.GetPropertyValue().GetLong());
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "DmxBeamAbility::OnBeamPropertyGridChange::DmxBeamOrient");
        return 0;
    } else if ("DmxBeamYOffset" == event.GetPropertyName()) {
        beam.SetBeamYOffset((float)event.GetPropertyValue().GetDouble());
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "DmxBeamAbility::OnBeamPropertyGridChange::DmxBeamYOffset");
        return 0;
    }
    return -1;
}

// ---- Preset ----

void AddPresetProperties(wxPropertyGridInterface* grid, DmxPresetAbility& preset, int num_channels) {
    int fixedChannelSize = preset.GetPresetsCount();
    int maxChannelSize = num_channels;
    if (fixedChannelSize > num_channels) {
        fixedChannelSize = num_channels;
    }

    auto p = grid->Append(new wxUIntProperty("Number of Fixed Channels", "DmxPresetSize", fixedChannelSize));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", maxChannelSize);
    p->SetEditor("SpinCtrl");

    while (maxChannelSize < (int)preset.GetPresetsCount()) {
        preset.PopPreset();
    }

    auto const& presets = preset.GetPresetSettings();
    int index = 0;
    for (auto const& pre : presets) {
        auto sp = grid->AppendIn(p, new wxUIntProperty(wxString::Format("Fixed Channel %d DMX Channel", 1 + index),
                                                       wxString::Format("DmxPresetChannel%d", index), pre.DMXChannel));
        sp->SetAttribute("Min", 1);
        sp->SetAttribute("Max", maxChannelSize);
        sp->SetEditor("SpinCtrl");

        sp = grid->AppendIn(p, new wxUIntProperty(wxString::Format("Fixed Channel %d DMX Value", 1 + index),
                                                  wxString::Format("DmxPresetValue%d", index), pre.DMXValue));
        sp->SetAttribute("Min", 0);
        sp->SetAttribute("Max", 255);
        sp->SetEditor("SpinCtrl");

        grid->AppendIn(p, new wxStringProperty(wxString::Format("Fixed Channel %d Description", 1 + index),
                                               wxString::Format("DmxPresetDesc%d", index), pre.Description));
        ++index;
    }
}

int OnPresetPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, DmxPresetAbility& preset, int num_channels, BaseObject* base) {
    if ("DmxPresetSize" == event.GetPropertyName()) {
        int presetSize = (int)event.GetPropertyValue().GetInteger();
        if (presetSize > num_channels) {
            presetSize = num_channels;
        }
        while (presetSize < (int)preset.GetPresetsCount()) {
            preset.PopPreset();
        }
        int diff = presetSize - preset.GetPresetsCount();
        for (int i = 0; i < diff; i++) {
            preset.AddPreset(preset.GetPresetsCount() + 1, 0, "");
        }
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                          OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxPresetAbility::OnPropertyGridChange::DmxPresetSize");
        return 0;
    } else if (event.GetPropertyName().StartsWith("DmxPresetSize.DmxPresetChannel")) {
        int dxmVal = (int)event.GetPropertyValue().GetInteger();
        wxString namekey = event.GetPropertyName();
        namekey.Replace("DmxPresetSize.DmxPresetChannel", "");
        int index = wxAtoi(namekey);
        if (index >= 0 && index < (int)preset.GetPresetsCount()) {
            preset.SetPresetChannel(index, dxmVal);
            base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "DmxPresetAbility::OnPropertyGridChange::DmxPresetChannel");
        }
    } else if (event.GetPropertyName().StartsWith("DmxPresetSize.DmxPresetValue")) {
        int dxmVal = (int)event.GetPropertyValue().GetInteger();
        wxString namekey = event.GetPropertyName();
        namekey.Replace("DmxPresetSize.DmxPresetValue", "");
        int index = wxAtoi(namekey);
        if (index >= 0 && index < (int)preset.GetPresetsCount()) {
            preset.SetPresetValue(index, dxmVal);
            base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "DmxPresetAbility::OnPropertyGridChange::DmxPresetValue");
        }
    } else if (event.GetPropertyName().StartsWith("DmxPresetSize.DmxPresetDesc")) {
        auto dxmDesc = event.GetPropertyValue().GetString();
        wxString namekey = event.GetPropertyName();
        namekey.Replace("DmxPresetSize.DmxPresetDesc", "");
        int index = wxAtoi(namekey);
        if (index >= 0 && index < (int)preset.GetPresetsCount()) {
            preset.SetPresetDescription(index, dxmDesc.ToStdString());
            base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "DmxPresetAbility::OnPropertyGridChange::DmxPresetDesc");
        }
    }
    return -1;
}

// ---- Color RGB ----

void AddColorRGBProperties(wxPropertyGridInterface* grid, const DmxColorAbilityRGB& color, bool pwm) {
    auto p = grid->Append(new wxUIntProperty("Red Channel", "DmxRedChannel", color.GetRedChannel()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");
    if (pwm) {
        p = grid->Append(new wxUIntProperty("   PWM Red Brightness", "DmxRedBrightness", color.GetRedBrightness()));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 200);
        p->SetEditor("SpinCtrl");
        p = grid->Append(new wxFloatProperty("   PWM Red Gamma", "DmxRedGamma", color.GetRedGamma()));
        p->SetAttribute("Min", 0.1F);
        p->SetAttribute("Max", 5.0F);
        p->SetEditor("SpinCtrlDouble");
    }

    p = grid->Append(new wxUIntProperty("Green Channel", "DmxGreenChannel", color.GetGreenChannel()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");
    if (pwm) {
        p = grid->Append(new wxUIntProperty("   PWM Green Brightness", "DmxGreenBrightness", color.GetGreenBrightness()));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 200);
        p->SetEditor("SpinCtrl");
        p = grid->Append(new wxFloatProperty("   PWM Green Gamma", "DmxGreenGamma", color.GetGreenGamma()));
        p->SetAttribute("Min", 0.1F);
        p->SetAttribute("Max", 5.0F);
        p->SetEditor("SpinCtrlDouble");
    }

    p = grid->Append(new wxUIntProperty("Blue Channel", "DmxBlueChannel", color.GetBlueChannel()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");
    if (pwm) {
        p = grid->Append(new wxUIntProperty("   PWM Blue Brightness", "DmxBlueBrightness", color.GetBlueBrightness()));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 200);
        p->SetEditor("SpinCtrl");
        p = grid->Append(new wxFloatProperty("   PWM Blue Gamma", "DmxBlueGamma", color.GetBlueGamma()));
        p->SetAttribute("Min", 0.1F);
        p->SetAttribute("Max", 5.0F);
        p->SetEditor("SpinCtrlDouble");
    }

    p = grid->Append(new wxUIntProperty("White Channel", "DmxWhiteChannel", color.GetWhiteChannel()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");
    if (pwm) {
        p = grid->Append(new wxUIntProperty("   PWM White Brightness", "DmxWhiteBrightness", color.GetWhiteBrightness()));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 200);
        p->SetEditor("SpinCtrl");
        p = grid->Append(new wxFloatProperty("   PWM White Gamma", "DmxWhiteGamma", color.GetWhiteGamma()));
        p->SetAttribute("Min", 0.1F);
        p->SetAttribute("Max", 5.0F);
        p->SetEditor("SpinCtrlDouble");
    }
}

int OnColorRGBPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, DmxColorAbilityRGB& color, BaseObject* base) {
    std::string propName = event.GetPropertyName().ToStdString();

    if ("DmxRedChannel" == propName) {
        color.SetRedChannel((int)event.GetPropertyValue().GetLong());
    } else if ("DmxGreenChannel" == propName) {
        color.SetGreenChannel((int)event.GetPropertyValue().GetLong());
    } else if ("DmxBlueChannel" == propName) {
        color.SetBlueChannel((int)event.GetPropertyValue().GetLong());
    } else if ("DmxWhiteChannel" == propName) {
        color.SetWhiteChannel((int)event.GetPropertyValue().GetLong());
    } else if ("DmxRedBrightness" == propName) {
        color.SetRedBrightness((int)event.GetPropertyValue().GetLong());
    } else if ("DmxGreenBrightness" == propName) {
        color.SetGreenBrightness((int)event.GetPropertyValue().GetLong());
    } else if ("DmxBlueBrightness" == propName) {
        color.SetBlueBrightness((int)event.GetPropertyValue().GetLong());
    } else if ("DmxWhiteBrightness" == propName) {
        color.SetWhiteBrightness((int)event.GetPropertyValue().GetLong());
    } else if ("DmxRedGamma" == propName) {
        color.SetRedGamma((float)event.GetPropertyValue().GetDouble());
    } else if ("DmxGreenGamma" == propName) {
        color.SetGreenGamma((float)event.GetPropertyValue().GetDouble());
    } else if ("DmxBlueGamma" == propName) {
        color.SetBlueGamma((float)event.GetPropertyValue().GetDouble());
    } else if ("DmxWhiteGamma" == propName) {
        color.SetWhiteGamma((float)event.GetPropertyValue().GetDouble());
    } else {
        return -1;
    }
    base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "DmxColorAbilityRGB::OnColorPropertyGridChange::" + propName);
    return 0;
}

// ---- Color CMY ----

void AddColorCMYProperties(wxPropertyGridInterface* grid, const DmxColorAbilityCMY& color, bool pwm) {
    auto p = grid->Append(new wxUIntProperty("Cyan Channel", "DmxCyanChannel", color.GetCyanChannel()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Magenta Channel", "DmxMagentaChannel", color.GetMagentaChannel()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Yellow Channel", "DmxYellowChannel", color.GetYellowChannel()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("White Channel", "DmxWhiteChannel", color.GetWhiteChannel()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");
}

int OnColorCMYPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, DmxColorAbilityCMY& color, BaseObject* base) {
    std::string propName = event.GetPropertyName().ToStdString();

    if ("DmxCyanChannel" == propName) {
        color.SetCyanChannel((int)event.GetPropertyValue().GetLong());
    } else if ("DmxMagentaChannel" == propName) {
        color.SetMagentaChannel((int)event.GetPropertyValue().GetLong());
    } else if ("DmxYellowChannel" == propName) {
        color.SetYellowChannel((int)event.GetPropertyValue().GetLong());
    } else if ("DmxWhiteChannel" == propName) {
        color.SetWhiteChannel((int)event.GetPropertyValue().GetLong());
    } else {
        return -1;
    }
    base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "DmxColorAbilityCMY::OnColorPropertyGridChange::" + propName);
    return 0;
}

// ---- Color Wheel ----

void AddColorWheelProperties(wxPropertyGridInterface* grid, const DmxColorAbilityWheel& color, bool pwm) {
    auto p = grid->Append(new wxUIntProperty("Color Wheel Channel", "DmxColorWheelChannel", color.GetWheelChannel()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Dimmer Channel", "DmxDimmerChannel", color.GetDimmerChannel()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 512);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Color Wheel Delay(ms)", "DmxColorWheelDelay", color.GetWheelDelay()));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 10000);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Color Wheel Size", "DmxColorWheelSize", color.GetColorWheelColorSize()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", DmxColorAbilityWheel::MAX_COLORS);
    p->SetEditor("SpinCtrl");

    auto const& colors = color.GetWheelColorSettings();
    int index = 0;
    for (auto const& col : colors) {
        grid->AppendIn(p,
                       new wxColourProperty(wxString::Format("Color %d", 1 + index),
                                            wxString::Format("DmxColorWheelColor%d", index), xlColorToWxColour(col.color)));
        auto sp = grid->AppendIn(p,
            new wxUIntProperty(wxString::Format("Color %d DMX", 1 + index),
                wxString::Format("DmxColorWheelDMX%d", index), col.dmxValue));
        sp->SetAttribute("Min", 0);
        sp->SetAttribute("Max", 255);
        sp->SetEditor("SpinCtrl");
        ++index;
    }
}

int OnColorWheelPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, DmxColorAbilityWheel& color, BaseObject* base) {
    if ("DmxColorWheelChannel" == event.GetPropertyName()) {
        color.SetWheelChannel((int)event.GetPropertyValue().GetLong());
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "DmxColorAbility::OnColorPropertyGridChange::DmxColorWheelChannel");
        return 0;
    } else if ("DmxDimmerChannel" == event.GetPropertyName()) {
        color.SetDimmerChannel((int)event.GetPropertyValue().GetLong());
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "DmxColorAbility::OnColorPropertyGridChange::DmxDimmerChannel");
        return 0;
    } else if ("DmxColorWheelDelay" == event.GetPropertyName()) {
        color.SetWheelDelay((int)event.GetPropertyValue().GetLong());
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "DmxColorAbility::OnColorPropertyGridChange::DmxColorWheelDelay");
        return 0;
    } else if ("DmxColorWheelSize" == event.GetPropertyName()) {
        int colorSize = (int)event.GetPropertyValue().GetInteger();
        while (colorSize < (int)color.GetColorWheelColorSize()) {
            color.PopColor();
        }
        int diff = colorSize - color.GetColorWheelColorSize();
        for (int i = 0; i < diff; i++) {
            auto lastValue = (color.GetLastColorDMX() + 50) % 255;
            color.AddWheelColor(xlRED, lastValue);
        }
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                          OutputModelManager::WORK_RELOAD_PROPERTYGRID, "DmxColorAbility::OnColorPropertyGridChange::DmxColorWheelSize");
        return 0;
    } else if (event.GetPropertyName().StartsWith("DmxColorWheelSize.DmxColorWheelDMX")) {
        int dxmVal = (int)event.GetPropertyValue().GetInteger();
        wxString namekey = event.GetPropertyName();
        namekey.Replace("DmxColorWheelSize.DmxColorWheelDMX", "");
        int index = wxAtoi(namekey);
        if (index >= 0 && index < (int)color.GetColorWheelColorSize()) {
            color.SetWheelColorDMX(index, dxmVal);
            base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "DmxColorAbility::OnColorPropertyGridChange::DmxColorWheelDMX");
        }
    } else if (event.GetPropertyName().StartsWith("DmxColorWheelSize.DmxColorWheelColor")) {
        wxColour wheelColour = *wxBLACK;
        wheelColour << event.GetProperty()->GetValue();
        wxString namekey = event.GetPropertyName();
        namekey.Replace("DmxColorWheelSize.DmxColorWheelColor", "");
        int index = wxAtoi(namekey);
        if (index >= 0 && index < (int)color.GetColorWheelColorSize()) {
            color.SetWheelColor(index, wxColourToXlColor(wheelColour));
            base->AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "DmxColorAbility::OnColorPropertyGridChange::DmxColorWheelColor");
        }
    }
    return -1;
}

} // namespace DmxAbilityPropertyHelpers
