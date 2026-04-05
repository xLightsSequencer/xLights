/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "AudioLoader.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <functional>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

#include <log.h>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

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
    const
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
p.channelCount = codecParams->ch_layout.nb_channels;
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

AVChannelLayout inChannelLayout;
    av_channel_layout_default(&inChannelLayout, _inputParams.channelCount);
    AVChannelLayout outChannelLayout;
    av_channel_layout_default(&outChannelLayout, _outputParams.channelCount);
    swr_alloc_set_opts2(&_swrContext, &outChannelLayout, _outputParams.sampleFormat, _outputParams.sampleRate, &inChannelLayout, _inputParams.sampleFormat, _inputParams.sampleRate, 0, nullptr);
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

#ifdef __clang__
#pragma clang diagnostic pop
#else
#pragma GCC diagnostic pop
#endif
