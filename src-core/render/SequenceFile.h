#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "DataLayer.h"
#include "JukeboxButtonData.h"
#include "pugixml.hpp"

#include <array>
#include <optional>
#include <string>
#include <vector>
#include <filesystem>

class SequenceElements;
class RenderContext;
class UICallbacks;
class AudioManager;
class EffectLayer;

enum class HEADER_INFO_TYPES {
    AUTHOR,
    AUTHOR_EMAIL,
    WEBSITE,
    SONG,
    ARTIST,
    ALBUM,
    URL,
    COMMENT,
    NUM_TYPES
};

struct PendingTiming {
    std::string name;
    std::string subType;
    int fixedInterval = 0;          // 0 = not fixed
    std::vector<int> starts;        // for VAMP/audacity timings
    std::vector<int> ends;
    std::vector<std::string> labels;
    // Metronome fields
    std::vector<std::string> tags;
    int minForRandomRange = -1;
    bool randomLabels = false;
};

struct AlternateAudioTrack {
    std::string path;       // resolved absolute path
    std::string shortname;  // user-defined label, e.g. "Drums"; "" → auto "Track1", "Track2"
    AudioManager* audio = nullptr;
};

class SequenceFile
{
    std::string mFilePath;

public:
    SequenceFile(const std::string& filepath, uint32_t frameMS = 0);
    ~SequenceFile();

    SequenceFile(const SequenceFile&) = delete;
    SequenceFile& operator=(const SequenceFile&) = delete;

    // Path accessors
    std::string GetFullPath() const { return mFilePath; }
    std::string GetName() const { return std::filesystem::path(mFilePath).stem().string(); }
    std::string GetFullName() const {
        std::filesystem::path p(mFilePath);
        std::error_code ec;
        if (std::filesystem::is_directory(p, ec)) return "";
        return p.filename().string();
    }
    std::string GetPath() const {
        std::filesystem::path p(mFilePath);
        std::error_code ec;
        if (std::filesystem::is_directory(p, ec)) return mFilePath;
        return p.parent_path().string();
    }
    std::string GetExt() const;
    void SetExt(const std::string& ext);
    void SetFullPath(const std::string& path) { mFilePath = path; }
    bool FileExists() const;

    static const std::string ERASE_MODE;
    static const std::string CANVAS_MODE;

    std::optional<pugi::xml_document> Open(const std::string& ShowDir, bool ignore_audio, const std::string& realFilePath);

    bool Save(SequenceElements& elements);
    bool BuildDocument(pugi::xml_document& doc, SequenceElements& elements);

    void ApplyPendingTimings(RenderContext* renderContext);
    DataLayerSet& GetDataLayers() { return mDataLayers; }

    const std::string& GetVersion() const { return version_string; }

    int GetSequenceDurationMS() const { return int(seq_duration * 1000); }
    double GetSequenceDurationDouble() const { return seq_duration; }
    std::string GetSequenceDurationString() const;

    void SetSequenceDurationMS(int length);
    void SetSequenceDuration(const std::string& length);
    void SetSequenceDuration(double length);

    const std::string& GetSequenceTiming() const { return seq_timing; }
    void SetSequenceTiming(const std::string& timing);
    int GetFrameMS() const;
    int GetFrequency() const;

    const std::string& GetSequenceType() const { return seq_type; }
    void SetSequenceType(const std::string& type);

    AudioManager* GetMedia() const { return audio; }
    const std::string& GetMediaFile() const { return media_file; }
    void SetMediaFile(const std::string& ShowDir, const std::string& filename, bool overwrite_tags);
    void ClearMediaFile();

    // Alternate audio tracks (stems)
    int GetAltTrackCount() const { return (int)alt_tracks.size(); }
    const AlternateAudioTrack& GetAltTrack(int idx) const { return alt_tracks[idx]; }
    void AddAltTrack(const std::string& ShowDir, const std::string& path, const std::string& shortname = "");
    void RemoveAltTrack(int idx);
    void SetAltTrackPath(const std::string& ShowDir, int idx, const std::string& path);
    void SetAltTrackShortname(int idx, const std::string& name);
    std::string GetAltTrackDisplayName(int idx) const;
    AudioManager* GetAltTrackMedia(int idx) const { return alt_tracks[idx].audio; }

    const std::string& GetHeaderInfo(HEADER_INFO_TYPES node_type) const;
    void SetHeaderInfo(HEADER_INFO_TYPES node_type, const std::string& node_value);

    std::string GetImageDir(UICallbacks* ui);
    void SetImageDir(const std::string& dir);

    void SetSequenceLoaded(bool value) { sequence_loaded = value; }
    bool GetSequenceLoaded() const { return sequence_loaded; }

