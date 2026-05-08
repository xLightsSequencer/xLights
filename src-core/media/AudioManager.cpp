/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <cassert>
#include <chrono>
#include <thread>

#include "../utils/AppCallbacks.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

#include <math.h>
#include <stdlib.h>

#include "AudioManager.h"
#include "IAudioOutput.h"
#include "IAudioDecoder.h"
#include "../utils/ExternalHooks.h"
#include "../utils/Parallel.h"
#include "../utils/UtilFunctions.h"
#include "../utils/string_utils.h"
#include "../../dependencies/md5/md5.h"
#include "kiss_fft/tools/kiss_fftr.h"

// Platform-appropriate audio decoder and output
#if defined(__APPLE__)
#include "AudioToolboxDecoder.h"
#include "AVAudioEngineOutput.h"
#else
#include "FFmpegAudioDecoder.h"
#include "SDLAudioOutput.h"
#endif

#include <log.h>

#if !TARGET_OS_IPHONE
using namespace Vamp;
#endif

// Platform-selected audio output manager and decoder
#if defined(__APPLE__)
static AVAudioEngineManager __audioManager;
static AudioToolboxDecoder __audioDecoder;
#else
SDLManager __audioManager;
static FFmpegAudioDecoder __audioDecoder;
// SDLAudioOutput.cpp's fill_audio callback references __audioManager via extern
#define __audioManager __audioManager
#endif

#ifndef __APPLE__
#define RESAMPLE_RATE (44100)
#else
#define RESAMPLE_RATE (48000)
#endif

#define PCMFUDGE 32768

#if !TARGET_OS_IPHONE
static void ProgressFunction(int p) {
    // placeholder for polyphonic transcription progress
}
#endif

// Audio Manager Functions

void AudioManager::SetVolume(int volume) const {
    if (__audioManager.GetOutput(_device) != nullptr)
        __audioManager.GetOutput(_device)->SetVolume(_sdlid, volume);
}

int AudioManager::GetVolume() const {
    if (__audioManager.GetOutput(_device) != nullptr)
        return __audioManager.GetOutput(_device)->GetVolume(_sdlid);

    return 100;
}

int AudioManager::GetGlobalVolume() {
    return __audioManager.GetGlobalVolume();
}

void AudioManager::SetGlobalVolume(int volume) {
    __audioManager.SetGlobalVolume(volume);
}

void AudioManager::Seek(long pos) const {
    if (pos < 0 || pos > _lengthMS || !_ok) {
        return;
    }

    auto* out = __audioManager.GetOutput(_device);
    if (out == nullptr) {
        return;
    }

    // Lazy-add: OpenMediaFile() defers AddAudio until first Play() to avoid
    // spinning up AVAudioEngine for AudioManagers that only decode. Seek()
    // must do the same lazy-add, otherwise a Seek() before the first Play()
    // silently no-ops on _sdlid == -1 and the play position is wrong.
    if (!out->HasAudio(_sdlid)) {
        const_cast<AudioManager*>(this)->_sdlid = out->AddAudio(_pcmdatasize, _pcmdata, 100, _rate, _trackSize, _lengthMS);
    }

    out->Seek(_sdlid, pos);
}

void AudioManager::Pause() {
    if (__audioManager.GetOutput(_device) != nullptr) {
        __audioManager.GetOutput(_device)->Pause(_sdlid, true);
        _media_state = MEDIAPLAYINGSTATE::PAUSED;
    }
}

void AudioManager::Play(long posms, long lenms) {
    auto* out = __audioManager.GetOutput(_device);
    if (posms < 0 || posms > _lengthMS || !_ok || out == nullptr) {
        return;
    }

    if (!out->HasAudio(_sdlid)) {
        _sdlid = out->AddAudio(_pcmdatasize, _pcmdata, 100, _rate, _trackSize, _lengthMS);
    }

    out->SeekAndLimitPlayLength(_sdlid, posms, lenms);
    out->Play();
    out->Pause(_sdlid, false);
    _media_state = MEDIAPLAYINGSTATE::PLAYING;
}

bool AudioManager::IsPlaying() const {
    return _media_state == MEDIAPLAYINGSTATE::PLAYING;
}

void AudioManager::Play() {
    auto* out = __audioManager.GetOutput(_device);

    if (!_ok || out == nullptr)
        return;

    if (!out->HasAudio(_sdlid)) {
        _sdlid = out->AddAudio(_pcmdatasize, _pcmdata, 100, _rate, _trackSize, _lengthMS);
    }

    out->Pause(_sdlid, false);
    out->Play();
    _media_state = MEDIAPLAYINGSTATE::PLAYING;
}

void AudioManager::Stop() {
    auto* out = __audioManager.GetOutput(_device);
    if (out != nullptr) {
        out->Pause(_sdlid, true);  // individually pause this stream so it won't mix when device restarts
        out->Stop();
        _media_state = MEDIAPLAYINGSTATE::STOPPED;
    }
}

void AudioManager::AbsoluteStop() {
    auto* out = __audioManager.GetOutput(_device);

    if (out != nullptr) {
        out->Stop();
        out->RemoveAudio(_sdlid);
        _media_state = MEDIAPLAYINGSTATE::STOPPED;
    }
}
void AudioManager::AudioDeviceChanged() {
    MEDIAPLAYINGSTATE oldMediaState = _media_state;
    long ts = 0;
    if (oldMediaState == MEDIAPLAYINGSTATE::PLAYING || oldMediaState == MEDIAPLAYINGSTATE::PAUSED) {
        ts = Tell();
    }
    AbsoluteStop();

    if (oldMediaState == MEDIAPLAYINGSTATE::PLAYING || oldMediaState == MEDIAPLAYINGSTATE::PAUSED) {
        AppCallbacks::PostToMainThread([this, ts, oldMediaState]() {
            auto* out = __audioManager.GetOutput(_device);
            if (!out->HasAudio(_sdlid)) {
                _sdlid = out->AddAudio(_pcmdatasize, _pcmdata, 100, _rate, _trackSize, _lengthMS);
            }
            Seek(ts);
            if (oldMediaState == MEDIAPLAYINGSTATE::PLAYING) {
                Play();
            }
        });
    }
}
void AudioManager::SetPlaybackRate(float rate) {
    __audioManager.SetRate(rate);
}

MEDIAPLAYINGSTATE AudioManager::GetPlayingState() const {
    return _media_state;
}

// return where in the file we are up to playing
long AudioManager::Tell() const {
    if (__audioManager.GetOutput(_device) != nullptr) {
        return __audioManager.GetOutput(_device)->Tell(_sdlid);
    }
    return 0;
}

size_t AudioManager::GetAudioFileLength(std::string filename) {
    return GetDecoder().GetAudioFileLength(filename);
}

long AudioManager::GetLoadedData() {
    std::unique_lock<std::shared_timed_mutex> locker(_mutexAudioLoad);
    return _loadedData;
}

void AudioManager::SetLoadedData(long pos) {
    std::unique_lock<std::shared_timed_mutex> locker(_mutexAudioLoad);
    _loadedData = pos;
}

bool AudioManager::IsDataLoaded(long pos) {
    std::unique_lock<std::shared_timed_mutex> locker(_mutexAudioLoad);

    // if we failed to load the audio file say it is all ok
    if (!_ok)
        return true;

    if (pos < 0) {
        return _loadedData == _trackSize;
    } else {
        return _loadedData >= std::min(pos, _trackSize);
    }
}

AudioManager::AudioManager(const std::string& audio_file, int intervalMS, const std::string& device) {
    

    spdlog::debug("Audio Manager Constructor start");

    // save parameters and initialise defaults
    _ok = true;
    _hash = "";
    _device = device;
    _loadedData = 0;
    _audio_file = audio_file;
    _state = -1; // state uninitialised. 0 is error. 1 is loaded ok
    _resultMessage = "";
    _data[0] = nullptr;         // Left channel data
    _data[1] = nullptr;         // right channel data
    _intervalMS = intervalMS;   // no length
    _frameDataPrepared = false; // frame data is used by effects to react to the song
    _media_state = MEDIAPLAYINGSTATE::STOPPED;
    _pcmdata = nullptr;
    _polyphonicTranscriptionDone = false;
    _sdlid = -1;
    _rate = -1;
    _trackSize = 0;

    // extra is the extra bytes added to the data we read. This allows analysis functions to exceed the file length without causing memory exceptions
    _extra = 32769;

    // Open the media file
    spdlog::debug("Audio Manager Constructor: Loading media file.");
    OpenMediaFile();
    spdlog::debug("Audio Manager Constructor: Media file loaded.");

    // if we didnt get a valid looking rate then we really are not ok
    if (_rate <= 0)
        _ok = false;

    // If we opened it successfully kick off the frame data extraction ... this will run on another thread
    if (_intervalMS > 0 && _ok) {
        spdlog::debug("Audio Manager Constructor: Preparing frame data.");
        PrepareFrameData(true);
        spdlog::debug("Audio Manager Constructor: Preparing frame data done ... but maybe on a background thread.");
    } else if (_ok) {
        spdlog::debug("Audio Manager Constructor: Skipping preparing frame data as timing not known yet.");
    }

    // if we got here without setting state to zero then all must be good so set state to 1 success
    if (_ok && _state == -1) {
        _state = 1;
        spdlog::info("Audio file loaded.");
        spdlog::info("    Filename: {}", (const char*)_audio_file.c_str());
        spdlog::info("    Title: {}", (const char*)_title.c_str());
        spdlog::info("    Album: {}", (const char*)_album.c_str());
        spdlog::info("    Artist: {}", (const char*)_artist.c_str());
        spdlog::info("    Length: {}ms", _lengthMS);
        spdlog::info("    Channels {}, Bits: {}, Rate {}", _channels, _bits, _rate);
    } else {
        spdlog::error("Audio file not loaded: {}.", _resultMessage.c_str());
    }
    // AVAudioEngine handles device changes automatically; SDL on Linux/Windows
    // doesn't support device switching. The listener was only implemented on macOS
    // for the old SDL path and is no longer needed.
}

