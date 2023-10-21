/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "DMXEffect.h"
#include "DMXPanel.h"
#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../models/Model.h"
#include "../models/ModelGroup.h"

#include "../../include/dmx-16.xpm"
#include "../../include/dmx-24.xpm"
#include "../../include/dmx-32.xpm"
#include "../../include/dmx-48.xpm"
#include "../../include/dmx-64.xpm"
#include "UtilFunctions.h"

DMXEffect::DMXEffect(int id) : RenderableEffect(id, "DMX", dmx_16, dmx_24, dmx_32, dmx_48, dmx_64)
{
    //ctor
}

DMXEffect::~DMXEffect()
{
    //dtor
}

void DMXEffect::RemapSelectedDMXEffectValues(Effect* effect, const std::vector<std::tuple<int, int, float, int>>& dmxmappings) const
{
    SettingsMap &settings = effect->GetSettings();
    SettingsMap oldSettings = settings;
    for (auto const &[ fromi, toi, scale, offset ] : dmxmappings)
    {
        auto froms = wxString::Format("%d", fromi);
        auto tos = wxString::Format("%d", toi);
        auto slider = oldSettings.Get("E_SLIDER_DMX" + froms, "NOTTHERE");
        auto vc = oldSettings.Get("E_VALUECURVE_DMX" + froms, "NOTTHERE");

        if (slider != "NOTTHERE")
        {
            int new_value = ((double)std::stoi(slider) * scale) + offset;
            settings["E_SLIDER_DMX" + tos] = std::to_string( new_value);
        }
        else
        {
            settings.erase("E_SLIDER_DMX" + tos);
        }
        if (vc != "NOTTHERE")
        {
            ValueCurve valc;
            valc.SetDivisor(1.0F);
            valc.SetLimits(DMX_MIN, DMX_MAX);
            valc.Deserialise(vc);
            valc.ScaleAndOffsetValues(scale, offset);
            settings["E_VALUECURVE_DMX" + tos] = valc.Serialise();
        }
        else
        {
            settings.erase("E_VALUECURVE_DMX" + tos);
        }
    }
}

xlEffectPanel *DMXEffect::CreatePanel(wxWindow *parent) {
    return new DMXPanel(parent);
}

static int GetPct(wxString val)
{
    int value = wxAtoi(val);
    return (value * 100) / 255;
}

