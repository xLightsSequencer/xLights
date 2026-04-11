/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "../utils/Base64.h"
#include <zstd.h>

#include "../utils/XsqFileScanner.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <spdlog/fmt/fmt.h>
#include <fstream>
#include <sstream>
#include <string_view>
#include <unordered_map>

#include "SequenceFile.h"
#include "SequenceElements.h"
#include "RenderContext.h"
#include "UICallbacks.h"
#include "media/AudioManager.h"
#include "../effects/EffectManager.h"
#include "ValueCurve.h"
#include "../effects/RenderableEffect.h"
#include "xLightsVersion.h"
#include "UtilFunctions.h"
#include "../utils/AppCallbacks.h"
#include "../utils/string_utils.h"
#include "../utils/FileUtils.h"
#include "RenderUtils.h"
#include "utils/ExternalHooks.h"

#include <log.h>

//     #define USE_COMPRESSION

const std::string SequenceFile::ERASE_MODE = "<rendered: erase-mode>";
const std::string SequenceFile::CANVAS_MODE = "<rendered: canvas-mode>";

SequenceFile::SequenceFile(const std::string& filepath, uint32_t frameMS) :
    mFilePath(filepath),
    seq_duration(30.0),
    seq_type("Animation"),
    seq_timing("50 ms"),
    supports_model_blending(true),
    is_open(false),
    was_converted(false),
    sequence_loaded(false),
    audio(nullptr)
{
    if (frameMS != 0) {
        seq_timing = std::to_string(frameMS) + " ms";
    }
    CreateNew();
}

SequenceFile::~SequenceFile()
{
    models.clear();
    timing_list.clear();
    if (audio != nullptr) {
        ValueCurve::SetAudio(nullptr);
        delete audio;
        audio = nullptr;
    }
}

std::string SequenceFile::GetExt() const
{
    auto ext = std::filesystem::path(mFilePath).extension().string();
    if (!ext.empty() && ext[0] == '.') {
        return ext.substr(1);
    }
    return ext;
}

void SequenceFile::SetExt(const std::string& ext)
{
    std::filesystem::path p(mFilePath);
    p.replace_extension(ext);
    mFilePath = p.string();
}

bool SequenceFile::FileExists() const
{
    return ::FileExists(mFilePath);
}

bool SequenceFile::IsXmlSequence(const std::string& filepath)
{
    std::ifstream file(filepath, std::ios::binary);
    if (!file) return false;
    char buf[1024];
    file.read(buf, sizeof(buf));
    std::string_view sv(buf, file.gcount());
    return sv.find("<xsequence") != std::string_view::npos;
}

bool SequenceFile::IsV3Sequence() const
{
    std::ifstream file(GetFullPath(), std::ios::binary);
    if (!file) return false;
    char buf[1024];
    file.read(buf, sizeof(buf));
    std::string_view sv(buf, file.gcount());
    return sv.find("<xsequence") != std::string_view::npos &&
           sv.find("<tr>") != std::string_view::npos;
}

bool SequenceFile::NeedsTimesCorrected() const
{
    std::ifstream file(GetFullPath(), std::ios::binary);
    if (!file) return true;
    char buf[1024];
    file.read(buf, sizeof(buf));
    std::string_view sv(buf, file.gcount());
    return !(sv.find("<xsequence") != std::string_view::npos &&
             sv.find("FixedPointTiming") != std::string_view::npos);
}

// GetLastView() is now inline in the header, returning mLastView

// GetPalettesNode removed — use SequenceElements::GetColorPalettes()

bool SequenceFile::SaveCopy() const
{
    std::filesystem::path archive_dir = std::filesystem::path(GetPath()) / "ArchiveV3";

    std::error_code ec;
    if (!std::filesystem::exists(archive_dir, ec)) {
        if (!std::filesystem::create_directory(archive_dir, ec)) return false;
    }

    std::filesystem::rename(GetFullPath(), archive_dir / GetFullName(), ec);
    return !ec;
}

void SequenceFile::SetSequenceType(const std::string& type)
{
    seq_type = type;
    if (type == "Animation" || type == "Effect") {
        SetMediaFile("", "", false);
        if (audio != nullptr) {
            ValueCurve::SetAudio(nullptr);
            delete audio;
            audio = nullptr;
        }
    }
}

int SequenceFile::GetFrameMS() const
{
    return std::strtol(seq_timing.c_str(), nullptr, 10);
}

int SequenceFile::GetFrequency() const
{
    int freq_ms = std::strtol(seq_timing.c_str(), nullptr, 10);
    return freq_ms > 0 ? (int)(1000 / freq_ms) : 20;
}

void SequenceFile::SetSequenceTiming(const std::string& timing)
{
    spdlog::info("Sequence timing set to " + timing);
    seq_timing = timing;
}

void SequenceFile::SetMediaFile(const std::string& ShowDir, const std::string& filename, bool overwrite_tags)
{
    media_file = FileUtils::FixFile(ShowDir, filename);

    if (audio != nullptr) {
        ValueCurve::SetAudio(nullptr);
        delete audio;
        audio = nullptr;
    }

    ObtainAccessToURL(filename);
    std::error_code ec;
    auto perms = std::filesystem::status(filename, ec).permissions();
    bool readable = !ec && (perms & std::filesystem::perms::owner_read) != std::filesystem::perms::none;
    if (!filename.empty() && ::FileExists(filename) && readable) {
        spdlog::debug("SetMediaFile: Creating audio manager");
        audio = new AudioManager(filename, GetFrameMS());

        if (audio != nullptr) {
            ValueCurve::SetAudio(audio);
            spdlog::info("SetMediaFile: Audio loaded. Audio frame interval {}ms. Our frame interval {}ms", audio->GetFrameInterval(), GetFrameMS());
            if (audio->GetFrameInterval() < 0 && GetFrameMS() > 0) {
                audio->SetFrameInterval(GetFrameMS());
            }
        }
    }

    if (overwrite_tags) {
        SetMetaMP3Tags();
    }
}

void SequenceFile::ClearMediaFile()
{
    if (audio != nullptr) {
        ValueCurve::SetAudio(nullptr);
        delete audio;
        audio = nullptr;
    }

    media_file = "";
    SetHeaderInfo(HEADER_INFO_TYPES::SONG, "");
    SetHeaderInfo(HEADER_INFO_TYPES::ARTIST, "");
    SetHeaderInfo(HEADER_INFO_TYPES::ALBUM, "");
    SetHeaderInfo(HEADER_INFO_TYPES::URL, "");
}

void SequenceFile::SetRenderMode(const std::string& mode)
{
    for (int i = 0; i < mDataLayers.GetNumLayers(); i++) {
        if (mDataLayers.GetDataLayer(i)->GetName() == "Nutcracker") {
            mDataLayers.GetDataLayer(i)->SetDataSource(mode);
            return;
        }
    }
}

std::string SequenceFile::GetRenderMode()
{
    for (int i = 0; i < mDataLayers.GetNumLayers(); i++) {
        if (mDataLayers.GetDataLayer(i)->GetName() == "Nutcracker") {
            return mDataLayers.GetDataLayer(i)->GetDataSource();
        }
    }
    return ERASE_MODE;
}

const std::string& SequenceFile::GetHeaderInfo(HEADER_INFO_TYPES node_type) const
{
    return header_info[static_cast<int>(node_type)];
}

void SequenceFile::SetHeaderInfo(HEADER_INFO_TYPES name_name, const std::string& node_value)
{
    header_info[static_cast<int>(name_name)] = node_value;
}

void SequenceFile::SetTimingSectionName(const std::string& section, const std::string& name)
{
    auto it = std::find(timing_list.begin(), timing_list.end(), section);
    if (it != timing_list.end()) {
        *it = name;
    }
}

void SequenceFile::DeleteTimingSection(const std::string& section)
{
    timing_list.erase(std::remove(timing_list.begin(), timing_list.end(), section), timing_list.end());
}

void SequenceFile::CreateNew()
{
    version_string = xlights_version_string;
    supports_model_blending = true;
    mDataLayers.AddDataLayer("Nutcracker", "<auto-generated>", ERASE_MODE);
}

std::optional<pugi::xml_document> SequenceFile::Open(const std::string& ShowDir, bool ignore_audio, const std::string& realFilePath)
{
    if (!FileExists())
        return std::nullopt;

    sequence_loaded = false;
    if (IsV3Sequence()) {
        DisplayError("Loading of xLights v3 Sequences is no longer supported.");
        return std::nullopt;
    }
    else if (IsXmlSequence(mFilePath)) {
        return LoadSequence(ShowDir, ignore_audio, realFilePath);
    }
    return std::nullopt;
}

