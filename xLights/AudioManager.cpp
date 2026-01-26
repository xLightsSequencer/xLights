/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/ffile.h>
#include <wx/log.h>
#include <wx/string.h>
#include <wx/wx.h>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>

#include <math.h>
#include <stdlib.h>

#include "AudioManager.h"
#include "ExternalHooks.h"
#include "Parallel.h"
#include "UtilFunctions.h"
#include "../xSchedule/md5.h"
#include "kiss_fft/tools/kiss_fftr.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}
#include "spdlog/spdlog.h"

using namespace Vamp;

// SDL Functions
int AudioData::__nextId = 0;
SDLManager __sdlManager;

#define SDL_INPUT_BUFFER_SIZE 8192

#ifndef __WXOSX__
#define DEFAULT_NUM_SAMPLES 1024
#define RESAMPLE_RATE (44100)
#define DEFAULT_RATE RESAMPLE_RATE
#else
// OSX recommendation is to keep the number of samples buffered very small at 256
// Also recommend sampling to 48000 as that's what the drivers do internally anyway
#define DEFAULT_NUM_SAMPLES 256
#define RESAMPLE_RATE (48000)
#define DEFAULT_RATE RESAMPLE_RATE
#endif

#ifndef CODEC_CAP_DELAY /* add compatibility for ffmpeg 3+ */
#define CODEC_CAP_DELAY AV_CODEC_CAP_DELAY
#endif

#ifndef CODEC_FLAG_GLOBAL_HEADER /* add compatibility for ffmpeg 3+ */
#define CODEC_FLAG_GLOBAL_HEADER AV_CODEC_FLAG_GLOBAL_HEADER
#endif

#define PCMFUDGE 32768

// Due to Ubuntu still using FFMpeg 4.x, we have to use some deprecated API's
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

void fill_audio(void* udata, Uint8* stream, int len) {
    // SDL 2.0
    SDL_memset(stream, 0, len);
    wxASSERT(strlen(SDL_GetError()) == 0);

    OutputSDL* sdl = (OutputSDL*)udata;
    std::mutex* audio_lock = sdl->GetAudioLock();

    std::unique_lock<std::mutex> locker(*audio_lock);

    auto media = sdl->GetAudio();

    for (const auto& it : media) {
        if (it->_audio_len == 0 || it->_paused) /*  Only  play  if  we  have  data  left and not paused */
        {
            // no data left
        } else {
            len = (len > it->_audio_len ? it->_audio_len : len); /*  Mix  as  much  data  as  possible  */
            int volume = it->_volume;
            if (__sdlManager.GetGlobalVolume() != 100) {
                volume = (volume * __sdlManager.GetGlobalVolume()) / 100;
            }
            // #ifdef __WXMSW__
            SDL_MixAudioFormat(stream, it->_audio_pos, AUDIO_S16SYS, len, volume);
            // wxASSERT(strlen(SDL_GetError()) == 0);
            // #else
            //  TODO we need to replace this on OSX/Linux
            //             SDL_MixAudio(stream, it->_audio_pos, len, volume);
            // #endif
            it->_audio_pos += len;
            it->_audio_len -= len;
        }
    }
}

#pragma region AudioData
AudioData::AudioData() {
    _savedpos = 0;
    _id = -10;
    _audio_len = 0;
    _audio_pos = nullptr;
    _volume = SDL_MIX_MAXVOLUME;
    _rate = 14400;
    _original_len = 0;
    _original_pos = nullptr;
    _lengthMS = 0;
    _trackSize = 0;
    _paused = false;
}

long AudioData::Tell() const {
    long pos = (long)(((((Uint64)(_original_len - _audio_len) / 4) * _lengthMS)) / _trackSize);
    return pos;
}

void AudioData::Seek(long ms) {
    if ((((Uint64)ms * _rate * 2 * 2) / 1000) > (Uint64)_original_len) {
        // I am not super sure about this
        spdlog::warn("ID {} Attempt to seek past the end of the loaded audio. Seeking to 0ms instead. Seek to {}ms. Length {}ms.", _id, ms, (long)(((Uint64)_original_len * 1000) / ((Uint64)_rate * 2 * 2)));
        ms = 0;
    }

    _audio_len = (long)((Uint64)_original_len - (((Uint64)ms * _rate * 2 * 2) / 1000));
    _audio_len -= _audio_len % 4;

    if (_audio_len > _original_len) {
        _audio_len = _original_len;
    }

    _audio_pos = _original_pos + (_original_len - _audio_len);

    spdlog::debug("ID {} Seeking to {}MS ... calculated audio_len: {}", _id, ms, _audio_len);
}

void AudioData::SeekAndLimitPlayLength(long ms, long len) {
    _audio_len = (long)(((Uint64)len * _rate * 2 * 2) / 1000);
    _audio_len -= _audio_len % 4;
    _audio_pos = _original_pos + (((Uint64)ms * _rate * 2 * 2) / 1000);

    
    spdlog::debug("ID {} Seeking to {}MS Length {}MS ... calculated audio_len: {}.", _id, ms, len, _audio_len);
}

void AudioData::SavePos() {
    // 
    _savedpos = Tell();
    // spdlog::info("Saving position {} {:X} as {}.", (long)_audio_len, (long)_audio_pos, _savedpos);
}

void AudioData::RestorePos() {
    // 
    Seek(_savedpos);
    // spdlog::info("Restoring position {} as {} {:X}.", _savedpos, (long)_audio_len, (long)_audio_pos);
}

#pragma endregion

#pragma region BaseSDL
BaseSDL::~BaseSDL() {
    CloseDevice();
}

bool BaseSDL::CloseDevice() {
    
    if (_dev > 0) {
        spdlog::debug("Pausing audio device {}.", _dev);
        SDL_ClearError();
        SDL_AudioStatus as = SDL_GetAudioDeviceStatus(_dev);
        wxASSERT(strlen(SDL_GetError()) == 0);
        if (as == SDL_AUDIO_PLAYING) {
            SDL_PauseAudioDevice(_dev, 1);
            wxASSERT(strlen(SDL_GetError()) == 0);
        }
        spdlog::debug("    Result '{}'", SDL_GetError());
        spdlog::debug("Closing audio device {}.", _dev);
        SDL_ClearError();
        SDL_CloseAudioDevice(_dev);
        wxASSERT(strlen(SDL_GetError()) == 0);
        spdlog::debug("    Result '{}'", SDL_GetError());
        _dev = 0;
    }

    return true;
}

void BaseSDL::DumpState(std::string device, int devid, SDL_AudioSpec* wanted, SDL_AudioSpec* actual) const {

    spdlog::debug("Current audio driver {}", SDL_GetCurrentAudioDriver());
    spdlog::debug("Output devices {}. Input devices {}.", SDL_GetNumAudioDevices(0), SDL_GetNumAudioDevices(1));
    spdlog::debug("Audio device '{}' opened {}. Device specification:", device, devid);
    SDL_AudioStatus as = SDL_GetAudioDeviceStatus(devid);
    spdlog::debug("    Audio device status ({}) {}", devid, (as == SDL_AUDIO_PAUSED) ? "Paused" : (as == SDL_AUDIO_PLAYING) ? "Playing"
                                                                                                                                     : "Stopped");
    spdlog::debug("    Size Asked {} Received {}", wanted->size, actual->size);
    spdlog::debug("    Channels Asked {} Received {}", wanted->channels, actual->channels);
    spdlog::debug("    Format Asked {:#x} Received {:#x}", wanted->format, actual->format);
    spdlog::debug("        Bitsize Asked {} Received {}", (int)SDL_AUDIO_BITSIZE(wanted->format), (int)SDL_AUDIO_BITSIZE(actual->format));
    spdlog::debug("        Float Asked {} Received {}", SDL_AUDIO_ISFLOAT(wanted->format) ? "True" : "False", SDL_AUDIO_ISFLOAT(actual->format) ? "True" : "False");
    spdlog::debug("        Big Endian Asked {} Received {}", SDL_AUDIO_ISBIGENDIAN(wanted->format) ? "True" : "False", SDL_AUDIO_ISBIGENDIAN(actual->format) ? "True" : "False");
    spdlog::debug("        Signed Asked {} Received {}", SDL_AUDIO_ISSIGNED(wanted->format) ? "True" : "False", SDL_AUDIO_ISSIGNED(actual->format) ? "True" : "False");
    spdlog::debug("    Frequency Asked {} Received {}", wanted->freq, actual->freq);
    spdlog::debug("    Padding Asked {} Received {}", wanted->padding, actual->padding);
    spdlog::debug("    Samples Asked {} Received {}", wanted->samples, actual->samples);
    spdlog::debug("    Silence Asked {} Received {}", wanted->silence, actual->silence);
}

bool BaseSDL::OpenDevice(bool input, int rate) {
    

    CloseDevice(); // just in case it is open

    _input = input;

    // SDL_AudioSpec
    if (input) {
        _wanted_spec.channels = 1;
        _wanted_spec.samples = SDL_INPUT_BUFFER_SIZE;
        _wanted_spec.callback = nullptr;
        _wanted_spec.userdata = nullptr;
        _wanted_spec.format = AUDIO_U16SYS;
    } else {
        _wanted_spec.channels = 2;
        _wanted_spec.samples = DEFAULT_NUM_SAMPLES;
        _wanted_spec.callback = fill_audio;
        _wanted_spec.userdata = this;
        _wanted_spec.format = AUDIO_S16SYS;
    }
    _wanted_spec.freq = rate;
    _wanted_spec.silence = 0;
    _wanted_spec.size = _wanted_spec.samples * _wanted_spec.channels * 2;

    SDL_AudioSpec actual_spec;
    const char* d = nullptr;
    if (_device != "") {
        d = _device.c_str();
    }
    spdlog::debug("Opening audio device. {}", d);
    SDL_ClearError();
    SDL_AudioDeviceID rc = SDL_OpenAudioDevice(d, input ? 1 : 0, &_wanted_spec, &actual_spec, 0);
    if (_device == "") {
        d = "<Default>";
    }
    spdlog::debug("    Result '{}'", SDL_GetError());
    if (rc < 2) {
        _dev = 0;
        return false;
    }
    _dev = rc;

    _state = SDLSTATE::SDLNOTPLAYING;

    spdlog::debug("Audio device opened {} -> Device: {}.", d, _dev);
    DumpState("", rc, &_wanted_spec, &actual_spec);

    return true;
}
#pragma endregion

#pragma region InputSDL

bool InputSDL::OpenDevice() {
    
    bool rc = BaseSDL::OpenDevice(true, DEFAULT_RATE);

    if (rc) {
        spdlog::debug("Unpausing audio input device {}.", _dev);
        SDL_ClearError();
        SDL_AudioStatus as = SDL_GetAudioDeviceStatus(_dev);
        wxASSERT(strlen(SDL_GetError()) == 0);
        if (as == SDL_AUDIO_PAUSED) {
            SDL_PauseAudioDevice(_dev, 0);
            wxASSERT(strlen(SDL_GetError()) == 0);
        }
        spdlog::debug("    Result '{}'", SDL_GetError());
    }

    return rc;
}

std::list<std::string> InputSDL::GetAudioDevices() {
    std::list<std::string> devices;

#ifdef __WXMSW__
    // TODO we need to this working on OSX/Linux
    // Only windows supports multiple audio devices ... I think .. well at least I know Linux doesn't
    int count = SDL_GetNumAudioDevices(1);
    wxASSERT(strlen(SDL_GetError()) == 0);

    for (int i = 0; i < count; i++) {
        devices.push_back(SDL_GetAudioDeviceName(i, 1));
        wxASSERT(strlen(SDL_GetError()) == 0);
    }
#endif

    return devices;
}

bool InputSDL::IsListening() const {
    return _listeners > 0;
}

void InputSDL::StopListening() {
    
    --_listeners;

    if (_listeners == 0) {
        CloseDevice();
    }

    spdlog::debug("SDL Stopping listening - listeners {}", _listeners);
}

void InputSDL::StartListening() {
    
    _listeners++;

    if (_listeners == 1) {
        if (!OpenDevice()) {
            spdlog::error("Could not open SDL audio input");
        } else {
            spdlog::debug("SDL Starting listening - '{}'", (const char*)_device.c_str());
        }
    }

    spdlog::debug("SDL Starting listening - listeners {}", _listeners);
}

void InputSDL::PurgeAllButInputAudio(int ms) const {
    uint8_t buffer[8192];
    uint32_t bytesNeeded = DEFAULT_RATE * ms / 1000 * 2;

    while (SDL_GetQueuedAudioSize(_dev) > bytesNeeded) {
        wxASSERT(strlen(SDL_GetError()) == 0);
        uint32_t avail = SDL_GetQueuedAudioSize(_dev);
        wxASSERT(strlen(SDL_GetError()) == 0);
        uint32_t toread = std::min(avail - bytesNeeded, (uint32_t)sizeof(buffer));
        uint32_t read = SDL_DequeueAudio(_dev, buffer, toread);
        wxASSERT(strlen(SDL_GetError()) == 0);
        wxASSERT(read == toread);
    }
}

