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

#include <functional>
#include <future>
#include <list>
#include <memory>
#include <shared_mutex>
#include <string>
#include <vector>

#include "xLightsVamp.h"
#include "IAudioOutput.h"
#include "IAudioDecoder.h"

enum class AUDIOSAMPLETYPE {
    RAW,
    BASS,
    TREBLE,
    CUSTOM,
    ALTO,
    NONVOCALS,
    ANY
};

typedef enum MEDIAPLAYINGSTATE {
    PLAYING,
    PAUSED,
    STOPPED
} MEDIAPLAYINGSTATE;

typedef std::function<void(int)> AudioManagerProgressCallback;

typedef struct FilteredAudioData {
    AUDIOSAMPLETYPE type;
    int lowNote = 0;
    int highNote = 127;
    float* data0 = nullptr;
    float* data1 = nullptr;
    int16_t* pcmdata = nullptr;
} FilteredAudioData;

class FrameData {
public:
    float max = 0;
    float min = 0;
    float spread = 0;
    std::vector<float> vu;
    std::vector<float> notes;
};

class AudioManager {
    std::shared_timed_mutex _mutex;
    std::shared_timed_mutex _mutexAudioLoad;
    long _loadedData = 0;
    std::vector<FrameData> _frameData;
    std::string _audio_file;
    xLightsVamp _vamp;
    long _rate = 44100;
    long _sampleRate = 48000;
    int _channels = 0;
    long _trackSize = 0;
    int _bits = 0;
    int _extra = 0;
    std::string _resultMessage;
    int _state = 0;
    float* _data[2]; // audio data
    uint8_t* _pcmdata = nullptr;
    long _pcmdatasize = 0;
    std::string _title;
    std::string _artist;
    std::string _album;
    int _intervalMS = 50;
    int _frameDataPreparedForInterval = -1;
    long _lengthMS = 0;
    bool _frameDataPrepared = false;
    float _bigmax = 0;
    float _bigspread = 0;
    float _bigmin = 0;
    float _bigspectogrammax = 0;
    MEDIAPLAYINGSTATE _media_state;
    bool _polyphonicTranscriptionDone = false;
    std::vector<FilteredAudioData*> _filtered;
    int _sdlid = 0;
    bool _ok = false;
    std::string _hash;
    std::future<void> _prepFrameData;
    std::future<void> _loadingAudio;
    std::string _device;

    long _bitRate = 0;
    std::map<std::string, std::string> _metaData;

    int OpenMediaFile();
    void PrepareFrameData(bool separateThread);
    void CalculateSpectrumAnalysis(const float* in, int n, float& max, int id, std::vector<float>& d) const;
    void SetLoadedData(long pos);

    void NormaliseFilteredAudioData(FilteredAudioData* fad);

public:
    static double MidiToFrequency(int midi);
    static std::string MidiToNote(int midi);
    bool IsOk() const {
        return _ok;
    }
    static size_t GetAudioFileLength(std::string filename);
    void Seek(long pos) const;
    void Pause();
    void Play();
    bool IsPlaying() const;
    void Play(long posms, long lenms);
    void Stop();
    void AbsoluteStop();
    long GetLoadedData();
    bool IsDataLoaded(long pos = -1);
    static void SetPlaybackRate(float rate);
    long GetSampleRate() const {
        return _sampleRate;
    }
    MEDIAPLAYINGSTATE GetPlayingState() const;
    long Tell() const;
    xLightsVamp* GetVamp() {
        return &_vamp;
    };
    AudioManager(const std::string& audio_file, int intervalMS = -1, const std::string& device = "");
    ~AudioManager();
    void SetVolume(int volume) const;
    int GetVolume() const;
    static void SetGlobalVolume(int volume);
    static int GetGlobalVolume();
    static std::list<std::string> GetAudioDevices();
    static std::list<std::string> GetInputAudioDevices();
    long GetTrackSize() const {
        return _trackSize;
    };
    long GetRate() const {
        return _rate;
    };
    long GetBitRate() const {
        return _bitRate;
    };
    const std::map<std::string, std::string> &GetMetaData() {
        return _metaData;
    }
    int GetChannels() const {
        return _channels;
    };
    int GetState() const {
        return _state;
    };
    std::string GetResultMessage() const {
        return _resultMessage;
    };
    std::string Title() const {
        return _title;
    };
    std::string Artist() const {
        return _artist;
    };
    std::string Album() const {
        return _album;
    };
    std::string FileName() const {
        return _audio_file;
    };
    std::string Hash();
    long LengthMS() const {
        return _lengthMS;
    };
    float GetFilteredRightData(long offset);
    float GetFilteredLeftData(long offset);
    float GetRawRightData(long offset);
    float GetRawLeftData(long offset);
    void SwitchTo(AUDIOSAMPLETYPE type, int lowNote = 0, int highNote = 127);
    void GetLeftDataMinMax(long start, long end, float& minimum, float& maximum, AUDIOSAMPLETYPE type = AUDIOSAMPLETYPE::ANY, int lowNote = -1, int highNote = -1);
    float* GetFilteredRightDataPtr(long offset);
    float* GetFilteredLeftDataPtr(long offset);
    float* GetRawRightDataPtr(long offset);
    float* GetRawLeftDataPtr(long offset);
    void SetStepBlock(int step, int block);
    void SetFrameInterval(int intervalMS);
    int GetFrameInterval() const {
        return _intervalMS;
    }
    const FrameData* GetFrameData(int frame, const std::string& timing, bool needNotes = false);
    const FrameData* GetFrameData(const std::string& timing, long ms, bool needNotes = false);
    void DoPrepareFrameData();
    void DoPolyphonicTranscription(AudioManagerProgressCallback progresscallback);
    bool IsPolyphonicTranscriptionDone() const {
        return _polyphonicTranscriptionDone;
    };

    FilteredAudioData* GetFilteredAudioData(AUDIOSAMPLETYPE type, int lowNote, int highNote);
    bool WriteCurrentAudio(const std::string& path, long bitrate);
    static bool EncodeAudio(const std::vector<float>& left_channel,
                            const std::vector<float>& right_channel,
                            size_t samplerate,
                            const std::string& filename,
                            AudioManager* copyFrom = nullptr);

    void AudioDeviceChanged();

    // Access the audio output manager (SDL on desktop, AVFoundation on iPad)
    static IAudioOutputManager* GetAudioManager();
    // Access the audio decoder (AudioToolbox on Apple, FFmpeg on Linux/Windows)
    static IAudioDecoder& GetDecoder();
};
