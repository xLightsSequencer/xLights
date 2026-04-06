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

using namespace Vamp;

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

static void ProgressFunction(int p) {
    // placeholder for polyphonic transcription progress
}

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

    if (__audioManager.GetOutput(_device) != nullptr)
        __audioManager.GetOutput(_device)->Seek(_sdlid, pos);
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
    if (__audioManager.GetOutput(_device) != nullptr) {
        __audioManager.GetOutput(_device)->Stop();
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
    if (_filtered.size() == 0) {
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
    if (needNotes && !_polyphonicTranscriptionDone) {
        // need to do the polyphonic stuff
        DoPolyphonicTranscription(ProgressFunction);
    }
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

    while (_filtered.size() > 0) {
        if (_filtered.back()->data0) {
            free(_filtered.back()->data0);
        }
        if (_filtered.back()->data1) {
            free(_filtered.back()->data1);
        }
        if (_filtered.back()->pcmdata) {
            free(_filtered.back()->pcmdata);
        }
        delete _filtered.back();
        _filtered.pop_back();
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

    // Register with audio output for playback
    if (_pcmdata != nullptr) {
        auto* out = __audioManager.GetOutput(_device);
        if (out != nullptr) {
            _sdlid = out->AddAudio(_pcmdatasize, _pcmdata, 100, _rate, _trackSize, _lengthMS);
        }
    }

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
    for (int i = 0; i < _trackSize; i++) {
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
    for (int i = 0; i < _trackSize; i++) {
        *data0 = *data0 * fscale;
        if (data1 != nullptr) {
            *data1 = *data1 * fscale;
            data1++;
        }
        data0++;
    }
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

    if (_data[0] == nullptr || _pcmdata == nullptr) {
        return;
    }
    if (_filtered.empty()) {
        // save original pcm
        FilteredAudioData* fad = new FilteredAudioData();
        long datasize = sizeof(float) * (_trackSize + _extra);
        fad->data0 = (float*)malloc(datasize);
        memcpy(fad->data0, _data[0], datasize);
        if (_data[1] != nullptr) {
            fad->data1 = (float*)malloc(datasize);
            memcpy(fad->data1, _data[1], datasize);
        }
        fad->pcmdata = (int16_t*)calloc(_pcmdatasize + PCMFUDGE, 1);
        memcpy(fad->pcmdata, _pcmdata, _pcmdatasize);
        fad->lowNote = 0;
        fad->highNote = 0;
        fad->type = AUDIOSAMPLETYPE::RAW;
        _filtered.push_back(fad);
    }

    FilteredAudioData* fad = nullptr;
    switch (type) {
    case AUDIOSAMPLETYPE::NONVOCALS: {
        // This assumes the vocals are in one track
        // grab it from my cache if i have it
        fad = GetFilteredAudioData(type, -1, -1);
        if (fad == nullptr) {
            fad = new FilteredAudioData();
            long datasize = sizeof(float) * (_trackSize + _extra);
            fad->data0 = (float*)malloc(datasize);
            if (_data[1] != nullptr) {
                fad->data1 = (float*)malloc(datasize);
            }
            fad->pcmdata = (int16_t*)calloc(_pcmdatasize + PCMFUDGE, 1);

            for (int i = 0; i < _trackSize; ++i) {
                float v = _data[0][i];
                if (_data[1]) {
                    float v1 = _data[1][i];
                    v = (v - v1);
                }
                fad->data0[i] = v;
                if (fad->data1)
                    fad->data1[i] = v;

                v = v * 32768;
                int v2 = (int)v;
                fad->pcmdata[i * _channels] = v2;
                if (_channels > 1) {
                    fad->pcmdata[i * _channels + 1] = v2;
                }
            }
            fad->lowNote = 0;
            fad->highNote = 0;
            fad->type = type;
            NormaliseFilteredAudioData(fad);
            _filtered.push_back(fad);
        }
    } break;
    case AUDIOSAMPLETYPE::RAW:
        // grab it from my cache if i have it
        fad = GetFilteredAudioData(type, -1, -1);
        break;
    case AUDIOSAMPLETYPE::ALTO:
    case AUDIOSAMPLETYPE::BASS:
    case AUDIOSAMPLETYPE::TREBLE:
    case AUDIOSAMPLETYPE::CUSTOM: {
        // grab it from my cache if i have it
        fad = GetFilteredAudioData(AUDIOSAMPLETYPE::ANY, lowNote, highNote);

        // if we didnt find it ... create it
        if (fad == nullptr) {
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
            // Now apply a windowing function to taper the edges of the filter, e.g.
            parallel_for(0, _trackSize, [fad, this, a, order](int i) {
                float lvalue = 0;
                float rvalue = 0;
                for (int j = 0; j < order; j++) {
                    int jj = i + j - order;
                    if (jj >= 0 && jj < _trackSize) {
                        lvalue += _data[0][jj] * a[order - j - 1];
                        if (_data[1]) {
                            rvalue += _data[1][jj] * a[order - j - 1];
                        }
                    }
                }
                fad->data0[i] = lvalue;

                lvalue = lvalue * 32768;
                int v2 = (int)lvalue;
                fad->pcmdata[i * _channels] = v2;
                if (_channels > 1) {
                    if (_data[1]) {
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
        }
    } break;
    case AUDIOSAMPLETYPE::ANY:
        break;
    }

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
    while (_filtered.size() == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    for (const auto& it : _filtered) {
        if ((type == AUDIOSAMPLETYPE::ANY || it->type == type) && (lowNote == -1 || (it->lowNote == lowNote && it->highNote == highNote))) {
            return it;
        }
    }
    return nullptr;
}

void AudioManager::GetLeftDataMinMax(long start, long end, float& minimum, float& maximum, AUDIOSAMPLETYPE type, int lowNote, int highNote) {
    
    while (!IsDataLoaded(end - 1)) {
        spdlog::debug("GetLeftDataMinMax waiting for data to be loaded.");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    minimum = 0;
    maximum = 0;

    FilteredAudioData* fad = GetFilteredAudioData(type, lowNote, highNote);
    if (!fad) {
        return;
    }

    for (int j = start; j < std::min(end, _trackSize); j++) {
        minimum = std::min(minimum, fad->data0[j]);
        maximum = std::max(maximum, fad->data0[j]);
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

