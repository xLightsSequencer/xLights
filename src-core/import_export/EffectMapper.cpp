/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "EffectMapper.h"

#include "effects/BufferStyles.h"
#include "effects/EffectManager.h"
#include "models/Model.h"
#include "models/ModelGroup.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/RenderContext.h"
#include "render/SequenceElements.h"
#include "render/SequenceMedia.h"
#include "render/SequencePackage.h"
#include "utils/string_utils.h"

#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>

#include <cstdio>

void MapXLightsEffects(EffectLayer* target, EffectLayer* src,
                       std::vector<EffectLayer*>& mapped, bool eraseExisting,
                       SequencePackage& xsqPkg, bool lock,
                       const std::map<std::string, std::string>& mapping,
                       bool convertRender,
                       const std::map<std::string, std::string>& mappingModelType)
{
    if (eraseExisting)
        target->DeleteAllEffects();

    for (int x = 0; x < src->GetEffectCount(); ++x) {
        Effect* ef = src->GetEffect(x);
        if (!target->HasEffectsInTimeRange(ef->GetStartTimeMS(), ef->GetEndTimeMS())) {
            std::string settings;
            if (xsqPkg.HasMedia() && xsqPkg.GetImportOptions()->IsImportActive()) {
                // attempt to import it and fix settings
                settings = xsqPkg.FixAndImportMedia(ef, target);
            } else {
                settings = ef->GetSettingsAsString();
            }

            // remove lock if it is there
            Replace(settings, ",X_Effect_Locked=True", "");

            // If this is a duplicate effect map the duplicate to the model the original model was mapped to
            if (ef->GetEffectIndex() == EffectManager::eff_DUPLICATE && Contains(settings, "E_CHOICE_Duplicate_Model")) {
                auto dupModel = ef->GetSettings()["E_CHOICE_Duplicate_Model"];
                auto it = mapping.find(dupModel);
                if (it != mapping.end()) {
                    Replace(settings, ",E_CHOICE_Duplicate_Model=" + dupModel, ",E_CHOICE_Duplicate_Model=" + it->second);
                }
            }
            if (ef->GetEffectIndex() == EffectManager::eff_PICTURES) {
                // if using embedded images, need to copy it over
                std::string v = ef->GetSettings()["E_TEXTCTRL_Pictures_Filename"];
                auto& sm = ef->GetParentEffectLayer()->GetParentElement()->GetSequenceElements()->GetSequenceMedia();
                auto& tm = target->GetParentElement()->GetSequenceElements()->GetSequenceMedia();
                if (sm.HasImage(v) && !tm.HasImage(v)) {
                    auto img = sm.GetImage(v);
                    if (img->IsEmbedded()) {
                        tm.AddEmbeddedImage(v, img->GetEmbeddedData());
                    }
                }
            }

            // if we are mapping the effect onto a group and it is a per preview render buffer then use the group's default camera
            //   unless there is a non-default 3D camera assigned to the effect, and it exists in the target layout
            if (!target->IsTimingLayer()) {
                RenderContext* rc = target->GetParentElement()->GetSequenceElements()->GetRenderContext();
                Model* m = rc->GetModel(target->GetParentElement()->GetModelName());
                if (m != nullptr) {
                    auto mg = dynamic_cast<const ModelGroup*>(m);
                    if (mg != nullptr) {
                        if (convertRender) {
                            auto buffer = ef->GetSettings()["B_CHOICE_BufferStyle"];
                            if (buffer == "Per Preview" || buffer == "Default" || buffer == "Single Line") {
                                Replace(settings, "B_CHOICE_BufferStyle=" + buffer,
                                        "B_CHOICE_BufferStyle=Per Model " + buffer);
                            } else if (buffer.empty()) {
                                if (Contains(settings, "B_CHOICE_BufferStyle")) {
                                    Replace(settings, "B_CHOICE_BufferStyle=",
                                            "B_CHOICE_BufferStyle=Per Model Default");
                                } else {
                                    if (!settings.empty()) {
                                        settings += ",";
                                    }
                                    settings += "B_CHOICE_BufferStyle=Per Model Default";
                                }
                            }
                        }
                        // so is it a per preview render buffer
                        auto rb = ef->GetSettings()["B_CHOICE_BufferStyle"];
                        if (BufferStyles::CanRenderBufferUseCamera(rb)) {
                            if (Contains(settings, "B_CHOICE_PerPreviewCamera")) {
                                // MoC - There isn't a way to just indicate "use group's default", so instead we grab it as
                                //   a setting for the effect.
                                // That way if the group default changes, there is no effect on old / mapped effects
                                auto newCamera = mg->GetDefaultCamera();
                                auto effCamera = ef->GetSettings()["B_CHOICE_PerPreviewCamera"];
                                if (effCamera != "2D" && effCamera != "Default" && rc->GetNamedCamera3D(effCamera)) {
                                    newCamera = effCamera;
                                }
                                Replace(settings, ",B_CHOICE_PerPreviewCamera=" + effCamera,
                                        ",B_CHOICE_PerPreviewCamera=" + newCamera);
                            } else {
                                settings += ",B_CHOICE_PerPreviewCamera=" + mg->GetDefaultCamera();
                            }
                        }
                    }
                } else {
                    spdlog::warn("MapXLightsEffects: target model '{}' not found in current layout while mapping effects.",
                                 target->GetParentElement()->GetModelName());
                }
            }

            Effect* ne = target->AddEffect(0, ef->GetEffectName(), settings, ef->GetPaletteAsString(),
                                           ef->GetStartTimeMS(), ef->GetEndTimeMS(), 0, false);
            if (lock) {
                ne->SetLocked(true);
            }
        }
    }
    mapped.push_back(src);
}