int InputSDL::GetAudio(uint8_t* buffer, int bufsize) {
    int ms = bufsize * 1000 / DEFAULT_RATE;
    PurgeAllButInputAudio(ms);

    SDL_ClearError();
    auto res = SDL_DequeueAudio(_dev, buffer, bufsize);
    wxASSERT(strlen(SDL_GetError()) == 0);
    return res;
}

int InputSDL::GetMax(int ms) const {
    // DumpState("", _inputdev, &_wanted_inputspec, &_wanted_inputspec);

    // Drop any audio less recent that the specified number of milliseconds ... this is necessary to make it responsive
    PurgeAllButInputAudio(ms);

    // grab the audio as an array of unsigned 16 bit values ... we will only look at the MSB
    int samplesNeeded = DEFAULT_RATE * ms / 1000;
    Uint8 buffer[SDL_INPUT_BUFFER_SIZE];
    memset(buffer, 0x00, sizeof(buffer));
    int read = 0;
    SDL_ClearError();
    read = SDL_DequeueAudio(_dev, buffer, sizeof(buffer));
    wxASSERT(strlen(SDL_GetError()) == 0);

    // if we didn't get anything bailout
    if (read == 0) {
        return -1;
    }

    // work out the maximum
    int max = 0;
    for (int i = std::max(0, read - samplesNeeded * 2); i < read - 1; i += 2) {
        if (buffer[i + 1] > max)
            max = buffer[i + 1];
        if (max == 255)
            break;
    }

    // spdlog::debug("samples needed {}, read {}, max {}", samplesNeeded, read / 2, max);

    // return the output scaled from 0-127 to 0-255
    return std::min((max - 127) * 2, 255);
}

std::vector<float> InputSDL::GetSpectrum(int ms) const {
    std::vector<float> res;

    // Drop any audio less recent that the specified number of milliseconds ... this is necessary to make it responsive
    PurgeAllButInputAudio(ms);

    // grab the audio as an array of unsigned 16 bit values ... we will only look at the MSB
    int samplesNeeded = DEFAULT_RATE * ms / 1000;
    Uint8 buffer[SDL_INPUT_BUFFER_SIZE];
    memset(buffer, 0x00, sizeof(buffer));
    int read = 0;
    SDL_ClearError();
    read = SDL_DequeueAudio(_dev, buffer, sizeof(buffer));
    wxASSERT(strlen(SDL_GetError()) == 0);

    // if we didn't get anything bailout
    if (read == 0) {
        return res;
    }

    int n = std::min(read / 2, samplesNeeded);
    float* in = (float*)malloc(n * sizeof(float));
    if (in == nullptr)
        return res;

    int j = 0;
    for (int i = std::max(0, read - samplesNeeded * 2); i < read - 1; i += 2) {
        *(in + j) = (float)(((int)buffer[i + 1] << 8) + (int)buffer[i]) / (float)0xFFF;
        j++;
    }

    // Now do the spectrum analysing
    int outcount = n / 2 + 1;
    kiss_fftr_cfg cfg;
    kiss_fft_cpx* out = (kiss_fft_cpx*)malloc(sizeof(kiss_fft_cpx) * (outcount));
    if (out != nullptr) {
        if ((cfg = kiss_fftr_alloc(outcount, 0 /*is_inverse_fft*/, nullptr, nullptr)) != nullptr) {
            kiss_fftr(cfg, in, out);
            free(cfg);
        }

        for (j = 0; j < 127; j++) {
            // choose the right bucket for this MIDI note
            double freq = 440.0 * exp2f(((double)j - 69.0) / 12.0);
            int start = freq * (double)n / (double)DEFAULT_RATE;
            double freqnext = 440.0 * exp2f(((double)j + 1.0 - 69.0) / 12.0);
            int end = freqnext * (double)n / (double)DEFAULT_RATE;

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
        }

        free(out);
    }

    free(in);

    return res;
}

void InputSDL::PurgeInput() {
    SDL_ClearQueuedAudio(_dev);
    wxASSERT(strlen(SDL_GetError()) == 0);
}
#pragma endregion

#pragma region OutputSDL
void OutputSDL::Reopen() {
    spdlog::debug("SDL Reopen");

    SDLSTATE oldstate = _state;

    if (_state == SDLSTATE::SDLPLAYING)
        Stop();

    std::unique_lock<std::mutex> locker(_audio_Lock);

    for (const auto& it : _audioData) {
        it->SavePos();
    }

    CloseDevice();

    if (!OpenDevice()) {
        // a problem
    } else {
        for (const auto& it : _audioData) {
            it->RestorePos();
        }

        if (oldstate == SDLSTATE::SDLPLAYING) {
            Play();
        }
    }

    spdlog::info("SDL reinitialised.");
}

OutputSDL::OutputSDL(const std::string& device) :
    BaseSDL(device) {
    

    _initialisedRate = DEFAULT_RATE;

    if (!OpenDevice()) {
        spdlog::error("Could not open SDL audio");
        return;
    }

    spdlog::debug("SDL initialized output: '{}'", _device);
}

OutputSDL::~OutputSDL() {
    Stop();

    std::unique_lock<std::mutex> locker(_audio_Lock);

    while (_audioData.size() > 0) {
        auto toremove = _audioData.front();
        _audioData.remove(toremove);
        delete toremove;
    }
}

std::list<std::string> OutputSDL::GetAudioDevices() {
    std::list<std::string> devices;

#ifdef __WXMSW__
    // TODO we need to this working on OSX/Linux
    // Only windows supports multiple audio devices ... I think .. well at least I know Linux doesn't
    int count = SDL_GetNumAudioDevices(0);
    wxASSERT(strlen(SDL_GetError()) == 0);

    for (int i = 0; i < count; i++) {
        devices.push_back(SDL_GetAudioDeviceName(i, 0));
        wxASSERT(strlen(SDL_GetError()) == 0);
    }
#endif

    return devices;
}

bool OutputSDL::OpenDevice() {
    bool res = BaseSDL::OpenDevice(false, _initialisedRate * _playbackrate);

    if (res) {
        Pause();
    }

    return res;
}

std::list<AudioData*> OutputSDL::GetAudio() const {
    return _audioData;
}

long OutputSDL::Tell(int id) {
    std::unique_lock<std::mutex> locker(_audio_Lock);

    auto d = GetData(id);

    if (d == nullptr)
        return 0;

    return d->Tell(); // amount of track size played
}

void OutputSDL::Seek(int id, long pos) {
    std::unique_lock<std::mutex> locker(_audio_Lock);

    auto d = GetData(id);

    if (d == nullptr)
        return;

    d->Seek(pos);
}

[[nodiscard]] std::mutex* OutputSDL::GetAudioLock() {
    return &_audio_Lock;
}

[[nodiscard]] bool OutputSDL::HasAudio(int id) const {
    return GetData(id) != nullptr;
}

// gets the spectrum for the current output frame
std::vector<float> OutputSDL::GetSpectrum(int ms) const {
    std::vector<float> res;

    if (_audioData.size() == 0)
        return res;

    int samplesNeeded = DEFAULT_RATE * ms / 1000;
    if (samplesNeeded % 2 != 0)
        samplesNeeded++;

    Uint8 buffer[SDL_INPUT_BUFFER_SIZE];
    memset(buffer, 0x00, sizeof(buffer));

    // we just work with the first audio track
    auto ad = _audioData.front();

    int read = std::min((int)sizeof(buffer), (int)ad->_audio_len);
    memcpy(buffer, ad->_audio_pos, read);

    int n = std::min(read / 2, samplesNeeded);
    float* in = (float*)malloc(n * sizeof(float));
    if (in == nullptr)
        return res;

    int j = 0;
    for (int i = std::max(0, read - samplesNeeded * 2); i < read - 1; i += 2) {
        *(in + j) = (float)(((int)buffer[i + 1] << 8) + (int)buffer[i]) / (float)0xFFF;
        j++;
    }

    // Now do the spectrum analysing
    int outcount = n / 2 + 1;
    kiss_fftr_cfg cfg;
    kiss_fft_cpx* out = (kiss_fft_cpx*)malloc(sizeof(kiss_fft_cpx) * (outcount));
    if (out != nullptr) {
        if ((cfg = kiss_fftr_alloc(outcount, 0 /*is_inverse_fft*/, nullptr, nullptr)) != nullptr) {
            kiss_fftr(cfg, in, out);
            free(cfg);
        }

        for (j = 0; j < 127; j++) {
            // choose the right bucket for this MIDI note
            double freq = 440.0 * exp2f(((double)j - 69.0) / 12.0);
            int start = freq * (double)n / (double)DEFAULT_RATE;
            double freqnext = 440.0 * exp2f(((double)j + 1.0 - 69.0) / 12.0);
            int end = freqnext * (double)n / (double)DEFAULT_RATE;

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
        }

        free(out);
    }

    free(in);

    return res;
}

void OutputSDL::SeekAndLimitPlayLength(int id, long pos, long len) {
    std::unique_lock<std::mutex> locker(_audio_Lock);

    auto d = GetData(id);

    if (d == nullptr)
        return;

    d->SeekAndLimitPlayLength(pos, len);
}

int OutputSDL::GetVolume(int id) {
    std::unique_lock<std::mutex> locker(_audio_Lock);

    auto d = GetData(id);

    if (d == nullptr)
        return 0;

    return (d->_volume * 100) / SDL_MIX_MAXVOLUME;
}

// volume is 0->100
void OutputSDL::SetVolume(int id, int volume) {
    std::unique_lock<std::mutex> locker(_audio_Lock);

    auto d = GetData(id);

    if (d == nullptr)
        return;

    if (volume > 100) {
        d->_volume = SDL_MIX_MAXVOLUME;
    } else if (volume < 0) {
        d->_volume = 0;
    } else {
        d->_volume = (volume * SDL_MIX_MAXVOLUME) / 100;
    }
}

AudioData* OutputSDL::GetData(int id) const {
    for (const auto& it : _audioData) {
        if (it->_id == id)
            return it;
    }

    return nullptr;
}

int OutputSDL::AddAudio(long len, Uint8* buffer, int volume, int rate, long tracksize, long lengthMS) {

    int id = AudioData::__nextId++;

    AudioData* ad = new AudioData();
    ad->_id = id;
    ad->_audio_len = 0;
    ad->_audio_pos = buffer;
    ad->_rate = rate;
    ad->_original_len = len;
    ad->_original_pos = buffer;
    ad->_lengthMS = lengthMS;
    ad->_trackSize = tracksize;
    ad->_paused = false;

    {
        std::unique_lock<std::mutex> locker(_audio_Lock);
        _audioData.push_back(ad);
    }

    SetVolume(id, volume);

    if (rate != _initialisedRate) {
        if (_audioData.size() != 1) {
            spdlog::warn("Playing multiple audio files with different sample rates with play at least one of them at the wrong speed.");
        }

        _initialisedRate = rate;
        Reopen();
    }

    spdlog::debug("SDL Audio Added: id: {}, device {}, rate: {}, len: {}, lengthMS: {}, trackSize: {}.", id, _dev, rate, len, lengthMS, tracksize);

    return id;
}

void OutputSDL::RemoveAudio(int id) {
    std::unique_lock<std::mutex> locker(_audio_Lock);
    auto toremove = GetData(id);
    if (toremove == nullptr)
        return;
    _audioData.remove(toremove);
    delete toremove;
    spdlog::debug("SDL Audio Removed: id: {} device: {}.", id, _dev);
}

void OutputSDL::Pause(int id, bool pause) {
    spdlog::debug("SDL Audio Pause: id: {}, device {}, pause {}.", id, _dev, pause);
    std::unique_lock<std::mutex> locker(_audio_Lock);
    auto topause = GetData(id);
    if (topause != nullptr)
        topause->Pause(pause);
}

void OutputSDL::SetRate(float rate) {
    if (_playbackrate != rate) {
        _playbackrate = rate;
        Reopen();
    }
}

void OutputSDL::Play() {
    spdlog::debug("SDL Audio Play on device {}", _dev);
    if (_dev == 0) {
        if (!OpenDevice()) {
            return;
        }
    }
    SDL_ClearError();
    SDL_AudioStatus as = SDL_GetAudioDeviceStatus(_dev);
    wxASSERT(strlen(SDL_GetError()) == 0);
    if (as == SDL_AUDIO_PAUSED || as == SDL_AUDIO_STOPPED) {
        SDL_PauseAudioDevice(_dev, 0);
        wxASSERT(strlen(SDL_GetError()) == 0);
    }

    wxASSERT(SDL_GetAudioDeviceStatus(_dev) == SDL_AUDIO_PLAYING);
    _state = SDLSTATE::SDLPLAYING;
}

