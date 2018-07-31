#include "VSAFile.h"
#include <wx/file.h>
#include <log4cpp/Category.hh>
#include <wx/filename.h>

VSAFile::VSAFile()
{
    init();
}

VSAFile::VSAFile(const std::string& filename)
{
    init();
    Load(filename);
}

void VSAFile::init()
{
    _filename = "";
    _frames = 0;
    _channelsPerFrame = 0;
    _frameMS = 0;
    _ok = false;
    _fh = nullptr;
    _num_tracks = 0;
    _num_audio = 0;
    _num_active_tracks = 0;
}

VSAFile::~VSAFile()
{
    Close();
}

void VSAFile::Load(const std::string& filename)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    Close();

    _filename = filename;
    _fh = new wxFile(filename);
    uint32_t num_evt_tracks = 0;

    if (_fh->IsOpened())
    {
        uint8_t version[12];
        _fh->Read(version, 12);
        uint8_t num_bytes;
        // Read license level
        _fh->Read(&num_bytes, 1);
        std::string level;
        level.resize(num_bytes);
        _fh->Read(&level[0], num_bytes);
        // Read options
        _fh->Read(&num_bytes, 1);
        std::string options;
        options.resize(num_bytes);
        _fh->Read(&options[0], num_bytes);
        // Read email
        _fh->Read(&num_bytes, 1);
        std::string email;
        email.resize(num_bytes);
        _fh->Read(&email[0], num_bytes);
        // Read number of events
        uint32_t num_events;
        _fh->Read(&num_events, 4);
        // Read other data
        uint32_t other_data;
        _fh->Read(&other_data, 4);

        // Read event type
        uint16_t str_bytes;
        _fh->Read(&str_bytes, 2);
        std::string event_type;
        event_type.resize(str_bytes);
        _fh->Read(&event_type[0], str_bytes);

        // read all the events
        for( int i=0; i < num_events; ++i ) {
            vsaEventRecord evt;

            // read track number
            _fh->Read(&evt.track, 2);

            // see if we have enough tracks
            if( evt.track+1 > num_evt_tracks ) {
                _events.resize(evt.track+1);
                num_evt_tracks = evt.track+1;
            }

            // read times and positions
            _fh->Read(&evt.start_time, 4);
            _fh->Read(&evt.end_time, 4);
            _fh->Read(&evt.start_pos, 4);
            _fh->Read(&evt.end_pos, 4);

            // read text field
            _fh->Read(&num_bytes, 1);
            evt.text.resize(num_bytes);
            _fh->Read(&evt.text[0], num_bytes);
            if( event_type == "CEventBarLinear" ) {
                _fh->Read(&evt.data[0], 12);
            } else if( event_type == "CEventBarPulse" ) {
                _fh->Read(&evt.data[0], 16);
            } else {
                wxMessageBox("Unsupported event type! Halted.");
                break;
            }

            // save the event
            _events[evt.track].push_back(evt);

            // check if on last event
            if( i+1 == num_events )
            {
                _ok = true;
                break;
            }

            // get next event type
            uint16_t next_evt;
            _fh->Read(&next_evt, 2);
            if( next_evt != 0x8001 ) {
                if( next_evt == 0xFFFF ) {
                    // Read event type
                    _fh->Read(&str_bytes, 2);
                    event_type.resize(str_bytes);
                    _fh->Read(&event_type[0], str_bytes);
                } else {
                    wxMessageBox("Unsupported event type! Halted.");
                    break;
                }
            }
        }

        // Read number of audio tracks
        _fh->Read(&_num_audio, 4);

        // read all the audio files
        for( int i=0; i < _num_audio; ++i ) {
            vsaAudioRecord aud;

            // read audio filename field
            _fh->Read(&num_bytes, 1);
            aud.name.resize(num_bytes);
            _fh->Read(&aud.name[0], num_bytes);

            // read audio devices field
            _fh->Read(&num_bytes, 1);
            aud.devices.resize(num_bytes);
            _fh->Read(&aud.devices[0], num_bytes);

            // save the audio record
            _audio.push_back(aud);
        }

        // Skip reading video till we get an example file that has video since it requires higher license level
        std::string dummy_read;
        dummy_read.resize(26);
        _fh->Read(&dummy_read[0], 26);

        // Read number of tracks
        _fh->Read(&_num_tracks, 4);

        // read all the track data
        for( int i=0; i < _num_tracks; ++i ) {
            vsaTrackRecord trk;

            // read track name field
            _fh->Read(&num_bytes, 1);
            trk.name.resize(num_bytes);
            _fh->Read(&trk.name[0], num_bytes);

            // Read track address
            _fh->Read(&trk.address, 4);

            // Read track controller
            _fh->Read(&trk.controller, 1);

            // Read track data1
            _fh->Read(&trk.data1, 11);

            // Read track limits
            _fh->Read(&trk.max_limit, 4);
            _fh->Read(&trk.min_limit, 4);
            _fh->Read(&trk.default_value, 4);

            // Read track enabled flag
            _fh->Read(&trk.enable, 4);
            if( trk.enable > 0 ) {
                _num_active_tracks++;
            }

            // save the audio record
            _tracks.push_back(trk);

            // read track port field
            _fh->Read(&num_bytes, 1);
            trk.port.resize(num_bytes);
            _fh->Read(&trk.port[0], num_bytes);

            // Read track data2
            _fh->Read(&trk.data2, 12);
        }
        uint32_t unknown_word1;
        uint32_t unknown_word2;
        uint32_t unknown_word3;
        uint32_t unknown_word4;
        _fh->Read(&unknown_word1, 4);
        _fh->Read(&unknown_word2, 4);
        _fh->Read(&unknown_word3, 4);
        _fh->Read(&_timing, 4);
        _fh->Read(&unknown_word4, 4);
    }
    else
    {
        logger_base.error("VSA file %s could not be opened.", (const char *)filename.c_str());
        Close();
    }
}

void VSAFile::Close()
{
    if (_fh != nullptr)
    {
        _fh->Close();
        delete _fh;

        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.info("VSA file %s closed.", (const char *)_filename.c_str());
    }

    // force vector deallocation
    std::vector< std::vector< vsaEventRecord > >().swap(_events);
    std::vector< vsaAudioRecord >().swap(_audio);
    std::vector< vsaVideoRecord >().swap(_video);
    std::vector< vsaTrackRecord >().swap(_tracks);

    init();
}

