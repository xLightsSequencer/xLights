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
#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>

#if !TARGET_OS_IPHONE
#include "xLightsVamp.h"
#endif
#include "IAudioOutput.h"
#include "IAudioDecoder.h"

enum class AUDIOSAMPLETYPE {
    RAW,
    BASS,
    TREBLE,
    CUSTOM,
    ALTO,
    NONVOCALS,
    LUFS,         // A3: BS.1770 K-weighted momentary-loudness envelope
    VOCALS,       // A8 (partial): center-channel extraction — M - α|S|
    CLASSIFIED,   // A7: raw signal gated by a SoundAnalysis class curve
    STEM_DRUMS,   // A8: isolated drums from HTDemucs
    STEM_BASS,    // A8: isolated bass from HTDemucs
    STEM_OTHER,   // A8: everything-else stem from HTDemucs
    STEM_VOCALS,  // A8: ML-isolated vocals from HTDemucs (full separation,
                  //     not to be confused with the M/S trick above)
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
#if !TARGET_OS_IPHONE
    xLightsVamp _vamp;
#endif
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
    // Guards `_filtered`. Filter computations (EnsureFilteredAudioData)
    // can run from the UI thread or render workers concurrently with
    // SetStemData/SetClassifyGate, which erase entries; iterating the
    // vector or holding a FilteredAudioData* across erase is otherwise
    // UAF.
    mutable std::recursive_mutex _filteredMutex;
    // A7: state for `AUDIOSAMPLETYPE::CLASSIFIED`. Populated by
    // `SetClassifyGate`. The gate curve is re-interpolated per-
    // sample inside `EnsureFilteredAudioData(CLASSIFIED)`.
    std::string _classifyGateClass;
    std::vector<float> _classifyGateCurve;
    float _classifyGateTimeStep = 1.0f;
    // A8: cached output of the HTDemucs stem separator. Set via
    // `SetStemData` and consumed by
    // `EnsureFilteredAudioData(STEM_*)`. All eight vectors must be
    // the same length (the track's sample count).
    std::vector<float> _stemDrumsL, _stemDrumsR;
    std::vector<float> _stemBassL, _stemBassR;
    std::vector<float> _stemOtherL, _stemOtherR;
    std::vector<float> _stemVocalsL, _stemVocalsR;
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
#if !TARGET_OS_IPHONE
    xLightsVamp* GetVamp() {
        return &_vamp;
    };
#endif
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
    void GetLeftDataMinMax(long start, long end, float& minimum, float& maximum, AUDIOSAMPLETYPE type = AUDIOSAMPLETYPE::ANY, int lowNote = -1, int highNote = -1, float* rms = nullptr);
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
#if !TARGET_OS_IPHONE
    void DoPolyphonicTranscription(AudioManagerProgressCallback progresscallback);
#endif
    bool IsPolyphonicTranscriptionDone() const {
        return _polyphonicTranscriptionDone;
    };

    FilteredAudioData* GetFilteredAudioData(AUDIOSAMPLETYPE type, int lowNote, int highNote);

    // A8: drop in the output of `StemSeparator::SeparateStems` so
    // the filter cache can serve it through `AUDIOSAMPLETYPE::
    // STEM_{DRUMS,BASS,OTHER,VOCALS}`. Each vector holds a full-track
    // mono signal; stereo is reconstructed from the L/R pairs. Any
    // stale STEM_* cache entries are evicted so subsequent
    // `EnsureFilteredAudioData(STEM_X)` rebuilds from the new data.
    // Pass empty vectors (or all-zero-length) to clear.
    void SetStemData(const std::vector<float>& drumsL, const std::vector<float>& drumsR,
                      const std::vector<float>& bassL, const std::vector<float>& bassR,
                      const std::vector<float>& otherL, const std::vector<float>& otherR,
                      const std::vector<float>& vocalsL, const std::vector<float>& vocalsR);
    bool HasStemData() const { return !_stemDrumsL.empty(); }

    // Write the currently selected audio (whatever AUDIOSAMPLETYPE
    // the most recent SwitchTo landed on — RAW, STEM_VOCALS, a
    // band-passed filter, etc.) to a temporary stereo float32 WAV
    // file. Returns the absolute path on success, an empty string
    // if no audio is loaded or the write failed. The caller owns the
    // file — delete when done. Used by the speech-to-lyrics pipeline
    // so the recognizer sees whatever the user has currently picked
    // in the waveform (e.g. an isolated vocals stem instead of the
    // full mix, which gets the recognizer hopelessly confused by
    // drums + bass + backing instruments).
    std::string WriteCurrentToTempWav() const;

    // A7: provide the per-second confidence curve that
    // `AUDIOSAMPLETYPE::CLASSIFIED` should gate the raw signal by.
    // Clearing any existing CLASSIFIED cache entry so the next
    // `EnsureFilteredAudioData(CLASSIFIED)` rebuilds with the new
    // curve. `timeStepSec` is the hop between consecutive confidence
    // samples (Apple's default is 1.0 s).
    void SetClassifyGate(const std::string& className,
                          const std::vector<float>& confidencePerStep,
                          float timeStepSec);

    // Lazy variant of `GetFilteredAudioData` — if the cache doesn't
    // already hold an entry matching `type`/`lowNote`/`highNote` it
    // builds one (same factory logic as `SwitchTo`) and returns it.
    // Unlike `SwitchTo`, this does NOT overwrite `_pcmdata`/`_data`
    // — it leaves playback untouched. Safe to call from the iPad
    // waveform bridge where we want filtered samples for display
    // only. Returns nullptr if the track isn't loaded yet.
    FilteredAudioData* EnsureFilteredAudioData(AUDIOSAMPLETYPE type, int lowNote, int highNote);
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
