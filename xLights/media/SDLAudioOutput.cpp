/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SDLAudioOutput.h"

#include <cassert>
#include <cmath>
#include <cstring>
#include <algorithm>

#include <log.h>

#include "kiss_fft/tools/kiss_fftr.h"

#ifndef __APPLE__
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

#define SDL_INPUT_BUFFER_SIZE 8192

// The global SDLManager instance. On non-Apple platforms this is defined in
// AudioManager.cpp and is the active audio output manager. On Apple platforms
// SDLAudioOutput is compiled but not used (AVAudioEngine is used instead);
// we still need a definition for fill_audio's global volume reference.
#if defined(__APPLE__)
static SDLManager __sdlManager;
#else
extern SDLManager __sdlManager;
#endif

// Due to Ubuntu still using FFMpeg 4.x, we have to use some deprecated API's
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

void fill_audio(void* udata, Uint8* stream, int len) {
    // SDL 2.0
    SDL_memset(stream, 0, len);
    assert(strlen(SDL_GetError()) == 0);

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
            SDL_MixAudioFormat(stream, it->_audio_pos, AUDIO_S16SYS, len, volume);
            it->_audio_pos += len;
            it->_audio_len -= len;
        }
    }
}

#ifdef __clang__
#pragma clang diagnostic pop
#else
#pragma GCC diagnostic pop
#endif

#pragma region AudioData
int AudioData::__nextId = 0;

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

    if (_audio_len > _original_len)
        _audio_len = _original_len;

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
    _savedpos = Tell();
}

void AudioData::RestorePos() {
    Seek(_savedpos);
}
#pragma endregion

#pragma region BaseSDL
BaseSDL::~BaseSDL() {
    CloseDevice();
}

bool BaseSDL::CloseDevice() {

    if (_dev > 0) {
        SDL_ClearError();
        SDL_AudioStatus as = SDL_GetAudioDeviceStatus(_dev);
        assert(strlen(SDL_GetError()) == 0);
        if (as == SDL_AUDIO_PLAYING) {
            SDL_PauseAudioDevice(_dev, 1);
            assert(strlen(SDL_GetError()) == 0);
        }
        SDL_ClearError();
        SDL_CloseAudioDevice(_dev);
        assert(strlen(SDL_GetError()) == 0);
        _dev = 0;
    }

    return true;
}