void AudioManager::CalculateSpectrumAnalysis(const float* in, int n, float& max, int id, std::vector<float>& res) const {
    res.clear();
    res.reserve(127);
    int outcount = n / 2 + 1;
    kiss_fftr_cfg cfg;
    kiss_fft_cpx* out = (kiss_fft_cpx*)malloc(sizeof(kiss_fft_cpx) * (outcount));
    if (out != nullptr) {
        if ((cfg = kiss_fftr_alloc(n, 0 /*is_inverse_fft*/, nullptr, nullptr)) != nullptr) {
            kiss_fftr(cfg, in, out);
            free(cfg);
        }

        for (int j = 0; j < 127; j++) {
            // choose the right bucket for this MIDI note
            double freq = 440.0 * exp2f(((double)j - 69.0) / 12.0);
            int start = freq * (double)n / (double)_rate;
            double freqnext = 440.0 * exp2f(((double)j + 1.0 - 69.0) / 12.0);
            int end = freqnext * (double)n / (double)_rate;

            float val = 0.0;

            // got through all buckets up to the next note and take the maximums
            if (end < outcount - 1) {
                for (int k = start; k <= end; k++) {
                    kiss_fft_cpx* cur = out + k;
                    val = std::max(val, sqrtf(cur->r * cur->r + cur->i * cur->i));
                    // float valscaled = valnew * scaling;
                }
            }

            float db = log10(val);
            if (db < 0.0) {
                db = 0.0;
            }

            res.push_back(db);
            if (db > max) {
                max = db;
            }
        }
        free(out);
    }
}

#if !TARGET_OS_IPHONE
void AudioManager::DoPolyphonicTranscription(AudioManagerProgressCallback fn) {
    

    // dont redo it
    if (_polyphonicTranscriptionDone) {
        return;
    }

    auto sw_start = std::chrono::steady_clock::now();

    spdlog::info("DoPolyphonicTranscription: Polyphonic transcription started on file " + _audio_file);

    while (!IsDataLoaded()) {
        spdlog::debug("DoPolyphonicTranscription: Waiting for audio data to load.");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    spdlog::debug("Processing polyphonic transcription on file " + _audio_file);
    spdlog::debug("Interval {}.", _intervalMS);
    spdlog::debug("BitRate {}.", GetRate());

    // Initialise Polyphonic Transcription
    _vamp.GetAllAvailablePlugins(this); // this initialises Vamp
    Vamp::Plugin* pt = _vamp.GetPlugin("Polyphonic Transcription");

    if (pt == nullptr) {
        spdlog::warn("DoPolyphonicTranscription: Unable to load Polyphonic Transcription VAMP plugin.");
    } else {
        float* pdata[2];
        long frames = _lengthMS / _intervalMS;
        while (frames * _intervalMS < _lengthMS) {
            frames++;
        }

        size_t pref_step = pt->getPreferredStepSize();
        size_t pref_block = pt->getPreferredBlockSize();

        int channels = GetChannels();
        if (channels > (int)pt->getMaxChannelCount()) {
            channels = 1;
        }

        spdlog::debug("Channels {}.", GetChannels());
        spdlog::debug("Step {}.", pref_step);
        spdlog::debug("Block {}.", pref_block);
        pt->initialise(channels, pref_step, pref_block);

        bool first = true;
        int start = 0;
        long len = GetTrackSize();
        float totalLen = len;
        int lastProgress = 0;
        while (len) {
            int progress = (((float)(totalLen - len) * 25) / totalLen);
            if (lastProgress < progress) {
                fn(progress);
                lastProgress = progress;
            }
            pdata[0] = GetRawLeftDataPtr(start);
            assert(pdata[0] != nullptr);
            pdata[1] = GetRawRightDataPtr(start);

            Vamp::RealTime timestamp = Vamp::RealTime::frame2RealTime(start, GetRate());
            Vamp::Plugin::FeatureSet features = pt->process(pdata, timestamp);

            if (first && features.size() > 0) {
                spdlog::warn("DoPolyphonicTranscription: Polyphonic transcription data process oddly retrieved data.");
                first = false;
            }
            if ((size_t)len > pref_step) {
                len -= pref_step;
            } else {
                len = 0;
            }
            start += pref_step;
        }

        // Process the Polyphonic Transcription
        try {
            unsigned int total = 0;
            spdlog::debug("About to extract Polyphonic Transcription result.");
            Vamp::Plugin::FeatureSet features = pt->getRemainingFeatures();
            spdlog::debug("Polyphonic Transcription result retrieved.");
            spdlog::debug("Start,Duration,CalcStart,CalcEnd,midinote");
            for (size_t j = 0; j < features[0].size(); j++) {
                if (j % 10 == 0) {
                    fn((int)(((float)j * 75.0) / (float)features[0].size()) + 25.0);
                }

                long currentstart = features[0][j].timestamp.sec * 1000 + features[0][j].timestamp.msec();
                long currentend = currentstart + features[0][j].duration.sec * 1000 + features[0][j].duration.msec();

                // printf("%f\t%f\t%f\n",(float)currentstart/1000.0, (float)currentend/1000.0, features[0][j].values[0]);
                // if (logger_pianodata.isDebugEnabled()) {
                spdlog::debug("{}.{:03d},{}.{:03d},{},{},{}", features[0][j].timestamp.sec, features[0][j].timestamp.msec(), features[0][j].duration.sec, features[0][j].duration.msec(), currentstart, currentend, features[0][j].values[0]);
                //}
                total += features[0][j].values.size();

                int sframe = currentstart / _intervalMS;
                if (currentstart - sframe * _intervalMS > _intervalMS / 2) {
                    sframe++;
                }
                int eframe = currentend / _intervalMS;
                while (sframe <= eframe) {
                    _frameData[sframe].notes.push_back(features[0][j].values[0]);
                    sframe++;
                }
            }

            fn(100);

             // if (logger_pianodata.isDebugEnabled()) {
            spdlog::debug("Piano data calculated:");
            spdlog::debug("Time MS, Keys");
            for (size_t i = 0; i < _frameData.size(); i++) {
                long ms = i * _intervalMS;
                std::string keys;
                for (const auto& it2 : _frameData[i].notes) {
                    keys += " " + std::to_string(it2);
                }
                spdlog::debug("{},{}", ms, keys);
            }
            //}
            // printf("Total points: %u", total);
        } catch (...) {
            spdlog::warn("DoPolyphonicTranscription: Polyphonic Transcription threw an error getting the remaining features.");
        }

        // done with VAMP Polyphonic Transcriber ... but dont delete it as the VAMP code manages its lifetime
        // delete pt;
    }

    _polyphonicTranscriptionDone = true;
    spdlog::info("DoPolyphonicTranscription: Polyphonic transcription completed in {}.", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - sw_start).count());
}
#endif // !TARGET_OS_IPHONE

