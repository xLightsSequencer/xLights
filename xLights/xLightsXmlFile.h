#ifndef XLIGHTSXMLFILE_H
#define XLIGHTSXMLFILE_H

#include <wx/filename.h>
#include <wx/xml/xml.h>
#include <wx/textctrl.h>
#include "sequencer/SequenceElements.h"
#include "DataLayer.h"

class SequenceElements;  // forward declaration needed due to circular dependency

class xLightsXmlFile : public wxFileName
{
    public:
        //xLightsXmlFile();
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

        const wxString HEADER_STRINGS[NUM_TYPES] =
        {
            "author",
            "author-email",
            "author-website",
            "song",
            "artist",
            "album",
            "MusicURL",
            "comment"
        };

        bool Open();

        void Save( SequenceElements& elements);
        wxXmlDocument& GetXmlDocument() { return seqDocument; }
        DataLayerSet& GetDataLayers() { return mDataLayers; }

        const wxString GetVersion() { return version_string; };

        int GetSequenceDurationMS() const { return int(seq_duration * 1000); }
        double GetSequenceDurationDouble() const { return seq_duration; }
        wxString GetSequenceDurationString() const;

        void SetSequenceDurationMS(int length);
        void SetSequenceDuration(const wxString& length);
        void SetSequenceDuration(double length);

        const wxString GetSequenceTiming() const { return seq_timing; }
        void SetSequenceTiming(  const wxString& timing );

        const wxString GetSequenceType() const { return seq_type; }
        void SetSequenceType( const wxString& type );

        const wxString GetMediaFile() const { return media_file; }
        void SetMediaFile( const wxString& filename, bool overwrite_tags );

        wxString GetHeaderInfo(HEADER_INFO_TYPES node_type) { return header_info[node_type]; }
        void SetHeaderInfo(HEADER_INFO_TYPES node_type, const wxString& node_value);

        void AddFixedTimingSection(wxString interval_name);
        void DeleteTimingSection(wxString section);
        void SetTimingSectionName(wxString section, wxString name);
        wxArrayString GetTimingList() { return timing_list; }
        void ProcessAudacityTimingFiles(const wxString& dir, const wxArrayString& filenames);

        bool IsOpen() { return is_open; }
        bool HasAudioMedia() { return has_audio_media; }
        int GetNumModels() { return models.GetCount(); }
        bool WasConverted() { return was_converted; }
        void AcknowledgeConversion() { was_converted = false; }  // called to turn off conversion warning
        bool IsV3Sequence();
        bool ExtractMetaTagsFromMP3(wxString filename);

        // static methods
        static void FixVersionDifferences(const wxString& filename);
        static bool IsXmlSequence(wxFileName &fname);

    protected:
    private:
        wxXmlDocument seqDocument;
        wxArrayString models;
        wxArrayString header_info;
        wxArrayString timing_list;
        wxString version_string;
        double seq_duration;
        wxString media_file;
        wxString seq_type;
        wxString seq_timing;
        bool is_open;
        bool has_audio_media;
        bool was_converted;
        DataLayerSet mDataLayers;

        void CreateNew();
        bool LoadSequence();
        bool LoadV3Sequence();
        bool Save();
        bool SaveCopy();
        void AddTimingDisplayElement( const wxString& name, const wxString& visible, const wxString& active );
        void AddDisplayElement( const wxString& name, const wxString& type, const wxString& visible, const wxString& collapsed, const wxString& active );
        wxXmlNode* AddElement( const wxString& name, const wxString& type );
        void AddEffect( wxXmlNode* node,
                        const wxString& name,
                        const wxString& data,
                        const wxString& protection,
                        const wxString& selected,
                        const wxString& id,
                        const wxString& start_time,
                        const wxString& end_time );
        void AddTimingEffect( wxXmlNode* node,
                              const wxString& label,
                              const wxString& protection,
                              const wxString& selected,
                              const wxString& start_time,
                              const wxString& end_time);
        wxXmlNode* AddChildXmlNode(wxXmlNode* node, const wxString& node_name, const wxString& node_data);
        wxXmlNode* AddChildXmlNode(wxXmlNode* node, const wxString& node_name);
        wxXmlNode* InsertChildXmlNode(wxXmlNode* node, wxXmlNode* following_node, const wxString& node_name);
        wxXmlNode* AddFixedTiming( const wxString& name, const wxString& timing );
        void SetNodeContent(wxXmlNode* node, const wxString& content);
        void CleanUpEffects();
        void UpdateNextId( const wxString& value );

        void FixVersionDifferences();

        void SetSequenceDuration(const wxString& length, wxXmlNode* node);

        static wxString InsertMissing(wxString str, wxString missing_array, bool INSERT);
};

#endif // XLIGHTSXMLFILE_H