void DMXEffect::SetDefaultParameters() {
    DMXPanel *dp = (DMXPanel*)panel;
    if (dp == nullptr) {
        return;
    }

    dp->ValueCurve_DMX1->SetActive(false);
    dp->ValueCurve_DMX2->SetActive(false);
    dp->ValueCurve_DMX3->SetActive(false);
    dp->ValueCurve_DMX4->SetActive(false);
    dp->ValueCurve_DMX5->SetActive(false);
    dp->ValueCurve_DMX6->SetActive(false);
    dp->ValueCurve_DMX7->SetActive(false);
    dp->ValueCurve_DMX8->SetActive(false);
    dp->ValueCurve_DMX9->SetActive(false);
    dp->ValueCurve_DMX10->SetActive(false);
    dp->ValueCurve_DMX11->SetActive(false);
    dp->ValueCurve_DMX12->SetActive(false);
    dp->ValueCurve_DMX13->SetActive(false);
    dp->ValueCurve_DMX14->SetActive(false);
    dp->ValueCurve_DMX15->SetActive(false);
    dp->ValueCurve_DMX16->SetActive(false);
    dp->ValueCurve_DMX17->SetActive(false);
    dp->ValueCurve_DMX18->SetActive(false);
    dp->ValueCurve_DMX19->SetActive(false);
    dp->ValueCurve_DMX20->SetActive(false);
    dp->ValueCurve_DMX21->SetActive(false);
    dp->ValueCurve_DMX22->SetActive(false);
    dp->ValueCurve_DMX23->SetActive(false);
    dp->ValueCurve_DMX24->SetActive(false);
    dp->ValueCurve_DMX25->SetActive(false);
    dp->ValueCurve_DMX26->SetActive(false);
    dp->ValueCurve_DMX27->SetActive(false);
    dp->ValueCurve_DMX28->SetActive(false);
    dp->ValueCurve_DMX29->SetActive(false);
    dp->ValueCurve_DMX30->SetActive(false);
    dp->ValueCurve_DMX31->SetActive(false);
    dp->ValueCurve_DMX32->SetActive(false);
    dp->ValueCurve_DMX33->SetActive(false);
    dp->ValueCurve_DMX34->SetActive(false);
    dp->ValueCurve_DMX35->SetActive(false);
    dp->ValueCurve_DMX36->SetActive(false);
    dp->ValueCurve_DMX37->SetActive(false);
    dp->ValueCurve_DMX38->SetActive(false);
    dp->ValueCurve_DMX39->SetActive(false);
    dp->ValueCurve_DMX40->SetActive(false);

    SetSliderValue(dp->Slider_DMX1, 0);
    SetSliderValue(dp->Slider_DMX2, 0);
    SetSliderValue(dp->Slider_DMX3, 0);
    SetSliderValue(dp->Slider_DMX4, 0);
    SetSliderValue(dp->Slider_DMX5, 0);
    SetSliderValue(dp->Slider_DMX6, 0);
    SetSliderValue(dp->Slider_DMX7, 0);
    SetSliderValue(dp->Slider_DMX8, 0);
    SetSliderValue(dp->Slider_DMX9, 0);
    SetSliderValue(dp->Slider_DMX10, 0);
    SetSliderValue(dp->Slider_DMX11, 0);
    SetSliderValue(dp->Slider_DMX12, 0);
    SetSliderValue(dp->Slider_DMX13, 0);
    SetSliderValue(dp->Slider_DMX14, 0);
    SetSliderValue(dp->Slider_DMX15, 0);
    SetSliderValue(dp->Slider_DMX16, 0);
    SetSliderValue(dp->Slider_DMX17, 0);
    SetSliderValue(dp->Slider_DMX18, 0);
    SetSliderValue(dp->Slider_DMX19, 0);
    SetSliderValue(dp->Slider_DMX20, 0);
    SetSliderValue(dp->Slider_DMX21, 0);
    SetSliderValue(dp->Slider_DMX22, 0);
    SetSliderValue(dp->Slider_DMX23, 0);
    SetSliderValue(dp->Slider_DMX24, 0);
    SetSliderValue(dp->Slider_DMX25, 0);
    SetSliderValue(dp->Slider_DMX26, 0);
    SetSliderValue(dp->Slider_DMX27, 0);
    SetSliderValue(dp->Slider_DMX28, 0);
    SetSliderValue(dp->Slider_DMX29, 0);
    SetSliderValue(dp->Slider_DMX30, 0);
    SetSliderValue(dp->Slider_DMX31, 0);
    SetSliderValue(dp->Slider_DMX32, 0);
    SetSliderValue(dp->Slider_DMX33, 0);
    SetSliderValue(dp->Slider_DMX34, 0);
    SetSliderValue(dp->Slider_DMX35, 0);
    SetSliderValue(dp->Slider_DMX36, 0);
    SetSliderValue(dp->Slider_DMX37, 0);
    SetSliderValue(dp->Slider_DMX38, 0);
    SetSliderValue(dp->Slider_DMX39, 0);
    SetSliderValue(dp->Slider_DMX40, 0);

    SetCheckBoxValue(dp->CheckBox_INVDMX1, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX2, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX3, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX4, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX5, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX6, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX7, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX8, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX9, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX10, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX11, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX12, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX13, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX14, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX15, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX16, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX17, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX18, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX19, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX20, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX21, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX22, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX23, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX24, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX25, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX26, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX27, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX28, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX29, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX30, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX31, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX32, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX33, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX34, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX35, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX36, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX37, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX38, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX39, false);
    SetCheckBoxValue(dp->CheckBox_INVDMX40, false);
}

