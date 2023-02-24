#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/filename.h>
#include <wx/xml/xml.h>
#include "sequencer/SequenceElements.h"
#include "DataLayer.h"
#include "AudioManager.h"
#include "Vixen3.h"

class SequenceElements;  // forward declaration needed due to circular dependency
class xLightsFrame;

WX_DECLARE_STRING_HASH_MAP( int, StringIntMap );

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

class xLightsXmlFile : public wxFileName
{
public:
    // xLightsXmlFile();
    xLightsXmlFile(const wxFileName& filename, uint32_t frameMS = 0);
    virtual ~xLightsXmlFile();

    const wxString HEADER_STRINGS[static_cast<int>(HEADER_INFO_TYPES::NUM_TYPES)] = {
        "author",
        "author-email",
        "author-website",
        "song",
        "artist",
        "album",
        "MusicURL",
        "comment"
    };

    static const wxString ERASE_MODE;
    static const wxString CANVAS_MODE;

    bool Open(const wxString& ShowDir, bool ignore_audio = false);

    void AddJukebox(wxXmlNode* node);
    void Save(SequenceElements& elements);
    wxXmlDocument& GetXmlDocument()
    {
        return seqDocument;
    }
    DataLayerSet& GetDataLayers()
    {
        return mDataLayers;
    }

    const wxString& GetVersion() const
    {
        return version_string;
    };

    int GetSequenceDurationMS() const
    {
        return int(seq_duration * 1000);
    }
    double GetSequenceDurationDouble() const
    {
        return seq_duration;
    }
    wxString GetSequenceDurationString() const;

    void SetSequenceDurationMS(int length);
    void SetSequenceDuration(const wxString& length);
    void SetSequenceDuration(double length);

    const wxString& GetSequenceTiming() const
    {
        return seq_timing;
    }
    void SetSequenceTiming(const wxString& timing);
    int GetFrameMS() const;
    int GetFrequency() const;

    const wxString& GetSequenceType() const
    {
        return seq_type;
    }
    void SetSequenceType(const wxString& type);

    // const wxString GetMediaFile() const { return media_file; }
    AudioManager* GetMedia() const
    {
        return audio;
    }
    const wxString& GetMediaFile() const
    {
        return media_file;
    }
    void SetMediaFile(const wxString& ShowDir, const wxString& filename, bool overwrite_tags);
    void ClearMediaFile();

    wxString GetHeaderInfo(HEADER_INFO_TYPES node_type) const;
    void SetHeaderInfo(HEADER_INFO_TYPES node_type, const wxString& node_value);

    wxString GetImageDir(wxWindow* parent);
    void SetImageDir(const wxString& dir);

    void SetSequenceLoaded(bool value)
    {
        sequence_loaded = value;
    }
    bool GetSequenceLoaded() const
    {
        return sequence_loaded;
    }

    void AddNewTimingSection(const std::string& interval_name, xLightsFrame* xLightsParent, const std::string& subType = "");
    void AddNewTimingSection(const std::string& interval_name, xLightsFrame* xLightsParent, std::vector<int>& starts,
                             std::vector<int>& ends, std::vector<std::string>& labels);
    void AddFixedTimingSection(const std::string& interval_name, xLightsFrame* xLightsParent);
    void DeleteTimingSection(const std::string& section);
    void SetTimingSectionName(const std::string& section, const std::string& name);
    bool TimingAlreadyExists(const std::string& section, xLightsFrame* xLightsParent);
    wxArrayString GetTimingList() const
    {
        return timing_list;
    }
    wxArrayString GetTimingList(const SequenceElements& seq_elements);
    void ProcessAudacityTimingFiles(const wxString& dir, const wxArrayString& filenames, xLightsFrame* xLightsParent);
    void ProcessLorTiming(const wxString& dir, const wxArrayString& filenames, xLightsFrame* xLightsParent);
    void ProcessXTiming(const wxString& dir, const wxArrayString& filenames, xLightsFrame* xLightsParent);
    void ProcessXTiming(wxXmlNode* node, xLightsFrame* xLightsParent);
    void ProcessPapagayo(const wxString& dir, const wxArrayString& filenames, xLightsFrame* xLightsParent);
    void ProcessSRT(const wxString& dir, const wxArrayString& filenames, xLightsFrame* xLightsParent);
    void ProcessLSPTiming(const wxString& dir, const wxArrayString& filenames, xLightsFrame* xLightsParent);
    void ProcessXLightsTiming(const wxString& dir, const wxArrayString& filenames, xLightsFrame* xLightsParent);
    void ProcessVixen3Timing(const wxString& dir, const wxArrayString& filenames, xLightsFrame* xLightsParent);
    static void AddMarksToLayer(const std::list<VixenTiming>& marks, EffectLayer* effectLayer, int frameMS);
    wxString UniqueTimingName(xLightsFrame* xLightsParent, wxString name) const;
    void UpdateVersion();
    void UpdateVersion(const std::string& version);
    void AdjustEffectSettingsForVersion(SequenceElements& elements, xLightsFrame* xLightsParent);

