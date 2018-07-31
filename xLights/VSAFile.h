#ifndef VSAFILE_H
#define VSAFILE_H

#include <wx/wx.h>
#include <string>

class wxFile;

class VSAFile
{
    public:
        VSAFile();
        VSAFile(const std::string& filename);
		void Load(const std::string& filename);
		virtual ~VSAFile();
		bool IsOk() const { return _ok; }
        void Close();

        struct vsaEventRecord {
            uint16_t track;
            uint32_t start_time;
            uint32_t end_time;
            uint32_t start_pos;
            uint32_t end_pos;
            std::string text;
            uint8_t data[16];
        };

        struct vsaAudioRecord {
            std::string name;
            std::string devices;
        };

        struct vsaVideoRecord {
            std::string name;
            std::string audio;
            std::string monitor;
            bool fullscreen;
            uint16_t x_offset;
            uint16_t y_offset;
        };

        struct vsaTrackRecord {
            std::string name;
            uint32_t address;
            uint8_t controller;
            uint8_t data1[11];
            uint32_t max_limit;
            uint32_t min_limit;
            uint32_t default_value;
            uint32_t enable;
            std::string port;
            uint8_t data2[12];
        };

        enum vsaControllers {
            MINISSC_SERVO,
            SV203_SERVO,
            SMI_MOTOR,
            MINISSC_RELAY,
            SV203_RELAY,
            DMX_SERVO,
            LPT_RELAY,
            DMX_RELAY,
            LPT_SERVO,
            MINISSC_DIMMER,
            LPT_DIMMER,
            DMX_DIMMER,
            PARALLAX_SERVO,
            POLOLU_SERVO,
            PICOPIC_SERVO,
            PICOPIC_RELAY,
            SSC32_SERVO,
            SSC32_RELAY,
            K108A_RELAY,
            BOC_SERVO,
            ENDURANCERC_SERVO,
            MAESTRO_SERVO,
            JRK_MOTOR,
            MAESTRO_RELAY
        };

        uint32_t GetNumActiveTracks() { return _num_active_tracks; }
        uint32_t GetTiming() { return _timing; }

        const std::vector< vsaTrackRecord > & GetTrackInfo() { return _tracks; }
        const std::vector< std::vector< vsaEventRecord > > & GetEventInfo() { return _events; }

    protected:
        void init();

    private:
        std::string _filename;
        size_t _frames;
        size_t _channelsPerFrame;
        size_t _frameMS;
        bool _ok;
        wxFile* _fh;
        uint32_t _num_tracks;
        uint32_t _num_audio;
        uint32_t _num_active_tracks;
        uint32_t _timing;
        std::vector< std::vector< vsaEventRecord > > _events;
        std::vector< vsaAudioRecord > _audio;
        std::vector< vsaVideoRecord > _video;
        std::vector< vsaTrackRecord > _tracks;
};

#endif // VSAFILE_H