void DMXEffect::adjustSettings(const std::string &version, Effect *effect, bool removeDefaults)
{
    // give the base class a chance to adjust any settings
    if (RenderableEffect::needToAdjustSettings(version))
    {
        RenderableEffect::adjustSettings(version, effect, removeDefaults);
    }

    SettingsMap &settings = effect->GetSettings();

    if (IsVersionOlder("2016.39", version))
    {
        if (settings.GetBool("E_CHECKBOX_Use_Dmx_Ramps")) {
            settings["E_VALUECURVE_DMX1"] = wxString::Format("Active=TRUE|Id=ID_VALUECURVE_DMX1|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|RV=TRUE|", GetPct(settings["E_SLIDER_DMX1"]), GetPct(settings["E_SLIDER_DMX1_Ramp"]));
            settings["E_VALUECURVE_DMX2"] = wxString::Format("Active=TRUE|Id=ID_VALUECURVE_DMX2|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|RV=TRUE|", GetPct(settings["E_SLIDER_DMX2"]), GetPct(settings["E_SLIDER_DMX2_Ramp"]));
            settings["E_VALUECURVE_DMX3"] = wxString::Format("Active=TRUE|Id=ID_VALUECURVE_DMX3|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|RV=TRUE|", GetPct(settings["E_SLIDER_DMX3"]), GetPct(settings["E_SLIDER_DMX3_Ramp"]));
            settings["E_VALUECURVE_DMX4"] = wxString::Format("Active=TRUE|Id=ID_VALUECURVE_DMX4|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|RV=TRUE|", GetPct(settings["E_SLIDER_DMX4"]), GetPct(settings["E_SLIDER_DMX4_Ramp"]));
            settings["E_VALUECURVE_DMX5"] = wxString::Format("Active=TRUE|Id=ID_VALUECURVE_DMX5|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|RV=TRUE|", GetPct(settings["E_SLIDER_DMX5"]), GetPct(settings["E_SLIDER_DMX5_Ramp"]));
            settings["E_VALUECURVE_DMX6"] = wxString::Format("Active=TRUE|Id=ID_VALUECURVE_DMX6|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|RV=TRUE|", GetPct(settings["E_SLIDER_DMX6"]), GetPct(settings["E_SLIDER_DMX6_Ramp"]));
            settings["E_VALUECURVE_DMX7"] = wxString::Format("Active=TRUE|Id=ID_VALUECURVE_DMX7|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|RV=TRUE|", GetPct(settings["E_SLIDER_DMX7"]), GetPct(settings["E_SLIDER_DMX7_Ramp"]));
            settings["E_VALUECURVE_DMX8"] = wxString::Format("Active=TRUE|Id=ID_VALUECURVE_DMX8|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|RV=TRUE|", GetPct(settings["E_SLIDER_DMX8"]), GetPct(settings["E_SLIDER_DMX8_Ramp"]));
            settings["E_VALUECURVE_DMX9"] = wxString::Format("Active=TRUE|Id=ID_VALUECURVE_DMX9|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|RV=TRUE|", GetPct(settings["E_SLIDER_DMX9"]), GetPct(settings["E_SLIDER_DMX9_Ramp"]));
            settings["E_VALUECURVE_DMX10"] = wxString::Format("Active=TRUE|Id=ID_VALUECURVE_DMX10|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|RV=TRUE|", GetPct(settings["E_SLIDER_DMX10"]), GetPct(settings["E_SLIDER_DMX10_Ramp"]));
            settings["E_VALUECURVE_DMX11"] = wxString::Format("Active=TRUE|Id=ID_VALUECURVE_DMX11|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|RV=TRUE|", GetPct(settings["E_SLIDER_DMX11"]), GetPct(settings["E_SLIDER_DMX11_Ramp"]));
            settings["E_VALUECURVE_DMX12"] = wxString::Format("Active=TRUE|Id=ID_VALUECURVE_DMX12|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|RV=TRUE|", GetPct(settings["E_SLIDER_DMX12"]), GetPct(settings["E_SLIDER_DMX12_Ramp"]));
            settings["E_VALUECURVE_DMX13"] = wxString::Format("Active=TRUE|Id=ID_VALUECURVE_DMX13|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|RV=TRUE|", GetPct(settings["E_SLIDER_DMX13"]), GetPct(settings["E_SLIDER_DMX13_Ramp"]));
            settings["E_VALUECURVE_DMX14"] = wxString::Format("Active=TRUE|Id=ID_VALUECURVE_DMX14|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|RV=TRUE|", GetPct(settings["E_SLIDER_DMX14"]), GetPct(settings["E_SLIDER_DMX14_Ramp"]));
            settings["E_VALUECURVE_DMX15"] = wxString::Format("Active=TRUE|Id=ID_VALUECURVE_DMX15|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|RV=TRUE|", GetPct(settings["E_SLIDER_DMX15"]), GetPct(settings["E_SLIDER_DMX15_Ramp"]));
            settings["E_VALUECURVE_DMX16"] = "Active=TRUE|Id=ID_VALUECURVE_DMX16|Type=Ramp|Min=0.00|Max=255.00|P1=0|P2=0|RV=TRUE|";
            settings["E_VALUECURVE_DMX17"] = "Active=TRUE|Id=ID_VALUECURVE_DMX17|Type=Ramp|Min=0.00|Max=255.00|P1=0|P2=0|RV=TRUE|";
            settings["E_VALUECURVE_DMX18"] = "Active=TRUE|Id=ID_VALUECURVE_DMX18|Type=Ramp|Min=0.00|Max=255.00|P1=0|P2=0|RV=TRUE|";
            settings.erase("E_SLIDER_DMX1");
            settings.erase("E_SLIDER_DMX2");
            settings.erase("E_SLIDER_DMX3");
            settings.erase("E_SLIDER_DMX4");
            settings.erase("E_SLIDER_DMX5");
            settings.erase("E_SLIDER_DMX6");
            settings.erase("E_SLIDER_DMX7");
            settings.erase("E_SLIDER_DMX8");
            settings.erase("E_SLIDER_DMX9");
            settings.erase("E_SLIDER_DMX10");
            settings.erase("E_SLIDER_DMX11");
            settings.erase("E_SLIDER_DMX12");
            settings.erase("E_SLIDER_DMX13");
            settings.erase("E_SLIDER_DMX14");
            settings.erase("E_SLIDER_DMX15");
        } else {
            settings["E_SLIDER_DMX16"] = "0";
            settings["E_SLIDER_DMX17"] = "0";
            settings["E_SLIDER_DMX18"] = "0";
        }
        settings.erase("E_CHOICE_Num_Dmx_Channels");
        settings.erase("E_CHECKBOX_Use_Dmx_Ramps");
        settings.erase("E_SLIDER_DMX1_Ramp");
        settings.erase("E_SLIDER_DMX2_Ramp");
        settings.erase("E_SLIDER_DMX3_Ramp");
        settings.erase("E_SLIDER_DMX4_Ramp");
        settings.erase("E_SLIDER_DMX5_Ramp");
        settings.erase("E_SLIDER_DMX6_Ramp");
        settings.erase("E_SLIDER_DMX7_Ramp");
        settings.erase("E_SLIDER_DMX8_Ramp");
        settings.erase("E_SLIDER_DMX9_Ramp");
        settings.erase("E_SLIDER_DMX10_Ramp");
        settings.erase("E_SLIDER_DMX11_Ramp");
        settings.erase("E_SLIDER_DMX12_Ramp");
        settings.erase("E_SLIDER_DMX13_Ramp");
        settings.erase("E_SLIDER_DMX14_Ramp");
        settings.erase("E_SLIDER_DMX15_Ramp");
    }
}