    bool IsOpen() const
    {
        return is_open;
    }
    bool HasAudioMedia() const
    {
        return audio != nullptr;
    }
    int GetNumModels() const
    {
        return models.GetCount();
    }
    bool WasConverted() const
    {
        return was_converted;
    }
    void AcknowledgeConversion()
    {
        was_converted = false;
    } // called to turn off conversion warning
    bool IsV3Sequence() const;
    bool NeedsTimesCorrected() const;
    void ConvertToFixedPointTiming();
    void SetMetaMP3Tags();
    static std::string GetFSEQForXSQ(const std::string& xsq, const std::string& fseqDirectory);
    static std::string GetMediaForXSQ(const std::string& xsq, const std::string& showDir, const std::list<std::string> mediaFolders);

    void SetRenderMode(const wxString& mode);
    wxString GetRenderMode();

    bool supportsModelBlending() const
    {
        return supports_model_blending;
    }
    void setSupportsModelBlending(bool b)
    {
        supports_model_blending = b;
    }

    int GetLastView() const;

    wxXmlNode* GetPalettesNode() const;

    // static methods
    static void FixVersionDifferences(const wxString& filename);
    static void FixEffectPresets(wxXmlNode* effects_node);
    static bool IsXmlSequence(wxFileName& fname);

private:
    wxXmlDocument seqDocument;
    wxArrayString models;
    wxArrayString header_info;
    wxArrayString timing_list;
    wxString version_string;
    double seq_duration = 0;
    wxString media_file;
    wxString seq_type;
    wxString seq_timing;
    wxString image_dir;
    bool supports_model_blending = false;
    bool is_open = false;
    bool was_converted = false;
    bool sequence_loaded = false; // flag to indicate the sequencer has been loaded with this xml data
    DataLayerSet mDataLayers;
    AudioManager* audio = nullptr;

    void CreateNew();
    bool LoadSequence(const wxString& ShowDir, bool ignore_audio = false);
    bool LoadV3Sequence();
    bool Save();
    bool SaveCopy() const;
    void AddTimingDisplayElement(const wxString& name, const wxString& visible, const wxString& active, const wxString &subType = "");
    void AddDisplayElement(const wxString& name, const wxString& type, const wxString& visible, const wxString& collapsed, const wxString& active, const wxString& renderDisabled);
    wxXmlNode* AddElement(const wxString& name, const wxString& type);
    int AddColorPalette(StringIntMap& paletteCache, const wxString& palette);
    void AddEffect(wxXmlNode* node,
                   StringIntMap& paletteCache,
                   const wxString& name,
                   const wxString& data,
                   const wxString& protection,
                   const wxString& selected,
                   const wxString& id,
                   const wxString& start_time,
                   const wxString& end_time);
    void AddTimingEffect(wxXmlNode* node,
                         const wxString& label,
                         const wxString& protection,
                         const wxString& selected,
                         const wxString& start_time,
                         const wxString& end_time);
    wxXmlNode* AddChildXmlNode(wxXmlNode* node, const wxString& node_name, const wxString& node_data);
    wxXmlNode* AddChildXmlNode(wxXmlNode* node, const wxString& node_name);
    wxXmlNode* InsertChildXmlNode(wxXmlNode* node, wxXmlNode* following_node, const wxString& node_name);
    wxXmlNode* AddFixedTiming(const wxString& name, const wxString& timing);
    void SetNodeContent(wxXmlNode* node, const wxString& content);
    void CleanUpEffects() const;
    void UpdateNextId(const wxString& value);

    // void FixVersionDifferences();

    // void SetSequenceDuration(const wxString& length, wxXmlNode* node);

    static wxString InsertMissing(wxString str, wxString missing_array, bool INSERT);

    void WriteEffects(EffectLayer* layer,
                      wxXmlNode* effect_layer_node,
                      StringIntMap& colorPalettes,
                      wxXmlNode* colorPalette_node,
                      StringIntMap& effectStrings,
                      wxXmlNode* effectDB_Node);
};
