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
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <string>
#include <vector>

#include <log.h>

#include "kiss_fft/tools/kiss_fftr.h"

// FFmpeg atempo filter for pitch-preserving speed change on non-Apple platforms.
// Apple platforms use AVAudioUnitTimePitch in AVAudioEngineOutput instead.
#ifndef __APPLE__
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
extern "C" {
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/version.h>
}
#ifdef __clang__
#pragma clang diagnostic pop
#else
#pragma GCC diagnostic pop
#endif

// Stretch int16 stereo interleaved PCM using FFmpeg's atempo filter.
// Returns a malloc-owned buffer; caller must free(). outputLen set on success.
// Returns nullptr on failure — caller should fall back to device-rate change.
static uint8_t* StretchAudioAtempo(const uint8_t* input, long inputLen,
                                    int sampleRate, float tempo, long& outputLen)
{
    outputLen = 0;
    if (inputLen <= 0 || sampleRate <= 0 || tempo <= 0.0f) return nullptr;

    // atempo supports [0.5, 2.0]; chain filters for wider ranges (e.g. 4x = atempo=2.0,atempo=2.0)
    std::string filterStr;
    float t = tempo;
    auto addFilter = [&](const char* f) {
        if (!filterStr.empty()) filterStr += ',';
        filterStr += f;
    };
    while (t > 2.0f + 1e-4f) { addFilter("atempo=2.0"); t /= 2.0f; }
    while (t < 0.5f - 1e-4f) { addFilter("atempo=0.5"); t *= 2.0f; }
    char atempoArg[32];
    snprintf(atempoArg, sizeof(atempoArg), "atempo=%.6f", (double)t);
    addFilter(atempoArg);

    const AVFilter* abuffersrc_flt  = avfilter_get_by_name("abuffer");
    const AVFilter* abuffersink_flt = avfilter_get_by_name("abuffersink");
    if (!abuffersrc_flt || !abuffersink_flt) return nullptr;

    AVFilterGraph* graph = avfilter_graph_alloc();
    if (!graph) return nullptr;

    AVFilterContext* srcCtx  = nullptr;
    AVFilterContext* sinkCtx = nullptr;

    char srcArgs[256];
    snprintf(srcArgs, sizeof(srcArgs),
             "time_base=1/%d:sample_rate=%d:sample_fmt=s16:channels=2:channel_layout=stereo",
             sampleRate, sampleRate);

    int ret = avfilter_graph_create_filter(&srcCtx, abuffersrc_flt, "in", srcArgs, nullptr, graph);
    if (ret < 0) {
        spdlog::warn("StretchAudioAtempo: abuffer create failed ({})", ret);
        avfilter_graph_free(&graph);
        return nullptr;
    }

    ret = avfilter_graph_create_filter(&sinkCtx, abuffersink_flt, "out", nullptr, nullptr, graph);
    if (ret < 0) {
        spdlog::warn("StretchAudioAtempo: abuffersink create failed ({})", ret);
        avfilter_graph_free(&graph);
        return nullptr;
    }

    {
        AVFilterInOut* filterIn  = avfilter_inout_alloc();
        AVFilterInOut* filterOut = avfilter_inout_alloc();
        if (!filterIn || !filterOut) {
            avfilter_inout_free(&filterIn);
            avfilter_inout_free(&filterOut);
            avfilter_graph_free(&graph);
            return nullptr;
        }
        filterOut->name = av_strdup("in");  filterOut->filter_ctx = srcCtx;  filterOut->pad_idx = 0; filterOut->next = nullptr;
        filterIn->name  = av_strdup("out"); filterIn->filter_ctx  = sinkCtx; filterIn->pad_idx  = 0; filterIn->next  = nullptr;

        ret = avfilter_graph_parse_ptr(graph, filterStr.c_str(), &filterIn, &filterOut, nullptr);
        avfilter_inout_free(&filterIn);
        avfilter_inout_free(&filterOut);
        if (ret < 0) {
            spdlog::warn("StretchAudioAtempo: graph_parse_ptr failed ({})", ret);
            avfilter_graph_free(&graph);
            return nullptr;
        }
    }

    ret = avfilter_graph_config(graph, nullptr);
    if (ret < 0) {
        spdlog::warn("StretchAudioAtempo: graph_config failed ({})", ret);
        avfilter_graph_free(&graph);
        return nullptr;
    }

    const long totalFrames = inputLen / (2L * (long)sizeof(int16_t));
    std::vector<uint8_t> outVec;
    outVec.reserve((size_t)((double)totalFrames / tempo * 1.1 + 8192) * 2 * sizeof(int16_t));

    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        avfilter_graph_free(&graph);
        return nullptr;
    }

    const int chunkFrames = 4096;
    long frameOffset = 0;
    long framesLeft  = totalFrames;

    auto drainSink = [&]() {
        while (true) {
            av_frame_unref(frame);
            ret = av_buffersink_get_frame(sinkCtx, frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
            if (ret < 0) { framesLeft = 0; break; }
            size_t bytes = (size_t)frame->nb_samples * 2 * sizeof(int16_t);
            size_t oldSz = outVec.size();
            outVec.resize(oldSz + bytes);
            memcpy(outVec.data() + oldSz, frame->data[0], bytes);
        }
    };

    while (framesLeft > 0) {
        int n = (int)std::min((long)chunkFrames, framesLeft);

        av_frame_unref(frame);
        frame->sample_rate = sampleRate;
        frame->format      = AV_SAMPLE_FMT_S16;
        frame->nb_samples  = n;
#if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(57, 28, 100)
        av_channel_layout_default(&frame->ch_layout, 2);
#else
        frame->channels       = 2;
        frame->channel_layout = AV_CH_LAYOUT_STEREO;
#endif
        if (av_frame_get_buffer(frame, 0) < 0) break;

        memcpy(frame->data[0], input + frameOffset * 2 * (long)sizeof(int16_t),
               (size_t)n * 2 * sizeof(int16_t));
        frame->pts = frameOffset;

        if (av_buffersrc_add_frame_flags(srcCtx, frame, AV_BUFFERSRC_FLAG_KEEP_REF) < 0) break;
        frameOffset += n;
        framesLeft  -= n;
        drainSink();
    }

    // Flush
    av_buffersrc_add_frame_flags(srcCtx, nullptr, 0);
    drainSink();

    av_frame_free(&frame);
    avfilter_graph_free(&graph);

    if (outVec.empty()) return nullptr;

    uint8_t* result = (uint8_t*)malloc(outVec.size());
    if (!result) return nullptr;
    memcpy(result, outVec.data(), outVec.size());
    outputLen = (long)outVec.size();
    return result;
}
#endif // !__APPLE__

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
static SDLManager __audioManager;
#else
extern SDLManager __audioManager;
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
            if (__audioManager.GetGlobalVolume() != 100) {
                volume = (volume * __audioManager.GetGlobalVolume()) / 100;
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
    if (_stretchedBuffer && _stretchedLen > 0) {
        // Each stretched frame represents _stretchRatio original frames
        long consumedStretched = (_stretchedLen - _audio_len) / 4;
        long consumedOriginal = (long)((double)consumedStretched * _stretchRatio + 0.5);
        return (long)(((Uint64)consumedOriginal * _lengthMS) / _trackSize);
    }
    long pos = (long)(((((Uint64)(_original_len - _audio_len) / 4) * _lengthMS)) / _trackSize);
    return pos;
}

