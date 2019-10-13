#ifndef VIDEOREADER_H
#define VIDEOREADER_H

#include <wx/wx.h>
#include <string>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include "msw_utils/d3video.h"

class VideoReader
{
public:
    static bool IsVideoFile(const std::string &filename);
    static long GetVideoLength(const std::string& filename);
	VideoReader(const std::string& filename, int width, int height, bool keepaspectratio, bool usenativeresolution = false, bool wantAlpha = false);
	~VideoReader();
	int GetLengthMS() const { return (int)_lengthMS; };
	void Seek(int timestampMS);
	AVFrame* GetNextFrame(int timestampMS, int gracetime = 0); // grace time is the minimum the video must be ahead before we bother to seek back to a frame
	bool IsValid() const { return _valid; };
	int GetWidth() const { return _width; };
	int GetHeight() const { return _height; };
	bool AtEnd() const { return _atEnd; };
    int GetPos();
    std::string GetFilename() const { return _filename; }
    int GetPixelChannels() const { return _wantAlpha ? 4 : 3; }

    static bool HW_ACCELERATION_ENABLED;
    static void InitHWAcceleration();
private:
    bool readFrame(int timestampMS);
    void reopenContext();
    
    int _maxwidth = 0;
    int _maxheight = 0;
	bool _valid;
    double _lengthMS;
    double _dtspersec;
    long _frames;
    int _frameMS;
    bool _wantAlpha = false;
	AVFormatContext* _formatContext;
	AVCodecContext* _codecContext;
	AVStream* _videoStream;
	int _streamIndex;
	int _width;
	int _height;
	AVFrame* _dstFrame; // the last frame
    AVFrame* _dstFrame2; // the second to the last frame
    AVFrame* _srcFrame; // the src frame
    int _curPos;
    SwsContext *_swsCtx;
    AVPacket _packet;
	AVPixelFormat _pixelFmt;
	bool _atEnd;
    std::string _filename;
    bool _videoToolboxAccelerated; 
    D3Video_va_dxva2_t* _dxva = nullptr;
};
#endif // VIDEOREADER_H