// ConvertToFixedPointTiming removed — conversion now done inline during pugixml LoadSequence

// UpdateMediaFileInXML removed - XML is now only built at save time

std::optional<pugi::xml_document> SequenceFile::LoadSequence(const std::string& ShowDir, bool ignore_audio, const std::string& realFilePath)
{
    if (realFilePath != mFilePath) {
        spdlog::info("LoadSequence: Loading sequence {} from {}", mFilePath, realFilePath);
    } else {
        spdlog::info("LoadSequence: Loading sequence {}", mFilePath);
    }

    // Load with pugixml
    pugi::xml_document loadDoc;
    auto result = loadDoc.load_file(realFilePath.c_str());
    if (!result) {
        spdlog::error("LoadSequence: XML file load failed: {}", result.description());
        return std::nullopt;
    }
    is_open = true;

    auto root = loadDoc.child("xsequence");
    if (!root) {
        // Try root element directly (the document root IS xsequence)
        root = loadDoc.first_child();
    }

    // Handle CompressedData sections
    for (auto e = root.child("CompressedData"); e; ) {
        int size = e.attribute("size").as_int(0);
        std::string b64Content = e.text().as_string("");
        std::vector<uint8_t> decoded = Base64::Decode(b64Content);
        std::vector<uint8_t> bytes(size + 50);
        int sz = ZSTD_decompress(bytes.data(), bytes.size(), decoded.data(), decoded.size());

        pugi::xml_document tempDoc;
        tempDoc.load_buffer(bytes.data(), sz);
        // Copy top-level elements from decompressed doc into our root
        for (auto child = tempDoc.first_child(); child; ) {
            auto next = child.next_sibling();
            root.append_copy(child);
            child = next;
        }
        auto next = e.next_sibling("CompressedData");
        root.remove_child(e);
        e = next;
    }

    supports_model_blending = std::string(root.attribute("ModelBlending").as_string("false")) == "true";

    bool needsTimesCorrection = NeedsTimesCorrected();

    std::string mediaFileName;
    const std::string& showDir = ShowDir;

    for (auto e : root.children()) {
        std::string ename = e.name();
        if (ename == "head") {
            for (auto element : e.children()) {
                std::string name = element.name();
                std::string content = element.text().as_string("");
                if (name == "version") {
                    version_string = content;
                } else if (name == "author") {
                    header_info[(int)HEADER_INFO_TYPES::AUTHOR] = UnXmlSafe(content);
                } else if (name == "author-email") {
                    header_info[(int)HEADER_INFO_TYPES::AUTHOR_EMAIL] = UnXmlSafe(content);
                } else if (name == "author-website") {
                    header_info[(int)HEADER_INFO_TYPES::WEBSITE] = UnXmlSafe(content);
                } else if (name == "song") {
                    header_info[(int)HEADER_INFO_TYPES::SONG] = UnXmlSafe(content);
                } else if (name == "artist") {
                    header_info[(int)HEADER_INFO_TYPES::ARTIST] = UnXmlSafe(content);
                } else if (name == "album") {
                    header_info[(int)HEADER_INFO_TYPES::ALBUM] = UnXmlSafe(content);
                } else if (name == "MusicURL") {
                    header_info[(int)HEADER_INFO_TYPES::URL] = UnXmlSafe(content);
                } else if (name == "comment") {
                    header_info[(int)HEADER_INFO_TYPES::COMMENT] = UnXmlSafe(content);
                } else if (name == "sequenceTiming") {
                    seq_timing = content;
                    spdlog::debug("LoadSequence: Sequence timing loaded from XML file. {}", seq_timing);
                } else if (name == "sequenceType") {
                    seq_type = content;
                } else if (name == "mediaFile") {
                    if (!ignore_audio) {
                        spdlog::debug("LoadSequence: mediaFile {}", content);
                        media_file = FileUtils::FixFile(showDir, content);
                        if (media_file != content) {
                            spdlog::debug("LoadSequence: mediaFile resolved to {}", media_file);
                        }
                        if (audio != nullptr) {
                            spdlog::debug("LoadSequence: removing prior audio.");
                            ValueCurve::SetAudio(nullptr);
                            delete audio;
                            audio = nullptr;
                        }
                        if (::FileExists(media_file)) {
                            std::error_code ec2;
                            auto p = std::filesystem::status(media_file, ec2).permissions();
                            if (!ec2 && (p & std::filesystem::perms::owner_read) != std::filesystem::perms::none) {
                                mediaFileName = media_file;
                            } else {
                                spdlog::error("LoadSequence: audio file not readable.");
                            }
                        } else {
                            spdlog::error("LoadSequence: audio file does not exist.");
                        }
                    }
                } else if (name == "sequenceDuration") {
                    SetSequenceDuration(content);
                } else if (name == "imageDir") {
                    image_dir = FileUtils::FixFile(showDir, content);
                }
            }
        } else if (ename == "ElementEffects") {
            for (auto element : e.children("Element")) {
                std::string type = element.attribute("type").as_string("");
                std::string ename2 = element.attribute("name").as_string("");
                if (type == "model") {
                    models.push_back(ename2);
                } else if (type == "timing") {
                    timing_list.push_back(ename2);
                }

                // Handle FixedPointTiming conversion inline
                if (needsTimesCorrection) {
                    for (auto layer : element.children()) {
                        for (auto effect : layer.children("Effect")) {
                            double t1 = std::strtod(effect.attribute("startTime").as_string("0"), nullptr);
                            double t2 = std::strtod(effect.attribute("endTime").as_string("0"), nullptr);
                            effect.attribute("startTime") = (int)(t1 * 1000.0);
                            effect.attribute("endTime") = (int)(t2 * 1000.0);
                        }
                    }
                }
            }
        } else if (ename == "DataLayers") {
            for (auto element : e.children("DataLayer")) {
                std::string name = element.attribute("name").as_string("");
                std::string source = element.attribute("source").as_string("");
                std::string data = element.attribute("data").as_string("");
                std::string num_frames_s = element.attribute("num_frames").as_string("0");
                std::string num_channels_s = element.attribute("num_channels").as_string("0");
                std::string channel_offset_s = element.attribute("channel_offset").as_string("0");
                std::string lor_params_s = element.attribute("lor_params").as_string("0");

                if (!data.empty() && data[0] != '<') {
                    data = FileUtils::FixFile("", data);
                }
                if (source != "<auto-generated>") {
                    source = FileUtils::FixFile("", source);
                }
                if (name == "Nutcracker") {
                    mDataLayers.RemoveDataLayer(0);
                }
                DataLayer* new_data_layer = mDataLayers.AddDataLayer(name, source, data);
                new_data_layer->SetNumFrames(std::strtol(num_frames_s.c_str(), nullptr, 10));
                new_data_layer->SetNumChannels(std::strtol(num_channels_s.c_str(), nullptr, 10));
                new_data_layer->SetChannelOffset(std::strtol(channel_offset_s.c_str(), nullptr, 10));
                new_data_layer->SetLORConvertParams(std::strtol(lor_params_s.c_str(), nullptr, 10));
            }
        } else if (ename == "lastView") {
            mLastView = std::strtol(e.text().as_string("0"), nullptr, 10);
        }
    }

    if (!mediaFileName.empty()) {
        ObtainAccessToURL(mediaFileName);
        spdlog::debug("LoadSequence: Creating audio manager");
        audio = new AudioManager(mediaFileName, GetFrameMS());
        ValueCurve::SetAudio(audio);
        spdlog::debug("LoadSequence: audio manager creation done");
    } else {
        spdlog::info("LoadSequence: No Audio loaded.");
    }

    spdlog::info("LoadSequence: Sequence timing interval {}ms.", GetFrameMS());
    spdlog::info("LoadSequence: Sequence loaded.");

    if (!is_open) {
        return std::nullopt;
    }
    return std::move(loadDoc);
}

// CleanUpEffects — dead code removed

std::string SequenceFile::GetSequenceDurationString() const
{
    return fmt::format("{:.3f}", seq_duration);
}

void SequenceFile::SetSequenceDurationMS(int length)
{
    SetSequenceDuration(length / 1000.0f);
}

void SequenceFile::SetSequenceDuration(double length)
{
    SetSequenceDuration(fmt::format("{:.3f}", length));
}

