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
#include "import_export/LOREdit.h"
#include "import_export/Vixen3.h"
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
#include <cstdlib>
#include <regex>

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

void MapS5(const EffectManager& effect_manager, int layer, EffectLayer* el, const LOREdit& lorEdit, const std::string& model, Model* m, int frequency, int offset, bool eraseExisting)
{
    if (el == nullptr)
        return;

    if (eraseExisting)
        el->DeleteAllEffects();

    bool channelBlock = (m != nullptr && m->GetDisplayAs() == DisplayAsType::ChannelBlock);

    auto st = lorEdit.GetSequencingType(model);

    if (st == loreditType::CHANNELS) {
        auto effects = lorEdit.GetChannelEffects(model, 0, m, offset);

        for (const auto& it : effects) {
            if (!el->HasEffectsInTimeRange(it.startMS, it.endMS)) {
                std::string palette = it.GetPalette();
                // for channel blocks we always use white as the colour comes from the channel block
                if (channelBlock)
                    palette = "C_BUTTON_Palette1=#ffffff,C_CHECKBOX_Palette1=1";
                std::string ef = it.GetxLightsEffect();
                if (ef != "") {
                    std::string settings = it.GetSettings(palette);
                    el->AddEffect(0, ef, settings, palette, it.startMS, it.endMS, false, false);
                }
            }
        }
    } else if (st == loreditType::TRACKS) {
        // pixel effects on a node ... not useful but whatever
        auto effects = lorEdit.GetTrackEffects(model, layer, offset);

        for (const auto& it : effects) {
            if (!el->HasEffectsInTimeRange(it.startMS, it.endMS)) {
                std::string palette = it.GetPalette();
                std::string ef = it.GetxLightsEffect();
                if (ef != "") {
                    std::string settings = it.GetSettings(palette);
                    el->AddEffect(0, ef, settings, palette, it.startMS, it.endMS, false, false);
                }
            }
        }
    }
}

void MapS5ChannelEffects(const EffectManager& effectManager, int node, EffectLayer* nl, Model* m, const LOREdit& lorEdit, const std::string& mapping, int frequency, int offset, bool eraseExisting)
{
    if (nl == nullptr)
        return;

    if (eraseExisting)
        nl->DeleteAllEffects();

    bool channelBlock = (m != nullptr && m->GetDisplayAs() == DisplayAsType::ChannelBlock);

    auto st = lorEdit.GetSequencingType(mapping);

    if (st == loreditType::CHANNELS) {
        auto effects = lorEdit.GetChannelEffects(mapping, node, m, offset);

        for (const auto& it : effects) {
            if (!nl->HasEffectsInTimeRange(it.startMS, it.endMS)) {
                std::string palette = it.GetPalette();
                // for channel blocks we always use white as the colour comes from the channel block
                if (channelBlock)
                    palette = "C_BUTTON_Palette1=#ffffff,C_CHECKBOX_Palette1=1";
                std::string ef = it.GetxLightsEffect();
                if (ef != "") {
                    std::string settings = it.GetSettings(palette);
                    nl->AddEffect(0, ef, settings, palette, it.startMS, it.endMS, false, false);
                }
            }
        }
    } else if (st == loreditType::TRACKS) {
        // pixel effects on a node ... not useful but whatever
        auto effects = lorEdit.GetTrackEffects(mapping, 0, offset);

        for (const auto& it : effects) {
            if (!nl->HasEffectsInTimeRange(it.startMS, it.endMS)) {
                std::string palette = it.GetPalette();
                std::string ef = it.GetxLightsEffect();
                if (ef != "") {
                    std::string settings = it.GetSettings(palette);
                    nl->AddEffect(0, ef, settings, palette, it.startMS, it.endMS, false, false);
                }
            }
        }
    }
}

void MapS5ChannelEffects(const EffectManager& effectManager, EffectLayer* layer, const LOREdit& lorEdit, const std::string& mapping, int frequency, int offset, bool eraseExisting)
{
    if (eraseExisting)
        layer->DeleteAllEffects();

    Model* m = layer->GetParentElement()->GetSequenceElements()->GetRenderContext()->GetModel(layer->GetParentElement()->GetModelName());
    bool channelBlock = (m != nullptr && m->GetDisplayAs() == DisplayAsType::ChannelBlock);

    static const std::regex regex(R"(\[(\d+),(\d+),(\d+)\]\[(.*)\])");
    std::smatch mm;
    if (std::regex_search(mapping, mm, regex)) {
        int const row = (int)std::strtol(mm[1].str().c_str(), nullptr, 10);
        int const col = (int)std::strtol(mm[2].str().c_str(), nullptr, 10);
        int const color = (int)std::strtol(mm[3].str().c_str(), nullptr, 10);
        std::string strColor = mm[4].str();
        std::string name = mm.prefix().str() + mm.suffix().str();

        auto effects = lorEdit.GetChannelEffects(name, row, col, color, offset);

        for (auto& it : effects) {
            if (!layer->HasEffectsInTimeRange(it.startMS, it.endMS)) {
                LOREdit::setNodeColor(strColor, it);
                std::string palette = it.GetPalette();
                // for channel blocks we always use white as the colour comes from the channel block
                if (channelBlock)
                    palette = "C_BUTTON_Palette1=#ffffff,C_CHECKBOX_Palette1=1";
                std::string ef = it.GetxLightsEffect();
                if (ef != "") {
                    std::string settings = it.GetSettings(palette);
                    layer->AddEffect(0, ef, settings, palette, it.startMS, it.endMS, false, false);
                }
            }
        }
    }
}