void OutputSDL::Stop() {
    spdlog::debug("SDL Audio Stop on device {}.", _dev);
    if (_dev > 0) {
        SDL_ClearError();
        _state = SDLSTATE::SDLNOTPLAYING;
        SDL_AudioStatus as = SDL_GetAudioDeviceStatus(_dev);
        wxASSERT(strlen(SDL_GetError()) == 0);
        if (as == SDL_AUDIO_PLAYING) {
            SDL_PauseAudioDevice(_dev, 1);
            wxASSERT(strlen(SDL_GetError()) == 0);
        }
        SDL_ClearQueuedAudio(_dev);
        wxASSERT(strlen(SDL_GetError()) == 0);
    }
}

void OutputSDL::Pause() {
    spdlog::debug("SDL Audio Pause on device {}.", _dev);
    if (_dev > 0) {
        SDL_ClearError();
        SDL_AudioStatus as = SDL_GetAudioDeviceStatus(_dev);
        wxASSERT(strlen(SDL_GetError()) == 0);
        if (as == SDL_AUDIO_PLAYING) {
            SDL_PauseAudioDevice(_dev, 1);
            wxASSERT(strlen(SDL_GetError()) == 0);
        }
        _state = SDLSTATE::SDLNOTPLAYING;
    }
}

void OutputSDL::Unpause() {
    spdlog::debug("SDL Audio Unpause on device {}.", _dev);
    if (_dev == 0) {
        if (!OpenDevice()) {
            return;
        }
    }
    SDL_ClearError();
    SDL_AudioStatus as = SDL_GetAudioDeviceStatus(_dev);
    wxASSERT(strlen(SDL_GetError()) == 0);
    if (as == SDL_AUDIO_PAUSED) {
        SDL_PauseAudioDevice(_dev, 0);
        wxASSERT(strlen(SDL_GetError()) == 0);
    }
    _state = SDLSTATE::SDLPLAYING;
}

void OutputSDL::TogglePause() {
    if (_state == SDLSTATE::SDLPLAYING) {
        Pause();
    } else {
        Unpause();
    }
}
#pragma endregion

#pragma region SDLManager
SDLManager::SDLManager() {

    if (SDL_Init(SDL_INIT_AUDIO)) {
        spdlog::error("Could not initialize SDL");
        return;
    }

#ifndef __WXMSW__
    // Only windows supports multiple audio devices ... I think .. well at least I know Linux doesn't
#else
    // override the default driver on windows so we can access the microphone
    if (SDL_AudioInit("directsound") != 0) {
        spdlog::error("Failed to access DirectSound ... Microphone won't be available.");
    }
#endif
    _initialised = true;
}

SDLManager::~SDLManager() {
    

    _inputs.clear();
    _outputs.clear();

    if (_initialised) {
#ifdef __WXMSW__
        SDL_AudioQuit();
#endif
        SDL_Quit();
    }

    //spdlog::debug("SDL uninitialized");
}

[[nodiscard]] InputSDL* SDLManager::GetInputSDL(const std::string& device) {
    if (!_initialised)
        return nullptr;

    auto d = device;
    if (d == "" || d == "(Default)")
        d = _defaultInput;

    if (_inputs.find(d) != _inputs.end())
        return _inputs[d].get();
    _inputs[d] = std::make_unique<InputSDL>(d);
    return _inputs[d].get();
}

[[nodiscard]] OutputSDL* SDLManager::GetOutputSDL(const std::string& device) {
    if (!_initialised)
        return nullptr;

    auto d = device;
    if (d == "" || d == "(Default)")
        d = _defaultOutput;

    if (_outputs.find(d) != _outputs.end())
        return _outputs[d].get();
    _outputs[d] = std::make_unique<OutputSDL>(d);
    return _outputs[d].get();
}

void SDLManager::SetGlobalVolume(int volume) {
    _globalVolume = volume;
}

[[nodiscard]] int SDLManager::GetGlobalVolume() const {
    return _globalVolume;
}

[[nodiscard]] bool SDLManager::IsNoAudio() const {
    return !_initialised;
}

void SDLManager::SetRate(float rate) {
    for (auto& it : _outputs) {
        it.second.get()->SetRate(rate);
    }
}
#pragma endregion

// Audio Manager Functions

void AudioManager::SetVolume(int volume) const {
    if (__sdlManager.GetOutputSDL(_device) != nullptr)
        __sdlManager.GetOutputSDL(_device)->SetVolume(_sdlid, volume);
}

int AudioManager::GetVolume() const {
    if (__sdlManager.GetOutputSDL(_device) != nullptr)
        return __sdlManager.GetOutputSDL(_device)->GetVolume(_sdlid);

    return 100;
}

int AudioManager::GetGlobalVolume() {
    return __sdlManager.GetGlobalVolume();
}

void AudioManager::SetGlobalVolume(int volume) {
    __sdlManager.SetGlobalVolume(volume);
}

void AudioManager::Seek(long pos) const {
    if (pos < 0 || pos > _lengthMS || !_ok) {
        return;
    }

    if (__sdlManager.GetOutputSDL(_device) != nullptr)
        __sdlManager.GetOutputSDL(_device)->Seek(_sdlid, pos);
}

void AudioManager::Pause() {
    if (__sdlManager.GetOutputSDL(_device) != nullptr) {
        __sdlManager.GetOutputSDL(_device)->Pause(_sdlid, true);
        _media_state = MEDIAPLAYINGSTATE::PAUSED;
    }
}

void AudioManager::Play(long posms, long lenms) {
    auto sdl = __sdlManager.GetOutputSDL(_device);
    if (posms < 0 || posms > _lengthMS || !_ok || sdl == nullptr) {
        return;
    }

    if (!sdl->HasAudio(_sdlid)) {
        _sdlid = sdl->AddAudio(_pcmdatasize, _pcmdata, 100, _rate, _trackSize, _lengthMS);
    }

    sdl->SeekAndLimitPlayLength(_sdlid, posms, lenms);
    sdl->Play();
    sdl->Pause(_sdlid, false);
    _media_state = MEDIAPLAYINGSTATE::PLAYING;
}

bool AudioManager::IsPlaying() const {
    return _media_state == MEDIAPLAYINGSTATE::PLAYING;
}

void AudioManager::Play() {
    auto sdl = __sdlManager.GetOutputSDL(_device);

    if (!_ok || sdl == nullptr)
        return;

    if (!sdl->HasAudio(_sdlid)) {
        _sdlid = sdl->AddAudio(_pcmdatasize, _pcmdata, 100, _rate, _trackSize, _lengthMS);
    }

    sdl->Pause(_sdlid, false);
    sdl->Play();
    _media_state = MEDIAPLAYINGSTATE::PLAYING;
}

void AudioManager::Stop() {
    if (__sdlManager.GetOutputSDL(_device) != nullptr) {
        __sdlManager.GetOutputSDL(_device)->Stop();
        _media_state = MEDIAPLAYINGSTATE::STOPPED;
    }
}

void AudioManager::AbsoluteStop() {
    auto sdl = __sdlManager.GetOutputSDL(_device);

    if (sdl != nullptr) {
        sdl->Stop();
        sdl->RemoveAudio(_sdlid);
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
        wxTheApp->CallAfter([this, ts, oldMediaState]() {
            auto sdl = __sdlManager.GetOutputSDL(_device);
            if (!sdl->HasAudio(_sdlid)) {
                _sdlid = sdl->AddAudio(_pcmdatasize, _pcmdata, 100, _rate, _trackSize, _lengthMS);
            }
            Seek(ts);
            if (oldMediaState == MEDIAPLAYINGSTATE::PLAYING) {
                Play();
            }
        });
    }
}
void AudioManager::SetPlaybackRate(float rate) {
    __sdlManager.SetRate(rate);
}

MEDIAPLAYINGSTATE AudioManager::GetPlayingState() const {
    return _media_state;
}

// return where in the file we are up to playing
long AudioManager::Tell() const {
    if (__sdlManager.GetOutputSDL(_device) != nullptr) {
        return __sdlManager.GetOutputSDL(_device)->Tell(_sdlid);
    }
    return 0;
}

size_t AudioManager::GetAudioFileLength(std::string filename) {
    
    AVFormatContext* formatContext = nullptr;

#if LIBAVFORMAT_VERSION_MAJOR < 58
    av_register_all();
#endif

    int res = avformat_open_input(&formatContext, filename.c_str(), nullptr, nullptr);
    if (res != 0) {
        if (formatContext != nullptr) {
            avformat_close_input(&formatContext);
            formatContext = nullptr;
        }
        return 0;
    }

    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        if (formatContext != nullptr) {
            avformat_close_input(&formatContext);
            formatContext = nullptr;
        }
        return 0;
    }

#if LIBAVCODEC_VERSION_MAJOR >= 59
    const
#endif
        AVCodec* cdc;
    int streamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, &cdc, 0);
    if (streamIndex < 0) {
        spdlog::error("AudioManager: Could not find any audio stream in " + filename);

        if (formatContext != nullptr) {
            avformat_close_input(&formatContext);
            formatContext = nullptr;
        }

        return 0;
    }

    AVStream* audioStream = formatContext->streams[streamIndex];

    if (audioStream != nullptr && audioStream->duration > 0 && audioStream->time_base.den > 0) {
        size_t duration = audioStream->duration * 1000 * audioStream->time_base.num / audioStream->time_base.den;

        if (formatContext != nullptr) {
            avformat_close_input(&formatContext);
            formatContext = nullptr;
        }

        return duration;
    } else if (audioStream != nullptr && audioStream->nb_frames > 0 && audioStream->time_base.den > 0) {
        size_t duration = audioStream->nb_frames * 1000 * audioStream->time_base.num / audioStream->time_base.den;

        if (formatContext != nullptr) {
            avformat_close_input(&formatContext);
            formatContext = nullptr;
        }

        return duration;
    }

    if (formatContext != nullptr) {
        avformat_close_input(&formatContext);
        formatContext = nullptr;
    }

    spdlog::error("AudioManager: Could not determine length of video " + filename);
    return 0;
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
        spdlog::error("Audio file not loaded: {}.", _resultMessage);
    }
    AddAudioDeviceChangeListener([this]() { AudioDeviceChanged(); });
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

