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

// FFmpeg-based audio loader for specialized use (HinksPixExportDialog).
// Most code should use IAudioDecoder / AudioManager instead.

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

extern "C" {
#include <libavformat/avformat.h>
struct AVFrame;
}

struct AudioParams {
    int channelCount;
    AVSampleFormat sampleFormat;
    int sampleRate;
    int bytesPerSample;

    AudioParams() :
        channelCount(0), sampleFormat(AVSampleFormat::AV_SAMPLE_FMT_NONE), sampleRate(0), bytesPerSample(0) {
    }
    AudioParams(int i_cc, AVSampleFormat i_sf, int i_sampleRate, int i_bytesPerSample) :
        channelCount(i_cc), sampleFormat(i_sf), sampleRate(i_sampleRate), bytesPerSample(i_bytesPerSample) {
    }
};

class AudioReaderDecoder;
enum class AudioReaderDecoderInitState {
    Ok,
    NoInit,
    FormatContextAllocFails,
    OpenFails,
    NoAudioStream,
    FindStreamInfoFails,
    CodecContextAllocFails,
    CodecOpenFails,
    FrameAllocFails,
    PacketAllocFails
};

class AudioResampler;
enum class AudioResamplerInitState {
    Ok,
    NoInit,
    InitFails,
    OutputInitFails
};

class AudioLoader {
public:
    AudioLoader(const std::string& path, size_t bitrate = 44100, bool forceLittleEndian = false);
    virtual ~AudioLoader();

    enum class State { Ok,
                       NoInit,
                       ReaderDecoderInitFails,
                       ResamplerInitFails,
                       LoadAudioFails };

    bool loadAudioData();

    AudioLoader::State state() const {
        return _state;
    }
    bool readerDecoderInitState(AudioReaderDecoderInitState& state) const;
    bool resamplerInitState(AudioResamplerInitState& state) const;

    // 16-bit stereo interleaved audio samples
    const std::vector<int16_t>& processedAudio() const {
        return _processedAudio;
    }

protected:
    void processDecodedAudio(const AVFrame*);
    void copyResampledAudio(int sampleCount);
    void flushResampleBuffer();

    const std::string _path;
    const bool _forceLittleEndian;
    State _state;
    std::unique_ptr<AudioReaderDecoder> _readerDecoder;
    std::unique_ptr<AudioResampler> _resampler;
    std::vector<int16_t> _processedAudio;
    std::unique_ptr<uint8_t[]> _resampleBuff;
    int _numInResampleBuffer;
    int _resampleBufferSampleCapacity;
    AudioParams _inputParams;
    AudioParams _resamplerInputParams;
    int _primingAdjustment;
    size_t _bitrate;
};