void MapS5ChannelEffects(const EffectManager& effectManager, int node, EffectLayer* nl, int nodes, const LOREdit& lorEdit, const std::string& mapping, int frequency, int offset, bool eraseExisting)
{
    if (nl == nullptr)
        return;

    if (eraseExisting)
        nl->DeleteAllEffects();

    Model* m = nl->GetParentElement()->GetSequenceElements()->GetRenderContext()->GetModel(nl->GetParentElement()->GetModelName());
    bool channelBlock = (m != nullptr && m->GetDisplayAs() == DisplayAsType::ChannelBlock);

    auto st = lorEdit.GetSequencingType(mapping);

    if (st == loreditType::CHANNELS) {
        auto effects = lorEdit.GetChannelEffects(mapping, node, nodes, offset);

        for (const auto& it : effects) {
            if (!nl->HasEffectsInTimeRange(it.startMS, it.endMS)) {
                std::string palette = it.GetPalette();
                // for channel blocks we always use white as the colour comes from the channel block
                if (channelBlock)
                    palette = "C_BUTTON_Palette1=#ffffff,C_CHECKBOX_Palette1=1";
                std::string ef = it.GetxLightsEffect();
                if (ef != "") {
                    std::string settings = it.GetSettings(palette);
                    nl->AddEffect(0, ef, settings, palette, it.startMS, it.endMS, false, false);
                }
            }
        }
    } else if (st == loreditType::TRACKS) {
        // pixel effects on a node ... not useful but whatever
        auto effects = lorEdit.GetTrackEffects(mapping, 0, offset);

        for (const auto& it : effects) {
            if (!nl->HasEffectsInTimeRange(it.startMS, it.endMS)) {
                std::string palette = it.GetPalette();
                std::string ef = it.GetxLightsEffect();
                if (ef != "") {
                    std::string settings = it.GetSettings(palette);
                    nl->AddEffect(0, ef, settings, palette, it.startMS, it.endMS, false, false);
                }
            }
        }
    }
}

void MapS5Effects(const EffectManager& effectManager, Element* model, const LOREdit& lorEdit, const std::string& mapping, int frequency, int offset, bool eraseExisting)
{
    auto st = lorEdit.GetSequencingType(mapping);
    Model* m = model->GetSequenceElements()->GetRenderContext()->GetModel(model->GetModelName());

    if (st == loreditType::CHANNELS) {
        if (m->GetNodeCount() == 1) {
            MapS5ChannelEffects(effectManager, 0, model->GetEffectLayer(0), m, lorEdit, mapping, frequency, offset, eraseExisting);
        } else {
            int lr, lc;
            lorEdit.GetModelChannels(mapping, lr, lc);

            if (lr == 1 && lc == 1) {
                MapS5ChannelEffects(effectManager, 0, model->GetEffectLayer(0), m, lorEdit, mapping, frequency, offset, eraseExisting);
            } else {
                for (uint32_t i = 0; i < m->GetNodeCount(); i++) {
                    NodeLayer* nl = model->GetNodeEffectLayer(i);
                    if (nl != nullptr) {
                        MapS5ChannelEffects(effectManager, i, nl, m, lorEdit, mapping, frequency, offset, eraseExisting);
                    }
                }
            }
        }
    } else if (st == loreditType::TRACKS) {
        for (int i = 0; i < lorEdit.GetModelLayers(mapping); i++) {
            if ((int)model->GetEffectLayerCount() < i + 1) {
                model->AddEffectLayer();
            }
            MapS5(effectManager, i, model->GetEffectLayer(i), lorEdit, mapping, m, frequency, offset, eraseExisting);
        }
    }
}

