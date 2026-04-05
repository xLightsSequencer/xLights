/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ShaderPreviewGenerator.h"

#include "../../xLightsMain.h"
#include "render/SequenceMedia.h"
#include "render/SequenceElements.h"
#include "render/Element.h"
#include "render/EffectLayer.h"
#include "effects/ShaderEffect.h"

#include <atomic>

void GenerateShaderPreview(ShaderMediaCacheEntry* entry, xLightsFrame* xl) {
    if (!entry || !xl || entry->GetShaderSource().empty()) return;

    if (entry->HasPreview()) return; // already cached

    ShaderConfig* config = entry->GetShaderConfig(&xl->GetSequenceElements());
    if (!config) return;

    // Prevent re-entrancy: a second click while rendering would add jobs to
    // renderProgressInfo and the poll loop would never drain -- stuck.
    static std::atomic<bool> s_generating{false};
    {
        bool expected = false;
        if (!s_generating.compare_exchange_strong(expected, true)) {
            xl->AbortRender(); // unblock the in-flight loop so it can exit
            return;
        }
    }
    struct GenerateGuard { ~GenerateGuard() { s_generating = false; } } _guard;

    // Build settings string with default values for all shader parameters
    std::string settings = "E_0FILEPICKERCTRL_IFS=" + entry->GetFilePath();
    settings += ",E_SLIDER_Shader_Speed=100";
    settings += ",E_TEXTCTRL_Shader_Offset_X=0,E_TEXTCTRL_Shader_Offset_Y=0";
    settings += ",E_TEXTCTRL_Shader_Zoom=0,E_TEXTCTRL_Shader_LeadIn=0";

    for (const auto& parm : config->GetParms()) {
        if (!parm.ShowParm()) continue;
        switch (parm._type) {
            case ShaderParmType::SHADER_PARM_FLOAT: {
                std::string key = "E_TEXTCTRL_" + parm.GetUndecoratedId(ShaderCtrlType::SHADER_CTRL_TEXTCTRL);
                char buf[64];
                snprintf(buf, sizeof(buf), "%.4f", parm._default);
                settings += "," + key + "=" + buf;
                break;
            }
            case ShaderParmType::SHADER_PARM_BOOL: {
                std::string key = "E_CHECKBOX_" + parm.GetUndecoratedId(ShaderCtrlType::SHADER_CTRL_CHECKBOX);
                settings += "," + key + "=" + (parm._default != 0.0 ? "1" : "0");
                break;
            }
            case ShaderParmType::SHADER_PARM_LONGCHOICE: {
                std::string key = "E_CHOICE_" + parm.GetUndecoratedId(ShaderCtrlType::SHADER_CTRL_CHOICE);
                auto choices = parm.GetChoices();
                if (!choices.empty()) {
                    int idx = (int)parm._default;
                    std::string choiceStr = choices[0];
                    for (const auto& [val, str] : parm._valueOptions) {
                        if (val == idx) { choiceStr = str; break; }
                    }
                    settings += "," + key + "=" + choiceStr;
                }
                break;
            }
            case ShaderParmType::SHADER_PARM_POINT2D: {
                std::string keyBase = parm.GetUndecoratedId(ShaderCtrlType::SHADER_CTRL_TEXTCTRL);
                char bufX[64], bufY[64];
                snprintf(bufX, sizeof(bufX), "%.4f", parm._defaultPt.x);
                snprintf(bufY, sizeof(bufY), "%.4f", parm._defaultPt.y);
                settings += ",E_TEXTCTRL_" + keyBase + "X=" + bufX;
                settings += ",E_TEXTCTRL_" + keyBase + "Y=" + bufY;
                break;
            }
            default:
                break;
        }
    }

    std::string palette = "C_BUTTON_Palette1=#FFFFFF,C_BUTTON_Palette2=#FF0000,"
                          "C_CHECKBOX_Palette1=1,C_CHECKBOX_Palette2=1";

    Model* presetModel = xl->GetPresetModel();
    SequenceElements& presetElements = xl->GetPresetSequenceElements();

    Element* elem = presetElements.GetElement(presetModel->GetName());
    if (!elem) return;

    for (const auto& it : elem->GetEffectLayers()) {
        it->DeleteAllEffects();
    }
    if (elem->GetEffectLayerCount() == 0) {
        elem->AddEffectLayer();
    }

    EffectLayer* el = elem->GetEffectLayer(0);
    el->AddEffect(0, "Shader", settings, palette, 0, 1000, false, false, true);

    // Render without holding _cacheMutex (Render uses wxYield which can re-enter)
    int frameTimeMs = 50;
    size_t numFrames = 20; // 1 second at 50ms
    auto frames = xl->RenderEffectToFrames(
        presetModel, xl->GetPresetSequenceData(),
        presetElements, numFrames, frameTimeMs);

    entry->SetPreviewFrames(std::move(frames), frameTimeMs);
}