void SequenceFile::SetSequenceDuration(const std::string& length)
{
    seq_duration = std::strtod(length.c_str(), nullptr);
}

std::string SequenceFile::GetImageDir(UICallbacks* ui)
{
    if (!image_dir.empty()) {
        return image_dir;
    }

    if (ui) {
        std::string chosen = ui->PromptForDirectory("Select Directory for storing image files for this sequence");
        if (chosen.empty()) {
            return "";
        }
        SetImageDir(chosen);
    }
    return image_dir;
}

void SequenceFile::SetImageDir(const std::string& dir)
{
    image_dir = dir;
}

void SequenceFile::UpdateVersion()
{
    version_string = xlights_version_string;
}

void SequenceFile::UpdateVersion(const std::string& version)
{
    version_string = version;
}

void SequenceFile::ProcessAudacityTimingFiles(const std::vector<std::string>& filenames, RenderContext* xLightsParent)
{
    for (size_t i = 0; i < filenames.size(); ++i) {
        std::filesystem::path next_file(filenames[i]);

        std::ifstream f(next_file);
        if (!f.is_open()) {
            return;
        }

        std::string filename = next_file.stem().string();

        while (TimingAlreadyExists(filename, xLightsParent)) {
            filename += "_1";
        }

        Element* element = xLightsParent->AddTimingElement(filename);
        EffectLayer* effectLayer = element->GetEffectLayer(0);

        // Read all lines
        std::vector<std::string> allLines;
        std::string rawLine;
        while (std::getline(f, rawLine)) {
            allLines.push_back(rawLine);
        }

        // scan the first 30 lines to see if it is tab delimited
        bool isTab = false;
        for (size_t r = 0; r < allLines.size() && r < 30 && !isTab; r++) {
            if (allLines[r].find('\t') != std::string::npos) {
                isTab = true;
            }
        }

        std::vector<std::string> start_times;
        std::vector<std::string> end_times;
        std::vector<std::string> labels;

        for (const auto& rawLine2 : allLines) {
            std::string line = rawLine2;
            // remove comments
            auto hashPos = line.find('#');
            if (hashPos != std::string::npos) {
                line.resize(hashPos);
            }

            // trim trailing spaces
            while (!line.empty() && line.back() == ' ') line.pop_back();
            if (line.empty()) continue;

            char delim = isTab ? '\t' : ' ';
            std::istringstream iss(line);
            std::string token;
            std::getline(iss, token, delim);
            start_times.push_back(token); //first column = start time
            std::getline(iss, token, delim);
            end_times.push_back(token); //second column = end time
            std::string label;
            if (std::getline(iss, token, delim)) {
                label = token;
            }
            while (std::getline(iss, token, delim)) {
                if (!token.empty()) {
                    label += " " + token;
                }
            }
            labels.push_back(label);
        }

        for (size_t j = 0; j < start_times.size(); j++) {
            double time1 = std::strtod(start_times[j].c_str(), nullptr);
            start_times[j] = std::to_string((int)(time1 * 1000.0));
            time1 = std::strtod(end_times[j].c_str(), nullptr);
            end_times[j] = std::to_string((int)(time1 * 1000.0));
        }

        for (size_t k = 0; k < start_times.size(); ++k) {
            int startTime = RoundToMultipleOfPeriod(std::strtol(start_times[k].c_str(), nullptr, 10), GetFrequency());
            int endTime = RoundToMultipleOfPeriod(std::strtol(end_times[k].c_str(), nullptr, 10), GetFrequency());
            if (startTime == endTime) {
                if (k == start_times.size() - 1) {
                    endTime = startTime + GetFrequency();
                } else {
                    endTime = RoundToMultipleOfPeriod(std::strtol(start_times[k + 1].c_str(), nullptr, 10), GetFrequency());
                }
            }

            effectLayer->AddEffect(0, labels[k], "", "", startTime, endTime, EFFECT_NOT_SELECTED, false);
        }
    }
}