void MapS5Effects(const EffectManager& effectManager, StrandElement* se, const LOREdit& lorEdit, const std::string& mapping, int frequency, int offset, bool eraseExisting)
{
    auto st = lorEdit.GetSequencingType(mapping);
    Model* m = se->GetSequenceElements()->GetRenderContext()->GetModel(se->GetModelName());

    if (st == loreditType::CHANNELS) {
        if (se->GetNodeLayerCount() == 1) {
            MapS5ChannelEffects(effectManager, 0, se->GetEffectLayer(0), 1, lorEdit, mapping, frequency, offset, eraseExisting);
        } else {
            int lr, lc;
            lorEdit.GetModelChannels(mapping, lr, lc);

            if (lr == 1 && lc == 1) {
                MapS5ChannelEffects(effectManager, 0, se->GetEffectLayer(0), 1, lorEdit, mapping, frequency, offset, eraseExisting);
            } else {
                int nodes = se->GetNodeLayerCount();
                for (int i = 0; i < nodes; i++) {
                    NodeLayer* nl = se->GetNodeEffectLayer(i);
                    if (nl != nullptr) {
                        MapS5ChannelEffects(effectManager, i, nl, nodes, lorEdit, mapping, frequency, offset, eraseExisting);
                    }
                }
            }
        }
    } else if (st == loreditType::TRACKS) {
        for (int i = 0; i < lorEdit.GetModelLayers(mapping); i++) {
            if ((int)se->GetEffectLayerCount() < i + 1) {
                se->AddEffectLayer();
            }
            MapS5(effectManager, i, se->GetEffectLayer(i), lorEdit, mapping, m, frequency, offset, eraseExisting);
        }
    }
}

void MapVixen3(Element* model, const Vixen3& vixen, const std::string& modelName, long offset, int frameMS, bool eraseExisting, int startLayer)
{
    if (eraseExisting) {
        for (const auto& it : model->GetEffectLayers()) {
            it->DeleteAllEffects();
        }
    }

    auto effects = vixen.GetEffects(modelName);

    for (const auto& it : effects) {
        long s = Vixen3::ConvertTiming(it.start + offset, frameMS);
        long e = Vixen3::ConvertTiming(it.end + offset, frameMS);

        // Vixen can have multiple effects in one time slot so add layers as needed
        EffectLayer* layer = nullptr;
        for (size_t li = startLayer; li < model->GetEffectLayerCount(); ++li) {
            if (!model->GetEffectLayer(li)->HasEffectsInTimeRange(s, e)) {
                layer = model->GetEffectLayer(li);
                break;
            }
        }

        if (layer == nullptr)
            layer = model->AddEffectLayer();

        // now we need to create the effect
        std::string newpalette = it.GetPalette();
        std::string newsettings = it.GetSettings();
        std::string type = it.GetXLightsType();
        if (type != "") {
            if (layer->GetParentElement()->GetSequenceElements()->GetEffectManager().GetEffectIndex(type) < 0) {
                spdlog::debug("Vixen 3 import {} -> {} is not a valid effect.", it.type, type);
            } else {
                layer->AddEffect(0, type, newsettings, newpalette, s, e, false, false);
            }
        }
    }
}

void MapVixen3Effects(const EffectManager& effectManager, Element* model, const Vixen3& vixen, const std::string& mapping, long offset, int frameMS, bool eraseExisting, int startLayer)
{
    spdlog::debug("Creating effects on model {} from {}", model->GetFullName(), mapping);
    MapVixen3(model, vixen, mapping, offset, frameMS, eraseExisting, startLayer);
}

void MapS5Effects(const EffectManager& effectManager, SubModelElement* se, const LOREdit& lorEdit, const std::string& mapping, int frequency, int offset, bool eraseExisting)
{
    if (dynamic_cast<StrandElement*>(se) != nullptr) {
        return MapS5Effects(effectManager, dynamic_cast<StrandElement*>(se), lorEdit, mapping, frequency, offset, eraseExisting);
    }

    auto st = lorEdit.GetSequencingType(mapping);
    Model* m = se->GetSequenceElements()->GetRenderContext()->GetModel(se->GetModelName());

    if (st == loreditType::CHANNELS) {
        if (m->GetNodeCount() == 1) {
            MapS5ChannelEffects(effectManager, 0, se->GetEffectLayer(0), m, lorEdit, mapping, frequency, offset, eraseExisting);
        } else {
            int lr, lc;
            lorEdit.GetModelChannels(mapping, lr, lc);

            if (lr == 1 && lc == 1) {
                MapS5ChannelEffects(effectManager, 0, se->GetEffectLayer(0), m, lorEdit, mapping, frequency, offset, eraseExisting);
            } else {
                for (uint32_t i = 0; i < m->GetNodeCount(); i++) {
                    NodeLayer* nl = se->GetNodeEffectLayer(i);
                    if (nl != nullptr) {
                        MapS5ChannelEffects(effectManager, i, nl, m, lorEdit, mapping, frequency, offset, eraseExisting);
                    }
                }
            }
        }
    } else if (st == loreditType::TRACKS) {
        for (int i = 0; i < lorEdit.GetModelLayers(mapping); i++) {
            if ((int)se->GetEffectLayerCount() < i + 1) {
                se->AddEffectLayer();
            }
            MapS5(effectManager, i, se->GetEffectLayer(i), lorEdit, mapping, m, frequency, offset, eraseExisting);
        }
    }
}
