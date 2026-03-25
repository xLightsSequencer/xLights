/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <cassert>
#include <format>
#include <sstream>

#include "DuplicateEffect.h"
#include "../render/RenderBuffer.h"
#include "UtilFunctions.h"
#include "../models/Model.h"
#include "../models/ModelManager.h"
#include "../xLightsMain.h"
#include "../render/SequenceElements.h"
#include "../xLightsApp.h"

#include "../../include/Duplicate_64.xpm"
#include "../../include/Duplicate_48.xpm"
#include "../../include/Duplicate_32.xpm"
#include "../../include/Duplicate_24.xpm"
#include "../../include/Duplicate_16.xpm"

DuplicateEffect::DuplicateEffect(int i) : RenderableEffect(i, "Duplicate", Duplicate_16, Duplicate_24, Duplicate_32, Duplicate_48, Duplicate_64)
{
    //ctor
}

DuplicateEffect::~DuplicateEffect()
{
    //dtor
}

std::list<std::string> DuplicateEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res = RenderableEffect::CheckEffectSettings(settings, media, model, eff, renderCache);

    // get the sequence elements
    auto& se = model->GetModelManager().GetXLightsFrame()->GetSequenceElements();

    auto element = se.GetElement(settings.Get("E_CHOICE_Duplicate_Model", ""));

    if (element == nullptr)
    {
        res.push_back(std::format("    ERR: Duplicate effect refers to model/submodel/strand '{}' which does not exist. Effect: Duplicate, Model: {}, Start {}", settings.Get("E_CHOICE_Duplicate_Model", ""), model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
    }
    else
    {
        auto layers = settings.GetInt("E_SPINCTRL_Duplicate_Layer", 0);
        auto ec = element->GetEffectLayer(layers - 1);
        if (layers == 0 || ec == nullptr || layers > element->GetEffectLayerCount())
        {
            res.push_back(std::format("    ERR: Duplicate effect refers to layer {} on model/submodel/strand '{}' which does not exist. Effect: Duplicate, Model: {}, Start {}", layers, settings.Get("E_CHOICE_Duplicate_Model", ""), model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
        }
        else
        {
            if (!ec->HasEffects())
            {
                res.push_back(std::format("    ERR: Duplicate effect refers to layer {} on model/submodel/strand '{}' which does not have any effects. Effect: Duplicate, Model: {}, Start {}", layers, settings.Get("E_CHOICE_Duplicate_Model", ""), model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
            }
        }
    }

    return res;
}

int DuplicateEffect::GetLayersForModel(const std::string& model)
{
    int res = 1;

    auto& se = xLightsApp::GetFrame()->GetSequenceElements();

    auto element = se.GetElement(model);

    if (element != nullptr)
    {
        res = element->GetEffectLayerCount();
    }

    return res;
}

void DuplicateEffect::Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer)
{
    // This should never ever be called as the Render::ProcessFrame should have converted this effect into the effects being duplicated
    assert(false);
}