void BaseSDL::DumpState(std::string device, int devid, SDL_AudioSpec* wanted, SDL_AudioSpec* actual) const {

    spdlog::debug("Current audio driver {}", SDL_GetCurrentAudioDriver());
    spdlog::debug("Output devices {}. Input devices {}.", SDL_GetNumAudioDevices(0), SDL_GetNumAudioDevices(1));
    spdlog::debug("Audio device '{}' opened {}. Device specification:", (const char*)device.c_str(), (int)devid);
    SDL_AudioStatus as = SDL_GetAudioDeviceStatus(devid);
    spdlog::debug("    Audio device status ({}) {}", (int)devid, (as == SDL_AUDIO_PAUSED) ? "Paused" : (as == SDL_AUDIO_PLAYING) ? "Playing"
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
    spdlog::debug("Opening audio device. {}", (const char*)d);
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

bool InputSDL::OpenDeviceInternal() {
    bool rc = BaseSDL::OpenDevice(true, DEFAULT_RATE);

    if (rc) {
        spdlog::debug("Unpausing audio input device {}.", _dev);
        SDL_ClearError();
        SDL_AudioStatus as = SDL_GetAudioDeviceStatus(_dev);
        assert(strlen(SDL_GetError()) == 0);
        if (as == SDL_AUDIO_PAUSED) {
            SDL_PauseAudioDevice(_dev, 0);
            assert(strlen(SDL_GetError()) == 0);
        }
        spdlog::debug("    Result '{}'", SDL_GetError());
    }

    return rc;
}

std::list<std::string> InputSDL::GetAudioDevices() {
    std::list<std::string> devices;

#ifdef _WIN32
    int count = SDL_GetNumAudioDevices(1);
    assert(strlen(SDL_GetError()) == 0);

    for (int i = 0; i < count; i++) {
        devices.push_back(SDL_GetAudioDeviceName(i, 1));
        assert(strlen(SDL_GetError()) == 0);
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
        if (!OpenDeviceInternal()) {
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
        assert(strlen(SDL_GetError()) == 0);
        uint32_t avail = SDL_GetQueuedAudioSize(_dev);
        assert(strlen(SDL_GetError()) == 0);
        uint32_t toread = std::min(avail - bytesNeeded, (uint32_t)sizeof(buffer));
        uint32_t read = SDL_DequeueAudio(_dev, buffer, toread);
        assert(strlen(SDL_GetError()) == 0);
        assert(read == toread);
    }
}

int InputSDL::GetAudio(uint8_t* buffer, int bufsize) {
    int ms = bufsize * 1000 / DEFAULT_RATE;
    PurgeAllButInputAudio(ms);

    SDL_ClearError();
    auto res = SDL_DequeueAudio(_dev, buffer, bufsize);
    assert(strlen(SDL_GetError()) == 0);
    return res;
}

int InputSDL::GetMax(int ms) const {
    PurgeAllButInputAudio(ms);

    int samplesNeeded = DEFAULT_RATE * ms / 1000;
    Uint8 buffer[SDL_INPUT_BUFFER_SIZE];
    memset(buffer, 0x00, sizeof(buffer));
    int read = 0;
    SDL_ClearError();
    read = SDL_DequeueAudio(_dev, buffer, sizeof(buffer));
    assert(strlen(SDL_GetError()) == 0);

    if (read == 0) {
        return -1;
    }

    int max = 0;
    for (int i = std::max(0, read - samplesNeeded * 2); i < read - 1; i += 2) {
        if (buffer[i + 1] > max)
            max = buffer[i + 1];
        if (max == 255)
            break;
    }

    return std::min((max - 127) * 2, 255);
}

std::vector<float> InputSDL::GetSpectrum(int ms) const {
    std::vector<float> res;

    PurgeAllButInputAudio(ms);

    int samplesNeeded = DEFAULT_RATE * ms / 1000;
    Uint8 buffer[SDL_INPUT_BUFFER_SIZE];
    memset(buffer, 0x00, sizeof(buffer));
    int read = 0;
    SDL_ClearError();
    read = SDL_DequeueAudio(_dev, buffer, sizeof(buffer));
    assert(strlen(SDL_GetError()) == 0);

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

    int outcount = n / 2 + 1;
    kiss_fftr_cfg cfg;
    kiss_fft_cpx* out = (kiss_fft_cpx*)malloc(sizeof(kiss_fft_cpx) * (outcount));
    if (out != nullptr) {
        if ((cfg = kiss_fftr_alloc(outcount, 0, nullptr, nullptr)) != nullptr) {
            kiss_fftr(cfg, in, out);
            free(cfg);
        }

        for (j = 0; j < 127; j++) {
            double freq = 440.0 * exp2f(((double)j - 69.0) / 12.0);
            int start = freq * (double)n / (double)DEFAULT_RATE;
            double freqnext = 440.0 * exp2f(((double)j + 1.0 - 69.0) / 12.0);
            int end = freqnext * (double)n / (double)DEFAULT_RATE;

            float val = 0.0;

            if (end < outcount - 1) {
                for (int k = start; k <= end; k++) {
                    kiss_fft_cpx* cur = out + k;
                    val = std::max(val, sqrtf(cur->r * cur->r + cur->i * cur->i));
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
    assert(strlen(SDL_GetError()) == 0);
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

    spdlog::debug("SDL initialized output: '{}'", (const char*)_device.c_str());
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

std::list<std::string> OutputSDL::GetOutputDevices() {
    std::list<std::string> devices;

#ifdef _WIN32
    int count = SDL_GetNumAudioDevices(0);
    assert(strlen(SDL_GetError()) == 0);

    for (int i = 0; i < count; i++) {
        devices.push_back(SDL_GetAudioDeviceName(i, 0));
        assert(strlen(SDL_GetError()) == 0);
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

    return d->Tell();
}

void OutputSDL::Seek(int id, long pos) {
    std::unique_lock<std::mutex> locker(_audio_Lock);

    auto d = GetData(id);

    if (d == nullptr)
        return;

    d->Seek(pos);
}

std::mutex* OutputSDL::GetAudioLock() {
    return &_audio_Lock;
}

bool OutputSDL::HasAudio(int id) const {
    return GetData(id) != nullptr;
}

std::vector<float> OutputSDL::GetSpectrum(int ms) const {
    std::vector<float> res;

    if (_audioData.size() == 0)
        return res;

    int samplesNeeded = DEFAULT_RATE * ms / 1000;
    if (samplesNeeded % 2 != 0)
        samplesNeeded++;

    Uint8 buffer[SDL_INPUT_BUFFER_SIZE];
    memset(buffer, 0x00, sizeof(buffer));

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

    int outcount = n / 2 + 1;
    kiss_fftr_cfg cfg;
    kiss_fft_cpx* out = (kiss_fft_cpx*)malloc(sizeof(kiss_fft_cpx) * (outcount));
    if (out != nullptr) {
        if ((cfg = kiss_fftr_alloc(outcount, 0, nullptr, nullptr)) != nullptr) {
            kiss_fftr(cfg, in, out);
            free(cfg);
        }

        for (j = 0; j < 127; j++) {
            double freq = 440.0 * exp2f(((double)j - 69.0) / 12.0);
            int start = freq * (double)n / (double)DEFAULT_RATE;
            double freqnext = 440.0 * exp2f(((double)j + 1.0 - 69.0) / 12.0);
            int end = freqnext * (double)n / (double)DEFAULT_RATE;

            float val = 0.0;

            if (end < outcount - 1) {
                for (int k = start; k <= end; k++) {
                    kiss_fft_cpx* cur = out + k;
                    val = std::max(val, sqrtf(cur->r * cur->r + cur->i * cur->i));
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

int OutputSDL::AddAudio(long len, uint8_t* buffer, int volume, int rate, long tracksize, long lengthMS) {

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
    spdlog::debug("SDL Audio Play on device {}.", _dev);
    if (_dev == 0) {
        if (!OpenDevice()) {
            return;
        }
    }
    SDL_ClearError();
    SDL_AudioStatus as = SDL_GetAudioDeviceStatus(_dev);
    assert(strlen(SDL_GetError()) == 0);
    if (as == SDL_AUDIO_PAUSED || as == SDL_AUDIO_STOPPED) {
        SDL_PauseAudioDevice(_dev, 0);
        assert(strlen(SDL_GetError()) == 0);
    }

    assert(SDL_GetAudioDeviceStatus(_dev) == SDL_AUDIO_PLAYING);
    _state = SDLSTATE::SDLPLAYING;
}

void OutputSDL::Stop() {
    if (_dev > 0) {
        SDL_ClearError();
        _state = SDLSTATE::SDLNOTPLAYING;
        SDL_AudioStatus as = SDL_GetAudioDeviceStatus(_dev);
        assert(strlen(SDL_GetError()) == 0);
        if (as == SDL_AUDIO_PLAYING) {
            SDL_PauseAudioDevice(_dev, 1);
            assert(strlen(SDL_GetError()) == 0);
        }
        SDL_ClearQueuedAudio(_dev);
        assert(strlen(SDL_GetError()) == 0);
    }
}

void OutputSDL::Pause() {
    spdlog::debug("SDL Audio Pause on device {}.", _dev);
    if (_dev > 0) {
        SDL_ClearError();
        SDL_AudioStatus as = SDL_GetAudioDeviceStatus(_dev);
        assert(strlen(SDL_GetError()) == 0);
        if (as == SDL_AUDIO_PLAYING) {
            SDL_PauseAudioDevice(_dev, 1);
            assert(strlen(SDL_GetError()) == 0);
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
    assert(strlen(SDL_GetError()) == 0);
    if (as == SDL_AUDIO_PAUSED) {
        SDL_PauseAudioDevice(_dev, 0);
        assert(strlen(SDL_GetError()) == 0);
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

#ifndef _WIN32
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
#ifdef _WIN32
        SDL_AudioQuit();
#endif
        SDL_Quit();
    }
}

InputSDL* SDLManager::GetInputSDL(const std::string& device) {
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

OutputSDL* SDLManager::GetOutputSDL(const std::string& device) {
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

IAudioOutput* SDLManager::GetOutput(const std::string& device) {
    return GetOutputSDL(device);
}

IAudioInput* SDLManager::GetInput(const std::string& device) {
    return GetInputSDL(device);
}

void SDLManager::SetGlobalVolume(int volume) {
    _globalVolume = volume;
}

int SDLManager::GetGlobalVolume() const {
    return _globalVolume;
}

bool SDLManager::IsNoAudio() const {
    return !_initialised;
}

void SDLManager::SetRate(float rate) {
    for (auto& it : _outputs) {
        it.second.get()->SetRate(rate);
    }
}

void SDLManager::SetDefaultInput(const std::string& input) {
    if (input == "(Default)")
        _defaultInput = "";
    else
        _defaultInput = input;
}

void SDLManager::SetDefaultOutput(const std::string& output) {
    if (output == "(Default)")
        _defaultOutput = "";
    else
        _defaultOutput = output;
}

std::list<std::string> SDLManager::GetOutputDevices() {
    return OutputSDL::GetOutputDevices();
}

std::list<std::string> SDLManager::GetInputDevices() {
    return InputSDL::GetAudioDevices();
}
#pragma endregion