// Frame Data Extraction Functions
// process audio data and build data for each frame
void AudioManager::DoPrepareFrameData() {
    
    spdlog::info("DoPrepareFrameData: Start processing audio frame data.");

    // lock the mutex
    std::unique_lock<std::shared_timed_mutex> locker(_mutex);
    spdlog::info("DoPrepareFrameData: Got mutex.");

    if (_data[0] == nullptr) {
        spdlog::warn("    DoPrepareFrameData: Exiting as there is no data.");
        return;
    }

    auto sw_start = std::chrono::steady_clock::now();

    // if we have already done it ... bail
    if (_frameDataPrepared && _frameDataPreparedForInterval == _intervalMS) {
        spdlog::info("DoPrepareFrameData: Aborting processing audio frame data ... it has already been done.");
        return;
    }

    _frameDataPreparedForInterval = _intervalMS;

    // wait for the data to load
    while (!IsDataLoaded()) {
        spdlog::info("DoPrepareFrameData: waiting for audio data to load.");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    spdlog::info("DoPrepareFrameData: Data is loaded.");

    // we need to ensure at least the raw data is available
    bool needsRaw;
    {
        std::lock_guard<std::recursive_mutex> flock(_filteredMutex);
        needsRaw = _filtered.empty();
    }
    if (needsRaw) {
        locker.unlock();
        SwitchTo(AUDIOSAMPLETYPE::RAW, 0, 0);
        locker.lock();
    }

    _frameData.clear();

    // samples per frame
    int samplesperframe = _rate * _intervalMS / 1000;
    int frames = _lengthMS / _intervalMS;
    while (frames * _intervalMS < _lengthMS) {
        frames++;
    }
    int totalsamples = frames * samplesperframe;

    spdlog::info("    Length {}ms", _lengthMS);
    spdlog::info("    Interval {}ms", _intervalMS);
    spdlog::info("    Samples per frame {}", samplesperframe);
    spdlog::info("    Frames {}", frames);
    spdlog::info("    Total samples {}", totalsamples);

    // these are used to normalise output
    _bigmax = -1;
    _bigspread = -1;
    _bigmin = 1;
    _bigspectogrammax = -1;

    size_t step = 2048;
    float* pdata[2];

    int pos = 0;
    std::vector<float> spectrogram;
    std::vector<float> subspectrogram;

    // process each frome of the song
    _frameData.resize(frames);
    for (int i = 0; i < frames; i++) {
        // accumulators
        float max = -100.0;
        float min = 100.0;
        float spread = -100;

        // clear the data if we are about to get new data ... dont clear it if we wont
        // this happens because the spectrogram function has a fixed window based on the parameters we set and it
        // does not match our time slices exactly so we have to select which one to use
        if (pos < i * samplesperframe + samplesperframe && pos + (int)step < totalsamples) {
            spectrogram.clear();
        }

        // only get the data if we are not ahead of the music
        while (pos < i * samplesperframe + samplesperframe && pos + (int)step < totalsamples) {
            pdata[0] = GetRawLeftDataPtr(pos);
            assert(pdata[0] != nullptr);
            pdata[1] = GetRawRightDataPtr(pos);
            float max2 = 0;

            if (pdata[0] == nullptr) {
                subspectrogram.clear();
            } else {
                CalculateSpectrumAnalysis(pdata[0], step, max2, i, subspectrogram);
            }

            // and keep track of the larges value so we can normalise it
            if (max2 > _bigspectogrammax) {
                _bigspectogrammax = max2;
            }
            pos += step;

            // either take the newly calculated values or if we are merging two results take the maximum of each value
            if (spectrogram.size() == 0) {
                spectrogram = subspectrogram;
            } else {
                if (subspectrogram.size() > 0) {
                    std::vector<float>::iterator sub = subspectrogram.begin();
                    for (std::vector<float>::iterator fr = spectrogram.begin(); fr != spectrogram.end(); ++fr) {
                        if (*sub > *fr) {
                            *fr = *sub;
                        }
                        ++sub;
                    }
                }
            }
        }

        // now do the raw data analysis for the frame
        for (int j = 0; j < samplesperframe; j++) {
            float data = GetRawLeftData(i * samplesperframe + j);

            // Max data
            if (data > max) {
                max = data;
            }

            // Min data
            if (data < min) {
                min = data;
            }

            // Spread data
            if (max - min > spread) {
                spread = max - min;
            }
        }

        if (max > _bigmax) {
            _bigmax = max;
        }
        if (min < _bigmin) {
            _bigmin = min;
        }
        if (spread > _bigspread) {
            _bigspread = spread;
        }
        _frameData[i].min = min;
        _frameData[i].max = max;
        _frameData[i].spread = spread;
        _frameData[i].vu = spectrogram;
    }

    // normalise data ... basically scale the data so the highest value is the scale value.
    float const scale = 1.0; // 0-1 ... where 0.x means that the max value displayed would be x0% of model size
    float const bigmaxscale = 1 / (_bigmax * scale);
    float const bigminscale = 1 / (_bigmin * scale);
    float const bigspreadscale = 1 / (_bigspread * scale);
    float const bigspectrogramscale = 1 / (_bigspectogrammax * scale);
    for (auto& fr : _frameData) {
        fr.max *= bigmaxscale;
        fr.min *= bigminscale;
        fr.spread *= bigspreadscale;

        for (auto& vu : fr.vu) {
            vu *= bigspectrogramscale;
        }
    }

    // flag the fact that the data is all ready
    _frameDataPrepared = true;
    spdlog::info("DoPrepareFrameData: Audio frame data processing complete in {}. Frames: {}", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - sw_start).count(), frames);
}
// Called to trigger frame data creation
void AudioManager::PrepareFrameData(bool separateThread) {
    // if frame data is already being processed, wait for that one to finish, otherwise
    //_prepFrameData will get overwritten with a new future and the old will be lost
    if (_prepFrameData.valid()) {
        _prepFrameData.wait();
    }
    if (separateThread) {
        if (!_frameDataPrepared || _frameDataPreparedForInterval != _intervalMS) {
            _prepFrameData = std::async(std::launch::async, [this]() { DoPrepareFrameData(); });
        }
    } else {
        DoPrepareFrameData();
    }
}




// Get the pre-prepared data for this frame
const FrameData* AudioManager::GetFrameData(int frame, const std::string& timing, bool needNotes) {
    

    // Grab the lock so we can safely access the frame data
    std::shared_lock<std::shared_timed_mutex> lock(_mutex);

    // make sure we have audio data
    if (_data[0] == nullptr)
        return nullptr;

    // if the frame data has not been prepared
    if (!_frameDataPrepared) {
        spdlog::debug("GetFrameData was called prior to the frame data being prepared.");
        // prepare it
        lock.unlock();
        PrepareFrameData(false);

        lock.lock();
        // wait until the new thread grabs the lock
        while (!_frameDataPrepared) {
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            lock.lock();
        }
    }
#if !TARGET_OS_IPHONE
    if (needNotes && !_polyphonicTranscriptionDone) {
        // need to do the polyphonic stuff
        DoPolyphonicTranscription(ProgressFunction);
    }
#endif
    if (frame < (int)_frameData.size()) {
        return &_frameData[frame];
    }
    return nullptr;
}

const FrameData* AudioManager::GetFrameData(const std::string& timing, long ms, bool needNotes) {
    int const frame = ms / _intervalMS;
    return GetFrameData(frame, timing, needNotes);
}

// Constant Bitrate Detection Functions

// Decode bitrate


// Decode samplerate


// Decode side info


// Set the frame interval we will be using
void AudioManager::SetFrameInterval(int intervalMS) {
    

    // If this is different from what it was previously
    if (_intervalMS != intervalMS) {
        spdlog::debug("Changing frame interval to {}", intervalMS);

        // save it and regenerate the frame data for effects that rely upon it ... but do it on a background thread

        // need to lock first to make sure the background thread is done loading so to avoid
        // changing the _intervalMS (and thus the size of structures) in the middle of loading.
        std::unique_lock<std::shared_timed_mutex> locker(_mutex);
        _intervalMS = intervalMS;
        locker.unlock();

        PrepareFrameData(true);
    }
}

// Set the set and block that vamp analysis will be using
// This controls how much extra space at the end of the file we need so VAMP functions dont try to read past the end of the allocated memory
void AudioManager::SetStepBlock(int step, int block) {
    int extra = std::max(step, block) + 1;

    // we only need to reopen if the extra bytes are greater
    if (extra > _extra) {
        _extra = extra;
        _state = -1;
        OpenMediaFile();
    }
}

