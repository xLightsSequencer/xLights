/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "AdjustEffect.h"
#include "AdjustPanel.h"
#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../models/Model.h"
#include "../models/ModelGroup.h"
#include "../xLightsApp.h"
#include "../xLightsMain.h"
#include "../TimingPanel.h"

#include "../../include/adjust16.xpm"
#include "../../include/adjust24.xpm"
#include "../../include/adjust32.xpm"
#include "../../include/adjust48.xpm"
#include "../../include/adjust64.xpm"
#include "UtilFunctions.h"

AdjustEffect::AdjustEffect(int id) :
    RenderableEffect(id, "Adjust", adjust16_xpm, adjust24_xpm, adjust32_xpm, adjust48_xpm, adjust64_xpm)
{
    //ctor
}

AdjustEffect::~AdjustEffect()
{
    //dtor
}

xlEffectPanel *AdjustEffect::CreatePanel(wxWindow *parent) {
    return new AdjustPanel(parent);
}

std::list<std::string> AdjustEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res;

    if (settings.Get("T_CHECKBOX_Canvas", "0") == "0") {
        res.push_back(wxString::Format("    WARN: Canvas mode not enabled on a Adjust effect. Without canvas mode Adjust is unlikely to do anything useful. Effect: Adjust, Model: %s, Start %s", model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }

    return res;
}

void AdjustEffect::SetDefaultParameters() {
    AdjustPanel *ap = (AdjustPanel*)panel;
    if (ap == nullptr) {
        return;
    }

	SetChoiceValue(ap->Choice_Action, "None");
    SetSpinValue(ap->SpinCtrl_Value1, 0);
    SetSpinValue(ap->SpinCtrl_Value2, 0);
    SetSpinValue(ap->SpinCtrl_NthChannel, 1);
    SetSpinValue(ap->SpinCtrl_StartingAt, 1);

    // Turn on canvas mode as this really only makes sense in canvas mode
    xLightsFrame* frame = xLightsApp::GetFrame();
    TimingPanel* layerBlendingPanel = frame->GetLayerBlendingPanel();
    layerBlendingPanel->CheckBox_Canvas->SetValue(true);
}

void AdjustEffect::AdjustChannels(bool singleColour, int numChannels, RenderBuffer& buffer, const std::string& action, int value1, int value2, int nth, int starting, int count)
{
    int channels = std::min(numChannels, buffer.BufferWi * buffer.BufferHt * (singleColour ? 1 : 3));
    int done = 0;

    for (int i = starting - 1; (count == 0 || done < count) && i < channels; i = i + nth) {
        ++done;
        int value = 0;
        xlColor c = xlBLACK;

        // get the channel value
        if (singleColour) {
            c = buffer.GetPixel(i % buffer.BufferWi, i / buffer.BufferWi);
            value = c.red;
        } else {
            c = buffer.GetPixel((i / 3) % buffer.BufferWi, (i / 3) / buffer.BufferWi);
            if (i % 3 == 0)
                value = c.red;
            else if (i % 3 == 1)
                value = c.green;
            else
                value = c.blue;
        }
        c.alpha = 255;

        // adjust it
        if (action == "None") {
            // do nothing
        } else if (action == "Adjust By Value") {
            value += value1;
        } else if (action == "Adjust By Percentage") {
            value += (value * value1) / 100;
        } else if (action == "Set Minimum") {
            value = std::max(value1, value);
        } else if (action == "Set Maximum") {
            value = std::min(value1, value);
        } else if (action == "Set Range") {
            if (value1 > value2)
                std::swap(value1, value2);
            value = std::min(value2, std::max(value1, value));
        } else if (action == "Shift With Wrap By Value") {
            value += value1;
            if (value < 0)
                value += 256;
            else if (value > 255)
                value -= 256;
        } else if (action == "Shift With Wrap By Percentage") {
        } else if (action == "Prevent Range") {
            if (value1 > value2)
                std::swap(value1, value2);
            if (value >= value1 && value <= value2) {
                // within restricted range ... move it to the closest value
                if (value - value1 < value2 - value)
                    value = value1;
                else
                    value = value2;
            }
        } else if (action == "Reverse") {
            value = 255 - value;
        }

        // force it in range
        value = std::min(255, std::max(0, value));

        // write the channel value
        if (singleColour) {
            c.red = value;
            c.green = value;
            c.blue = value;
            buffer.SetPixel(i % buffer.BufferWi, i / buffer.BufferWi, c, false, true, true);
        } else {
            if (i % 3 == 0)
                c.red = value;
            else if (i % 3 == 1)
                c.green = value;
            else
                c.blue = value;
            buffer.SetPixel((i / 3) % buffer.BufferWi, (i / 3) / buffer.BufferWi, c);
        }
    }
}

void AdjustEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer)
{
    int num_channels = 0;
    std::string string_type = "";

    Model* model_info = buffer.GetModel();
    if (model_info == nullptr)
        num_channels = buffer.BufferWi * buffer.BufferHt * 3;
    else {
        num_channels = model_info->GetNumChannels();
        string_type = model_info->GetStringType();
    }

    auto action = SettingsMap.Get("CHOICE_Action", "None");
    auto value1 = SettingsMap.GetInt("SPINCTRL_Value1", 0);
    auto value2 = SettingsMap.GetInt("SPINCTRL_Value2", 0);
    auto nth = SettingsMap.GetInt("SPINCTRL_NthChannel", 1);
    auto starting = SettingsMap.GetInt("SPINCTRL_StartingAt", 1);
    auto count = SettingsMap.GetInt("SPINCTRL_Count", 0);

    AdjustChannels(StartsWith(string_type, "Single Color"), num_channels, buffer, action, value1, value2, nth, starting, count);
}