bool DMXEffect::SetDMXSinglColorPixel(int chan, int num_channels, const SettingsMap &SettingsMap, double eff_pos, xlColor& color, RenderBuffer &buffer)
{
    if( num_channels >= chan ) {
        std::string name = wxString::Format("DMX%d", chan).ToStdString();
        int value = GetValueCurveInt(name, 0, SettingsMap, eff_pos, DMX_MIN, DMX_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

        if (SettingsMap.GetBool("CHECKBOX_INV" + name, false))
        {
            value = 255 - value;
        }

        color.red = value;
        color.green = value;
        color.blue = value;
        buffer.SetPixel(chan-1, 0, color, false, false, true);
        return false;
    } else {
        return true;
    }
}

void DMXEffect::SetColorBasedOnStringType(int value, int slot, xlColor& color, const std::string& string_type)
{
    const char which_color = string_type[slot-1];
    if( which_color == 'R' ) {
        color.red = value;
    } else if( which_color == 'G' ) {
        color.green = value;
    } else if( which_color == 'B' ) {
        color.blue = value;
    }
}

bool DMXEffect::SetDMXRGBNode(int node, int num_channels, const SettingsMap &SettingsMap, double eff_pos, xlColor& color, RenderBuffer &buffer, const std::string& string_type)
{
    bool return_val = false;
    color = xlBLACK;
    int base_chan = ((node-1)*3+1);
    if( num_channels >= base_chan || buffer.BufferWi < node) {
        std::string name = wxString::Format("DMX%d", base_chan).ToStdString();
        int value = GetValueCurveInt(name, 0, SettingsMap, eff_pos, DMX_MIN, DMX_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

        if (SettingsMap.GetBool("CHECKBOX_INV" + name, false)) {
            value = 255 - value;
        }

        SetColorBasedOnStringType(value, 1, color, string_type);
        if( num_channels >= base_chan+1 ) {
            name = wxString::Format("DMX%d", base_chan+1);
            value = GetValueCurveInt(name, 0, SettingsMap, eff_pos, DMX_MIN, DMX_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

            if (SettingsMap.GetBool("CHECKBOX_INV" + name, false)) {
                value = 255 - value;
            }

            SetColorBasedOnStringType(value, 2, color, string_type);
            if( num_channels >= base_chan+2 ) {
                name = wxString::Format("DMX%d", base_chan+2);
                value = GetValueCurveInt(name, 0, SettingsMap, eff_pos, DMX_MIN, DMX_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

                if (SettingsMap.GetBool("CHECKBOX_INV" + name, false)) {
                    value = 255 - value;
                }

                SetColorBasedOnStringType(value, 3, color, string_type);
            } else {
                return_val = true;
            }
        } else {
            return_val = true;
        }
        buffer.SetPixel(node-1, 0, color, false, false, true);
    } else {
        return_val = true;
    }
    return return_val;
}

void DMXEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    double eff_pos = buffer.GetEffectTimeIntervalPosition();

    if (buffer.cur_model == "") {
        return;
    }
    Model* model_info = buffer.GetModel();
    if (model_info == nullptr) {
        return;
    }

    int num_channels = model_info->GetNumChannels();

    const std::string& string_type = model_info->GetStringType();

    xlColor color = xlBLACK;

    if (StartsWith(string_type, "Single Color")) {

        // handle channels for single color nodes
        for (uint32_t i = 1; i <= 40; ++i)
        {
            if (SetDMXSinglColorPixel(i, num_channels, SettingsMap, eff_pos, color, buffer))
                return;
        }
   } else {
        // handle channels for 3 color nodes
        for (uint32_t i = 1; i <= 40 / 3; ++i) {
            if (SetDMXRGBNode(i, num_channels, SettingsMap, eff_pos, color, buffer, string_type))
                return;
        }
    }
}

void DMXEffect::SetPanelStatus(Model *cls) {
    DMXPanel *p = (DMXPanel*)panel;
    if (p == nullptr) {
        return;
    }
    if (cls == nullptr) {
        return;
    }

    Model* m = cls;
    if (cls->GetDisplayAs() == "ModelGroup") {
        m = dynamic_cast<ModelGroup*>(cls)->GetFirstModel();
        if (m == nullptr) m = cls;
    }

    int num_channels = m->GetNumChannels();

    for(int i = 1; i <= DMX_CHANNELS; ++i) {
        wxString label_ctrl = wxString::Format("ID_STATICTEXT_DMX%d", i);
        std::string name = m->GetNodeName(i-1);
        wxStaticText* label = (wxStaticText*)(p->FindWindowByName(label_ctrl));
        if( label != nullptr ) {
            if( name == "" ) {
                label->SetLabel(wxString::Format("Channel%d:", i));
            } else {
                label->SetLabel(wxString::Format("%s:", name));
            }
        }
        wxString slider_ctrl = wxString::Format("ID_SLIDER_DMX%d", i);
        wxSlider* slider = (wxSlider*)(p->FindWindowByName(slider_ctrl));
        wxString vc_ctrl = wxString::Format("ID_VALUECURVE_DMX%d", i);
        wxBitmapButton* curve = (wxBitmapButton*)(p->FindWindowByName(vc_ctrl));
        wxString text_ctrl = wxString::Format("IDD_TEXTCTRL_DMX%d", i);
        wxBitmapButton* text = (wxBitmapButton*)(p->FindWindowByName(text_ctrl));
        wxString inv_ctrl = wxString::Format("ID_CHECKBOX_INVDMX%d", i);
        wxBitmapButton* inv = (wxBitmapButton*)(p->FindWindowByName(inv_ctrl));
        if (i > num_channels) {
            if( label != nullptr ) label->Enable(false);
            if( slider != nullptr ) slider->Enable(false);
            if( curve != nullptr ) curve->Enable(false);
            if( text != nullptr ) text->Enable(false);
            if (inv != nullptr)
                inv->Enable(false);
        } else {
            if( label != nullptr ) label->Enable(true);
            if( slider != nullptr ) slider->Enable(true);
            if( curve != nullptr ) curve->Enable(true);
            if (text != nullptr)
                text->Enable(true);
            if (inv != nullptr)
                inv->Enable(true);
        }
    }
    p->FlexGridSizer_Main->Layout();
    p->Refresh();
}
