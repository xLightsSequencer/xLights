/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <sstream>

#include "DuplicateEffect.h"
#include "DuplicatePanel.h"
#include "../RenderBuffer.h"
#include "UtilFunctions.h"
#include "../models/Model.h"
#include "../models/ModelManager.h"
#include "../xLightsMain.h"
#include "../sequencer/SequenceElements.h"
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
    std::list<std::string> res;

    // get the sequence elements
    auto& se = model->GetModelManager().GetXLightsFrame()->GetSequenceElements();

    auto element = se.GetElement(settings.Get("E_CHOICE_Duplicate_Model", ""));

    if (element == nullptr)
    {
        res.push_back(wxString::Format("    ERR: Duplicate effect refers to model/submodel/strand '%s' which does not exist. Effect: Duplicate, Model: %s, Start %s", settings.Get("E_CHOICE_Duplicate_Model", ""), model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }
    else
    {
        auto layers = settings.GetInt("E_SPINCTRL_Duplicate_Layer", 0);
        auto ec = element->GetEffectLayer(layers - 1);
        if (layers == 0 || ec == nullptr || layers > element->GetEffectLayerCount())
        {
            res.push_back(wxString::Format("    ERR: Duplicate effect refers to layer %d on model/submodel/strand '%s' which does not exist. Effect: Duplicate, Model: %s, Start %s", layers, settings.Get("E_CHOICE_Duplicate_Model", ""), model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
        }
        else
        {
            if (!ec->HasEffects())
            {
                res.push_back(wxString::Format("    ERR: Duplicate effect refers to layer %d on model/submodel/strand '%s' which does not have any effects. Effect: Duplicate, Model: %s, Start %s", layers, settings.Get("E_CHOICE_Duplicate_Model", ""), model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
            }
        }
    }

    return res;
}

int DuplicateEffect::GetLayersForModel(const wxString& model)
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

xlEffectPanel *DuplicateEffect::CreatePanel(wxWindow *parent) {
    return new DuplicatePanel(parent);
}

void DuplicateEffect::SetDefaultParameters() {
    DuplicatePanel *p = (DuplicatePanel*)panel;

    SetChoiceValue(p->Choice_Model, "");
    SetCheckBoxValue(p->CheckBox1, false);
    SetCheckBoxValue(p->CheckBox2, false);
    SetCheckBoxValue(p->CheckBox3, false);
    SetCheckBoxValue(p->CheckBox_Override_Palette, false);
    SetSpinValue(p->SpinCtrl_Layer, 1);
}

void DuplicateEffect::SetPanelStatus(Model* cls)
{
    DuplicatePanel* dp = (DuplicatePanel*)panel;
    if (dp == nullptr)
        return;

    // remove all the models
    dp->Choice_Model->Clear();

    // get the sequence elements
    auto& se = cls->GetModelManager().GetXLightsFrame()->GetSequenceElements();

    for (const auto& it : se.GetAllElementNamesWithEffectsExtended())
    {
        if (it != cls->GetFullName()) dp->Choice_Model->AppendString(it);
    }
}

void DuplicateEffect::Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer)
{
    // This should never ever be called as the Render::ProcessFrame should have converted this effect into the effects being duplicated
    wxASSERT(false);
}

