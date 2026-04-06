/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Timing import functions extracted from render/SequenceFile.cpp

#include <wx/zipstrm.h>
#include <wx/wfstream.h>
#include <wx/mstream.h>

#include <pugixml.hpp>

#include "../render/SequenceFile.h"
#include "../render/RenderContext.h"
#include "../xLightsMain.h"
#include "UtilFunctions.h"
#include "ui/shared/utils/wxUtilities.h"
#include "utils/ExternalHooks.h"
#include "../render/RenderUtils.h"
#include "Vixen3.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <filesystem>

#include <log.h>

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
    if (auto* frame = dynamic_cast<xLightsFrame*>(xLightsParent)) {
        frame->SetCursor(wxCURSOR_WAIT);
    }

    for (size_t i = 0; i < filenames.size(); ++i)
    {
        const std::string& filepath = filenames[i];
        std::string filestem = std::filesystem::path(filepath).stem().string();
        spdlog::info("Decompressing LSP file {}", filepath);

        wxFileInputStream fin(filepath);
        wxZipInputStream zin(fin);
        wxZipEntry *ent = zin.GetNextEntry();

        pugi::xml_document seq_xml;

        while (ent != nullptr)
        {
            if (ent->GetName() == "Sequence")
            {
                spdlog::info("Extracting timing tracks from {}/{}", filepath, std::string(ent->GetName().c_str()));

                // Read the zip stream into a memory buffer for pugixml
                wxMemoryOutputStream memOut;
                zin.Read(memOut);
                wxStreamBuffer* buf = memOut.GetOutputStreamBuffer();
                seq_xml.load_buffer(buf->GetBufferStart(), buf->GetBufferSize());

                pugi::xml_node e = seq_xml.first_child();

                if (strcmp(e.name(), "MusicalSequence") == 0)
                {
                    for (pugi::xml_node tts = e.first_child(); tts; tts = tts.next_sibling())
                    {
                        if (strcmp(tts.name(), "TimingTracks") == 0)
                        {
                            for (pugi::xml_node t = tts.first_child(); t; t = t.next_sibling())
                            {
                                if (strcmp(t.name(), "Track") == 0) {
                                    std::string name = UniqueTimingName(xLightsParent, filestem);
                                    spdlog::info("  Track: {}", name);
                                    EffectLayer* effectLayer = nullptr;
                                    int present = 0;
                                    for (pugi::xml_node is = t.first_child(); is; is = is.next_sibling()) {
                                        if (strcmp(is.name(), "Intervals") == 0) {
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
                                                            // we take the only the first 4 after we have found 7s
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
                    }
                }
            }
            ent = zin.GetNextEntry();
        }
    }
    if (auto* frame = dynamic_cast<xLightsFrame*>(xLightsParent)) {
        frame->SetCursor(wxCURSOR_ARROW);
    }
}

void SequenceFile::ProcessXLightsTiming(const std::vector<std::string>& filenames, RenderContext* xLightsParent) {
    xLightsFrame* frame = dynamic_cast<xLightsFrame*>(xLightsParent);
    if (frame != nullptr) {
        frame->SetCursor(wxCURSOR_WAIT);
    }
    Element* element = nullptr;
    EffectLayer* effectLayer = nullptr;

    for (size_t i = 0; i < filenames.size(); ++i)
    {
        std::filesystem::path next_file(filenames[i]);

        spdlog::info("Loading sequence file {}", next_file.string());
        SequenceFile file(next_file.string());
        auto loadDoc = file.LoadSequence(next_file.parent_path().string(), true, next_file.string());
        if (!loadDoc) continue;

        SequenceElements se(xLightsParent);
        se.SetFrequency(file.GetFrequency());
        if (frame != nullptr) {
            se.SetViewsManager(frame->GetViewsManager()); // This must come first before LoadSequencerFile.
        }
        se.LoadSequencerFile(file, *loadDoc, xLightsParent->GetShowDirectory());
        if (frame != nullptr) {
            file.AdjustEffectSettingsForVersion(se, frame);
        }

        std::vector<TimingElement *> elements;
        wxArrayString names;
        for (size_t e = 0; e < se.GetElementCount(); e++) {
            Element *el = se.GetElement(e);
            if (el->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
                TimingElement *ti =dynamic_cast<TimingElement*>(el);
                if (ti->GetFixedTiming() == 0) {
                    elements.push_back(ti);
                    names.Add(el->GetName());
                }
            }
        }
        wxMultiChoiceDialog dlg(dynamic_cast<wxWindow*>(xLightsParent), "Select timing tracks to import", "Import Timing Tracks", names);
        if (dlg.ShowModal() == wxID_OK) {
            wxArrayInt selections = dlg.GetSelections();

            for (int i1 = 0; i1 < (int)selections.size(); i1++) {
                TimingElement *ti = elements[selections[i1]];
                if (sequence_loaded) {
                    element = xLightsParent->AddTimingElement(ti->GetName());
                } else {
                    // Pre-load: store as pending timing with starts/ends/labels
                    PendingTiming pt;
                    pt.name = ti->GetName();
                    for (int x = 0; x < (int)ti->GetEffectLayerCount(); x++) {
                        EffectLayer *src = ti->GetEffectLayer(x);
                        for (int ef = 0; ef < src->GetEffectCount(); ef++) {
                            Effect *effect = src->GetEffect(ef);
                            pt.starts.push_back(effect->GetStartTimeMS());
                            pt.ends.push_back(effect->GetEndTimeMS());
                            pt.labels.push_back(effect->GetEffectName());
                        }
                    }
                    mPendingTimings.push_back(std::move(pt));
                    timing_list.push_back(ti->GetName());
                    continue;
                }
                for (int x = 0; x < (int)ti->GetEffectLayerCount(); x++) {
                    EffectLayer *src = ti->GetEffectLayer(x);
                    effectLayer = element->GetEffectLayer(x);
                    if (effectLayer == nullptr) {
                        effectLayer = element->AddEffectLayer();
                    }
                    for (int ef = 0; ef < src->GetEffectCount(); ef++) {
                        Effect *effect = src->GetEffect(ef);
                        effectLayer->AddEffect(0, effect->GetEffectName(), "", "", effect->GetStartTimeMS(), effect->GetEndTimeMS(), EFFECT_NOT_SELECTED, false);
                    }
                }
            }
        }
    }
    if (frame != nullptr) {
        frame->SetCursor(wxCURSOR_ARROW);
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

void SequenceFile::ProcessVixen3Timing(const std::vector<std::string>& filenames, RenderContext* xLightsParent) {

    if (auto* frame = dynamic_cast<xLightsFrame*>(xLightsParent)) {
        frame->SetCursor(wxCURSOR_WAIT);
    }

    for (size_t i = 0; i < filenames.size(); ++i)
    {
        const std::string& filepath = filenames[i];

        spdlog::info("Loading Vixen 3 file {}", filepath);

        Vixen3 vixenFile(filepath);

        auto timings = vixenFile.GetTimings();
        wxArrayString markNames;
        for (auto it: timings)
        {
            markNames.push_back(it);
        }

        wxMultiChoiceDialog dlg(dynamic_cast<wxWindow*>(xLightsParent), "Select timing tracks to import", "Import Timing Tracks", markNames);

        if (dlg.ShowModal() == wxID_OK) {
            wxArrayInt selections = dlg.GetSelections();

            for (int i1 = 0; i1 < (int)selections.size(); i1++) {
                
                std::string sel = markNames[selections[i1]].ToStdString();

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
    }

    if (auto* frame = dynamic_cast<xLightsFrame*>(xLightsParent)) {
        frame->SetCursor(wxCURSOR_ARROW);
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
            //EffectLayer* effectLayerWord = element->GetEffectLayer(1);

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