void AudioData::Seek(long ms) {
    if (_stretchedBuffer && _stretchedLen > 0) {
        // Map ms position in original audio to position in stretched buffer
        long origFrames     = (long)((Uint64)ms * _rate / 1000);
        long stretchFrames  = (long)((double)origFrames / _stretchRatio);
        long byteOffset     = stretchFrames * 4;
        byteOffset -= byteOffset % 4;
        if (byteOffset < 0) byteOffset = 0;
        if (byteOffset > _stretchedLen) byteOffset = _stretchedLen;
        _audio_len = _stretchedLen - byteOffset;
        _audio_pos = _stretchedBuffer + byteOffset;
        spdlog::debug("ID {} Seeking to {}MS (stretched) ... stretched audio_len: {}", _id, ms, _audio_len);
        return;
    }

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
    if (_stretchedBuffer && _stretchedLen > 0) {
        long startFrames  = (long)((double)((Uint64)ms * _rate / 1000) / _stretchRatio);
        long lenFrames    = (long)((double)((Uint64)len * _rate / 1000) / _stretchRatio);
        long byteStart    = startFrames * 4;
        long byteCount    = lenFrames * 4;
        byteStart -= byteStart % 4;
        byteCount -= byteCount % 4;
        if (byteStart > _stretchedLen) byteStart = _stretchedLen;
        if (byteStart + byteCount > _stretchedLen) byteCount = _stretchedLen - byteStart;
        _audio_len = byteCount;
        _audio_pos = _stretchedBuffer + byteStart;
        spdlog::debug("ID {} SeekAndLimit to {}MS len {}MS (stretched) ... audio_len: {}.", _id, ms, len, _audio_len);
        return;
    }

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
    // Device always runs at the natural sample rate. Speed changes are achieved
    // by pre-processing audio through StretchAudioAtempo() (pitch is preserved).
    bool res = BaseSDL::OpenDevice(false, _initialisedRate);

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

    // Pre-stretch audio if speed is not 1.0 so Seek() will use the stretched buffer
#ifndef __APPLE__
    if (_playbackrate != 1.0f) {
        long stretchedLen = 0;
        uint8_t* stretched = StretchAudioAtempo(buffer, len, rate, _playbackrate, stretchedLen);
        if (stretched) {
            ad->_stretchedBuffer = stretched;
            ad->_stretchedLen    = stretchedLen;
            ad->_stretchRatio    = _playbackrate;
            ad->Seek(0);
        } else {
            spdlog::warn("SDL AddAudio: StretchAudioAtempo failed, audio will play at 1x speed instead of {}x", _playbackrate);
        }
    }
#endif

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

void OutputSDL::RebuildStretchedBuffers() {
#ifndef __APPLE__
    std::unique_lock<std::mutex> locker(_audio_Lock);
    for (auto* ad : _audioData) {
        free(ad->_stretchedBuffer);
        ad->_stretchedBuffer = nullptr;
        ad->_stretchedLen    = 0;
        ad->_stretchRatio    = 1.0f;

        if (_playbackrate != 1.0f) {
            long stretchedLen = 0;
            uint8_t* stretched = StretchAudioAtempo(ad->_original_pos, ad->_original_len,
                                                    ad->_rate, _playbackrate, stretchedLen);
            if (stretched) {
                ad->_stretchedBuffer = stretched;
                ad->_stretchedLen    = stretchedLen;
                ad->_stretchRatio    = _playbackrate;
            } else {
                spdlog::warn("SDL RebuildStretchedBuffers: StretchAudioAtempo failed for id {}", ad->_id);
            }
        }
    }
#endif
}

void OutputSDL::SetRate(float rate) {
    if (_playbackrate == rate) return;

    SDLSTATE oldstate = _state;
    if (_state == SDLSTATE::SDLPLAYING) Stop();

    // Save playback positions before rebuilding buffers
    {
        std::unique_lock<std::mutex> locker(_audio_Lock);
        for (const auto& it : _audioData) {
            it->SavePos();
        }
    }

    _playbackrate = rate;

    // Rebuild stretched buffers (time-consuming FFmpeg processing; audio is stopped).
    // RebuildStretchedBuffers() holds _audio_Lock internally.
    RebuildStretchedBuffers();

    // Restore playback positions; no device close/reopen needed since OpenDevice()
    // is independent of _playbackrate (device always runs at natural sample rate).
    {
        std::unique_lock<std::mutex> locker(_audio_Lock);
        for (const auto& it : _audioData) {
            it->RestorePos();
        }
    }

    if (oldstate == SDLSTATE::SDLPLAYING) {
        Play();
    }

    spdlog::info("SDL: playback rate changed to {:.2f}x (pitch-preserving)", rate);
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