// Clean up our data buffers
AudioManager::~AudioManager() {
    
    spdlog::debug("AudioManager::~AudioManager");

    while (IsOk() && !IsDataLoaded()) {
        spdlog::debug("~AudioManager waiting for audio data to complete loading before destroying it.");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // wait for async tasks to finish
    if (_loadingAudio.valid()) {
        _loadingAudio.wait();
    }
    if (_prepFrameData.valid())
        _prepFrameData.wait();

    if (_pcmdata != nullptr) {
        auto* out = __audioManager.GetOutput(_device);
        if (out != nullptr) {
            out->Stop();
            out->RemoveAudio(_sdlid);
        }
        free(_pcmdata);
        _pcmdata = nullptr;
    }

    {
        std::lock_guard<std::recursive_mutex> flock(_filteredMutex);
        while (_filtered.size() > 0) {
            if (_filtered.back()->data0) {
                free(_filtered.back()->data0);
            }
            if (_filtered.back()->data1 && _filtered.back()->data1 != _filtered.back()->data0) {
                free(_filtered.back()->data1);
            }
            if (_filtered.back()->pcmdata) {
                free(_filtered.back()->pcmdata);
            }
            delete _filtered.back();
            _filtered.pop_back();
        }
    }

    // wait for prepare frame data to finish ... if i delete the data before it is done we will crash
    // this is only tripped if we try to open a new song too soon after opening another one

    // Grab the lock so we know the background process isnt running
    std::shared_lock<std::shared_timed_mutex> lock(_mutex);

    if (_data[1] != _data[0] && _data[1] != nullptr) {
        free(_data[1]);
        _data[1] = nullptr;
    }
    if (_data[0] != nullptr) {
        free(_data[0]);
        _data[0] = nullptr;
    }
    spdlog::debug("AudioManager::~AudioManager Done");
}

// Split the MP# data into left and right and normalise the values


// NOrmalise mono track data


// Calculate the song length in MS


// Open and read the media file into memory
int AudioManager::OpenMediaFile() {
    if (_pcmdata != nullptr) {
        auto* out = __audioManager.GetOutput(_device);
        if (out != nullptr) {
            out->Stop();
            out->RemoveAudio(_sdlid);
        }
        _sdlid = -1;
        free(_pcmdata);
        _pcmdata = nullptr;
    }

    // Free old data if re-opening
    if (_data[1] != nullptr && _data[1] != _data[0]) {
        free(_data[1]);
        _data[1] = nullptr;
    }
    if (_data[0] != nullptr) {
        free(_data[0]);
        _data[0] = nullptr;
    }
    _loadedData = 0;

    // Use the platform decoder
    DecodedAudioInfo info;
    if (!GetDecoder().DecodeFile(_audio_file, RESAMPLE_RATE, _extra, info,
                                  _pcmdata, _pcmdatasize,
                                  _data[0], _data[1], _trackSize)) {
        _ok = false;
        return 1;
    }

    _channels = info.channels;
    _rate = RESAMPLE_RATE;
    _sampleRate = info.sampleRate;
    _bitRate = info.bitRate;
    _bits = info.bitsPerSample;
    _lengthMS = info.lengthMS;
    _title = info.title;
    _artist = info.artist;
    _album = info.album;
    _metaData = info.metadata;
    SetLoadedData(_trackSize);

    // Defer registering with the audio output until Play() is actually called.
    // Eager registration here forces AVAudioEngine setup (mixer node, IO unit,
    // audio unit render resources) for every AudioManager — even ones that are
    // only loaded for decoding (e.g. batch render), which never play. Each
    // AVAudioEngine instance accumulates dispatch queues / audio-unit pool
    // blocks that Apple's audio framework retains process-wide. Play(),
    // Play(posms,lenms), and AudioDeviceChanged() already handle the
    // HasAudio/AddAudio lazy-add path, so this is safe.
    return 0;
}











IAudioOutputManager* AudioManager::GetAudioManager() {
    return &__audioManager;
}

IAudioDecoder& AudioManager::GetDecoder() {
    return __audioDecoder;
}





// Access a single piece of track data
float AudioManager::GetFilteredLeftData(long offset) {
    
    while (!IsDataLoaded(offset)) {
        spdlog::debug("GetLeftData waiting for data to be loaded.");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (_data[0] == nullptr || offset > _trackSize) {
        return 0;
    }
    return _data[0][offset];
}

float AudioManager::GetRawLeftData(long offset) {
    
    while (!IsDataLoaded(offset)) {
        spdlog::debug("GetLeftData waiting for data to be loaded.");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    FilteredAudioData* fad = GetFilteredAudioData(AUDIOSAMPLETYPE::RAW, -1, -1);

    if (fad != nullptr && fad->data0 != nullptr && offset <= _trackSize) {
        return fad->data0[offset];
    }
    return 0;
}

double AudioManager::MidiToFrequency(int midi) {
    return pow(2.0, (midi - 69.0) / 12.0) * 440;
}

std::string AudioManager::MidiToNote(int midi) {
    static std::vector<std::string> notes = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    };
    int offset = midi % 12;
    int octave = midi / 12 - 1;
    return std::string(notes[offset]) + std::to_string(octave);
}

void AudioManager::NormaliseFilteredAudioData(FilteredAudioData* fad) {
    // PCM Data is the displayed waveform
    int16_t* pcm = fad->pcmdata;
    int min = 32000;
    int max = -32000;
    for (long i = 0; i < _pcmdatasize / (long)sizeof(int16_t); i++) {
        if (*pcm > max)
            max = *pcm;
        if (*pcm < min)
            min = *pcm;
        pcm++;
    }

    double scale = 1.0;
    double mm = (double)std::max(max, abs(min));
    assert(mm <= 32768.0);
    if (mm > 0) {
        scale = 32768.0 / mm;
    }

    // extra scaling based on the amount of frequency chosen
    if (fad->highNote - fad->lowNote != 0) {
        scale *= 128 / (fad->highNote - fad->lowNote);
    }
    // but dont let the scaling get out of hand
    if (scale > 10.0)
        scale = 10.0;

    pcm = fad->pcmdata;
    for (long i = 0; i < _pcmdatasize / (long)sizeof(int16_t); i++) {
        int newv = ((double)(*pcm) * scale);

        // clip if necessary
        if (newv > 32767)
            newv = 32767;
        if (newv < -32768)
            newv = -32768;

        *pcm = newv;
        pcm++;
    }

    float* data0 = fad->data0;
    float* data1 = fad->data1;
    float fmax = -99.0;
    float fmin = 99.0;
    for (long i = 0; i < _trackSize; i++) {
        if (*data0 > fmax)
            fmax = *data0;
        if (*data0 < fmin)
            fmin = *data0;
        if (data1 != nullptr) {
            if (*data1 > fmax)
                fmax = *data1;
            if (*data1 < fmin)
                fmin = *data1;
            data1++;
        }
        data0++;
    }

    double fscale = 1.0;
    mm = std::max(fmax, abs(fmin));
    assert(mm <= 32767.0);
    if (fmax > 0) {
        fscale = 1.0 / mm;
    }

    // data is the played music
    data0 = fad->data0;
    data1 = fad->data1;
    for (long i = 0; i < _trackSize; i++) {
        *data0 = *data0 * fscale;
        if (data1 != nullptr) {
            *data1 = *data1 * fscale;
            data1++;
        }
        data0++;
    }
}

FilteredAudioData* AudioManager::EnsureFilteredAudioData(AUDIOSAMPLETYPE type, int lowNote, int highNote) {
    static const double pi2 = 6.283185307;

    if (type == AUDIOSAMPLETYPE::BASS) {
        lowNote = 48;
        highNote = 60;
    } else if (type == AUDIOSAMPLETYPE::TREBLE) {
        lowNote = 60;
        highNote = 72;
    } else if (type == AUDIOSAMPLETYPE::ALTO) {
        lowNote = 72;
        highNote = 84;
    }

    std::lock_guard<std::recursive_mutex> flock(_filteredMutex);

    // Fast path: already cached.
    for (const auto& it : _filtered) {
        if ((type == AUDIOSAMPLETYPE::ANY || it->type == type) &&
            (lowNote == -1 || (it->lowNote == lowNote && it->highNote == highNote))) {
            return it;
        }
    }

    if (_data[0] == nullptr || _pcmdata == nullptr) {
        return nullptr;
    }

    // Wait for the full track to finish loading before running any
    // filter — the derived filters iterate `_trackSize` samples, and
    // the tail is uninitialised (calloc'd zeros on iPad) until the
    // async decoder thread catches up. Reading zeros from the tail
    // makes L/R look identical there and drives NONVOCALS / VOCALS
    // peak estimates wrongly toward zero.
    if (type != AUDIOSAMPLETYPE::RAW && _trackSize > 0) {
        int waits = 0;
        while (!IsDataLoaded(_trackSize - 1) && waits < 100) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            waits++;
        }
    }

    // Ensure the RAW cache entry exists — all downstream filters read
    // from `_data[0]` which is a snapshot of the original signal.
    if (_filtered.empty()) {
        FilteredAudioData* raw = new FilteredAudioData();
        long datasize = sizeof(float) * (_trackSize + _extra);
        raw->data0 = (float*)malloc(datasize);
        memcpy(raw->data0, _data[0], datasize);
        if (_data[1] != nullptr) {
            raw->data1 = (float*)malloc(datasize);
            memcpy(raw->data1, _data[1], datasize);
        }
        raw->pcmdata = (int16_t*)calloc(_pcmdatasize + PCMFUDGE, 1);
        memcpy(raw->pcmdata, _pcmdata, _pcmdatasize);
        raw->lowNote = 0;
        raw->highNote = 0;
        raw->type = AUDIOSAMPLETYPE::RAW;
        _filtered.push_back(raw);
        if (type == AUDIOSAMPLETYPE::RAW) return raw;
    }

    // All derived filters must read from the pristine RAW cache
    // entry, not from `_data[0]`/`_data[1]` — SwitchTo overwrites
    // those with the currently-selected filter's output for
    // playback, so reading from them chains filters (e.g. the user
    // picks BASS then NONVOCALS → NONVOCALS ends up subtracting
    // bass-filtered L from bass-filtered R instead of raw L from
    // raw R).
    const FilteredAudioData* rawFad = nullptr;
    for (const auto& it : _filtered) {
        if (it->type == AUDIOSAMPLETYPE::RAW) {
            rawFad = it;
            break;
        }
    }
    const float* srcL = rawFad ? rawFad->data0 : _data[0];
    const float* srcR = (rawFad && rawFad->data1) ? rawFad->data1
                                                  : (_data[1] ? _data[1] : nullptr);

    FilteredAudioData* fad = nullptr;
    switch (type) {
    case AUDIOSAMPLETYPE::NONVOCALS: {
        fad = new FilteredAudioData();
        long datasize = sizeof(float) * (_trackSize + _extra);
        fad->data0 = (float*)malloc(datasize);
        if (_data[1] != nullptr) {
            fad->data1 = (float*)malloc(datasize);
        }
        fad->pcmdata = (int16_t*)calloc(_pcmdatasize + PCMFUDGE, 1);

        // Detect a mono / channel-aliased load (decoder stored the
        // left-channel pointer into both slots). For those files the
        // classic M/S cancellation is mathematically zero everywhere —
        // fall back to the raw signal so the user sees a usable
        // waveform instead of a flat line.
        const bool stereoDistinct = (srcR != nullptr && srcR != srcL);
        const long ch = (long)_channels;
        if (stereoDistinct) {
            for (long i = 0; i < _trackSize; ++i) {
                float v = srcL[i] - srcR[i];
                fad->data0[i] = v;
                if (fad->data1) fad->data1[i] = v;
                int v2 = (int)(v * 32768);
                if (v2 > 32767) v2 = 32767;
                if (v2 < -32768) v2 = -32768;
                fad->pcmdata[i * ch] = (int16_t)v2;
                if (ch > 1) fad->pcmdata[i * ch + 1] = (int16_t)v2;
            }
        } else {
            spdlog::info("NONVOCALS: mono / aliased channel; falling back to raw waveform");
            for (long i = 0; i < _trackSize; ++i) {
                float v = srcL[i];
                fad->data0[i] = v;
                if (fad->data1) fad->data1[i] = v;
                int v2 = (int)(v * 32768);
                if (v2 > 32767) v2 = 32767;
                if (v2 < -32768) v2 = -32768;
                fad->pcmdata[i * ch] = (int16_t)v2;
                if (ch > 1) fad->pcmdata[i * ch + 1] = (int16_t)v2;
            }
        }
        fad->lowNote = 0;
        fad->highNote = 0;
        fad->type = type;
        NormaliseFilteredAudioData(fad);
        _filtered.push_back(fad);
    } break;
    case AUDIOSAMPLETYPE::RAW:
        // Handled by the RAW-ensure block above.
        break;
    case AUDIOSAMPLETYPE::ALTO:
    case AUDIOSAMPLETYPE::BASS:
    case AUDIOSAMPLETYPE::TREBLE:
    case AUDIOSAMPLETYPE::CUSTOM: {
        double lowHz = MidiToFrequency(lowNote);
        double highHz = MidiToFrequency(highNote);

        fad = new FilteredAudioData();

        long datasize = sizeof(float) * (_trackSize + _extra);
        fad->data0 = (float*)malloc(datasize);
        if (_data[1] != nullptr) {
            fad->data1 = (float*)malloc(datasize);
        }
        fad->pcmdata = (int16_t*)calloc(_pcmdatasize + PCMFUDGE, 1);

        // Normalize f_c and w_c so that pi is equal to the Nyquist angular frequency
        float f1_c = lowHz / _rate;
        float f2_c = highHz / _rate;
        const int order = 513; // 1025 is awesome but slow
        float a[order];
        float w1_c = pi2 * f1_c;
        float w2_c = pi2 * f2_c;
        int middle = order / 2.0; /*Integer division, dropping remainder*/
        for (int i = -1 * (order / 2); i <= order / 2; i++) {
            if (i == 0) {
                a[middle] = (2.0 * f2_c) - (2.0 * f1_c);
            } else {
                a[i + middle] = sin(w2_c * i) / (M_PI * i) - sin(w1_c * i) / (M_PI * i);
            }
        }
        parallel_for(0, _trackSize, [fad, this, a, order, srcL, srcR](int i) {
            float lvalue = 0;
            float rvalue = 0;
            for (int j = 0; j < order; j++) {
                int jj = i + j - order;
                if (jj >= 0 && jj < _trackSize) {
                    lvalue += srcL[jj] * a[order - j - 1];
                    if (srcR) {
                        rvalue += srcR[jj] * a[order - j - 1];
                    }
                }
            }
            fad->data0[i] = lvalue;

            lvalue = lvalue * 32768;
            int v2 = (int)lvalue;
            fad->pcmdata[i * _channels] = v2;
            if (_channels > 1) {
                if (srcR) {
                    fad->data1[i] = rvalue;
                    rvalue = rvalue * 32768;
                    v2 = (int)rvalue;
                    fad->pcmdata[i * _channels + 1] = v2;
                } else {
                    fad->pcmdata[i * _channels + 1] = v2;
                }
            }
        });

        fad->lowNote = lowNote;
        fad->highNote = highNote;
        fad->type = type;
        NormaliseFilteredAudioData(fad);
        _filtered.push_back(fad);
    } break;
    case AUDIOSAMPLETYPE::VOCALS: {
        // A8 (partial): centre-channel extraction. Mid = (L+R)/2 is
        // where centre-panned vocals live; Side = (L-R)/2 is where
        // stereo-panned instruments live. The hard-gate form
        //   v = sign(M) * max(0, |M| - α·|S|)
        // zeros out samples where stereo content dominates,
        // producing a visually distinct envelope from raw. Requires
        // stereo audio — mono / channel-aliased tracks fall back to
        // the raw signal since there's nothing to cancel.
        fad = new FilteredAudioData();
        long datasize = sizeof(float) * (_trackSize + _extra);
        fad->data0 = (float*)malloc(datasize);
        if (_data[1] != nullptr) {
            fad->data1 = (float*)malloc(datasize);
        }
        fad->pcmdata = (int16_t*)calloc(_pcmdatasize + PCMFUDGE, 1);

        const bool stereoDistinct = (srcR != nullptr && srcR != srcL);
        const float alpha = 1.5f;
        if (stereoDistinct) {
            const long ch = (long)_channels;
            for (long i = 0; i < _trackSize; ++i) {
                float L = srcL[i];
                float R = srcR[i];
                float M = 0.5f * (L + R);
                float S = 0.5f * (L - R);
                float absM = std::fabs(M);
                float absS = std::fabs(S);
                float mag = absM - alpha * absS;
                if (mag < 0) mag = 0;
                float v = (M >= 0 ? 1.0f : -1.0f) * mag;
                fad->data0[i] = v;
                if (fad->data1) fad->data1[i] = v;
                int v2 = int(v * 32768);
                if (v2 > 32767) v2 = 32767;
                if (v2 < -32768) v2 = -32768;
                fad->pcmdata[i * ch] = int16_t(v2);
                if (ch > 1) fad->pcmdata[i * ch + 1] = int16_t(v2);
            }
        } else {
            spdlog::info("VOCALS: mono / aliased channel; falling back to raw waveform");
            const long ch = (long)_channels;
            for (long i = 0; i < _trackSize; ++i) {
                float v = srcL[i];
                fad->data0[i] = v;
                if (fad->data1) fad->data1[i] = v;
                int v2 = (int)(v * 32768);
                if (v2 > 32767) v2 = 32767;
                if (v2 < -32768) v2 = -32768;
                fad->pcmdata[i * ch] = (int16_t)v2;
                if (ch > 1) fad->pcmdata[i * ch + 1] = (int16_t)v2;
            }
        }
        fad->lowNote = 0;
        fad->highNote = 0;
        fad->type = type;
        NormaliseFilteredAudioData(fad);
        _filtered.push_back(fad);
    } break;
    case AUDIOSAMPLETYPE::LUFS: {
        // A3: BS.1770 K-weighting → 400 ms momentary loudness envelope.
        // `data0` is filled with a sign-alternating signal whose
        // absolute value is the normalized loudness (LUFS mapped into
        // [0,1] over the range [-60, 0] dBFS). That way the waveform
        // min/max bucket loop sees ±loudness per bucket and renders a
        // symmetric envelope, the same visual idiom as the raw and
        // band-filtered waveforms.
        //
        // LUFS is a pure visualization — `pcmdata` is copied unchanged
        // from the RAW cache so that `SwitchTo(LUFS)` on desktop does
        // not replace the audible signal with the ±alternating
        // envelope (which would be a piercing buzz).
        fad = new FilteredAudioData();
        long datasize = sizeof(float) * (_trackSize + _extra);
        fad->data0 = (float*)calloc(1, datasize);
        if (_data[1] != nullptr) {
            fad->data1 = (float*)calloc(1, datasize);
        }
        fad->pcmdata = (int16_t*)calloc(_pcmdatasize + PCMFUDGE, 1);
        // Copy from the RAW cache entry (not `_pcmdata`, which may
        // already carry a previous filter's signal after a prior
        // SwitchTo) so playback is preserved regardless of ordering.
        {
            FilteredAudioData* rawEntry = nullptr;
            for (const auto& it : _filtered) {
                if (it->type == AUDIOSAMPLETYPE::RAW) { rawEntry = it; break; }
            }
            if (rawEntry && rawEntry->pcmdata) {
                memcpy(fad->pcmdata, rawEntry->pcmdata, _pcmdatasize);
            }
        }

        // RBJ cookbook coefficients at our sample rate for:
        //   1) +4 dB high-shelf (fc 1681.97 Hz, Q ~0.7071)
        //   2) high-pass (fc 38.14 Hz, Q ~0.5003)
        // These approximate the BS.1770 K-weighting curve for any
        // sample rate — ITU specifies 48 kHz coefficients; deriving
        // them here via cookbook avoids quality drop at 44.1 kHz.
        auto buildShelf = [this](double& b0, double& b1, double& b2,
                                 double& a1, double& a2,
                                 double fc, double Q, double gainDb) {
            double A = std::pow(10.0, gainDb / 40.0);
            double w0 = pi2 * fc / double(_rate);
            double cw = std::cos(w0);
            double sw = std::sin(w0);
            double alpha = sw / (2.0 * Q);
            double tsA = 2.0 * std::sqrt(A) * alpha;
            double B0 =    A*((A+1) + (A-1)*cw + tsA);
            double B1 = -2*A*((A-1) + (A+1)*cw);
            double B2 =    A*((A+1) + (A-1)*cw - tsA);
            double A0 =      (A+1) - (A-1)*cw + tsA;
            double A1 =   2*((A-1) - (A+1)*cw);
            double A2 =      (A+1) - (A-1)*cw - tsA;
            b0 = B0/A0; b1 = B1/A0; b2 = B2/A0;
            a1 = A1/A0; a2 = A2/A0;
        };
        auto buildHP = [this](double& b0, double& b1, double& b2,
                              double& a1, double& a2,
                              double fc, double Q) {
            double w0 = pi2 * fc / double(_rate);
            double cw = std::cos(w0);
            double sw = std::sin(w0);
            double alpha = sw / (2.0 * Q);
            double B0 =  (1 + cw) / 2;
            double B1 = -(1 + cw);
            double B2 =  (1 + cw) / 2;
            double A0 =   1 + alpha;
            double A1 =  -2*cw;
            double A2 =   1 - alpha;
            b0 = B0/A0; b1 = B1/A0; b2 = B2/A0;
            a1 = A1/A0; a2 = A2/A0;
        };
        double sb0, sb1, sb2, sa1, sa2;
        double hb0, hb1, hb2, ha1, ha2;
        buildShelf(sb0, sb1, sb2, sa1, sa2, 1681.974450955, 0.7071752369554, 4.0);
        buildHP   (hb0, hb1, hb2, ha1, ha2, 38.13547087602444, 0.5003270373238773);

        // Apply both biquads and square. Direct-form-II-transposed.
        std::vector<float> kw(_trackSize, 0.0f);
        double sz1 = 0, sz2 = 0;
        double hz1 = 0, hz2 = 0;
        for (long i = 0; i < _trackSize; i++) {
            double x = srcL[i];
            double y = sb0 * x + sz1;
            sz1 = sb1 * x - sa1 * y + sz2;
            sz2 = sb2 * x - sa2 * y;
            double y2 = hb0 * y + hz1;
            hz1 = hb1 * y - ha1 * y2 + hz2;
            hz2 = hb2 * y - ha2 * y2;
            kw[i] = float(y2 * y2);
        }

        // 400 ms moving mean via running sum. Window must be at least
        // 1 sample long.
        const long W = std::max<long>(1, (_rate * 4) / 10);
        double sum = 0;
        const long initEnd = std::min<long>(W, _trackSize);
        for (long i = 0; i < initEnd; i++) sum += kw[i];
        const long half = W / 2;
        for (long i = 0; i < _trackSize; i++) {
            const long windowSamples = std::min<long>(W, _trackSize);
            double ms = sum / double(windowSamples);
            double lufs = -0.691 + 10.0 * std::log10(std::max(ms, 1e-10));
            // Map LUFS [-60, 0] → norm [0, 1].
            double norm = (lufs + 60.0) / 60.0;
            if (norm < 0) norm = 0; else if (norm > 1) norm = 1;
            float signedVal = (i & 1) ? -float(norm) : float(norm);
            // Place the output at i - half so the window is centred on
            // sample i, not trailing. Edge samples get the edge value.
            long outIdx = i - half;
            if (outIdx >= 0 && outIdx < _trackSize) {
                fad->data0[outIdx] = signedVal;
            }
            // Slide the window.
            long nextIn = i + 1;
            long nextOut = nextIn - W;
            if (nextIn < _trackSize && nextOut >= 0) {
                sum += kw[nextIn] - kw[nextOut];
            } else if (nextIn < _trackSize) {
                sum += kw[nextIn];
            }
        }
        // Mirror to the right channel so the existing draw path (which
        // may average both channels) stays consistent.
        if (fad->data1 && _data[1]) {
            memcpy(fad->data1, fad->data0, datasize);
        }
        fad->lowNote = 0;
        fad->highNote = 0;
        fad->type = type;
        _filtered.push_back(fad);
    } break;
    case AUDIOSAMPLETYPE::CLASSIFIED: {
        // A7: raw signal gated by the class-confidence curve set via
        // `SetClassifyGate`. Each sample is multiplied by the
        // linearly-interpolated confidence for its timestamp so
        // moments where the selected class dominates stay at full
        // amplitude and other moments fade toward silence — both in
        // the display waveform AND in the playback PCM stream.
        if (_classifyGateCurve.empty() || _classifyGateTimeStep <= 0) {
            break;
        }
        fad = new FilteredAudioData();
        long datasize = sizeof(float) * (_trackSize + _extra);
        fad->data0 = (float*)malloc(datasize);
        if (srcR) {
            fad->data1 = (float*)malloc(datasize);
        }
        fad->pcmdata = (int16_t*)calloc(_pcmdatasize + PCMFUDGE, 1);

        const double stepSamples = double(_classifyGateTimeStep) * double(_rate);
        const int curveN = int(_classifyGateCurve.size());
        for (long i = 0; i < _trackSize; ++i) {
            double t = double(i) / stepSamples;
            int i0 = int(std::floor(t));
            if (i0 < 0) i0 = 0;
            if (i0 >= curveN) i0 = curveN - 1;
            int i1 = std::min(i0 + 1, curveN - 1);
            double frac = t - std::floor(t);
            if (frac < 0) frac = 0; else if (frac > 1) frac = 1;
            float gate = float(_classifyGateCurve[i0] * (1.0 - frac) +
                               _classifyGateCurve[i1] * frac);
            float L = srcL[i] * gate;
            fad->data0[i] = L;
            int v2 = (int)(L * 32768);
            if (v2 > 32767) v2 = 32767;
            if (v2 < -32768) v2 = -32768;
            fad->pcmdata[i * _channels] = int16_t(v2);
            if (fad->data1) {
                float R = srcR[i] * gate;
                fad->data1[i] = R;
                int v2r = (int)(R * 32768);
                if (v2r > 32767) v2r = 32767;
                if (v2r < -32768) v2r = -32768;
                if (_channels > 1) fad->pcmdata[i * _channels + 1] = int16_t(v2r);
            } else if (_channels > 1) {
                fad->pcmdata[i * _channels + 1] = int16_t(v2);
            }
        }
        fad->lowNote = 0;
        fad->highNote = 0;
        fad->type = type;
        _filtered.push_back(fad);
    } break;
    case AUDIOSAMPLETYPE::STEM_DRUMS:
    case AUDIOSAMPLETYPE::STEM_BASS:
    case AUDIOSAMPLETYPE::STEM_OTHER:
    case AUDIOSAMPLETYPE::STEM_VOCALS: {
        // A8: build the FilteredAudioData from the cached stem
        // buffers set by `SetStemData`. Picks the right vector pair
        // per stem type; absent data → return nullptr so the bridge
        // / draw path falls back to raw.
        const std::vector<float>* L = nullptr;
        const std::vector<float>* R = nullptr;
        switch (type) {
        case AUDIOSAMPLETYPE::STEM_DRUMS:  L = &_stemDrumsL;  R = &_stemDrumsR; break;
        case AUDIOSAMPLETYPE::STEM_BASS:   L = &_stemBassL;   R = &_stemBassR; break;
        case AUDIOSAMPLETYPE::STEM_OTHER:  L = &_stemOtherL;  R = &_stemOtherR; break;
        case AUDIOSAMPLETYPE::STEM_VOCALS: L = &_stemVocalsL; R = &_stemVocalsR; break;
        default: break;
        }
        if (L == nullptr || L->empty()) {
            break; // no stems separated yet
        }
        fad = new FilteredAudioData();
        long datasize = sizeof(float) * (_trackSize + _extra);
        fad->data0 = (float*)calloc(1, datasize);
        if (srcR) {
            fad->data1 = (float*)calloc(1, datasize);
        }
        fad->pcmdata = (int16_t*)calloc(_pcmdatasize + PCMFUDGE, 1);
        const long n = std::min<long>(_trackSize, long(L->size()));
        const bool haveR = (R && !R->empty() && R->size() >= size_t(n));
        for (long i = 0; i < n; i++) {
            float l = (*L)[i];
            float r = haveR ? (*R)[i] : l;
            fad->data0[i] = l;
            if (fad->data1) fad->data1[i] = r;
            int vl = (int)(l * 32768);
            if (vl > 32767) vl = 32767;
            if (vl < -32768) vl = -32768;
            fad->pcmdata[i * _channels] = int16_t(vl);
            if (_channels > 1) {
                int vr = (int)(r * 32768);
                if (vr > 32767) vr = 32767;
                if (vr < -32768) vr = -32768;
                fad->pcmdata[i * _channels + 1] = int16_t(vr);
            }
        }
        fad->lowNote = 0;
        fad->highNote = 0;
        fad->type = type;
        _filtered.push_back(fad);
    } break;
    case AUDIOSAMPLETYPE::ANY:
        break;
    }

    // `NormaliseFilteredAudioData` only scales `pcmdata`, but the
    // waveform display reads `data0`. For the M/S-based filters the
    // output is inherently small (NONVOCALS: L-R, VOCALS: M-α|S|)
    // so the un-scaled `data0` reads as a near-flat line next to
    // raw. Rescale data0/data1 to a visible peak here. LUFS stays
    // in its already-normalised [-1, 1] range so we skip it; the
    // bandpass filters (BASS/TREBLE/ALTO/CUSTOM) get the same
    // rescale so their waveforms read at a comparable height to
    // raw regardless of how much energy the band carries.
    if (fad != nullptr &&
        type != AUDIOSAMPLETYPE::RAW &&
        type != AUDIOSAMPLETYPE::LUFS &&
        type != AUDIOSAMPLETYPE::CLASSIFIED &&
        type != AUDIOSAMPLETYPE::STEM_DRUMS &&
        type != AUDIOSAMPLETYPE::STEM_BASS &&
        type != AUDIOSAMPLETYPE::STEM_OTHER &&
        type != AUDIOSAMPLETYPE::STEM_VOCALS) {
        float peak = 0;
        for (long i = 0; i < _trackSize; i++) {
            float a = std::fabs(fad->data0[i]);
            if (a > peak) peak = a;
        }
        if (peak > 1e-6f && peak < 0.9f) {
            float scale = 0.9f / peak;
            if (scale > 20.0f) scale = 20.0f;
            for (long i = 0; i < _trackSize; i++) {
                fad->data0[i] *= scale;
                if (fad->data1 && fad->data1 != fad->data0) fad->data1[i] *= scale;
            }
        }
    }

    return fad;
}

void AudioManager::SwitchTo(AUDIOSAMPLETYPE type, int lowNote, int highNote) {
    while (!IsDataLoaded()) {

        spdlog::debug("SwitchTo waiting for data to be loaded.");
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    std::unique_lock<std::shared_timed_mutex> locker(_mutex);

    // Cant be playing when switching
    bool wasPlaying = IsPlaying();
    if (wasPlaying) {
        Pause();
    }

    FilteredAudioData* fad = EnsureFilteredAudioData(type, lowNote, highNote);

    if (fad && _pcmdata && fad->pcmdata) {
        memcpy(_pcmdata, fad->pcmdata, _pcmdatasize);
    }

    {
        long datasize = sizeof(float) * (_trackSize + _extra);
        if (fad && _data[0] && fad->data0) {
            memcpy(_data[0], fad->data0, datasize);
        }

        if (fad && _data[1] && fad->data1) {
            memcpy(_data[1], fad->data1, datasize);
        }
    }

    if (wasPlaying) {
        Play();
    }
}

FilteredAudioData* AudioManager::GetFilteredAudioData(AUDIOSAMPLETYPE type, int lowNote, int highNote) {
    for (;;) {
        std::unique_lock<std::recursive_mutex> flock(_filteredMutex);
        if (!_filtered.empty()) {
            for (const auto& it : _filtered) {
                if ((type == AUDIOSAMPLETYPE::ANY || it->type == type) && (lowNote == -1 || (it->lowNote == lowNote && it->highNote == highNote))) {
                    return it;
                }
            }
            return nullptr;
        }
        flock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void AudioManager::SetStemData(const std::vector<float>& drumsL, const std::vector<float>& drumsR,
                                const std::vector<float>& bassL, const std::vector<float>& bassR,
                                const std::vector<float>& otherL, const std::vector<float>& otherR,
                                const std::vector<float>& vocalsL, const std::vector<float>& vocalsR) {
    std::unique_lock<std::shared_timed_mutex> locker(_mutex);
    _stemDrumsL = drumsL;   _stemDrumsR = drumsR;
    _stemBassL = bassL;     _stemBassR = bassR;
    _stemOtherL = otherL;   _stemOtherR = otherR;
    _stemVocalsL = vocalsL; _stemVocalsR = vocalsR;
    std::lock_guard<std::recursive_mutex> flock(_filteredMutex);
    // Evict any cached STEM_* entries so the next
    // `EnsureFilteredAudioData(STEM_X)` rebuilds from the new data.
    for (auto it = _filtered.begin(); it != _filtered.end(); ) {
        AUDIOSAMPLETYPE t = (*it)->type;
        if (t == AUDIOSAMPLETYPE::STEM_DRUMS || t == AUDIOSAMPLETYPE::STEM_BASS ||
            t == AUDIOSAMPLETYPE::STEM_OTHER || t == AUDIOSAMPLETYPE::STEM_VOCALS) {
            if ((*it)->data0) free((*it)->data0);
            if ((*it)->data1 && (*it)->data1 != (*it)->data0) free((*it)->data1);
            if ((*it)->pcmdata) free((*it)->pcmdata);
            delete *it;
            it = _filtered.erase(it);
        } else {
            ++it;
        }
    }
}

std::string AudioManager::WriteCurrentToTempWav() const {
    // Snapshot the playback buffers under the audio lock so a
    // concurrent SwitchTo() can't repoint them out from under us
    // mid-write. SwitchTo takes a unique_lock on _mutex, so this
    // shared_lock serializes against it.
    std::shared_lock<std::shared_timed_mutex> locker(const_cast<std::shared_timed_mutex&>(_mutex));
    if (_data[0] == nullptr || _trackSize <= 0) return {};

    const float* L = _data[0];
    // Mono sources leave _data[1] either null or aliased to _data[0];
    // either way duplicate the left channel into the right so the
    // output is always stereo float32 (what every recognizer accepts).
    const float* R = (_data[1] != nullptr) ? _data[1] : _data[0];

    const auto frames = (uint32_t)_trackSize;
    const auto sr = (uint32_t)(_rate > 0 ? _rate : 44100);

    // Build a unique temp path. Use the audio file's basename (when
    // available) so multiple concurrent xLights instances can't race
    // each other's exports for different sequences.
    std::error_code ec;
    auto tmpDir = std::filesystem::temp_directory_path(ec);
    if (ec) tmpDir = std::filesystem::path("/tmp");
    std::string stem = std::filesystem::path(_audio_file).stem().string();
    if (stem.empty()) stem = "audio";
    auto tsNS = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()).count();
    std::string filename = "xlights_audio_" + stem + "_" + std::to_string(tsNS) + ".wav";
    std::filesystem::path outPath = tmpDir / filename;

    // Stereo float32 WAV, RIFF + fmt(IEEE float, 3) + data.
    // Header layout (44 bytes):
    //   "RIFF" <fileSize-8> "WAVE"
    //   "fmt " 16 (3 PCM_FLOAT) (2 channels) <sr> <byteRate> <blockAlign> <bitsPerSample=32>
    //   "data" <dataSize>
    const uint16_t channels      = 2;
    const uint16_t bitsPerSample = 32;
    const uint16_t blockAlign    = channels * (bitsPerSample / 8);
    const uint32_t byteRate      = sr * blockAlign;
    const uint32_t dataSize      = frames * blockAlign;
    const uint32_t fileSize      = 36 + dataSize;

    std::ofstream f(outPath, std::ios::binary);
    if (!f) return {};

    auto write_u32 = [&](uint32_t v) { f.write(reinterpret_cast<const char*>(&v), 4); };
    auto write_u16 = [&](uint16_t v) { f.write(reinterpret_cast<const char*>(&v), 2); };

    f.write("RIFF", 4);   write_u32(fileSize);  f.write("WAVE", 4);
    f.write("fmt ", 4);   write_u32(16);
    write_u16(3);                  // WAVE_FORMAT_IEEE_FLOAT
    write_u16(channels);
    write_u32(sr);
    write_u32(byteRate);
    write_u16(blockAlign);
    write_u16(bitsPerSample);
    f.write("data", 4);   write_u32(dataSize);

    // Interleave L/R samples. _data is full-track-length mono arrays
    // per channel; iterate once writing a stereo sample per frame.
    // Buffered std::ofstream handles the millions-of-writes pattern
    // fine on modern OSes; switching to a contiguous std::vector
    // staging buffer + one write() didn't measurably help in
    // benchmarks here.
    for (uint32_t i = 0; i < frames; ++i) {
        float l = L[i];
        float r = R[i];
        f.write(reinterpret_cast<const char*>(&l), sizeof(float));
        f.write(reinterpret_cast<const char*>(&r), sizeof(float));
    }
    if (!f.good()) {
        f.close();
        std::filesystem::remove(outPath, ec);
        return {};
    }
    return outPath.string();
}

void AudioManager::SetClassifyGate(const std::string& className,
                                    const std::vector<float>& confidencePerStep,
                                    float timeStepSec) {
    std::unique_lock<std::shared_timed_mutex> locker(_mutex);
    _classifyGateClass = className;
    _classifyGateCurve = confidencePerStep;
    _classifyGateTimeStep = timeStepSec > 0 ? timeStepSec : 1.0f;
    std::lock_guard<std::recursive_mutex> flock(_filteredMutex);
    // Evict any cached CLASSIFIED entry so the next
    // EnsureFilteredAudioData(CLASSIFIED) rebuilds against the new
    // curve.
    for (auto it = _filtered.begin(); it != _filtered.end(); ) {
        if ((*it)->type == AUDIOSAMPLETYPE::CLASSIFIED) {
            if ((*it)->data0) free((*it)->data0);
            if ((*it)->data1 && (*it)->data1 != (*it)->data0) free((*it)->data1);
            if ((*it)->pcmdata) free((*it)->pcmdata);
            delete *it;
            it = _filtered.erase(it);
        } else {
            ++it;
        }
    }
}

void AudioManager::GetLeftDataMinMax(long start, long end, float& minimum, float& maximum, AUDIOSAMPLETYPE type, int lowNote, int highNote, float* rms) {

    while (!IsDataLoaded(end - 1)) {
        spdlog::debug("GetLeftDataMinMax waiting for data to be loaded.");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    minimum = 0;
    maximum = 0;
    if (rms) *rms = 0;

    FilteredAudioData* fad = GetFilteredAudioData(type, lowNote, highNote);
    if (!fad) {
        return;
    }

    long last = std::min(end, _trackSize);
    double sumSq = 0;
    long count = 0;
    for (long j = start; j < last; j++) {
        float v = fad->data0[j];
        minimum = std::min(minimum, v);
        maximum = std::max(maximum, v);
        if (rms) {
            sumSq += double(v) * double(v);
            count++;
        }
    }
    if (rms && count > 0) {
        *rms = (float)std::sqrt(sumSq / double(count));
    }
}

// Access a single piece of track data
float AudioManager::GetFilteredRightData(long offset) {
    
    while (!IsDataLoaded(offset)) {
        spdlog::debug("GetRightData waiting for data to be loaded.");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (_data[1] == nullptr || offset > _trackSize) {
        return 0;
    }
    return _data[1][offset];
}

float AudioManager::GetRawRightData(long offset) {
    
    while (!IsDataLoaded(offset)) {
        spdlog::debug("GetRightData waiting for data to be loaded.");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    FilteredAudioData* fad = GetFilteredAudioData(AUDIOSAMPLETYPE::RAW, -1, -1);

    if (fad != nullptr && fad->data1 != nullptr && offset <= _trackSize) {
        return fad->data1[offset];
    }
    return 0;
}

// Access track data but get a pointer so you can then read a block directly
float* AudioManager::GetFilteredLeftDataPtr(long offset) {
    
    while (!IsDataLoaded(offset)) {
        spdlog::debug("GetLeftDataPtr waiting for data to be loaded.");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    assert(_data[0] != nullptr);
    if (offset > _trackSize) {
        return nullptr;
    }
    return &_data[0][offset];
}

float* AudioManager::GetRawLeftDataPtr(long offset) {
    
    while (!IsDataLoaded(offset)) {
        spdlog::debug("GetLeftDataPtr waiting for data to be loaded.");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    FilteredAudioData* fad = GetFilteredAudioData(AUDIOSAMPLETYPE::RAW, -1, -1);

    if (fad != nullptr && offset <= _trackSize)
        return &fad->data0[offset];
    return nullptr;
}

float* AudioManager::GetRawRightDataPtr(long offset) {
    
    while (!IsDataLoaded(offset)) {
        spdlog::debug("GetRightDataPtr waiting for data to be loaded.");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    FilteredAudioData* fad = GetFilteredAudioData(AUDIOSAMPLETYPE::RAW, -1, -1);

    if (fad != nullptr && fad->data1 != nullptr && offset <= _trackSize)
        return &fad->data1[offset];
    return nullptr;
}

// Access track data but get a pointer so you can then read a block directly
float* AudioManager::GetFilteredRightDataPtr(long offset) {
    
    while (!IsDataLoaded(offset)) {
        spdlog::debug("GetRightDataPtr waiting for data to be loaded.");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    assert(_data[1] != nullptr);
    if (offset > _trackSize) {
        return nullptr;
    }
    return &_data[1][offset];
}

// AudioManager::Hash is a portable MD5 over the audio track's sample
// data — used by iPad's spectrogram cache to invalidate when the
// audio content changes. Kept outside the VAMP `#if` gate below so
// iOS builds (which exclude VAMP plugin code) still get this
// symbol. Desktop callers use it too.
std::string AudioManager::Hash() {
    if (_hash == "") {

        while (!IsDataLoaded(_trackSize)) {
            spdlog::debug("GetLeftDataPtr waiting for data to be loaded.");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        MD5 md5;
        md5.update((unsigned char*)_data[0], sizeof(float) * _trackSize);
        md5.finalize();
        _hash = md5.hexdigest();
    }

    return _hash;
}

#if !TARGET_OS_IPHONE
// xLightsVamp Functions
xLightsVamp::xLightsVamp() {

    spdlog::debug("Constructing xLightsVamp");
    _loader = Vamp::HostExt::PluginLoader::getInstance();
}

xLightsVamp::~xLightsVamp() {
    while (_loadedPlugins.size() > 0) {
        delete _loadedPlugins.back();
        _loadedPlugins.pop_back();
    }
}

// extract the features data from a Vamp plugins output
void xLightsVamp::ProcessFeatures(Vamp::Plugin::FeatureList& feature, std::vector<int>& starts, std::vector<int>& ends, std::vector<std::string>& labels) {
    bool hadDuration = true;

    for (size_t x = 0; x < feature.size(); x++) {
        int start = feature[x].timestamp.msec() + feature[x].timestamp.sec * 1000;
        starts.push_back(start);

        if (!hadDuration) {
            ends.push_back(start);
        }
        hadDuration = feature[x].hasDuration;

        if (hadDuration) {
            int end = start + feature[x].duration.msec() + feature[x].duration.sec * 1000;
            ends.push_back(end);
        }
        labels.push_back(feature[x].label);
    }

    if (!hadDuration) {
        ends.push_back(starts[starts.size() - 1]);
    }
}

// Load plugins
void xLightsVamp::LoadPlugins(AudioManager* paudio) {
    // dont need to load it twice
    if (_loadedPlugins.size() > 0) {
        return;
    }

    
    spdlog::debug("Loading plugins.");

    Vamp::HostExt::PluginLoader::PluginKeyList pluginList = _loader->listPlugins();

    spdlog::debug("Plugins found {}.", pluginList.size());

    for (size_t x = 0; x < pluginList.size(); x++) {
        Vamp::Plugin* p = _loader->loadPlugin(pluginList[x], paudio->GetRate());
        if (p == nullptr) {
            // skip any that dont load
            continue;
        }
        _loadedPlugins.push_back(p);
    }
}

// Get & load plugins that return timing marks
std::list<std::string> xLightsVamp::GetAvailablePlugins(AudioManager* paudio) {
    std::list<std::string> ret;

    // Load the plugins in case they have not already been loaded
    LoadPlugins(paudio);

    for (const auto& it : _loadedPlugins) {
        Plugin::OutputList outputs = it->getOutputDescriptors();

        for (const auto& j : outputs) {
            if (j.sampleType == Plugin::OutputDescriptor::FixedSampleRate ||
                j.sampleType == Plugin::OutputDescriptor::OneSamplePerStep ||
                !j.hasFixedBinCount ||
                (j.hasFixedBinCount && j.binCount > 1)) {
                // We are filering out this from our return array
                continue;
            }

            std::string name = it->getName();

            if (outputs.size() > 1) {
                // This is not the plugin's only output.
                // Use "plugin name: output name" as the effect name,
                // unless the output name is the same as the plugin name
                std::string outputName = j.name;
                if (outputName != name) {
                    std::ostringstream stringStream;
                    stringStream << name << ": " << outputName.c_str();
                    name = stringStream.str();
                }
            }

            _plugins[name] = it;
        }
    }

    for (std::map<std::string, Vamp::Plugin*>::iterator it = _plugins.begin(); it != _plugins.end(); ++it) {
        ret.push_back(it->first);
    }

    return ret;
}

// Get a list of all plugins
std::list<std::string> xLightsVamp::GetAllAvailablePlugins(AudioManager* paudio) {
    std::list<std::string> ret;

    // load the plugins if they have not already been loaded
    LoadPlugins(paudio);

    for (const auto& it : _loadedPlugins) {
        Plugin::OutputList outputs = it->getOutputDescriptors();

        for (const auto& j : outputs) {
            std::string name = it->getName();

            if (outputs.size() > 1) {
                // This is not the plugin's only output.
                // Use "plugin name: output name" as the effect name,
                // unless the output name is the same as the plugin name
                std::string outputName = j.name;
                if (outputName != name) {
                    std::ostringstream stringStream;
                    stringStream << name << ": " << outputName.c_str();
                    name = stringStream.str();
                }
            }

            _allplugins[name] = it;
        }
    }

    for (const auto& it : _allplugins) {
        ret.push_back(it.first);
    }

    return ret;
}

// Get a plugin
Vamp::Plugin* xLightsVamp::GetPlugin(std::string name) {
    Plugin* p = _plugins[name];

    if (p == nullptr) {
        p = _allplugins[name];
    }

    return p;
}
#endif // !TARGET_OS_IPHONE

std::list<std::string> AudioManager::GetAudioDevices() {
    return __audioManager.GetOutputDevices();
}

std::list<std::string> AudioManager::GetInputAudioDevices() {
    return __audioManager.GetInputDevices();
}

bool AudioManager::WriteCurrentAudio(const std::string& path, long bitrate) {
    spdlog::info("AudioManager::WriteCurrentAudio() - {} samples to export to '{}'", _trackSize, path.c_str());
    std::vector<float> leftData(_data[0], _data[0] + _trackSize);
    std::vector<float> rightData(_data[1], _data[1] + _trackSize);
    return GetDecoder().EncodeToFile(leftData, rightData, bitrate, path);
}

bool AudioManager::EncodeAudio(const std::vector<float>& left_channel,
                               const std::vector<float>& right_channel,
                               size_t sampleRate,
                               const std::string& filename,
                               AudioManager* /*copyFrom*/) {
    return GetDecoder().EncodeToFile(left_channel, right_channel, sampleRate, filename);
}

