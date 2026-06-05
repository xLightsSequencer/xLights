/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Timing import functions (LSP / xLights / Vixen3 / ElevenLabs) — wx-free
// core implementations so both the desktop and iPad clients share them.
// The xLights and Vixen3 importers are two-step (Get*TimingTrackNames +
// Process*) so the per-track chooser can be hosted by whichever UI drives
// the import (desktop wxMultiChoiceDialog / iPad multi-select sheet).

#include "render/SequenceFile.h"
#include "render/SequenceElements.h"
#include "render/RenderContext.h"
#include "render/Element.h"
#include "render/EffectLayer.h"
#include "render/Effect.h"
#include "render/RenderUtils.h"
#include "import_export/Vixen3.h"
#include "utils/ZipUtils.h"
#include "utils/ExternalHooks.h"

#include <pugixml.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <list>

static std::string DecodeLSPTTColour(int att)
{
    switch (att)
    {
    case 1:    return "x";
    case 2:    return "R";
    case 4:    return "G";
    case 8:    return "B";
    case 16:   return "Y";
    case 32:   return "P";
    case 64:   return "O";
    case 128:  return "z";
    case 256:  return "Go";
    case 512:  return "W";
    case 1024: return "System";
    default:   break;
    }
    return std::to_string(att);
}

void SequenceFile::ProcessLSPTiming(const std::vector<std::string>& filenames, RenderContext* xLightsParent)
{
    for (size_t i = 0; i < filenames.size(); ++i)
    {
        const std::string& filepath = filenames[i];
        std::string filestem = std::filesystem::path(filepath).stem().string();
        spdlog::info("Decompressing LSP file {}", filepath);

        auto seqXmlData = ZipUtils::ReadEntry(filepath, "Sequence");
        if (!seqXmlData) {
            spdlog::warn("LSP file {} had no 'Sequence' entry", filepath);
            continue;
        }
        spdlog::info("Extracting timing tracks from {}/Sequence", filepath);

        pugi::xml_document seq_xml;
        seq_xml.load_buffer(seqXmlData->data(), seqXmlData->size());

        pugi::xml_node e = seq_xml.first_child();
        if (strcmp(e.name(), "MusicalSequence") != 0) {
            continue;
        }

        for (pugi::xml_node tts = e.first_child(); tts; tts = tts.next_sibling())
        {
            if (strcmp(tts.name(), "TimingTracks") != 0) {
                continue;
            }
            for (pugi::xml_node t = tts.first_child(); t; t = t.next_sibling())
            {
                if (strcmp(t.name(), "Track") != 0) {
                    continue;
                }
                std::string name = UniqueTimingName(xLightsParent, filestem);
                spdlog::info("  Track: {}", name);
                EffectLayer* effectLayer = nullptr;
                int present = 0;
                for (pugi::xml_node is = t.first_child(); is; is = is.next_sibling()) {
                    if (strcmp(is.name(), "Intervals") != 0) {
                        continue;
                    }
                    for (pugi::xml_node ti = is.first_child(); ti; ti = ti.next_sibling()) {
                        if (strcmp(ti.name(), "TimeInterval") == 0) {
                            if (std::string(ti.attribute("eff").as_string("")) == "7") {
                                present |= ti.attribute("att").as_int(0);
                            }
                        }
                    }

                    int mask = 1;
                    for (size_t i1 = 0; i1 < 10; i1++) {
                        if (present & mask) {
                            std::string tname = UniqueTimingName(xLightsParent, DecodeLSPTTColour(mask) + "-" + name);
                            spdlog::info("  Adding timing track {}({})", tname, mask);
                            Element* element = xLightsParent->AddTimingElement(tname);
                            effectLayer = element->GetEffectLayer(0);

                            int last = 0;
                            bool sevenfound = false;
                            bool fourfound = false;
                            for (pugi::xml_node ti = is.first_child(); ti; ti = ti.next_sibling()) {
                                if (strcmp(ti.name(), "TimeInterval") == 0) {
                                    if (std::string(ti.attribute("eff").as_string("")) == "7" && (ti.attribute("att").as_int(0) & mask)) {
                                        sevenfound = true;
                                        int start = last;
                                        int end = RoundToMultipleOfPeriod((int)(ti.attribute("pos").as_double(0.0) * 50.0 / 4410.0), GetFrequency());
                                        if (start != end)
                                        {
                                            effectLayer->AddEffect(0, "", "", "", start, end, EFFECT_NOT_SELECTED, false);
                                            last = end;
                                        }
                                    }
                                    // we take only the first 4 after we have found 7s
                                    else if (std::string(ti.attribute("eff").as_string("")) == "4" && sevenfound && !fourfound && (ti.attribute("att").as_int(0) & mask)) {
                                        fourfound = true;
                                        int start = last;
                                        int end = RoundToMultipleOfPeriod((int)(ti.attribute("pos").as_double(0.0) * 50.0 / 4410.0), GetFrequency());
                                        if (start != end)
                                        {
                                            effectLayer->AddEffect(0, "", "", "", start, end, EFFECT_NOT_SELECTED, false);
                                            last = end;
                                        }
                                    }
                                }
                            }
                        }
                        mask = mask << 1;
                    }
                }
            }
        }
    }
}