void MapXLightsStrandEffects(EffectLayer* target, const std::string& name,
                             std::map<std::string, EffectLayer*>& layerMap,
                             SequenceElements& seqEl,
                             std::vector<EffectLayer*>& mapped, bool eraseExisting,
                             SequencePackage& xsqPkg, bool lock,
                             const std::map<std::string, std::string>& mapping,
                             const std::map<std::string, std::string>& mappingModelType)
{
    EffectLayer* src = layerMap[name];
    if (src == nullptr) {
        Element* srcEl = seqEl.GetElement(name);
        if (srcEl == nullptr) {
            std::printf("Source strand %s doesn't exist\n", name.c_str());
            return;
        }
        src = srcEl->GetEffectLayer(0);
    }
    if (src != nullptr) {
        MapXLightsEffects(target, src, mapped, eraseExisting, xsqPkg, lock, mapping, false, mappingModelType);
    } else {
        std::printf("Source strand %s doesn't exist\n", name.c_str());
    }
}

void MapXLightsEffects(Element* target,
                       const std::string& name,
                       SequenceElements& seqEl,
                       std::map<std::string, Element*>& elementMap,
                       std::map<std::string, EffectLayer*>& layerMap,
                       std::vector<EffectLayer*>& mapped, bool eraseExisting,
                       SequencePackage& xsqPkg, bool lock,
                       const std::map<std::string, std::string>& mapping, bool convertRender,
                       const std::map<std::string, std::string>& mappingModelType)
{
    if (target->GetType() == ElementType::ELEMENT_TYPE_STRAND) {
        auto const strandName = fmt::format("Strand {}", ((StrandElement*)target)->GetStrand() + 1);
        spdlog::debug("Mapping xLights effect from {} to {}{}.", name, target->GetFullName(), strandName);
    } else {
        spdlog::debug("Mapping xLights effect from {} to {}.", name, target->GetFullName());
    }

    EffectLayer* src = layerMap[name];
    Element* el = elementMap[name];

    std::string srcModelType = "Unknown";
    auto it = mappingModelType.find(name);
    if (it != mappingModelType.end()) {
        srcModelType = it->second;
    }
    bool cr = convertRender;
    if (srcModelType == "ModelGroup") {
        cr = false;
    }

    if (src != nullptr) {
        MapXLightsEffects(target->GetEffectLayer(0), src, mapped, eraseExisting, xsqPkg, lock, mapping, cr, mappingModelType);
        return;
    }

    if (el == nullptr) {
        el = seqEl.GetElement(name);
    }

    if (el == nullptr) {
        spdlog::debug("Mapping xLights effect from {} to {} failed as the effect was not found in the source sequence.", name, target->GetName());
        return;
    }

    while (target->GetEffectLayerCount() < el->GetEffectLayerCount()) {
        target->AddEffectLayer();
    }
    for (size_t x = 0; x < el->GetEffectLayerCount(); ++x) {
        target->GetEffectLayer(x)->SetLayerName(el->GetEffectLayer(x)->GetLayerName());
        MapXLightsEffects(target->GetEffectLayer(x), el->GetEffectLayer(x), mapped, eraseExisting, xsqPkg, lock, mapping, cr, mappingModelType);
    }
}