void SequenceFile::ProcessLorTiming(const std::vector<std::string>& filenames, RenderContext* xLightsParent)
{
    for (size_t i = 0; i < filenames.size(); ++i )
    {
        std::filesystem::path next_file(filenames[i]);

        if (!::FileExists(next_file.string())) {
            DisplayError("LOR Timing: Failed to open file: '" + next_file.string() + "'");
            return;
        }

        std::string filename = next_file.stem().string();

        std::vector<std::string> grid_times;
        std::vector<std::string> timing_options;

        pugi::xml_document input_xml;
        pugi::xml_parse_result result = input_xml.load_file(next_file.string().c_str());
        if( !result )
        {
            DisplayError("LOR Timing: Failed to load XML file: '" + next_file.string() + "'");
            return;
        }

        pugi::xml_node input_root = input_xml.first_child();

        for(pugi::xml_node e = input_root.first_child(); e; e = e.next_sibling() )
        {
            if (strcmp(e.name(), "timingGrids") == 0)
            {
                for(pugi::xml_node grids = e.first_child(); grids; grids = grids.next_sibling() )
                {
                    if (strcmp(grids.name(), "timingGrid") == 0)
                    {
                        std::string grid_type = grids.attribute("type").as_string("");
                        if( grid_type == "freeform" )
                        {
                            std::string grid_name = grids.attribute("name").as_string("");
                            std::string grid_id = grids.attribute("saveID").as_string("");
                            if( grid_name.empty() )
                            {
                                grid_name = "Unnamed" + grid_id;
                            }
                            timing_options.push_back(grid_name);
                        }
                    }
                }
            }
        }

        auto* ui = xLightsParent->GetUICallbacks();
        if (!ui) return;
        std::vector<std::string> timing_grids = ui->ChooseFromList(
            "Choose Timing Grid to use for timing import:", timing_options);
        if (timing_grids.empty()) {
            return;
        }

        for(pugi::xml_node e = input_root.first_child(); e; e = e.next_sibling() )
        {
            if (strcmp(e.name(), "timingGrids") == 0)
            {
                for(pugi::xml_node grids = e.first_child(); grids; grids = grids.next_sibling() )
                {
                    if (strcmp(grids.name(), "timingGrid") == 0)
                    {
                        std::string grid_name = grids.attribute("name").as_string("");
                        std::string grid_id = grids.attribute("saveID").as_string("");
                        if( grid_name.empty() )
                        {
                            grid_name = "Unnamed" + grid_id;
                        }
                        for (size_t i1 = 0; i1 < timing_grids.size(); i1++ )
                        {
                            if( grid_name == timing_grids[i1] )
                            {
                                std::string new_timing_name = UniqueTimingName(xLightsParent,  filename + ": " + grid_name);
                                Element* element = xLightsParent->AddTimingElement(new_timing_name);
                                EffectLayer* effectLayer = element->GetEffectLayer(0);

                                grid_times.clear();
                                for(pugi::xml_node effect = grids.first_child(); effect; effect = effect.next_sibling() )
                                {
                                    int time = effect.attribute("centisecond").as_int(0) * 10;
                                    grid_times.push_back(std::to_string(time));
                                }

                                for (size_t k = 0; k < grid_times.size()-1; ++k )
                                {
                                    int startTime = RoundToMultipleOfPeriod(std::strtol(grid_times[k].c_str(), nullptr, 10),GetFrequency());
                                    int endTime = RoundToMultipleOfPeriod(std::strtol(grid_times[k+1].c_str(), nullptr, 10),GetFrequency());
                                    effectLayer->AddEffect(0,"","","",startTime,endTime,EFFECT_NOT_SELECTED,false);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

std::string SequenceFile::UniqueTimingName(RenderContext* xLightsParent, std::string name) const
{
    std::string testname = RemoveUnsafeXmlChars(name);
    int testnamenum = 1;
    bool ok;
    do
    {
        ok = true;
        int num_elements = xLightsParent->GetSequenceElements().GetElementCount();
        for (int i = 0; i < num_elements; ++i)
        {
            Element* element = xLightsParent->GetSequenceElements().GetElement(i);
            if (element->GetType() == ElementType::ELEMENT_TYPE_TIMING)
            {
                if (element->GetName() == testname)
                {
                    testname = name + "_" + std::to_string(testnamenum++);
                    ok = false;
                    break;
                }
            }
        }
    } while (!ok);
    return testname;
}

void SequenceFile::ProcessXTiming(const pugi::xml_node& node, RenderContext* xLightsParent)
{
    std::string name = UnXmlSafe(node.attribute("name").as_string(""));
    //std::string v = node.attribute("SourceVersion").as_string("");
    std::string st = node.attribute("subType").as_string("");

    name = UniqueTimingName(xLightsParent, name);

    Element* element = xLightsParent->AddTimingElement(name, st);
    EffectLayer* effectLayer = nullptr;

    int l = 0;
    for (pugi::xml_node layers = node.first_child(); layers; layers = layers.next_sibling())
    {
        if (strcmp(layers.name(), "EffectLayer") == 0)
        {
            l++;
            if (l == 1)
            {
                effectLayer = element->GetEffectLayer(0);
            }
            else
            {
                effectLayer = element->AddEffectLayer();
            }

            for (pugi::xml_node effects = layers.first_child(); effects; effects = effects.next_sibling())
            {
                if (strcmp(effects.name(), "Effect") == 0)
                {
                    std::string label = UnXmlSafe(effects.attribute("label").as_string(""));
                    std::string start = effects.attribute("starttime").as_string("");
                    std::string end = effects.attribute("endtime").as_string("");
                    int s = std::strtol(start.c_str(), nullptr, 10);
                    int e = std::strtol(end.c_str(), nullptr, 10);

                    if (s % GetFrameMS() != 0)
                    {
                        s -= s % GetFrameMS();
                    }
                    if (e % GetFrameMS() != 0)
                    {
                        e -= e % GetFrameMS();
                    }
                    effectLayer->AddEffect(0, label, "", "", s, e, EFFECT_NOT_SELECTED, false);
                }
            }
        }
    }
}

void SequenceFile::ProcessXTiming(const std::vector<std::string>& filenames, RenderContext* xLightsParent)
{
    for (size_t i = 0; i < filenames.size(); ++i)
    {
        const std::string& filepath = filenames[i];
        std::string filename = std::filesystem::path(filepath).stem().string();

        if (!::FileExists(filepath)) {
            DisplayError("xTiming: Failed to open file: '" + filepath + "'");
            return;
        }

        pugi::xml_document input_xml;
        pugi::xml_parse_result parse_result = input_xml.load_file(filepath.c_str());
        if (!parse_result)
        {
            DisplayError("xTiming: Failed to load XML file: '" + filepath + "'");
            return;
        }

        pugi::xml_node e = input_xml.first_child();

        if (strcmp(e.name(), "timing") == 0)
        {
            ProcessXTiming(e, xLightsParent);
        }
        else if (strcmp(e.name(), "timings") == 0)
        {
            for (pugi::xml_node node = e.first_child(); node; node = node.next_sibling())
            {
                if (strcmp(node.name(), "timing") == 0)
                {
                    ProcessXTiming(node, xLightsParent);
                }
            }
        }
    }

    if (GetSequenceLoaded()) {
        GetTimingList(xLightsParent->GetSequenceElements());
    }
}

static std::string RemoveTabs(const std::string& s, size_t tabs)
{
    size_t start = 0;
    for (size_t i = 0; i < tabs && start < s.size(); i++) {
        if (s[start] == '\t') {
            ++start;
        }
    }
    return s.substr(start);
}

static bool IsAllDigits(const std::string& s) {
    return !s.empty() && s.find_first_not_of("0123456789") == std::string::npos;
}

void SequenceFile::ProcessPapagayo(const std::vector<std::string>& filenames, RenderContext* xLightsParent)
{
    for (size_t i = 0; i < filenames.size(); ++i)
    {
        int linenum = 1;
        const std::string& filepath = filenames[i];
        spdlog::info("Loading papagayo file {}", filepath);

        std::ifstream f(filepath);
        if (!f.is_open())
        {
            DisplayError("Failed to open file: " + filepath);
            return;
        }

        // Helper to read next line from the file
        std::vector<std::string> fileLines;
        {
            std::string tmp;
            while (std::getline(f, tmp)) fileLines.push_back(tmp);
        }
        size_t curLine = 0;
        auto nextLine = [&]() -> std::string {
            return curLine < fileLines.size() ? fileLines[curLine++] : "";
        };

        std::string line = nextLine();
        // Case-insensitive comparison
        {
            std::string lower = line;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            if (lower != "lipsync version 1") {
                DisplayError(fmt::format("Invalid papagayo file @line {} (header '{}')", linenum, line));
                return;
            }
        }

        nextLine(); // filename which we ignore
        linenum++;

        line = nextLine();
        int samppersec = IsAllDigits(line) ? std::strtol(line.c_str(), nullptr, 10) : -1;
        linenum++;
        if (samppersec < 1) {
            DisplayError(fmt::format("Invalid file @line {} ('{}' samples per sec)", linenum, line));
        }
        int ms = 1000 / samppersec;

        int maxframe = 4 * 60 * samppersec;
        if (GetMedia() != nullptr) {
            maxframe = GetMedia()->LengthMS() / ms;
        }
        int offset = 0;
        if (auto* ui = xLightsParent->GetUICallbacks()) {
            offset = ui->PromptForNumber("Enter the number of frames to offset the papagayo data by", "Offset", 0, 0, maxframe);
        }

        line = nextLine();
        int numsamp = IsAllDigits(line) ? std::strtol(line.c_str(), nullptr, 10) : -1;
        linenum++;
        if (numsamp < 1) {
            DisplayError(fmt::format("Invalid file @line {} ('{}' song samples)", linenum, line));
        }

        line = nextLine();
        int numvoices = IsAllDigits(line) ? std::strtol(line.c_str(), nullptr, 10) : -1;
        linenum++;
        if (numvoices < 1) {
            DisplayError(fmt::format("Invalid file @line {} ('{}' voices)", linenum, line));
        }
        spdlog::info("    Voices {}", numvoices);

        for (int v = 1; v <= numvoices; ++v)
        {
            std::string name = "Voice " + std::to_string(v);
            name = UniqueTimingName(xLightsParent, name);
            spdlog::info("    Loading voice {} into timing track {}.", v, name);

            std::string voicename = nextLine();
            linenum++;
            if (voicename.empty()) {
                DisplayError(fmt::format("Missing voice# {} of {}", v, numvoices));
                return;
            }

            nextLine(); //all phrases for voice, "|" delimiter
            linenum++;
            std::string desc = fmt::format("voice# {} '{}' @line {}", v, voicename, linenum);

            line = RemoveTabs(nextLine(), 1);
            int numphrases = IsAllDigits(line) ? std::strtol(line.c_str(), nullptr, 10) : -1;
            linenum++;
            if (numphrases < 0) {
                DisplayError(fmt::format("Invalid file @line {} ('{}' phrases for {})", linenum, line, desc));
            }

            Element* element = xLightsParent->AddTimingElement(name);
            EffectLayer *el1 = element->GetEffectLayer(0);
            EffectLayer *el2 = element->AddEffectLayer();
            EffectLayer *el3 = element->AddEffectLayer();

            for (int p = 1; p <= numphrases; ++p)
            {
                std::string label = RemoveTabs(nextLine(), 2);
                linenum++;
                if (label.empty()) {
                    DisplayError(fmt::format("Missing phrase# {} of {} for {}", p, numphrases, desc));
                    return;
                }

                line = RemoveTabs(nextLine(), 2);
                int start = IsAllDigits(line) ? (offset + std::strtol(line.c_str(), nullptr, 10)) * ms : 0;
                linenum++;

                line = RemoveTabs(nextLine(), 2);
                int end = IsAllDigits(line) ? (offset + std::strtol(line.c_str(), nullptr, 10)) * ms : 0;
                linenum++;
                desc = fmt::format("voice# {}, phrase {} '{}', start frame {} end frame {} @line {}", v, p, label, start, end, linenum);

                el1->AddEffect(0, label, "", "", start, end, EFFECT_NOT_SELECTED, false);

                line = RemoveTabs(nextLine(), 2);
                int numwords = IsAllDigits(line) ? std::strtol(line.c_str(), nullptr, 10) : -1;
                linenum++;
                if (numwords < 0) {
                    DisplayError(fmt::format("Invalid file @line {} ('{}' words for {})", linenum, line, desc));
                }

                for (int w = 1; w <= numwords; ++w)
                {
                    line = RemoveTabs(nextLine(), 3);
                    linenum++;
                    auto space1 = line.find(' ');
                    label = line.substr(0, space1);
                    if (label.empty()) {
                        DisplayError(fmt::format("Missing word# {} of {} for {}", w, numwords, desc));
                        return;
                    }

                    auto space2 = line.find(' ', space1 + 1);
                    std::string ss = line.substr(space1 + 1, space2 - space1 - 1);
                    start = IsAllDigits(ss) ? (offset + std::strtol(ss.c_str(), nullptr, 10)) * ms : 0;
                    linenum++;

                    auto space3 = line.find(' ', space2 + 1);
                    ss = line.substr(space2 + 1, space3 - space2 - 1);
                    end = IsAllDigits(ss) ? (offset + std::strtol(ss.c_str(), nullptr, 10)) * ms : 0;
                    linenum++;
                    desc = fmt::format("voice# {}, phrase# {}, word {} '{}', start frame {} end frame {} @line {}", v, p, w, label, start, end, linenum);

                    el2->AddEffect(0, label, "", "", start, end, EFFECT_NOT_SELECTED, false);

                    ss = (space3 != std::string::npos) ? line.substr(space3 + 1) : "";
                    int numphonemes = IsAllDigits(ss) ? std::strtol(ss.c_str(), nullptr, 10) : -1;
                    linenum++;
                    if (numphonemes < 0) {
                        DisplayError(fmt::format("Invalid file @line {} ('{}' phonemes for {})", linenum, line, desc));
                    }

                    int outerend = end;
                    for (int ph = 1; ph <= numphonemes; ++ph)
                    {
                        line = RemoveTabs(nextLine(), 4);
                        linenum++;
                        auto space4 = line.find(' ');

                        ss = line.substr(0, space4);
                        end = IsAllDigits(ss) ? (offset + std::strtol(ss.c_str(), nullptr, 10)) * ms : 0;
                        linenum++;

                        if (ph != 1) {
                            el3->AddEffect(0, label, "", "", start, end, EFFECT_NOT_SELECTED, false);
                        }
                        label = (space4 != std::string::npos) ? line.substr(space4 + 1) : "";
                        if (label.empty()) {
                            DisplayError(fmt::format("Missing phoneme# {} of {} for {}", ph, numphonemes, desc));
                            return;
                        }
                        start = end;

                        if (ph == numphonemes) {
                            end = outerend;
                            el3->AddEffect(0, label, "", "", start, end, EFFECT_NOT_SELECTED, false);
                        }
                    }
                }
            }
        }
    }
}

static std::vector<std::string> SplitString(const std::string& s, char delim)
{
    std::vector<std::string> result;
    size_t start = 0;
    while (start <= s.size()) {
        auto pos = s.find(delim, start);
        result.push_back(s.substr(start, pos - start));
        if (pos == std::string::npos) break;
        start = pos + 1;
    }
    return result;
}

static std::string ReadSRTLine(const std::vector<std::string>& lines, size_t& idx, int linenum, long& startMS, long& endMS)
{
    startMS = 0;
    endMS = 0;

    if (idx >= lines.size()) return "";

    int l = std::strtol(lines[idx++].c_str(), nullptr, 10);
    while (idx < lines.size() && l < linenum) {
        l = std::strtol(lines[idx++].c_str(), nullptr, 10);
    }
    if (l > linenum) return "";
    if (idx >= lines.size()) return "";

    //00:00:06,580 --> 00:00:08,580
    std::string times = lines[idx++];
    if (times.find("-->") != std::string::npos)
    {
        auto c1 = SplitString(times, ':');
        if (c1.size() == 5) {
            int sH = std::strtol(c1[0].c_str(), nullptr, 10);
            int sM = std::strtol(c1[1].c_str(), nullptr, 10);
            auto c2 = SplitString(c1[2], ',');
            if (c2.size() == 2) {
                int sS = std::strtol(c2[0].c_str(), nullptr, 10);
                auto c3 = SplitString(c2[1], ' ');
                if (c3.size() == 3) {
                    int sMS = std::strtol(c3[0].c_str(), nullptr, 10);
                    int eH = std::strtol(c3[2].c_str(), nullptr, 10);
                    int eM = std::strtol(c1[3].c_str(), nullptr, 10);
                    auto c4 = SplitString(c1[4], ',');
                    if (c4.size() == 2) {
                        int eS = std::strtol(c4[0].c_str(), nullptr, 10);
                        int eMS_val = std::strtol(c4[1].c_str(), nullptr, 10);
                        startMS = sH * 3600000 + sM * 60000 + sS * 1000 + sMS;
                        endMS = eH * 3600000 + eM * 60000 + eS * 1000 + eMS_val;
                    }
                }
            }
        }
    }

    if (idx >= lines.size()) return "";

    std::string result;
    while (idx < lines.size() && !lines[idx].empty()) {
        if (!result.empty()) result += " ";
        result += Trim(lines[idx]);
        idx++;
    }
    if (idx < lines.size()) idx++; // skip blank line
    return result;
}

void SequenceFile::ProcessSRT(const std::vector<std::string>& filenames, RenderContext* xLightsParent)
{
    for (size_t i = 0; i < filenames.size(); ++i)
    {
        const std::string& filepath = filenames[i];
        spdlog::info("Loading srt file {}", filepath);

        std::ifstream f(filepath);
        if (!f.is_open()) {
            DisplayError("Failed to open file: " + filepath);
            return;
        }

        // Read all lines
        std::vector<std::string> lines;
        std::string tmp;
        while (std::getline(f, tmp)) lines.push_back(tmp);

        std::string name = std::filesystem::path(filepath).stem().string();
        name = UniqueTimingName(xLightsParent, name);
        spdlog::info("    Loading into timing track {}.", name);

        Element* element = xLightsParent->AddTimingElement(name);
        EffectLayer* el1 = element->GetEffectLayer(0);

        long startMS;
        long endMS;
        int linenum = 1;
        size_t idx = 0;

        std::string line = ReadSRTLine(lines, idx, linenum++, startMS, endMS);

        do {
            if (!line.empty() && endMS > startMS) {
                el1->AddEffect(0, line, "", "", startMS, endMS, EFFECT_NOT_SELECTED, false);
            }
            line = ReadSRTLine(lines, idx, linenum++, startMS, endMS);
        } while (idx < lines.size());
    }
}

std::vector<std::string> SequenceFile::GetTimingList(const SequenceElements& seq_elements)
{
    timing_list.clear();
    int num_elements = seq_elements.GetElementCount();
    for(int i = 0; i < num_elements; ++i)
    {
        Element* element = seq_elements.GetElement(i);
        if( element->GetType() == ElementType::ELEMENT_TYPE_TIMING )
        {
            timing_list.push_back(element->GetName());
        }
    }
    return timing_list;
}

// Legacy Save()/WriteEffects/AddJukebox removed — Save now uses BuildDocument + pugixml

static void WriteEffectsPugi(EffectLayer* layer,
    pugi::xml_node& effect_layer_node,
    std::unordered_map<std::string, int>& colorPalettes,
    pugi::xml_node& colorPalette_node,
    std::unordered_map<std::string, int>& effectStrings,
    pugi::xml_node& effectDB_Node)
{
    int num_effects = layer->GetEffectCount();
    for (int k = 0; k < num_effects; ++k) {
        Effect* effect = layer->GetEffect(k);
        std::string settings = effect->GetSettingsAsString();
        int size = effectStrings.size();
        int ref = effectStrings[settings] - 1;
        if (ref == -1) {
            ref = size;
            effectStrings[settings] = ref + 1;
            auto dbNode = effectDB_Node.append_child("Effect");
            dbNode.text().set(settings);
        }

        auto effect_node = effect_layer_node.append_child("Effect");
        effect_node.append_attribute("ref") = ref;
        effect_node.append_attribute("name") = XmlSafe(effect->GetEffectName());
        if (effect->GetProtected()) {
            effect_node.append_attribute("protected") = "1";
        }
        if (effect->GetSelected()) {
            effect_node.append_attribute("selected") = "1";
        }
        if (effect->GetID()) {
            effect_node.append_attribute("id") = effect->GetID();
        }
        effect_node.append_attribute("startTime") = effect->GetStartTimeMS();
        effect_node.append_attribute("endTime") = effect->GetEndTimeMS();
        std::string palette = effect->GetPaletteAsString();
        if (!palette.empty()) {
            size = colorPalettes.size();
            int pref = colorPalettes[palette] - 1;
            if (pref == -1) {
                pref = size;
                colorPalettes[palette] = pref + 1;
                auto palNode = colorPalette_node.append_child("ColorPalette");
                palNode.text().set(palette);
            }
            effect_node.append_attribute("palette") = pref;
        }
    }
}

bool SequenceFile::BuildDocument(pugi::xml_document& doc, SequenceElements& seq_elements)
{
    UpdateVersion();

    auto root = doc.append_child("xsequence");
    root.append_attribute("BaseChannel") = "0";
    root.append_attribute("ChanCtrlBasic") = "0";
    root.append_attribute("ChanCtrlColor") = "0";
    root.append_attribute("FixedPointTiming") = "1";
    root.append_attribute("ModelBlending") = seq_elements.SupportsModelBlending() ? "true" : "false";

    // Head
    auto head = root.append_child("head");
    head.append_child("version").text().set(version_string);
    head.append_child("author").text().set(XmlSafe(GetHeaderInfo(HEADER_INFO_TYPES::AUTHOR)));
    head.append_child("author-email").text().set(XmlSafe(GetHeaderInfo(HEADER_INFO_TYPES::AUTHOR_EMAIL)));
    head.append_child("author-website").text().set(XmlSafe(GetHeaderInfo(HEADER_INFO_TYPES::WEBSITE)));
    head.append_child("song").text().set(XmlSafe(GetHeaderInfo(HEADER_INFO_TYPES::SONG)));
    head.append_child("artist").text().set(XmlSafe(GetHeaderInfo(HEADER_INFO_TYPES::ARTIST)));
    head.append_child("album").text().set(XmlSafe(GetHeaderInfo(HEADER_INFO_TYPES::ALBUM)));
    head.append_child("MusicURL").text().set(XmlSafe(GetHeaderInfo(HEADER_INFO_TYPES::URL)));
    head.append_child("comment").text().set(XmlSafe(GetHeaderInfo(HEADER_INFO_TYPES::COMMENT)));
    head.append_child("sequenceTiming").text().set(seq_timing);
    head.append_child("sequenceType").text().set(seq_type);
    head.append_child("mediaFile").text().set(media_file);
    head.append_child("sequenceDuration").text().set(GetSequenceDurationString());
    head.append_child("imageDir").text().set(image_dir);

    // ColorPalettes and EffectDB (populated during element iteration)
    std::unordered_map<std::string, int> colorPalettes;
    auto colorPalette_node = root.append_child("ColorPalettes");
    std::unordered_map<std::string, int> effectStrings;
    auto effectDB_Node = root.append_child("EffectDB");

    // SequenceMedia
    seq_elements.GetSequenceMedia().SaveToXml(root);

    // DataLayers
    auto data_layer = root.append_child("DataLayers");
    for (int i = 0; i < mDataLayers.GetNumLayers(); ++i) {
        DataLayer* layer = mDataLayers.GetDataLayer(i);
        auto layer_node = data_layer.append_child("DataLayer");
        layer_node.append_attribute("lor_params") = layer->GetLORConvertParams();
        layer_node.append_attribute("channel_offset") = layer->GetChannelOffset();
        layer_node.append_attribute("num_channels") = layer->GetNumChannels();
        layer_node.append_attribute("num_frames") = layer->GetNumFrames();
        layer_node.append_attribute("data") = layer->GetDataSource();
        layer_node.append_attribute("source") = layer->GetSource();
        layer_node.append_attribute("name") = layer->GetName();
    }

    // DisplayElements and ElementEffects
    auto display_node = root.append_child("DisplayElements");
    auto elements_node = root.append_child("ElementEffects");

    // lastView
    auto last_view_node = root.append_child("lastView");
    last_view_node.text().set(seq_elements.GetCurrentView());

    // TimingTags
    auto timing_tags_node = root.append_child("TimingTags");
    for (int i = 0; i < 10; ++i) {
        auto tag_node = timing_tags_node.append_child("Tag");
        tag_node.append_attribute("number") = i;
        tag_node.append_attribute("position") = seq_elements.GetTagPosition(i);
    }

    int num_elements = seq_elements.GetElementCount();
    for (int i = 0; i < num_elements; ++i) {
        Element* element = seq_elements.GetElement(i);

        // DisplayElements entry
        auto display_element_node = display_node.append_child("Element");
        display_element_node.append_attribute("collapsed") = element->GetCollapsed();
        if (element->IsRenderDisabled()) {
            display_element_node.append_attribute("RenderDisabled") = "1";
        }
        display_element_node.append_attribute("type") = (element->GetType() == ElementType::ELEMENT_TYPE_TIMING ? "timing" : "model");
        display_element_node.append_attribute("name") = element->GetName();
        if (element->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
            TimingElement* te = dynamic_cast<TimingElement*>(element);
            display_element_node.append_attribute("visible") = te->GetMasterVisible();
        } else {
            display_element_node.append_attribute("visible") = element->GetVisible();
        }

        // ElementEffects entry
        auto element_effects_node = elements_node.append_child("Element");
        element_effects_node.append_attribute("type") = (element->GetType() == ElementType::ELEMENT_TYPE_TIMING ? "timing" : "model");
        element_effects_node.append_attribute("name") = element->GetName();

        if (element->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
            TimingElement* tm = dynamic_cast<TimingElement*>(element);
            display_element_node.append_attribute("views") = tm->GetViews();
            display_element_node.append_attribute("active") = tm->GetActive();
            if (!tm->GetSubType().empty()) {
                display_element_node.append_attribute("subType") = tm->GetSubType();
            }
            if (tm->GetFixedTiming()) {
                element_effects_node.append_attribute("fixed") = tm->GetFixedTiming();
                element_effects_node.append_child("EffectLayer");
            } else {
                for (int j = 0; j < (int)tm->GetEffectLayerCount(); ++j) {
                    EffectLayer* layer = tm->GetEffectLayer(j);
                    auto effect_layer_node = element_effects_node.append_child("EffectLayer");
                    for (int k = 0; k < layer->GetEffectCount(); ++k) {
                        Effect* effect = layer->GetEffect(k);
                        auto effect_node = effect_layer_node.append_child("Effect");
                        effect_node.append_attribute("label") = effect->GetEffectName();
                        if (effect->GetProtected()) {
                            effect_node.append_attribute("protected") = "1";
                        }
                        if (effect->GetSelected()) {
                            effect_node.append_attribute("selected") = "1";
                        }
                        effect_node.append_attribute("startTime") = effect->GetStartTimeMS();
                        effect_node.append_attribute("endTime") = effect->GetEndTimeMS();
                    }
                }
            }
        } else if (element->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
            ModelElement* me = dynamic_cast<ModelElement*>(element);
            for (int j = 0; j < (int)me->GetEffectLayerCount(); ++j) {
                EffectLayer* layer = me->GetEffectLayer(j);
                auto effect_layer_node = element_effects_node.append_child("EffectLayer");
                if (!layer->GetLayerName().empty()) {
                    effect_layer_node.append_attribute("layerName") = layer->GetLayerName();
                }
                WriteEffectsPugi(layer, effect_layer_node, colorPalettes, colorPalette_node, effectStrings, effectDB_Node);
            }

            for (int strand = 0; strand < me->GetSubModelAndStrandCount(); strand++) {
                SubModelElement* se = me->GetSubModel(strand);
                int num_layers = se->GetEffectLayerCount();
                pugi::xml_node effect_layer_node;

                StrandElement* strEl = dynamic_cast<StrandElement*>(se);
                for (int j = 0; j < num_layers; ++j) {
                    EffectLayer* layer = se->GetEffectLayer(j);
                    if (layer->GetEffectCount() != 0 || !layer->GetLayerName().empty()) {
                        auto eln = element_effects_node.append_child(strEl == nullptr ? "SubModelEffectLayer" : "Strand");
                        if (strEl != nullptr) {
                            eln.append_attribute("index") = strEl->GetStrand();
                            if (j == 0) {
                                effect_layer_node = eln;
                            }
                        }
                        if (!layer->GetLayerName().empty()) {
                            eln.append_attribute("layerName") = layer->GetLayerName();
                        }
                        if (j > 0) {
                            eln.append_attribute("layer") = j;
                        }
                        if (!se->GetName().empty()) {
                            eln.append_attribute("name") = se->GetName();
                        }
                        WriteEffectsPugi(layer, eln, colorPalettes, colorPalette_node, effectStrings, effectDB_Node);
                    }
                }
                if (strEl != nullptr) {
                    for (int n = 0; n < strEl->GetNodeLayerCount(); n++) {
                        NodeLayer* nlayer = strEl->GetNodeLayer(n);
                        if (nlayer->GetEffectCount() == 0) continue;
                        if (!effect_layer_node) {
                            effect_layer_node = element_effects_node.append_child("Strand");
                            effect_layer_node.append_attribute("index") = strEl->GetStrand();
                            if (!se->GetName().empty()) {
                                effect_layer_node.append_attribute("name") = se->GetName();
                            }
                        }
                        auto neffect_layer_node = effect_layer_node.append_child("Node");
                        neffect_layer_node.append_attribute("index") = n;
                        if (!nlayer->GetNodeName().empty()) {
                            neffect_layer_node.append_attribute("name") = nlayer->GetNodeName();
                        }
                        WriteEffectsPugi(nlayer, neffect_layer_node, colorPalettes, colorPalette_node, effectStrings, effectDB_Node);
                    }
                }
            }
        }
    }

    // Jukebox
    if (!_jukeboxButtons.empty()) {
        SaveJukeboxButtons(root, _jukeboxButtons);
    }

    return true;
}

// function used to save sequence data
bool SequenceFile::Save(SequenceElements& seq_elements)
{
    pugi::xml_document doc;
    if (!BuildDocument(doc, seq_elements)) {
        return false;
    }

    if (!doc.save_file(mFilePath.c_str(), "  ")) {
        return false;
    }

    MarkNewFileRevision(GetFullPath());
    return true;
}
// Legacy SaveToDoc removed — Save now uses BuildDocument + pugixml

bool SequenceFile::TimingAlreadyExists(const std::string & section, RenderContext* xLightsParent)
{
    if( sequence_loaded )
    {
        timing_list = GetTimingList(xLightsParent->GetSequenceElements());
    }
    else
    {
        timing_list = GetTimingList();
    }
    for (size_t i = 0; i < timing_list.size(); ++i )
    {
        if( timing_list[i] == section )
        {
            return true;
        }
    }
    return false;
}

bool SequenceFile::TimingMatchesModelName(const std::string& section, RenderContext* xLightsParent) {
    if (sequence_loaded) {
        SequenceElements& mSequenceElements = xLightsParent->GetSequenceElements();
        if (mSequenceElements.ElementExists(section)) {
            return true;
        }
    }
    return false;
}

void SequenceFile::AddNewTimingSection(const std::string& filename, RenderContext* xLightsParent,
                                         std::vector<int>& starts, std::vector<int>& ends, std::vector<std::string>& labels)
{
    if (!sequence_loaded) {
        PendingTiming pt;
        pt.name = filename;
        pt.starts = starts;
        pt.ends = ends;
        pt.labels = labels;
        mPendingTimings.push_back(std::move(pt));
        timing_list.push_back(filename);
        return;
    }

    // some QM plugins dont return items sorted appropriately
    typedef struct tm {
        int start;
        int end;
        std::string label;
        int duration() const
        {
            return end - start;
        }
        static bool sort_func(const struct tm& a, const struct tm& b)
        {
            if (a.start == b.start)
                return a.duration() < b.duration();

            return a.start < b.start;
        }
    } tm;

    std::vector<tm> tms;

    tms.resize(starts.size());
    for (size_t k = 0; k < starts.size(); k++) {
        tms[k] = tm({ RoundToMultipleOfPeriod(starts[k], GetFrequency()),
                   RoundToMultipleOfPeriod(ends[k], GetFrequency()),
                   labels[k] });
    }

    std::sort(begin(tms), end(tms), tm::sort_func);

    Element* element = xLightsParent->AddTimingElement(filename);
    EffectLayer* effectLayer = element->GetEffectLayer(0);

    int prev_start = -1;
    int prev_end = -1;

    for (size_t k = 0; k < tms.size(); k++) {
        int start = tms[k].start;
        int end = tms[k].end;

        // if this timing mark overlaps the prior one then force it to start at the end of the prior one
        if (start < prev_end) {
            start = prev_end;
        }

        if (k < tms.size() - 1 && tms[k + 1].start < end) // the next timing starts before this one ends ... in which case this ends = that start
        {
            end = tms[k + 1].start;
        }

        // if this timing mark starts before the prior one ended then start it after the prior one
        if (start < prev_end)
        {
            start = prev_end;
        }

        // if it now starts after it ends then skip it as it totally overlapped with the last timing mark
        if (start > end) {
            continue;
        }

        if (start > GetSequenceDurationMS()) {
            continue; // dont add timing marks after the end of the song
        } else if (start == prev_start && end == prev_end) {
            continue;            // skip duplicates
        } else if (start == end) // dont add zero length timing marks
        {
            // zero length timing marks are not valid ... but if the following start is greater than the current start then use that as the end
            if (k == tms.size() - 1) {
                // special case use the end of the sequence
                end = RoundToMultipleOfPeriod(GetSequenceDurationMS(), GetFrequency());
                if (start == end) {
                    continue;
                }
            } else {
                if (tms[k + 1].start > start) {
                    end = tms[k + 1].start;
                } else {
                    // even using the next start would make it zero length so we have to skip this one
                    continue;
                }
            }
        }

        prev_start = start;
        prev_end = end;

        effectLayer->AddEffect(0, tms[k].label, "", "", start, end, EFFECT_NOT_SELECTED, false);
    }
}

void SequenceFile::AddNewTimingSection(const std::string & interval_name, RenderContext* xLightsParent, const std::string& subType)
{
    if (sequence_loaded) {
        xLightsParent->AddTimingElement(interval_name, subType);
    } else {
        PendingTiming pt;
        pt.name = interval_name;
        pt.subType = subType;
        mPendingTimings.push_back(std::move(pt));
        timing_list.push_back(interval_name);
    }
}

void SequenceFile::AddFixedTimingSection(const std::string& interval_name, RenderContext* xLightsParent)
{
    if (sequence_loaded) {
        if (interval_name == "Empty" || (interval_name != "25ms" && interval_name != "50ms" && interval_name != "100ms" && !EndsWith(interval_name, "ms Metronome"))) {
            xLightsParent->AddTimingElement(interval_name);
        } else {
            int interval = std::strtol((interval_name).c_str(), nullptr, 10);
            TimingElement* element = xLightsParent->AddTimingElement(interval_name);
            element->SetFixedTiming(interval);
            EffectLayer* effectLayer = element->GetEffectLayer(0);
            int time = 0;
            int end_time = GetSequenceDurationMS();
            while (time <= end_time) {
                int next_time = (time + interval <= end_time) ? time + interval : end_time;
                int startTime = RoundToMultipleOfPeriod(time, GetFrequency());
                int endTime = RoundToMultipleOfPeriod(next_time, GetFrequency());
                effectLayer->AddEffect(0, "", "", "", startTime, endTime, EFFECT_NOT_SELECTED, false);
                time += interval;
            }
        }
    } else {
        PendingTiming pt;
        pt.name = interval_name;
        if (interval_name != "Empty" && (interval_name == "25ms" || interval_name == "50ms" || interval_name == "100ms" || EndsWith(interval_name, "ms Metronome"))) {
            pt.fixedInterval = std::strtol((interval_name).c_str(), nullptr, 10);
        }
        mPendingTimings.push_back(std::move(pt));
        timing_list.push_back(interval_name);
    }
}

void SequenceFile::AddFixedTimingSection(const std::string& interval_name, int interval, RenderContext* xLightsParent)
{
    if (sequence_loaded) {
        TimingElement* element = xLightsParent->AddTimingElement(interval_name);
        element->SetFixedTiming(interval);
        EffectLayer* effectLayer = element->GetEffectLayer(0);
        int time = 0;
        int end_time = GetSequenceDurationMS();
        while (time <= end_time) {
            int next_time = (time + interval <= end_time) ? time + interval : end_time;
            int startTime = RoundToMultipleOfPeriod(time, GetFrequency());
            int endTime = RoundToMultipleOfPeriod(next_time, GetFrequency());
            effectLayer->AddEffect(0, "", "", "", startTime, endTime, EFFECT_NOT_SELECTED, false);
            time += interval;
        }
    } else {
        PendingTiming pt;
        pt.name = interval_name;
        pt.fixedInterval = interval;
        mPendingTimings.push_back(std::move(pt));
        timing_list.push_back(interval_name);
    }
}

void SequenceFile::AddMetronomeLabelTimingSection(const std::string& interval_name, int _interval, const std::vector<std::string>& tags, RenderContext* xLightsParent, int minForRandomRange, bool randomLabels) {
    if (!sequence_loaded) {
        PendingTiming pt;
        pt.name = interval_name;
        pt.fixedInterval = _interval;
        pt.tags = tags;
        pt.minForRandomRange = minForRandomRange;
        pt.randomLabels = randomLabels;
        mPendingTimings.push_back(std::move(pt));
        timing_list.push_back(interval_name);
        return;
    }

    std::vector<std::string> effectiveTags = tags;
    if (effectiveTags.empty()) {
        for (int i = 1; i <= 10; ++i) {
            effectiveTags.push_back(std::to_string(i));
        }
    }

    TimingElement* element = xLightsParent->AddTimingElement(interval_name);
    EffectLayer* effectLayer = element->GetEffectLayer(0);
    int time{ 0 };
    int id{ 0 };
    int end_time = GetSequenceDurationMS();
    int lastRandomState = -1;
    while (time < end_time) {
        int interval = minForRandomRange == -1 ? _interval : intRand(minForRandomRange, _interval);
        int next_time = (time + interval <= end_time) ? time + interval : end_time;
        int startTime = RoundToMultipleOfPeriod(time, GetFrequency());
        int endTime = RoundToMultipleOfPeriod(next_time, GetFrequency());

        std::string label;
        if (randomLabels) {
            int tagIndex;
            do {
                tagIndex = intRand(0, effectiveTags.size() - 1);
            } while (tagIndex == lastRandomState && effectiveTags.size() > 1);
            lastRandomState = tagIndex;
            label = effectiveTags[tagIndex];
        } else {
            label = effectiveTags[id % effectiveTags.size()];
        }

        effectLayer->AddEffect(0, label, "", "", startTime, endTime, EFFECT_NOT_SELECTED, false);
        time += interval;
        id++;
    }
}

void SequenceFile::ApplyPendingTimings(RenderContext* xLightsParent)
{
    if (mPendingTimings.empty()) return;

    for (auto& pt : mPendingTimings) {
        if (!pt.starts.empty()) {
            // VAMP/audacity timing with starts/ends/labels
            AddNewTimingSection(pt.name, xLightsParent, pt.starts, pt.ends, pt.labels);
        } else if (!pt.tags.empty()) {
            // Metronome with tags
            AddMetronomeLabelTimingSection(pt.name, pt.fixedInterval, pt.tags, xLightsParent, pt.minForRandomRange, pt.randomLabels);
        } else if (pt.fixedInterval > 0) {
            // Fixed timing with explicit interval
            AddFixedTimingSection(pt.name, pt.fixedInterval, xLightsParent);
        } else if (!pt.subType.empty() || pt.fixedInterval == 0) {
            // Simple timing section (possibly with subType)
            if (pt.name == "Empty" || (pt.name != "25ms" && pt.name != "50ms" && pt.name != "100ms" && !EndsWith(pt.name, "ms Metronome"))) {
                AddNewTimingSection(pt.name, xLightsParent, pt.subType);
            } else {
                AddFixedTimingSection(pt.name, xLightsParent);
            }
        }
    }
    mPendingTimings.clear();
}

void SequenceFile::SetMetaMP3Tags()
{
    if (audio != nullptr)
    {
        SetHeaderInfo(HEADER_INFO_TYPES::SONG, audio->Title());
        SetHeaderInfo(HEADER_INFO_TYPES::ARTIST, audio->Artist());
        SetHeaderInfo(HEADER_INFO_TYPES::ALBUM, audio->Album());
    }
}

std::string SequenceFile::GetFSEQForXSQ(const std::string& xsq, const std::string& fseqDirectory)
{
    if (!::FileExists(xsq))
        return "";

    std::filesystem::path fn(xsq);
    fn.replace_extension("fseq");

    if (!::FileExists(fn.string())) {
        fn = std::filesystem::path(fseqDirectory) / fn.filename();

        if (!::FileExists(fn.string())) {
            return "";
        }
    }

    return fn.string();
}

std::string SequenceFile::GetMediaForXSQ(const std::string& xsq, const std::string& showDir, const std::list<std::string> mediaFolders)
{
    if (!::FileExists(xsq))
        return "";

    XsqFileInfo info = ScanXsqFile(xsq);
    std::string mediaName = info.mediaFile;

    if (!mediaName.empty()) {
        if (!::FileExists(mediaName)) {
            std::string mediaFilename = std::filesystem::path(mediaName).filename().string();
            for (const std::string& md : mediaFolders) {
                std::string tmn = md + ::GetPathSeparator() + mediaFilename;
                if (::FileExists(tmn)) {
                    mediaName = tmn;
                    break;
                }
            }
            if (!::FileExists(mediaName)) {
                const std::string fixedMN = FileUtils::FixFile(showDir, mediaName);
                if (!::FileExists(fixedMN)) {
                    mediaName = "";
                } else {
                    mediaName = fixedMN;
                }
            }
        }
    }

    return mediaName;
}

void SequenceFile::AdjustEffectSettingsForVersion(SequenceElements& elements, RenderContext* ctx)
{
    std::string ver = GetVersion();
    std::vector<RenderableEffect*> effects(ctx->GetEffectManager().size());
    int count = 0;
    for (int x = 0; x < (int)ctx->GetEffectManager().size(); x++) {
        RenderableEffect* eff = ctx->GetEffectManager()[x];
        if (eff->needToAdjustSettings(ver)) {
            effects[x] = eff;
            count++;
        }
    }
    if (count > 0) {
        for (size_t i = 0; i < elements.GetElementCount(); i++) {
            Element* elem = elements.GetElement(i);
            if (elem->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
                ModelElement* me = dynamic_cast<ModelElement*>(elem);
                for (int j = 0; j < (int)elem->GetEffectLayerCount(); j++) {
                    EffectLayer* layer = elem->GetEffectLayer(j);
                    for (int k = 0; k < (int)layer->GetEffectCount(); k++) {
                        Effect* eff = layer->GetEffect(k);
                        if (eff != nullptr && eff->GetEffectIndex() >= 0 && effects[eff->GetEffectIndex()] != nullptr) {
                            effects[eff->GetEffectIndex()]->adjustSettings(ver, eff);
                        }
                    }
                }
                for (int s = 0; s < me->GetSubModelAndStrandCount(); s++) {
                    SubModelElement* se = me->GetSubModel(s);
                    for (int j = 0; j < (int)se->GetEffectLayerCount(); j++) {
                        EffectLayer* layer = se->GetEffectLayer(j);
                        for (int k = 0; k < (int)layer->GetEffectCount(); k++) {
                            Effect* eff = layer->GetEffect(k);
                            if (eff != nullptr && eff->GetEffectIndex() >= 0 && effects[eff->GetEffectIndex()] != nullptr) {
                                effects[eff->GetEffectIndex()]->adjustSettings(ver, eff);
                            }
                        }
                    }
                    if (se->GetType() == ElementType::ELEMENT_TYPE_STRAND) {
                        StrandElement* ste = dynamic_cast<StrandElement*>(se);
                        for (int k = 0; k < ste->GetNodeLayerCount(); k++) {
                            NodeLayer* nlayer = ste->GetNodeLayer(k);
                            for (int l = 0; l < nlayer->GetEffectCount(); l++) {
                                Effect* eff = nlayer->GetEffect(l);
                                if (eff != nullptr && eff->GetEffectIndex() >= 0 && effects[eff->GetEffectIndex()] != nullptr) {
                                    effects[eff->GetEffectIndex()]->adjustSettings(ver, eff);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

static void ReplaceAll(std::string& str, const std::string& from, const std::string& to)
{
    size_t pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos) {
        str.replace(pos, from.size(), to);
        pos += to.size();
    }
}

std::string SequenceFile::InsertMissing(std::string str, std::string missing_array, bool INSERT)
{
    // Tokenize by "|" - consume first two tokens, then process pairs
    std::vector<std::string> tokens;
    size_t start = 0;
    while (start < missing_array.size()) {
        auto pos = missing_array.find('|', start);
        tokens.push_back(missing_array.substr(start, pos - start));
        if (pos == std::string::npos) break;
        start = pos + 1;
    }

    // Skip first two tokens, then process pairs
    for (size_t i = 2; i + 1 < tokens.size(); i += 2) {
        const std::string& token1 = tokens[i];
        const std::string& token2 = tokens[i + 1];
        auto pos = str.find(token1);
        if (pos == std::string::npos && INSERT) {
            std::string replacement = "," + token2 + "</td>";
            ReplaceAll(str, "</td>", replacement);
        } else if (pos != std::string::npos && pos > 0 && !INSERT) {
            ReplaceAll(str, token1, token2);
        }
    }
    return str;
}
