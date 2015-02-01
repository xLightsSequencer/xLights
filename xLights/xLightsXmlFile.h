#ifndef XLIGHTSXMLFILE_H
#define XLIGHTSXMLFILE_H

#include <wx/filename.h>
#include <wx/xml/xml.h>
#include <wx/textctrl.h>
#include "sequencer/SequenceElements.h"

class SequenceElements;  // forward declaration needed due to circular dependency

class xLightsXmlFile : public wxFileName
{
    public:
        xLightsXmlFile();
        xLightsXmlFile(const wxFileName &filename);
        virtual ~xLightsXmlFile();

        enum HEADER_INFO_TYPES
        {
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

        bool Load();
        void New();
        bool Convert();
        void Save();
        void Save( SequenceElements& elements);
        void Clear();
        void FreeMemory();
        wxXmlDocument& GetXmlDocument() { return seqDocument; }

        bool NeedsConversion() { return needs_conversion; }

        const wxString GetVersion() { return version_string; };

        int GetSequenceDurationMS() const { return int(seq_duration * 1000); }
        double GetSequenceDurationDouble() const { return seq_duration; }
        wxString GetSequenceDurationString() const;

        void SetSequenceDurationMS(int length);
        void SetSequenceDuration(const wxString& length);

        const wxString GetSequenceTiming() const { return seq_timing; }
        void SetSequenceTiming(  const wxString& timing );

        const wxString GetSequenceType() const { return seq_type; }
        void SetSequenceType( const wxString& type );

        const wxString GetMediaFile() const { return media_file; }
        void SetMediaFile( const wxString& filename );

        wxString GetHeaderInfo(HEADER_INFO_TYPES val) { return header_info[val]; }
        void SetHeaderInfo(wxArrayString info);

        void AddFixedTimingSection(wxString interval_name);
        void DeleteTimingSection(wxString section);
        void SetTimingSectionName(wxString section, wxString name);
        wxArrayString GetTimingList() { return timing_list; }
        void ProcessAudacityTimingFiles(const wxString& dir, const wxArrayString& filenames);

        bool IsLoaded() { return is_loaded; }
        bool HasAudioMedia() { return has_audio_media; }
        int GetNumModels() { return models.GetCount(); }
        void FixVersionDifferences();

    protected:
    private:
        wxArrayString models;
        wxArrayString timing_protection;
        wxArrayString timing;
        wxArrayString label_protection;
        wxArrayString labels;
        wxArrayString effect_protection;
        wxArrayString effects;
        wxXmlDocument seqDocument;
        wxArrayString header_info;
        wxArrayString timing_list;
        bool is_loaded;
        bool needs_conversion;
        wxString version_string;
        wxString latest_version;
        double seq_duration;
        wxString media_file;
        wxString seq_type;
        wxString seq_timing;
        bool has_audio_media;

        void Init();
        wxXmlNode* AddChildXmlNode(wxXmlNode* node, const wxString& node_name, const wxString& node_data);
        wxXmlNode* AddChildXmlNode(wxXmlNode* node, const wxString& node_name);
        void AddTimingAttributes(wxXmlNode* node, const wxString& name, const wxString& visible, const wxString& active);
        void SetNodeContent(wxXmlNode* node, const wxString& content);
        wxString InsertMissing(wxString str, wxString missing_array, bool INSERT);
        void SearchForMedia();
        void SetSequenceDuration(const wxString& length, wxXmlNode* node);
        void SetSequenceDuration(double length);

};

#endif // XLIGHTSXMLFILE_H