void AudioManager::DoPolyphonicTranscription(wxProgressDialog* dlg, AudioManagerProgressCallback fn) {
    // dont redo it
    if (_polyphonicTranscriptionDone) {
        return;
    }

    wxStopWatch sw;

    spdlog::info("DoPolyphonicTranscription: Polyphonic transcription started on file " + _audio_file);

    while (!IsDataLoaded()) {
        spdlog::debug("DoPolyphonicTranscription: Waiting for audio data to load.");
        wxMilliSleep(100);
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
        float const totalLen = len;
        int lastProgress = 0;
        while (len) {
            int const progress = (((float)(totalLen - len) * 25) / totalLen);
            if (lastProgress < progress) {
                fn(dlg, progress);
                lastProgress = progress;
            }
            pdata[0] = GetRawLeftDataPtr(start);
            wxASSERT(pdata[0] != nullptr);
            pdata[1] = GetRawRightDataPtr(start);

            Vamp::RealTime timestamp = Vamp::RealTime::frame2RealTime(start, GetRate());
            Vamp::Plugin::FeatureSet features = pt->process(pdata, timestamp);

            if (first && features.size() > 0) {
                spdlog::warn("DoPolyphonicTranscription: Polyphonic transcription data process oddly retrieved data.");
                first = false;
            }
            if (len > pref_step) {
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
                    fn(dlg, (int)(((float)j * 75.0) / (float)features[0].size()) + 25.0);
                }

                long const currentstart = features[0][j].timestamp.sec * 1000 + features[0][j].timestamp.msec();
                long const currentend = currentstart + features[0][j].duration.sec * 1000 + features[0][j].duration.msec();

                // printf("%f\t%f\t%f\n",(float)currentstart/1000.0, (float)currentend/1000.0, features[0][j].values[0]);
                //if (logger_pianodata.isDebugEnabled()) {
                spdlog::debug("{}.{:03d},{}.{:03d},{},{},{}", features[0][j].timestamp.sec, features[0][j].timestamp.msec(), features[0][j].duration.sec, features[0][j].duration.msec(), currentstart, currentend, features[0][j].values[0]);
                //}
                total += features[0][j].values.size();

                int sframe = currentstart / _intervalMS;
                if (currentstart - sframe * _intervalMS > _intervalMS / 2) {
                    sframe++;
                }
                int const eframe = currentend / _intervalMS;
                while (sframe <= eframe) {
                    _frameData[sframe].notes.push_back(features[0][j].values[0]);
                    sframe++;
                }
            }

            fn(dlg, 100);

            //if (logger_pianodata.isDebugEnabled()) {
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
    spdlog::info("DoPolyphonicTranscription: Polyphonic transcription completed in {}.", sw.Time());
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

    wxStopWatch sw;

    // if we have already done it ... bail
    if (_frameDataPrepared && _frameDataPreparedForInterval == _intervalMS) {
        spdlog::info("DoPrepareFrameData: Aborting processing audio frame data ... it has already been done.");
        return;
    }

    _frameDataPreparedForInterval = _intervalMS;

    // wait for the data to load
    while (!IsDataLoaded()) {
        spdlog::info("DoPrepareFrameData: waiting for audio data to load.");
        wxMilliSleep(1000);
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
    int const samplesperframe = _rate * _intervalMS / 1000;
    int frames = _lengthMS / _intervalMS;
    while (frames * _intervalMS < _lengthMS) {
        frames++;
    }
    int const totalsamples = frames * samplesperframe;

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
        if (pos < i * samplesperframe + samplesperframe && pos + step < totalsamples) {
            spectrogram.clear();
        }

        // only get the data if we are not ahead of the music
        while (pos < i * samplesperframe + samplesperframe && pos + step < totalsamples) {
            pdata[0] = GetRawLeftDataPtr(pos);
            wxASSERT(pdata[0] != nullptr);
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
    spdlog::info("DoPrepareFrameData: Audio frame data processing complete in {}. Frames: {}", sw.Time(), frames);
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

void AudioManager::LoadAudioData(bool separateThread, AVFormatContext* formatContext, AVCodecContext* codecContext, AVStream* audioStream, AVFrame* frame) {
    if (separateThread) {
        _loadingAudio = std::async(std::launch::async, [this, formatContext, codecContext, audioStream, frame]() { DoLoadAudioData(formatContext, codecContext, audioStream, frame); });
    } else {
        DoLoadAudioData(formatContext, codecContext, audioStream, frame);
    }
}

void ProgressFunction(wxProgressDialog* pd, int p) {
    if (pd != nullptr) {
        pd->Update(p);
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
            wxMilliSleep(5);
            lock.lock();
        }
    }
    if (needNotes && !_polyphonicTranscriptionDone) {
        // need to do the polyphonic stuff
        wxProgressDialog dlg("Processing Audio", "");
        DoPolyphonicTranscription(&dlg, ProgressFunction);
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
int AudioManager::decodebitrateindex(int bitrateindex, int version, int layertype) {
    switch (version) {
    case 0: // v2.5
    case 2: // v2
        switch (layertype) {
        case 0:
        default:
            // invalid
            return 0;
        case 1: // L3
        case 2: // L2
            if (bitrateindex == 0 || bitrateindex == 0x0F) {
                return 0;
            } else if (bitrateindex < 8) {
                return 8 * bitrateindex;
            } else {
                return 64 + (bitrateindex - 8) * 16;
            }
        case 3: // L1
            if (bitrateindex == 0 || bitrateindex == 0x0F) {
                return 0;
            } else {
                return 16 + bitrateindex * 16;
            }
        }
    case 3: // v1
        switch (layertype) {
        case 0:
        default:
            // invalid
            return 0;
        case 1: // L3
            if (bitrateindex == 0 || bitrateindex == 0x0F) {
                return 0;
            } else if (bitrateindex < 6) {
                return 32 + (bitrateindex - 1) * 8;
            } else if (bitrateindex < 9) {
                return 64 + (bitrateindex - 6) * 16;
            } else if (bitrateindex < 14) {
                return 128 + (bitrateindex - 9) * 32;
            } else {
                return 320;
            }
        case 2: // L2
            if (bitrateindex == 0 || bitrateindex == 0x0F) {
                return 0;
            } else if (bitrateindex < 3) {
                return 32 + (bitrateindex - 1) * 16;
            } else if (bitrateindex < 5) {
                return 56 + (bitrateindex - 3) * 8;
            } else if (bitrateindex < 9) {
                return 80 + (bitrateindex - 5) * 16;
            } else if (bitrateindex < 13) {
                return 160 + (bitrateindex - 9) * 32;
            } else {
                return 320 + (bitrateindex - 13) * 64;
            }
        case 3: // L1
            if (bitrateindex == 0 || bitrateindex == 0x0F) {
                return 0;
            } else {
                return bitrateindex * 32;
            }
        }
    case 1:
    default:
        // invalid
        return 0;
    }
}

// Decode samplerate
int AudioManager::decodesamplerateindex(int samplerateindex, int version) const {
    switch (version) {
    case 0: // v2.5
        switch (samplerateindex) {
        case 0:
            return 11025;
        case 1:
            return 12000;
        case 2:
            return 8000;
        case 3:
        default:
            return 0;
        }
    case 2: // v2
        switch (samplerateindex) {
        case 0:
            return 22050;
        case 1:
            return 24000;
        case 2:
            return 16000;
        case 3:
        default:
            return 0;
        }
    case 3: // v1
        switch (samplerateindex) {
        case 0:
            return 44100;
        case 1:
            return 48000;
        case 2:
            return 32000;
        case 3:
        default:
            return 0;
        }
    case 1:
    default:
        // invalid
        return 0;
    }
}

// Decode side info
int AudioManager::decodesideinfosize(int version, int mono) {
    if (version == 3) // v1
    {
        if (mono == 3) // mono
        {
            return 17;
        } else {
            return 32;
        }
    } else {
        if (mono == 3) // mono
        {
            return 9;
        } else {
            return 17;
        }
    }
}

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
    RemoveAudioDeviceChangeListener();

    while (IsOk() && !IsDataLoaded()) {
        spdlog::debug("~AudioManager waiting for audio data to complete loading before destroying it.");
        wxMilliSleep(100);
    }

    // wait for async tasks to finish
    if (_loadingAudio.valid()) {
        _loadingAudio.wait();
    }
    if (_prepFrameData.valid())
        _prepFrameData.wait();

    if (_pcmdata != nullptr) {
        auto sdl = __sdlManager.GetOutputSDL(_device);
        if (sdl != nullptr) {
            sdl->Stop();
            sdl->RemoveAudio(_sdlid);
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
void AudioManager::SplitTrackDataAndNormalize(signed short* trackData, long trackSize, float* leftData, float* rightData) const {
    for (size_t i = 0; i < trackSize; i++) {
        float lSample = trackData[i * 2];
        leftData[i] = lSample / 32768.0f;
        float rSample = trackData[(i * 2) + 1];
        rightData[i] = rSample / 32768.0f;
    }
}

// NOrmalise mono track data
void AudioManager::NormalizeMonoTrackData(signed short* trackData, long trackSize, float* leftData) {
    for (size_t i = 0; i < trackSize; i++) {
        signed short lSample = trackData[i];
        leftData[i] = (float)lSample / (float)32768;
    }
}

// Calculate the song length in MS
long AudioManager::CalcLengthMS() const {
    float seconds = (float)_trackSize * (1.0f / (float)_rate);
    return (long)(seconds * 1000.0f);
}

// Open and read the media file into memory
int AudioManager::OpenMediaFile() {
    
    int err = 0;

    if (_pcmdata != nullptr) {
        auto sdl = __sdlManager.GetOutputSDL(_device);
        if (sdl != nullptr) {
            sdl->Stop();
            sdl->RemoveAudio(_sdlid);
        }
        _sdlid = -1;
        free(_pcmdata);
        _pcmdata = nullptr;
    }

// Initialize FFmpeg codecs
#if LIBAVFORMAT_VERSION_MAJOR < 58
    av_register_all();
#endif

    AVFormatContext* formatContext = nullptr;
    int res = avformat_open_input(&formatContext, ToUTF8(_audio_file).c_str(), nullptr, nullptr);
    if (res != 0) {
        spdlog::error("avformat_open_input Error opening the file {} => {}.", _audio_file, res);
        _ok = false;
        wxASSERT(false);
        return 1;
    }

    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        avformat_close_input(&formatContext);
        formatContext = nullptr;
        spdlog::error("avformat_find_stream_info Error finding the stream info {}.", _audio_file);
        _ok = false;
        wxASSERT(false);
        return 1;
    }

    // Find the audio stream
#if LIBAVCODEC_VERSION_MAJOR >= 59
    const
#endif
        AVCodec* cdc = nullptr;
    int streamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, &cdc, 0);
    if (streamIndex < 0) {
        avformat_close_input(&formatContext);
        formatContext = nullptr;
        spdlog::error("av_find_best_stream Could not find any audio stream in the file {}.", _audio_file);
        _ok = false;
        wxASSERT(false);
        return 1;
    }

    AVStream* audioStream = formatContext->streams[streamIndex];
    if (audioStream == nullptr) {
        avformat_close_input(&formatContext);
        formatContext = nullptr;
        spdlog::error("formatContext->streams[{}] was nullptr.", streamIndex);
        _ok = false;
        wxASSERT(false);
        return 1;
    } else if (audioStream->codecpar == nullptr) {
        avformat_close_input(&formatContext);
        formatContext = nullptr;
        spdlog::error("formatContext->codecpar was nullptr.");
        _ok = false;
        wxASSERT(false);
        return 1;
    }

    AVCodecContext* codecContext = avcodec_alloc_context3(cdc);
    wxASSERT(codecContext != nullptr);

    avcodec_parameters_to_context(codecContext, audioStream->codecpar);

    if (avcodec_open2(codecContext, cdc, nullptr) < 0) {
        avformat_close_input(&formatContext);
        formatContext = nullptr;
        spdlog::error("avcodec_open2 Couldn't open the context with the decoder {}.", _audio_file);
        _ok = false;
        wxASSERT(false);
        return 1;
    }

#if LIBAVFORMAT_VERSION_MAJOR < 59
    _channels = codecContext->channels;
    #else
    _channels = codecContext->ch_layout.nb_channels;
    #endif

    wxASSERT(_channels > 0);

#ifdef RESAMPLE_RATE
    _rate = RESAMPLE_RATE;
#else
    _rate = codecContext->sample_rate;
#endif
    wxASSERT(_rate > 0);

    _sampleRate = codecContext->sample_rate;
    _bitRate = codecContext->bit_rate;

    _bits = av_get_bytes_per_sample(codecContext->sample_fmt);
    wxASSERT(_bits > 0);

    /* Get Track Size */
    GetTrackMetrics(formatContext, codecContext, audioStream);

    if (!_ok) {
        avformat_close_input(&formatContext);
        formatContext = nullptr;
        wxASSERT(false);
        return 1;
    }

    // Check if we have read this before ... if so dump the old data
    if (_data[1] != nullptr && _data[1] != _data[0]) {
        free(_data[1]);
        _data[1] = nullptr;
    }
    if (_data[0] != nullptr) {
        free(_data[0]);
        _data[0] = nullptr;
    }
    _loadedData = 0;

    long size = sizeof(float) * (_trackSize + _extra);
    _data[0] = (float*)calloc(size, 1);

    if (_data[0] == nullptr) {
        avformat_close_input(&formatContext);
        formatContext = nullptr;
        spdlog::error("Unable to allocate {} memory to load audio file {}.", size, _audio_file);
        _ok = false;
        wxASSERT(false);
        return 1;
    }

    memset(_data[0], 0x00, size);
    if (_channels == 2) {
        _data[1] = (float*)calloc(size, 1);
        if (_data[1] == nullptr) {
            avformat_close_input(&formatContext);
            formatContext = nullptr;
            spdlog::error("Unable to allocate {} memory to load audio file {}.", size, _audio_file);
            _ok = false;
            wxASSERT(false);
            return 1;
        }
        memset(_data[1], 0x00, size);
    } else {
        _data[1] = _data[0];
    }

    LoadTrackData(formatContext, codecContext, audioStream);

    if (!_ok) {
        avformat_close_input(&formatContext);
        formatContext = nullptr;
        wxASSERT(false);
        return 1;
    }

    // only initialise if we successfully got data
    if (_pcmdata != nullptr) {
        // long total_len = (_lengthMS * _rate * 2 * 2) / 1000;
        // total_len -= total_len % 4;
        auto sdl = __sdlManager.GetOutputSDL(_device);
        if (sdl != nullptr) {
            _sdlid = sdl->AddAudio(_pcmdatasize, _pcmdata, 100, _rate, _trackSize, _lengthMS);
        }
    }

    return err;
}

void AudioManager::LoadTrackData(AVFormatContext* formatContext, AVCodecContext* codecContext, AVStream* audioStream) {
    

    spdlog::debug("Preparing to load song data.");

    // setup our conversion format ... we need to convert the input to a standard format before we can process anything
    int out_channels = 2;

    AVFrame* frame = av_frame_alloc();
    if (frame == nullptr) {
        spdlog::error("av_frame_alloc ... error allocating frame.");
        _resultMessage = "Error allocating the frame";
        _state = 0;
        _ok = false;
        return;
    }

    _pcmdatasize = _trackSize * out_channels * 2;
    _pcmdata = (Uint8*)calloc(_pcmdatasize + PCMFUDGE, 1); // PCMFUDGE is a fudge because some ogg files dont read consistently
    if (_pcmdata == nullptr) {
        _pcmdatasize = 0;
        spdlog::error("Error allocating memory for pcm data: {}", (long)_pcmdatasize + PCMFUDGE);
        _ok = false;
        av_frame_free(&frame);
        return;
    }

    ExtractMP3Tags(formatContext);

    LoadAudioData(true, formatContext, codecContext, audioStream, frame);
}

void AudioManager::DoLoadAudioData(AVFormatContext* formatContext, AVCodecContext* codecContext, AVStream* audioStream, AVFrame* frame) {
    if (formatContext == nullptr || codecContext == nullptr || audioStream == nullptr || frame == nullptr) {
        av_frame_free(&frame);
        return;
    }

    
    spdlog::debug("DoLoadAudioData: Doing load of song data.");

    wxStopWatch sw;

    long read = 0;
    int lastpct = 0;
    int status;

    // setup our conversion format ... we need to convert the input to a standard format before we can process anything
    int out_channels = 2;

    AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    int out_sample_rate = _rate;
    
    AVDictionaryEntry *tag = nullptr;
    while ((tag = av_dict_get(formatContext->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
        _metaData[tag->key] = tag->value;
    }

    AVPacket* readingPacket = av_packet_alloc();
    // av_init_packet( readingPacket );

#define CONVERSION_BUFFER_SIZE 192000
    uint8_t* out_buffer = (uint8_t*)av_malloc(CONVERSION_BUFFER_SIZE * out_channels * 2); // 1 second of audio

#if LIBAVFORMAT_VERSION_MAJOR < 59
    uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
    int64_t in_channel_layout = av_get_default_channel_layout(codecContext->channels);
    struct SwrContext* au_convert_ctx = swr_alloc_set_opts(nullptr, out_channel_layout, out_sample_fmt, out_sample_rate,
                                                           in_channel_layout, codecContext->sample_fmt, codecContext->sample_rate, 0, nullptr);
    #else
    AVChannelLayout out_channel_layout;
    av_channel_layout_default(&out_channel_layout, out_channels);
    AVChannelLayout in_channel_layout;
    av_channel_layout_default(&in_channel_layout, codecContext->ch_layout.nb_channels);
    struct SwrContext* au_convert_ctx = nullptr;
    swr_alloc_set_opts2(&au_convert_ctx, &out_channel_layout, out_sample_fmt, out_sample_rate, &in_channel_layout, codecContext->sample_fmt, codecContext->sample_rate, 0, nullptr);
    #endif

    if (au_convert_ctx == nullptr) {
        spdlog::error("DoLoadAudioData: swe_alloc_set_opts was null");
        // let it go as it may be the cause of a crash
        wxASSERT(false);
    }

    swr_init(au_convert_ctx);

    // start at the beginning
    av_seek_frame(formatContext, 0, 0, AVSEEK_FLAG_ANY);

    // Read the packets in a loop
    while ((status = av_read_frame(formatContext, readingPacket)) == 0) {
        if (readingPacket->stream_index == audioStream->index)
            LoadAudioFromFrame(formatContext, codecContext, readingPacket, frame, au_convert_ctx,
                               false, out_channels, out_buffer, read, lastpct);

        // You *must* call av_free_packet() after each call to av_read_frame() or else you'll leak memory
        av_packet_unref(readingPacket);
    }

    if (status == AVERROR_EOF && readingPacket->stream_index == audioStream->index)
        LoadAudioFromFrame(formatContext, codecContext, readingPacket, frame, au_convert_ctx,
                           true, out_channels, out_buffer, read, lastpct);

    // Some codecs will cause frames to be buffered up in the decoding process. If the CODEC_CAP_DELAY flag
    // is set, there can be buffered up frames that need to be flushed, so we'll do that
    if (codecContext->codec != nullptr && (codecContext->codec->capabilities & CODEC_CAP_DELAY) != 0) {
        // Decode all the remaining frames in the buffer, until the end is reached
        while ((status = av_read_frame(formatContext, readingPacket)) == 0) {
            if (readingPacket->stream_index == audioStream->index)
                LoadAudioFromFrame(formatContext, codecContext, readingPacket, frame, au_convert_ctx,
                                   false, out_channels, out_buffer, read, lastpct);

            av_packet_unref(readingPacket);
        }

        if (status == AVERROR_EOF && readingPacket->stream_index == audioStream->index)
            LoadAudioFromFrame(formatContext, codecContext, readingPacket, frame, au_convert_ctx,
                               true, out_channels, out_buffer, read, lastpct);
    }

    int numDrained = swr_convert(au_convert_ctx, &out_buffer, CONVERSION_BUFFER_SIZE, nullptr, 0);
    if (numDrained > 0)
        LoadResampledAudio(numDrained, out_channels, out_buffer, read, lastpct);

#ifdef RESAMPLE_RATE
    {
        std::unique_lock<std::shared_timed_mutex> locker(_mutexAudioLoad);
        if (_trackSize < _loadedData) {
            // loaded more than we anticipated consuming some of the extra space
            _extra -= (_loadedData - _trackSize);
        }
        _trackSize = _loadedData;
    }
#endif
    wxASSERT(_trackSize == _loadedData);

    // Clean up!
    swr_free(&au_convert_ctx);
    av_free(out_buffer);
    av_packet_free(&readingPacket);
    av_frame_free(&frame);

    avformat_close_input(&formatContext);

    spdlog::debug("DoLoadAudioData: Song data loaded in {}. Read: {}", sw.Time(), read);
}

void AudioManager::LoadAudioFromFrame(AVFormatContext* formatContext, AVCodecContext* codecContext, AVPacket* decodingPacket, AVFrame* frame, SwrContext* au_convert_ctx, bool receivedEOF, int out_channels, uint8_t* out_buffer, long& read, int& lastpct) {
    int status = avcodec_send_packet(codecContext, receivedEOF ? nullptr : decodingPacket);
    if (status == 0) {
        do {
            status = avcodec_receive_frame(codecContext, frame);
            if (status == AVERROR_EOF)
                break;
            if (status == 0)
                LoadDecodedAudioFromFrame(frame, formatContext, au_convert_ctx, out_channels, out_buffer, read, lastpct);
        } while (status != AVERROR(EAGAIN));
    }
}

void AudioManager::LoadDecodedAudioFromFrame(AVFrame* frame, AVFormatContext* formatContext, SwrContext* au_convert_ctx, int out_channels, uint8_t* out_buffer, long& read, int& lastpct) {
    
    int outSamples = 0;

    try {
        if (*(frame->data) == nullptr) {
            spdlog::error("LoadDecodedAudioFromFrame: frame->data was a pointer to a nullptr.");
            // let this go maybe it causes the crash
            wxASSERT(false);
        }
        if (frame->nb_samples == 0) {
            spdlog::error("LoadDecodedAudioFromFrame: frame->nb_samples was 0.");
            // let this go maybe it causes the crash
            wxASSERT(false);
        }

        outSamples = swr_convert(au_convert_ctx, &out_buffer, CONVERSION_BUFFER_SIZE, (const uint8_t**)frame->data, frame->nb_samples);
    } catch (...) {
        spdlog::error("LoadDecodedAudioFromFrame: swr_convert threw an exception.");
        wxASSERT(false);
        std::unique_lock<std::shared_timed_mutex> locker(_mutexAudioLoad);
        _trackSize = _loadedData; // makes it looks like we are done
        return;
    }

    LoadResampledAudio(outSamples, out_channels, out_buffer, read, lastpct);
}

void AudioManager::LoadResampledAudio(int sampleCount, int out_channels, uint8_t* out_buffer, long& read, int& lastpct) {
    

    if (read + sampleCount > _trackSize) {
        // I dont understand why this happens ... add logging when i can
        // I have seen this happen with a wma file ... but i dont know why
        spdlog::warn(std::string("LoadResampledAudio: This shouldnt happen ... read [" + wxString::Format("%li", (long)read) + "] + nb_samples [" + wxString::Format("%i", sampleCount) + "] > _tracksize [" + wxString::Format("%li", (long)_trackSize) + "] ."));

        // we've consumed some of the "extra" space, make sure we reduce that
        _extra -= (read + sampleCount - _trackSize);
        // override the track size
        _trackSize = read + sampleCount;
    }

    // copy the PCM data into the PCM buffer for playing
    wxASSERT(_pcmdatasize + PCMFUDGE > read * out_channels * sizeof(uint16_t) + sampleCount * out_channels * sizeof(uint16_t));
    memcpy(_pcmdata + (read * out_channels * sizeof(uint16_t)), out_buffer, sampleCount * out_channels * sizeof(uint16_t));

    // possible optimization here... we ask resampler for S16 data and then convert that to floating-point?
    for (int i = 0; i < sampleCount; i++) {
        int16_t s = *(int16_t*)(out_buffer + i * sizeof(int16_t) * out_channels);
        _data[0][read + i] = ((float)s) / (float)0x8000;
        if (_channels > 1) {
            s = *(int16_t*)(out_buffer + i * sizeof(int16_t) * out_channels + sizeof(int16_t));
            _data[1][read + i] = ((float)s) / (float)0x8000;
        }
    }
    read += sampleCount;
    SetLoadedData(read);
    int progress = read * 100 / _trackSize;
    if (progress >= lastpct + 10) {
        // spdlog::debug("DoLoadAudioData: Progress {}", progress);
        lastpct = progress / 10 * 10;
    }
}

SDLManager* AudioManager::GetSDLManager() {
    return &__sdlManager;
}

void AudioManager::GetTrackMetrics(AVFormatContext* formatContext, AVCodecContext* codecContext, AVStream* audioStream) {
    spdlog::debug("Getting track metrics.");

    _trackSize = 0;

    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        spdlog::error("av_frame_alloc ... error allocating frame.");
        _resultMessage = "Error allocating the frame";
        _state = 0;
        _ok = false;
        return;
    } else
        spdlog::info("av_frame_alloc okay");

    AVPacket* readingPacket = av_packet_alloc();
    // av_init_packet( readingPacket );

    // start at the beginning
    av_seek_frame(formatContext, 0, 0, AVSEEK_FLAG_ANY);

    // Read the packets in a loop
    while (av_read_frame(formatContext, readingPacket) == 0) {
        if (readingPacket->stream_index == audioStream->index) {
            int status = avcodec_send_packet(codecContext, readingPacket);
            if (status == 0) {
                do {
                    status = avcodec_receive_frame(codecContext, frame);
                    if (status == AVERROR_EOF)
                        break;
                    _trackSize += frame->nb_samples;
                } while (status != AVERROR(EAGAIN));
            }
        }

        // You *must* call av_free_packet() after each call to av_read_frame() or else you'll leak memory
        av_packet_unref(readingPacket);
    }

    av_packet_free(&readingPacket);

    // Clean up!
    av_frame_free(&frame);

    double lengthInSeconds = double(_trackSize) / codecContext->sample_rate;
    _lengthMS = long(floor(lengthInSeconds * 1000));

#ifdef RESAMPLE_RATE
    // if we resample, we need to estimate the new size
    float f = _trackSize;
    f *= RESAMPLE_RATE;
    f /= codecContext->sample_rate;
    _trackSize = f;
    _extra += 2048; // add some extra space just in case the estimate is not accurate
#endif

    spdlog::info("    Track Size: {}, Time Base Den: {} => Length {}ms", _trackSize, codecContext->time_base.den, _lengthMS);
}

void AudioManager::ExtractMP3Tags(AVFormatContext* formatContext) {
    AVDictionaryEntry* tag = av_dict_get(formatContext->metadata, "title", nullptr, 0);
    if (tag != nullptr) {
        _title = tag->value;
    }
    tag = av_dict_get(formatContext->metadata, "album", nullptr, 0);
    if (tag != nullptr) {
        _album = tag->value;
    }
    tag = av_dict_get(formatContext->metadata, "artist", nullptr, 0);
    if (tag != nullptr) {
        _artist = tag->value;
    }
}

// Access a single piece of track data
float AudioManager::GetFilteredLeftData(long offset) {
    
    while (!IsDataLoaded(offset)) {
        spdlog::debug("GetLeftData waiting for data to be loaded.");
        wxMilliSleep(100);
    }

    if (_data[0] == nullptr || offset > _trackSize) {
        return 0;
    }
    return _data[0][offset];
}

float AudioManager::GetRawLeftData(long offset) {
    
    while (!IsDataLoaded(offset)) {
        spdlog::debug("GetLeftData waiting for data to be loaded.");
        wxMilliSleep(100);
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
    return wxString::Format("%s%d", notes[offset], octave).ToStdString();
}

void AudioManager::NormaliseFilteredAudioData(FilteredAudioData* fad) {
    // PCM Data is the displayed waveform
    int16_t* pcm = fad->pcmdata;
    int min = 32000;
    int max = -32000;
    for (int i = 0; i < (_pcmdatasize) / sizeof(int16_t); i++) {
        if (*pcm > max)
            max = *pcm;
        if (*pcm < min)
            min = *pcm;
        pcm++;
    }

    double scale = 1.0;
    double mm = (double)std::max(max, abs(min));
    wxASSERT(mm <= 32768.0);
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
    for (int i = 0; i < (_pcmdatasize) / sizeof(int16_t); i++) {
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
    wxASSERT(mm <= 32767.0);
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
        wxMilliSleep(50);
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
        wxMilliSleep(100);
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
        wxMilliSleep(100);
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
        wxMilliSleep(100);
    }

    if (_data[1] == nullptr || offset > _trackSize) {
        return 0;
    }
    return _data[1][offset];
}

float AudioManager::GetRawRightData(long offset) {
    
    while (!IsDataLoaded(offset)) {
        spdlog::debug("GetRightData waiting for data to be loaded.");
        wxMilliSleep(100);
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
        wxMilliSleep(100);
    }

    wxASSERT(_data[0] != nullptr);
    if (offset > _trackSize) {
        return nullptr;
    }
    return &_data[0][offset];
}

float* AudioManager::GetRawLeftDataPtr(long offset) {
    
    while (!IsDataLoaded(offset)) {
        spdlog::debug("GetLeftDataPtr waiting for data to be loaded.");
        wxMilliSleep(100);
    }

    FilteredAudioData* fad = GetFilteredAudioData(AUDIOSAMPLETYPE::RAW, -1, -1);

    if (fad != nullptr && offset <= _trackSize)
        return &fad->data0[offset];
    return nullptr;
}

float* AudioManager::GetRawRightDataPtr(long offset) {
    
    while (!IsDataLoaded(offset)) {
        spdlog::debug("GetRightDataPtr waiting for data to be loaded.");
        wxMilliSleep(100);
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
        wxMilliSleep(100);
    }

    wxASSERT(_data[1] != nullptr);
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
            wxMilliSleep(100);
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

            std::string name = std::string(wxString::FromUTF8(it->getName().c_str()).c_str());

            if (outputs.size() > 1) {
                // This is not the plugin's only output.
                // Use "plugin name: output name" as the effect name,
                // unless the output name is the same as the plugin name
                std::string outputName = std::string(wxString::FromUTF8(j.name.c_str()).c_str());
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
            std::string name = std::string(wxString::FromUTF8(it->getName().c_str()).c_str());

            if (outputs.size() > 1) {
                // This is not the plugin's only output.
                // Use "plugin name: output name" as the effect name,
                // unless the output name is the same as the plugin name
                std::string outputName = std::string(wxString::FromUTF8(j.name.c_str()).c_str());
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
    return OutputSDL::GetAudioDevices();
}

std::list<std::string> AudioManager::GetInputAudioDevices() {
    return InputSDL::GetAudioDevices();
}

bool AudioManager::WriteCurrentAudio(const std::string& path, long bitrate) {
    

    spdlog::info("AudioManager::WriteCurrentAudio() - {} samples to export to '{}'", _trackSize, path.c_str());

    std::vector<float> leftData(_data[0], _data[0] + _trackSize);
    std::vector<float> rightData(_data[1], _data[1] + _trackSize);
    return EncodeAudio(leftData,
                       rightData,
                       bitrate,
                       path);
}

// #define AUDIOWRITE_DEBUG

#ifdef AUDIOWRITE_DEBUG

std::string DecodeAVLevel(int level) {
    switch (level) {
    case AV_LOG_QUIET:
        return "QUIET";
    case AV_LOG_PANIC:
        return "PANIC";
    case AV_LOG_FATAL:
        return "FATAL";
    case AV_LOG_ERROR:
        return "ERROR";
    case AV_LOG_WARNING:
        return "WARNING";
    case AV_LOG_INFO:
        return "INFO";
    case AV_LOG_VERBOSE:
        return "VERBOSE";
    case AV_LOG_DEBUG:
        return "DEBUG";
    case AV_LOG_TRACE:
        return "TRACE";
    default:
        return "UNKNOWN";
    }
}

void my_av_log_callback_am(void* ptr, int level, const char* fmt, va_list vargs) {
    static char message[8192] = { 0 };

    // Create the actual message
    vsnprintf(message, sizeof(message), fmt, vargs);

    // strip off carriage return
    if (strlen(message) > 0) {
        if (message[strlen(message) - 1] == '\n') {
            message[strlen(message) - 1] = 0x00;
        }
    }

    
    spdlog::debug("WriteAudioFile: lvl: {} ({}) msg: {}.", (const char*)DecodeAVLevel(level).c_str(), level, static_cast<const char*>(message));
}
#endif

bool AudioManager::EncodeAudio(const std::vector<float>& left_channel,
                               const std::vector<float>& right_channel,
                               size_t sampleRate,
                               const std::string& filename,
                               AudioManager *copyFrom) {
    spdlog::debug("Writing {} samples to {} at sample rate {}", left_channel.size(), (const char*)filename.c_str(), sampleRate);

    if (left_channel.size() != right_channel.size()) {
        spdlog::error("Left and right channel sizes do not match");
        return false;
    }

#if LIBAVFORMAT_VERSION_MAJOR < 58
    avcodec_register_all();
    av_register_all();
#endif

#ifdef AUDIOWRITE_DEBUG
    av_log_set_level(AV_LOG_TRACE);
    av_log_set_callback(my_av_log_callback_am);
#endif

    // Find the MP3 encoder
#ifdef AUDIOWRITE_DEBUG
    spdlog::debug("avcodec_find_encoder");
#endif
    
    AVCodecID codecId = EndsWith(filename, "m4a") ? AV_CODEC_ID_AAC : AV_CODEC_ID_MP3;
    const AVCodec* codec = codecId == AV_CODEC_ID_AAC ? avcodec_find_encoder_by_name("aac_at") : nullptr;
    if (!codec) {
        codec = avcodec_find_encoder(codecId);
    }
    if (!codec) {
        codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    }
    if (!codec) {
        spdlog::error("MP3/MP2/AAC encoder not found");
#ifdef AUDIOWRITE_DEBUG
        av_log_set_callback(nullptr);
#endif
        return false;
    }

    // Allocate the output media context
    AVFormatContext* format_context = nullptr;
#ifdef AUDIOWRITE_DEBUG
    spdlog::debug("avformat_alloc_output_context2");
#endif
    avformat_alloc_output_context2(&format_context, nullptr, nullptr, filename.c_str());
    if (!format_context) {
        spdlog::error("Could not deduce output format from file extension");
#ifdef AUDIOWRITE_DEBUG
        av_log_set_callback(nullptr);
#endif
        return false;
    }
    
    if (copyFrom) {
        for (auto &tag : copyFrom->GetMetaData()) {
            av_dict_set(&format_context->metadata, tag.first.c_str(), tag.second.c_str(), 0);
        };
    }

    // Create a new audio stream in the output file container
    // AVStream* audio_stream = avformat_new_stream(format_context, nullptr);
#ifdef AUDIOWRITE_DEBUG
    spdlog::debug("avformat_new_stream");
#endif
    AVStream* audio_stream = avformat_new_stream(format_context, codec);
    if (!audio_stream) {
        avformat_free_context(format_context);
        spdlog::error("Could not create new audio stream");
#ifdef AUDIOWRITE_DEBUG
        av_log_set_callback(nullptr);
#endif
        return false;
    }

#ifdef AUDIOWRITE_DEBUG
    spdlog::debug("avcodec_alloc_context3");
#endif
    AVCodecContext* codec_context = avcodec_alloc_context3(codec);
    if (!codec_context) {
        avformat_free_context(format_context);
        spdlog::error("Could not allocate an encoding context");
#ifdef AUDIOWRITE_DEBUG
        av_log_set_callback(nullptr);
#endif
        return false;
    }

    // Check the sample rate is ok
    if (codec->supported_samplerates) {
        const int* p = codec->supported_samplerates;
        bool rateOK = false;
        while (*p != 0) {
            spdlog::debug("    Encoder supports sample rate {}", *p);
            if (*p == sampleRate) {
                rateOK = true;
                break;
            }
            p++;
        }
        if (!rateOK) {
            spdlog::error("Encoder does not support sample rate {}", sampleRate);
            avformat_free_context(format_context);
#ifdef AUDIOWRITE_DEBUG
            av_log_set_callback(nullptr);
#endif
            return false;
        }
    }

    codec_context->sample_fmt = AV_SAMPLE_FMT_FLTP; // Planar float format
    codec_context->sample_rate = sampleRate;
    if (copyFrom && copyFrom->GetBitRate() > 0) {
        codec_context->bit_rate = copyFrom->GetBitRate();
    }

#if LIBAVFORMAT_VERSION_MAJOR < 59
    codec_context->channel_layout = AV_CH_LAYOUT_STEREO;
    codec_context->channels = 2;
#else
    AVChannelLayout ch_layout;
    av_channel_layout_default(&ch_layout, 2);

    // Set the channel layout in the codec context
    av_channel_layout_copy(&codec_context->ch_layout, &ch_layout);

    // Free the channel layout when done
    av_channel_layout_uninit(&ch_layout);

    codec_context->ch_layout.nb_channels = 2;
#endif

    std::vector<int16_t> s16Data;
    // Check the format is ok
    {
        const enum AVSampleFormat* p = codec->sample_fmts;
        bool fmtOK = false;
        bool hasS16 = false;
        while (*p != AV_SAMPLE_FMT_NONE) {
            spdlog::debug("    Encoder supports sample format {}", av_get_sample_fmt_name(*p));
            if (*p == codec_context->sample_fmt) {
                fmtOK = true;
                break;
            }
            if (*p == AV_SAMPLE_FMT_S16) {
                hasS16 = true;
            }
            p++;
        }
        if (!fmtOK && hasS16) {
            //audiotoolbox aac encoder on Mac only supports S16 fmt
            s16Data.resize(left_channel.size() * 2);
            for (int x = 0; x < left_channel.size(); x++) {
                float lsample = std::clamp(left_channel[x], -1.0f, 1.0f);
                float rsample = std::clamp(right_channel[x], -1.0f, 1.0f);
                s16Data[x * 2] = (int16_t)round(lsample * 32767.0f);
                s16Data[x * 2 + 1] = (int16_t)round(rsample * 32767.0f);
            }
            codec_context->sample_fmt = AV_SAMPLE_FMT_S16;
            fmtOK = true;
        }
        if (!fmtOK) {
            spdlog::error("Encoder does not support sample format {}",
                              av_get_sample_fmt_name(codec_context->sample_fmt));
            avformat_free_context(format_context);
#ifdef AUDIOWRITE_DEBUG
            av_log_set_callback(nullptr);
#endif
            return false;
        }
    }

    audio_stream->time_base = AVRational{ 1, codec_context->sample_rate };

    if (format_context->oformat->flags & AVFMT_GLOBALHEADER) {
        codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    // Open the codec
#ifdef AUDIOWRITE_DEBUG
    spdlog::debug("avcodec_open2");
#endif
    if (avcodec_open2(codec_context, codec, nullptr) < 0) {
        avcodec_free_context(&codec_context);
        avformat_free_context(format_context);
        spdlog::error("Could not open codec");
#ifdef AUDIOWRITE_DEBUG
        av_log_set_callback(nullptr);
#endif
        return false;
    }

    // Copy the codec context parameters to the stream
#ifdef AUDIOWRITE_DEBUG
    spdlog::debug("avcodec_parameters_from_context");
#endif
    if (avcodec_parameters_from_context(audio_stream->codecpar, codec_context) < 0) {
        avcodec_free_context(&codec_context);
        avformat_free_context(format_context);
        spdlog::error("Could not copy codec parameters");
#ifdef AUDIOWRITE_DEBUG
        av_log_set_callback(nullptr);
#endif
        return false;
    }

    // Open the output file
    if (!(format_context->oformat->flags & AVFMT_NOFILE)) {
#ifdef AUDIOWRITE_DEBUG
        spdlog::debug("avio_open");
#endif
        if (avio_open(&format_context->pb, filename.c_str(), AVIO_FLAG_WRITE) < 0) {
            avcodec_free_context(&codec_context);
            avformat_free_context(format_context);
            spdlog::error("Could not open output file");
#ifdef AUDIOWRITE_DEBUG
            av_log_set_callback(nullptr);
#endif
            return false;
        }
    } else {
        avcodec_free_context(&codec_context);
        avformat_free_context(format_context);
        spdlog::error("avio open was being skipped");
#ifdef AUDIOWRITE_DEBUG
        av_log_set_callback(nullptr);
#endif
        return false;
    }

    // Write the stream header
#ifdef AUDIOWRITE_DEBUG
    spdlog::debug("avformat_write_header");
#endif
    if (avformat_write_header(format_context, nullptr) < 0) {
        avcodec_free_context(&codec_context);
        avio_closep(&format_context->pb);
        avformat_free_context(format_context);
        spdlog::error("Error occurred when opening output file");
#ifdef AUDIOWRITE_DEBUG
        av_log_set_callback(nullptr);
#endif
        return false;
    }

    // Allocate the audio frame
#ifdef AUDIOWRITE_DEBUG
    spdlog::debug("av_frame_alloc");
#endif
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        avcodec_free_context(&codec_context);
        avio_closep(&format_context->pb);
        avformat_free_context(format_context);
        spdlog::error("Could not allocate audio frame");
#ifdef AUDIOWRITE_DEBUG
        av_log_set_callback(nullptr);
#endif
        return false;
    }
    frame->nb_samples = codec_context->frame_size;
    frame->format = codec_context->sample_fmt;
#if LIBAVFORMAT_VERSION_MAJOR >= 59
    frame->time_base = codec_context->time_base;
#endif
    
#if LIBAVFORMAT_VERSION_MAJOR < 59
    frame->channels = codec_context->channels;
#else
    if (av_channel_layout_copy(&frame->ch_layout, &codec_context->ch_layout) < 0) {
        avcodec_free_context(&codec_context);
        av_frame_free(&frame);
        avio_closep(&format_context->pb);
        avformat_free_context(format_context);
        spdlog::error("Could not copy channel layout");
#ifdef AUDIOWRITE_DEBUG
        av_log_set_callback(nullptr);
#endif
        return false;
    }
#endif

    // Allocate the buffer for the frame data
    // int buffer_size = av_samples_get_buffer_size(nullptr, codec_context->channels, frame->nb_samples, codec_context->sample_fmt, 0);
#ifdef AUDIOWRITE_DEBUG
    spdlog::debug("av_samples_get_buffer_size");
#endif
    int buffer_size = av_samples_get_buffer_size(nullptr,
#if LIBAVFORMAT_VERSION_MAJOR < 59
                                                 codec_context->channels
#else
                                                 codec_context->ch_layout.nb_channels
#endif
                                                 ,
                                                 frame->nb_samples, codec_context->sample_fmt, 1);
    if (buffer_size < 0) {
        avcodec_free_context(&codec_context);
        av_frame_free(&frame);
        avio_closep(&format_context->pb);
        avformat_free_context(format_context);
        spdlog::error("Could not get sample buffer size");
#ifdef AUDIOWRITE_DEBUG
        av_log_set_callback(nullptr);
#endif
        return false;
    }

    spdlog::debug("Buffer size: {}", buffer_size);

    if (buffer_size == 0) {
        avcodec_free_context(&codec_context);
        av_frame_free(&frame);
        avio_closep(&format_context->pb);
        avformat_free_context(format_context);
        spdlog::error("Buffer size determined to be zero ... that aint right.");
#ifdef AUDIOWRITE_DEBUG
        av_log_set_callback(nullptr);
#endif
        return false;
    }

#ifdef AUDIOWRITE_DEBUG
    spdlog::debug("av_malloc");
#endif
    uint8_t* frame_buffer = (uint8_t*)av_malloc(buffer_size);
    if (!frame_buffer) {
        avcodec_free_context(&codec_context);
        av_frame_free(&frame);
        avio_closep(&format_context->pb);
        avformat_free_context(format_context);
        spdlog::error("Could not allocate frame buffer");
#ifdef AUDIOWRITE_DEBUG
        av_log_set_callback(nullptr);
#endif
        return false;
    }

    // if (avcodec_fill_audio_frame(frame, codec_context->channels, codec_context->sample_fmt, frame_buffer, buffer_size, 0) < 0) {
#ifdef AUDIOWRITE_DEBUG
    spdlog::debug("avcodec_fill_audio_frame");
#endif
    if (avcodec_fill_audio_frame(frame,
#if LIBAVFORMAT_VERSION_MAJOR < 59
                                 codec_context->channels
#else
                                 codec_context->ch_layout.nb_channels
#endif
                                 ,
                                 codec_context->sample_fmt, frame_buffer, buffer_size, 0) < 0) {
        avcodec_free_context(&codec_context);
        av_frame_free(&frame);
        avio_closep(&format_context->pb);
        avformat_free_context(format_context);
        spdlog::error("Could not fill audio frame");
        av_free(frame_buffer);
#ifdef AUDIOWRITE_DEBUG
        av_log_set_callback(nullptr);
#endif
        return false;
    }

    // Allocate the packet
#ifdef AUDIOWRITE_DEBUG
    spdlog::debug("av_packet_alloc");
#endif
    AVPacket* packet = av_packet_alloc();
    if (!packet) {
        avcodec_free_context(&codec_context);
        av_frame_free(&frame);
        avio_closep(&format_context->pb);
        avformat_free_context(format_context);
        spdlog::error("Could not allocate packet");
        av_free(frame_buffer);
#ifdef AUDIOWRITE_DEBUG
        av_log_set_callback(nullptr);
#endif
        return false;
    }

    // Encode the samples
    int sample_index = 0;
    while (sample_index < left_channel.size()) {
        // Set up the frame data pointers
        frame->pts = sample_index;
        int mx = codec_context->frame_size;
        if ((sample_index + mx) > left_channel.size()) {
            mx = left_channel.size() - sample_index;
        }
#if LIBAVFORMAT_VERSION_MAJOR >= 60
        frame->duration = mx;
#endif
        if (s16Data.empty()) {
            float* left_ptr = (float*)(frame->data[0]);
            float* right_ptr = (float*)(frame->data[1]);
            for (int i = 0; i < codec_context->frame_size; ++i) {
                if (sample_index >= left_channel.size()) {
                    left_ptr[i] = 0;
                    right_ptr[i] = 0;
                } else {
                    left_ptr[i] = left_channel[sample_index];
                    right_ptr[i] = right_channel[sample_index];
                    ++sample_index;
                }
            }
        } else {
            memcpy(frame->data[0], &s16Data[sample_index * 2], mx * sizeof(int16_t) * 2);
            sample_index += mx;
        }


        // Send the frame to the encoder
#ifdef AUDIOWRITE_DEBUG
        spdlog::debug("avcodec_send_frame");

        // some extra validations to check possible reasons why avcodec_send_frame might return invalid without writing to log
        if (!avcodec_is_open(codec_context))
            spdlog::error("Codec is not open");
        if (!av_codec_is_encoder(codec_context->codec))
            spdlog::error("Codec is not an encoder");

#endif
        int ret = avcodec_send_frame(codec_context, frame);
        if (ret < 0) {
            avcodec_free_context(&codec_context);
            av_frame_free(&frame);
            av_packet_free(&packet);
            avio_closep(&format_context->pb);
            avformat_free_context(format_context);
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, errbuf, sizeof(errbuf));
            spdlog::error("Error sending the frame to the encoder: {}", errbuf);
            av_free(frame_buffer);
#ifdef AUDIOWRITE_DEBUG
            av_log_set_callback(nullptr);
#endif
            return false;
        }
        if (sample_index >= left_channel.size()) {
            // at the end, flush everything
            avcodec_send_frame(codec_context, nullptr);
        }

        // Receive the encoded packets
        while (ret >= 0) {
#ifdef AUDIOWRITE_DEBUG
            spdlog::debug("avcodec_receive_packet");
#endif
            ret = avcodec_receive_packet(codec_context, packet);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            } else if (ret < 0) {
                avcodec_free_context(&codec_context);
                av_frame_free(&frame);
                av_packet_free(&packet);
                avio_closep(&format_context->pb);
                avformat_free_context(format_context);
                char errbuf[AV_ERROR_MAX_STRING_SIZE];
                av_strerror(ret, errbuf, sizeof(errbuf));
                spdlog::error("Error receiving encoded packet: {}", errbuf);
                av_free(frame_buffer);
#ifdef AUDIOWRITE_DEBUG
                av_log_set_callback(nullptr);
#endif
                return false;
            }

            // Write the packet
#ifdef AUDIOWRITE_DEBUG
            spdlog::debug("av_packet_rescale_ts");
#endif
            av_packet_rescale_ts(packet, codec_context->time_base, audio_stream->time_base);
            packet->stream_index = audio_stream->index;
#ifdef AUDIOWRITE_DEBUG
            spdlog::debug("av_interleaved_write_frame");
#endif
            ret = av_interleaved_write_frame(format_context, packet);
            if (ret < 0) {
                av_packet_unref(packet);
                avcodec_free_context(&codec_context);
                av_frame_free(&frame);
                av_packet_free(&packet);
                avio_closep(&format_context->pb);
                avformat_free_context(format_context);
                char errbuf[AV_ERROR_MAX_STRING_SIZE];
                av_strerror(ret, errbuf, sizeof(errbuf));
                spdlog::error("Error writing packet to output file: {}", errbuf);
                av_free(frame_buffer);
#ifdef AUDIOWRITE_DEBUG
                av_log_set_callback(nullptr);
#endif
                return false;
            }

#ifdef AUDIOWRITE_DEBUG
            spdlog::debug("av_packet_unref");
#endif
            av_packet_unref(packet);
        }
    }

    // Write the trailer
#ifdef AUDIOWRITE_DEBUG
    spdlog::debug("av_write_trailer");
#endif
    av_write_trailer(format_context);

    // Clean up
    avcodec_free_context(&codec_context);
    av_frame_free(&frame);
    av_packet_free(&packet);
    avio_closep(&format_context->pb);
    avformat_free_context(format_context);
    av_free(frame_buffer);

#ifdef AUDIOWRITE_DEBUG
    av_log_set_callback(nullptr);
#endif

    return true;
}

class AudioReaderDecoder {
public:
    AudioReaderDecoder(const std::string& path);
    virtual ~AudioReaderDecoder();

    AudioReaderDecoderInitState initialize();
    AudioReaderDecoderInitState initState() const {
        return _initState;
    }

    bool readAndDecode(std::function<void(const AVFrame*)> callback);

    bool getAudioParams(AudioParams& p);

protected:
    const std::string _path;
    AudioReaderDecoderInitState _initState;
    int _streamIndex;
    AVFormatContext* _formatContext;
    AVCodecContext* _codecContext;
    AVPacket* _packet;
    AVFrame* _frame;
};

AudioReaderDecoder::AudioReaderDecoder(const std::string& path) :
    _path(path), _initState(AudioReaderDecoderInitState::NoInit), _streamIndex(-1), _formatContext(nullptr), _codecContext(nullptr), _packet(nullptr), _frame(nullptr) {
}

AudioReaderDecoder::~AudioReaderDecoder() {
    if (_frame != nullptr)
        ::av_frame_free(&_frame);
    if (_packet != nullptr)
        ::av_packet_free(&_packet);
    if (_codecContext != nullptr)
        ::avcodec_free_context(&_codecContext);
    if (_formatContext != nullptr)
        ::avformat_free_context(_formatContext);
}

#define SetStateAndReturn(a) \
    {                        \
        _initState = a;      \
        return a;            \
    }

AudioReaderDecoderInitState AudioReaderDecoder::initialize() {
    if (_initState != AudioReaderDecoderInitState::NoInit)
        return _initState;

    _formatContext = ::avformat_alloc_context();
    if (_formatContext == nullptr)
        SetStateAndReturn(AudioReaderDecoderInitState::FormatContextAllocFails);

    int status = ::avformat_open_input(&_formatContext, _path.c_str(), nullptr, nullptr);
    if (status != 0)
        SetStateAndReturn(AudioReaderDecoderInitState::OpenFails);

    status = ::avformat_find_stream_info(_formatContext, nullptr);
    if (status < 0)
        SetStateAndReturn(AudioReaderDecoderInitState::FindStreamInfoFails);
#if LIBAVCODEC_VERSION_MAJOR >= 59
    const
#endif
        AVCodec* codec = nullptr;
    _streamIndex = ::av_find_best_stream(_formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
    if (_streamIndex == -1)
        SetStateAndReturn(AudioReaderDecoderInitState::NoAudioStream);

    _codecContext = ::avcodec_alloc_context3(codec);
    if (_codecContext == nullptr)
        SetStateAndReturn(AudioReaderDecoderInitState::CodecContextAllocFails);

    avcodec_parameters_to_context(_codecContext, _formatContext->streams[_streamIndex]->codecpar);

    status = ::avcodec_open2(_codecContext, codec, nullptr);
    if (status != 0)
        SetStateAndReturn(AudioReaderDecoderInitState::CodecOpenFails);

    _packet = ::av_packet_alloc();
    if (_packet == nullptr)
        SetStateAndReturn(AudioReaderDecoderInitState::PacketAllocFails);
    //::av_init_packet( _packet );

    _frame = ::av_frame_alloc();
    if (_frame == nullptr)
        SetStateAndReturn(AudioReaderDecoderInitState::FrameAllocFails);

    ::av_seek_frame(_formatContext, _streamIndex, 0, AVSEEK_FLAG_ANY);

    SetStateAndReturn(AudioReaderDecoderInitState::Ok);
}

bool AudioReaderDecoder::getAudioParams(AudioParams& p) {
    if (_initState == AudioReaderDecoderInitState::NoInit)
        initialize();
    if (_initState != AudioReaderDecoderInitState::Ok)
        return false;

    p.sampleFormat = _codecContext->sample_fmt;

    const AVCodecParameters* codecParams = _formatContext->streams[_streamIndex]->codecpar;
#if LIBAVFORMAT_VERSION_MAJOR < 59
    p.channelCount = codecParams->channels;
#else
    p.channelCount = codecParams->ch_layout.nb_channels;
#endif
    p.sampleRate = codecParams->sample_rate;
    p.bytesPerSample = ::av_get_bytes_per_sample(_codecContext->sample_fmt);

    return true;
}

bool AudioReaderDecoder::readAndDecode(std::function<void(const AVFrame*)> callback) {
    if (_initState == AudioReaderDecoderInitState::NoInit)
        initialize();
    if (_initState != AudioReaderDecoderInitState::Ok)
        return false;

    int status;
    for (bool receivedEOF = false; !receivedEOF;) {
        while ((status = ::av_read_frame(_formatContext, _packet)) == 0) {
            if (_packet->stream_index == _streamIndex)
                break;
            ::av_packet_unref(_packet);
        }

        if (status == AVERROR_EOF)
            receivedEOF = true;

        status = ::avcodec_send_packet(_codecContext, receivedEOF ? nullptr : _packet);
        if (status == 0) {
            do {
                status = ::avcodec_receive_frame(_codecContext, _frame);
                if (status == AVERROR_EOF)
                    break;

                if (status == 0)
                    callback(_frame);
            } while (status != AVERROR(EAGAIN));
        }
        ::av_packet_unref(_packet);
    }

    return true;
}

class AudioResampler {
public:
    AudioResampler(const AudioParams& inputParams, int inMaxSampleCount, const AudioParams& outputParams);
    virtual ~AudioResampler();

    AudioResamplerInitState initialize();
    AudioResamplerInitState initState() const {
        return _initState;
    }

    int convert(const uint8_t* nonPlanarPtr, int n);
    int flush();

    int numConverted() const {
        return _numConverted;
    }
    const uint8_t* const* outputBuffers() const {
        return _dstData;
    }

protected:
    const AudioParams _inputParams;
    const int _maxInSampleCount;
    const AudioParams _outputParams;
    int _maxReturnedSampleCount;
    uint8_t** _dstData;
    AudioResamplerInitState _initState;
    SwrContext* _swrContext;
    int _numConverted;
};

AudioResampler::AudioResampler(const AudioParams& inputParams, int maxInSampleCount, const AudioParams& outputParams) :
    _inputParams(inputParams), _maxInSampleCount(maxInSampleCount), _outputParams(outputParams), _maxReturnedSampleCount(0), _dstData(nullptr), _initState(AudioResamplerInitState::NoInit), _swrContext(nullptr), _numConverted(0) {
}

AudioResampler::~AudioResampler() {
    if (_dstData != nullptr) {
        int n = (::av_sample_fmt_is_planar(_outputParams.sampleFormat) != 0) ? _outputParams.channelCount : 1;
        for (int i = 0; i < n; ++i)
            ::av_freep(&_dstData[i]);
        ::av_freep(&_dstData);
    }

    if (_swrContext != nullptr) {
        ::swr_close(_swrContext);
        ::swr_free(&_swrContext);
    }
}

AudioResamplerInitState AudioResampler::initialize() {
    if (_initState != AudioResamplerInitState::NoInit)
        return _initState;

#if LIBAVFORMAT_VERSION_MAJOR < 59
    uint64_t inChannelLayout = ::av_get_default_channel_layout(_inputParams.channelCount);
    uint64_t outChannelLayout = ::av_get_default_channel_layout(_outputParams.channelCount);
    _swrContext = ::swr_alloc_set_opts(nullptr,
                                       outChannelLayout, _outputParams.sampleFormat, _outputParams.sampleRate,
                                       inChannelLayout, _inputParams.sampleFormat, _inputParams.sampleRate,
                                       0, nullptr);
#else
    AVChannelLayout inChannelLayout;
    av_channel_layout_default(&inChannelLayout, _inputParams.channelCount);
    AVChannelLayout outChannelLayout;
    av_channel_layout_default(&outChannelLayout, _outputParams.channelCount);
    swr_alloc_set_opts2(&_swrContext, &outChannelLayout, _outputParams.sampleFormat, _outputParams.sampleRate, &inChannelLayout, _inputParams.sampleFormat, _inputParams.sampleRate, 0, nullptr);
#endif

    ::swr_init(_swrContext);
    if (::swr_is_initialized(_swrContext) == 0)
        SetStateAndReturn(AudioResamplerInitState::InitFails);

    _maxReturnedSampleCount = ::swr_get_out_samples(_swrContext, _maxInSampleCount);

    int dst_linesize = 0;
    int status = ::av_samples_alloc_array_and_samples(&_dstData, &dst_linesize, _outputParams.channelCount, _maxReturnedSampleCount, _outputParams.sampleFormat, 0);
    if (status <= 0)
        SetStateAndReturn(AudioResamplerInitState::OutputInitFails);

    SetStateAndReturn(AudioResamplerInitState::Ok);
}

#undef SetStateAndReturn

int AudioResampler::convert(const uint8_t* nonPlanarPtr, int n) {
    if (_initState == AudioResamplerInitState::NoInit)
        initialize();
    if (_initState != AudioResamplerInitState::Ok)
        return 0;

    return ::swr_convert(_swrContext, _dstData, _maxReturnedSampleCount, &nonPlanarPtr, n);
}

int AudioResampler::flush() {
    return ::swr_convert(_swrContext, _dstData, _maxReturnedSampleCount, nullptr, 0);
}

namespace {
    int16_t swap_endian(int16_t s) {
        int8_t* ch = (int8_t*)&s;
        std::swap(ch[0], ch[1]);
        return *(int16_t*)ch;
    }
}

AudioLoader::AudioLoader(const std::string& path, size_t bitrate, bool forceLittleEndian /*=false*/) :
    _path(path), _forceLittleEndian(forceLittleEndian), _state(AudioLoader::State::NoInit), _numInResampleBuffer(0), _resampleBufferSampleCapacity(0), _primingAdjustment(0), _bitrate(bitrate) {
    // format-specific adjustment for "priming samples"
    size_t pos;
    if ((pos = path.rfind('.')) != std::string::npos) {
        std::string ext(path.substr(pos));
        if (ext == ".mp3")
            _primingAdjustment = 1152;
    }
}

AudioLoader::~AudioLoader() {
}

#define SetStateAndReturn(a, b) \
    {                           \
        _state = a;             \
        return b;               \
    }

bool AudioLoader::loadAudioData() {
    _readerDecoder.reset(new AudioReaderDecoder(_path));

    if (_readerDecoder->initialize() != AudioReaderDecoderInitState::Ok)
        SetStateAndReturn(AudioLoader::State::ReaderDecoderInitFails, false);

    // ReaderDecoder has already successfully initialized so no need to check return value
    _readerDecoder->getAudioParams(_inputParams);

    // We always feed the resampler with interleaved (aka packed) data
    _resamplerInputParams = _inputParams;
    _resamplerInputParams.sampleFormat = ::av_get_packed_sample_fmt(_inputParams.sampleFormat);

    AudioParams outputParams = { 2, AV_SAMPLE_FMT_S16, (int)_bitrate, 2 };

    _resampler.reset(new AudioResampler(_resamplerInputParams, _resamplerInputParams.sampleRate, outputParams));
    if (_resampler->initialize() != AudioResamplerInitState::Ok)
        SetStateAndReturn(AudioLoader::State::ResamplerInitFails, false);

    _resampleBufferSampleCapacity = _inputParams.sampleRate;

    int bufferSize = _resampleBufferSampleCapacity * _inputParams.channelCount * _inputParams.bytesPerSample;

    _resampleBuff.reset(new uint8_t[bufferSize]);
    ::memset(_resampleBuff.get(), 0, bufferSize);

    std::function<void(const AVFrame*)> callback = [this](const AVFrame* frame) {
        this->processDecodedAudio(frame);
    };

    if (!_readerDecoder->readAndDecode(callback))
        SetStateAndReturn(AudioLoader::State::LoadAudioFails, false);

    flushResampleBuffer();

    int numFlushed = _resampler->flush();
    if (numFlushed > 0)
        copyResampledAudio(numFlushed);

    if (_forceLittleEndian) {
        int i = 1;
        char c = *(char*)&i;
        if (c == 0) // running on big-endian architecture
        {
            for (auto iter = _processedAudio.begin(); iter != _processedAudio.end(); ++iter)
                *iter = swap_endian(*iter);
        }
    }

    SetStateAndReturn(AudioLoader::State::Ok, true);
}

bool AudioLoader::readerDecoderInitState(AudioReaderDecoderInitState& state) const {
    if (_readerDecoder == nullptr)
        return false;

    state = _readerDecoder->initState();
    return true;
}

bool AudioLoader::resamplerInitState(AudioResamplerInitState& state) const {
    if (_resampler == nullptr)
        return false;

    state = _resampler->initState();
    return true;
}

void AudioLoader::processDecodedAudio(const AVFrame* frame) {
    int sampleCount = frame->nb_samples;
    int numToCopy = std::min(_resampleBufferSampleCapacity - _numInResampleBuffer, sampleCount);
    bool needToInterleaveSamples = (_inputParams.sampleFormat != _resamplerInputParams.sampleFormat);
    int n = _inputParams.channelCount * _inputParams.bytesPerSample;

    if (!needToInterleaveSamples) {
        ::memcpy(_resampleBuff.get() + _numInResampleBuffer * n, frame->data[0], numToCopy * n);
    } else {
        uint8_t* dst = _resampleBuff.get() + _numInResampleBuffer * n;
        for (int i = 0; i < numToCopy; ++i) {
            for (int ii = 0; ii < _inputParams.channelCount; ++ii) {
                const uint8_t* src = &frame->data[ii][i * _inputParams.bytesPerSample];
                ::memcpy(dst, src, _inputParams.bytesPerSample);
                dst += _inputParams.bytesPerSample;
            }
        }
    }
    _numInResampleBuffer += numToCopy;

    // Resample buffer was filled... need to resample and preserve leftovers from this frame
    if (_numInResampleBuffer == _resampleBufferSampleCapacity) {
        int numConverted = _resampler->convert(_resampleBuff.get(), _resampleBufferSampleCapacity);
        int numLeftovers = sampleCount - numToCopy;

        copyResampledAudio(numConverted);

        if (!needToInterleaveSamples) {
            ::memcpy(_resampleBuff.get(), frame->data[0] + numToCopy * n, numLeftovers * n);
        } else {
            int srcStartIndex = numToCopy * _inputParams.bytesPerSample;
            uint8_t* dst = _resampleBuff.get();
            for (int i = 0; i < numLeftovers; ++i) {
                for (int ii = 0; ii < _inputParams.channelCount; ++ii) {
                    const uint8_t* src = &frame->data[ii][srcStartIndex + i * _inputParams.bytesPerSample];
                    ::memcpy(dst, src, _inputParams.bytesPerSample);
                    dst += _inputParams.bytesPerSample;
                }
            }
        }

        _numInResampleBuffer = sampleCount - numToCopy;
    }
}

void AudioLoader::copyResampledAudio(int sampleCount) {
    auto output = _resampler->outputBuffers();
    const int16_t* ptr = (const int16_t*)output[0];
    for (int i = 0; i < sampleCount; ++i) {
        _processedAudio.push_back(*ptr++);
        _processedAudio.push_back(*ptr++);
    }
}

void AudioLoader::flushResampleBuffer() {
    if (_numInResampleBuffer == 0)
        return;

    int numConverted = _resampler->convert(_resampleBuff.get(), std::min(_numInResampleBuffer + _primingAdjustment, _resampleBufferSampleCapacity));
    copyResampledAudio(numConverted);

    _numInResampleBuffer = 0;
}
