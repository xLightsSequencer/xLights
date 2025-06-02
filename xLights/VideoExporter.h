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

extern "C"
{
    struct AVCodec;
    struct AVCodecContext;
    struct AVFormatContext;
    struct AVFrame;
    struct AVPacket;
    struct SwsContext;
}

#include <cstdint>
#include <functional>
#include <string>

class wxWindow;
class wxAppProgressIndicator;

#define MAX_EXPORT_BUFFER_FRAMES 20

// Uncomment this to turn on avlib debug logging
//#define VIDEOWRITE_DEBUG

void my_av_log_callback(void* ptr, int level, const char* fmt, va_list vargs);

class GenericVideoExporter
{
public:
    struct Params
    {
        int   pfmt;             // AVPixelFormat enum
        int   width;
        int   height;
        int   fps;              // limited to constant-FPS input and output currently
        int   audioSampleRate;  // assumes stereo input/output
        std::string videoCodec;
        int videoBitrate;
    };

    // Callbacks provide the video and audio for each frame
    typedef std::function< bool( AVFrame * ,uint8_t* /*buf*/, int/*bufSize*/, unsigned /*frameIndex*/ ) > GetVideoFrameCb;
    typedef std::function< bool( float* /*leftCh*/, float* /*rightCh*/, int /*frameSize*/ ) > GetAudioFrameCb;

    // Callback to allow the exporter to query the client on whether to abort the export
    typedef std::function< bool() > QueryForCancelCb;

    // Callback to allow the exporter to report export progress to the client (0-100 scale)
    typedef std::function< void( int ) > ProgressReportCb;

    GenericVideoExporter( const std::string& outPath, const Params& inParams, bool videoOnly = false );
    virtual ~GenericVideoExporter();

    void setGetVideoCallback(const std::function<bool(AVFrame*, uint8_t*, int, unsigned)>& f) { _getVideo = f; }
    void setGetAudioCallback(const std::function<bool(float*, float*, int)>& f) { _getAudio = f; }
    void setQueryForCancelCallback(const std::function<bool()>& f) { _queryForCancel = f; }
    void setProgressReportCallback(const std::function<void(int)>& f) { _progressReporter = f; }

    void initialize();
    void exportFrames( int videoFrameCount );
    void completeExport();

    const Params& inputParams() const { return _inParams; }
    const Params& outputParams() const { return _outParams; }

protected:
    bool initializeVideo( const AVCodec* codec );
    void initializeAudio( const AVCodec* codec );
    void initializeFrames();
    void initializePackets();

    int pushVideoUntilPacketFilled( int startFrameIndex );
    void pushAudioUntilPacketFilled();

    void cleanup();

    const std::string       _path;
    const bool              _videoOnly;
    Params                  _inParams;
    Params                  _outParams;
    int64_t                 _ptsIncrement = 0LL;
    SwsContext*             _swsContext = nullptr;
    AVFormatContext*        _formatContext = nullptr;
    AVCodecContext*         _videoCodecContext = nullptr;
    AVCodecContext*         _audioCodecContext = nullptr;
    AVFrame*                _colorConversionFrame = nullptr;
    AVFrame*                _videoFrames[MAX_EXPORT_BUFFER_FRAMES];
    AVFrame*                _audioFrame = nullptr;
    AVPacket*               _videoPacket = nullptr;
    AVPacket*               _audioPacket = nullptr;
    uint32_t                _curVideoFrame = 0;
    int64_t                 _curPts = 0LL;

    std::function<bool(AVFrame*, uint8_t*, int, unsigned)> _getVideo;
    std::function<bool(float*, float*, int)> _getAudio;
    std::function<bool()> _queryForCancel;
    std::function<void(int)> _progressReporter;
};

class VideoExporter : public GenericVideoExporter
{
public:
    VideoExporter( wxWindow *parent,
                   int width, int height, float scaleFactor,
                   unsigned int frameDuration, unsigned int frameCount,
                   int audioChannelCount, int audioSampleRate,
                   const std::string& outPath, const std::string& codec,
                   int videoBitrate,
                   int output_width = 0, int output_height = 0);

    bool Export(wxAppProgressIndicator * appIndicator);

protected:
    wxWindow * const    _parent;
    unsigned int        _frameCount = 0u;
};