std::vector<std::string> SequenceFile::GetXLightsTimingTrackNames(const std::string& filename, RenderContext* xLightsParent)
{
    std::vector<std::string> names;

    std::filesystem::path next_file(filename);
    spdlog::info("Loading sequence file {}", next_file.string());
    SequenceFile file(next_file.string());
    auto loadDoc = file.LoadSequence(next_file.parent_path().string(), true, next_file.string());
    if (!loadDoc) {
        return names;
    }

    SequenceElements se(xLightsParent);
    se.SetFrequency(file.GetFrequency());
    if (auto* vm = xLightsParent->GetSequenceElements().GetViewsManager()) {
        se.SetViewsManager(vm); // must come before LoadSequencerFile
    }
    se.LoadSequencerFile(file, *loadDoc, xLightsParent->GetShowDirectory());
    file.AdjustEffectSettingsForVersion(se, xLightsParent);

    for (size_t e = 0; e < se.GetElementCount(); e++) {
        Element* el = se.GetElement(e);
        if (el->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
            TimingElement* ti = dynamic_cast<TimingElement*>(el);
            if (ti != nullptr && ti->GetFixedTiming() == 0) {
                names.push_back(el->GetName());
            }
        }
    }
    return names;
}

void SequenceFile::ProcessXLightsTiming(const std::string& filename, const std::vector<int>& selectedIndices, RenderContext* xLightsParent)
{
    std::filesystem::path next_file(filename);
    spdlog::info("Loading sequence file {}", next_file.string());
    SequenceFile file(next_file.string());
    auto loadDoc = file.LoadSequence(next_file.parent_path().string(), true, next_file.string());
    if (!loadDoc) {
        return;
    }

    SequenceElements se(xLightsParent);
    se.SetFrequency(file.GetFrequency());
    if (auto* vm = xLightsParent->GetSequenceElements().GetViewsManager()) {
        se.SetViewsManager(vm); // must come before LoadSequencerFile
    }
    se.LoadSequencerFile(file, *loadDoc, xLightsParent->GetShowDirectory());
    file.AdjustEffectSettingsForVersion(se, xLightsParent);

    std::vector<TimingElement*> elements;
    for (size_t e = 0; e < se.GetElementCount(); e++) {
        Element* el = se.GetElement(e);
        if (el->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
            TimingElement* ti = dynamic_cast<TimingElement*>(el);
            if (ti != nullptr && ti->GetFixedTiming() == 0) {
                elements.push_back(ti);
            }
        }
    }

    // Empty selection imports every enumerated track; otherwise the indices
    // are positions into the GetXLightsTimingTrackNames() ordering.
    std::vector<int> indices = selectedIndices;
    if (indices.empty()) {
        for (int k = 0; k < (int)elements.size(); ++k) {
            indices.push_back(k);
        }
    }

    for (int idx : indices) {
        if (idx < 0 || idx >= (int)elements.size()) {
            continue;
        }
        TimingElement* ti = elements[idx];
        if (sequence_loaded) {
            Element* element = xLightsParent->AddTimingElement(ti->GetName());
            for (int x = 0; x < (int)ti->GetEffectLayerCount(); x++) {
                EffectLayer* src = ti->GetEffectLayer(x);
                EffectLayer* effectLayer = element->GetEffectLayer(x);
                if (effectLayer == nullptr) {
                    effectLayer = element->AddEffectLayer();
                }
                for (int ef = 0; ef < src->GetEffectCount(); ef++) {
                    Effect* effect = src->GetEffect(ef);
                    effectLayer->AddEffect(0, effect->GetEffectName(), "", "", effect->GetStartTimeMS(), effect->GetEndTimeMS(), EFFECT_NOT_SELECTED, false);
                }
            }
        } else {
            // Pre-load: store as pending timing with starts/ends/labels
            PendingTiming pt;
            pt.name = ti->GetName();
            for (int x = 0; x < (int)ti->GetEffectLayerCount(); x++) {
                EffectLayer* src = ti->GetEffectLayer(x);
                for (int ef = 0; ef < src->GetEffectCount(); ef++) {
                    Effect* effect = src->GetEffect(ef);
                    pt.starts.push_back(effect->GetStartTimeMS());
                    pt.ends.push_back(effect->GetEndTimeMS());
                    pt.labels.push_back(effect->GetEffectName());
                }
            }
            mPendingTimings.push_back(std::move(pt));
            timing_list.push_back(ti->GetName());
        }
    }
}

void AddVixenMarksToLayer(const std::list<VixenTiming>& marks, EffectLayer* effectLayer, int frameMS) {
    int32_t last = 0;
    for (const auto& it : marks)
    {
        int st = Vixen3::ConvertTiming(it.start, frameMS);
        int en = Vixen3::ConvertTiming(it.end, frameMS);

        if (st < last) st = last;
        if (st < en)
        {
            effectLayer->AddEffect(0, it.label, "", "", st, en, EFFECT_NOT_SELECTED, false);
            last = en;
        }
        else
        {
            // Timing mark dropped because we could not fit it in
        }
    }
}