    void AddNewTimingSection(const std::string& interval_name, RenderContext* renderContext, const std::string& subType = "");
    void AddNewTimingSection(const std::string& interval_name, RenderContext* renderContext, std::vector<int>& starts,
                             std::vector<int>& ends, std::vector<std::string>& labels);
    void AddFixedTimingSection(const std::string& interval_name, RenderContext* renderContext);
    void AddFixedTimingSection(const std::string& interval_name, int interval_ms, RenderContext* renderContext);
    void AddMetronomeLabelTimingSection(const std::string& interval_name, int interval, const std::vector<std::string>& tags,  RenderContext* renderContext, int minForRandomRange = -1, bool randomTags = false);
    void DeleteTimingSection(const std::string& section);
    void SetTimingSectionName(const std::string& section, const std::string& name);
    bool TimingAlreadyExists(const std::string& section, RenderContext* renderContext);
    bool TimingMatchesModelName(const std::string& section, RenderContext* renderContext);
    std::vector<std::string> GetTimingList() const { return timing_list; }
    std::vector<std::string> GetTimingList(const SequenceElements& seq_elements);
    void ProcessAudacityTimingFiles(const std::vector<std::string>& filenames, RenderContext* renderContext);
    void ProcessLorTiming(const std::vector<std::string>& filenames, RenderContext* renderContext);
    void ProcessXTiming(const std::vector<std::string>& filenames, RenderContext* renderContext);
    void ProcessXTiming(const pugi::xml_node& node, RenderContext* renderContext);
    void ProcessPapagayo(const std::vector<std::string>& filenames, RenderContext* renderContext);
    void ProcessSRT(const std::vector<std::string>& filenames, RenderContext* renderContext);
    void ProcessLSPTiming(const std::vector<std::string>& filenames, RenderContext* renderContext);
    void ProcessXLightsTiming(const std::vector<std::string>& filenames, RenderContext* renderContext);
    void ProcessVixen3Timing(const std::vector<std::string>& filenames, RenderContext* renderContext);
    void ProcessElevenLabsTimingFiles(const std::vector<std::string>& filenames, RenderContext* xLightsParent);
    std::string UniqueTimingName(RenderContext* renderContext, std::string name) const;
    void UpdateVersion();
    void UpdateVersion(const std::string& version);
    void AdjustEffectSettingsForVersion(SequenceElements& elements, RenderContext* ctx);

    bool IsOpen() const { return is_open; }
    bool HasAudioMedia() const { return audio != nullptr; }
    int GetNumModels() const { return models.size(); }
    bool WasConverted() const { return was_converted; }
    void AcknowledgeConversion() { was_converted = false; }
    bool IsV3Sequence() const;
    bool NeedsTimesCorrected() const;
    void SetMetaMP3Tags();
    static std::string GetFSEQForXSQ(const std::string& xsq, const std::string& fseqDirectory);
    static std::string GetMediaForXSQ(const std::string& xsq, const std::string& showDir, const std::list<std::string> mediaFolders);

    void SetRenderMode(const std::string& mode);
    std::string GetRenderMode();

    bool supportsModelBlending() const { return supports_model_blending; }
    void setSupportsModelBlending(bool b) { supports_model_blending = b; }

    // Jukebox button data (sequence-owned, UI syncs from here)
    JukeboxButtonMap& GetJukeboxButtons() { return _jukeboxButtons; }
    const JukeboxButtonMap& GetJukeboxButtons() const { return _jukeboxButtons; }

    int GetLastView() const { return mLastView; }

    // static methods
    static bool IsXmlSequence(const std::string& filepath);

private:
    std::vector<std::string> models;
    std::array<std::string, (int)HEADER_INFO_TYPES::NUM_TYPES> header_info;
    std::vector<std::string> timing_list;
    std::vector<PendingTiming> mPendingTimings;
    std::string version_string;
    double seq_duration = 0;
    std::string media_file;
    std::string seq_type;
    std::string seq_timing;
    std::string image_dir;
    bool supports_model_blending = false;
    int mLastView = 0;
    bool is_open = false;
    bool was_converted = false;
    bool sequence_loaded = false;
    DataLayerSet mDataLayers;
    AudioManager* audio = nullptr;
    std::vector<AlternateAudioTrack> alt_tracks;
    JukeboxButtonMap _jukeboxButtons;

    void CreateNew();
    std::optional<pugi::xml_document> LoadSequence(const std::string& ShowDir, bool ignore_audio, const std::string& realFilePath);
    bool SaveCopy() const;

    static std::string InsertMissing(std::string str, std::string missing_array, bool INSERT);
};