std::vector<std::string> SequenceFile::GetVixen3TimingTrackNames(const std::string& filename)
{
    std::vector<std::string> names;
    spdlog::info("Loading Vixen 3 file {}", filename);
    Vixen3 vixenFile(filename);
    for (const auto& it : vixenFile.GetTimings()) {
        names.push_back(it);
    }
    return names;
}

void SequenceFile::ProcessVixen3Timing(const std::string& filename, const std::vector<int>& selectedIndices, RenderContext* xLightsParent)
{
    spdlog::info("Loading Vixen 3 file {}", filename);
    Vixen3 vixenFile(filename);

    std::vector<std::string> names;
    for (const auto& it : vixenFile.GetTimings()) {
        names.push_back(it);
    }

    std::vector<int> indices = selectedIndices;
    if (indices.empty()) {
        for (int k = 0; k < (int)names.size(); ++k) {
            indices.push_back(k);
        }
    }

    for (int idx : indices) {
        if (idx < 0 || idx >= (int)names.size()) {
            continue;
        }
        const std::string& sel = names[idx];

        if (vixenFile.GetTimingType(sel) == "Phrase")
        {
            TimingElement* element = xLightsParent->AddTimingElement(UniqueTimingName(xLightsParent, sel));
            EffectLayer* effectLayer = element->GetEffectLayer(0);
            if (effectLayer == nullptr) {
                effectLayer = element->AddEffectLayer();
            }

            AddVixenMarksToLayer(vixenFile.GetTimings(sel), effectLayer, GetFrameMS());
            effectLayer = element->AddEffectLayer();
            AddVixenMarksToLayer(vixenFile.GetRelatedTiming(sel, "Word"), effectLayer, GetFrameMS());
            effectLayer = element->AddEffectLayer();
            AddVixenMarksToLayer(vixenFile.GetRelatedTiming(sel, "Phoneme"), effectLayer, GetFrameMS());
        }
        else
        {
            TimingElement* element = xLightsParent->AddTimingElement(UniqueTimingName(xLightsParent, sel));
            EffectLayer* effectLayer = element->GetEffectLayer(0);
            if (effectLayer == nullptr) {
                effectLayer = element->AddEffectLayer();
            }

            AddVixenMarksToLayer(vixenFile.GetTimings(sel), effectLayer, GetFrameMS());
        }
    }
}

void SequenceFile::ProcessElevenLabsTimingFiles(const std::vector<std::string>& filenames, RenderContext* xLightsParent) {
    for (const std::string &nf : filenames) {
        try {
            const std::filesystem::path next_file(nf);
            auto roudTimestoMilli = [&](float start, float end) {
                int const startTime = RoundToMultipleOfPeriod((int)(start * 1000.0F), GetFrequency());
                int endTime = RoundToMultipleOfPeriod((int)(end * 1000.0F), GetFrequency());
                if (startTime == endTime) {
                    endTime = RoundToMultipleOfPeriod(startTime + GetFrequency(), GetFrequency());
                }
                return std::make_pair(startTime, endTime);
            };

            std::ifstream f(next_file);
            if (!f.is_open()) {
                return;
            }

            std::string filename = next_file.stem().string();

            while (TimingAlreadyExists(filename, xLightsParent)) {
                filename += "_1";
            }

            nlohmann::json data;
            std::ifstream inputFile(next_file);
            inputFile >> data;

            Element* element = xLightsParent->AddTimingElement(filename);
            EffectLayer* effectLayerPhrase = element->GetEffectLayer(0);
            EffectLayer* effectLayerWord = element->AddEffectLayer();

            // Read all lines
            for (auto const& segments : data["segments"]) {
                auto const start_time = segments["start_time"].get<float>();
                auto const end_time = segments["end_time"].get<float>();

                auto [startTime, endTime] = roudTimestoMilli(start_time, end_time);
                effectLayerPhrase->AddEffect(0, segments["text"].get<std::string>(), "", "", startTime, endTime, EFFECT_NOT_SELECTED, false);
                for (auto const& word : segments["words"]) {
                    auto const cword = word["text"].get<std::string>();
                    if (cword.empty()) {
                        continue;
                    }
                    bool const hasText = std::any_of(cword.begin(), cword.end(), [](unsigned char ch) { return !std::isspace(ch); });
                    if (!hasText) {
                        continue;
                    }
                    auto const word_start_time = word["start_time"].get<float>();
                    auto const word_end_time = word["end_time"].get<float>();
                    auto [wordStartTime, wordEndTime] = roudTimestoMilli(word_start_time, word_end_time);
                    effectLayerWord->AddEffect(0, cword, "", "", wordStartTime, wordEndTime, EFFECT_NOT_SELECTED, false);
                }
            }
        } catch (const std::exception& ex) {
            spdlog::error("Error processing timing file {}: {}", nf, ex.what());
        }
    }
}
