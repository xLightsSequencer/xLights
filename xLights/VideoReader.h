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
    static void SetHardwareAcceleratedVideo(bool accel);
    static bool IsHardwareAcceleratedVideo() { return HW_ACCELERATION_ENABLED; }
    static void InitHWAcceleration();
private:
    static bool HW_ACCELERATION_ENABLED;
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
	AVFormatContext* _formatContext = nullptr;
	AVCodecContext* _codecContext = nullptr;
	AVStream* _videoStream = nullptr;
    AVCodec* _decoder = nullptr;
    AVBufferRef* _hw_device_ctx = nullptr;
	int _streamIndex;
	int _width;
	int _height;
	AVFrame* _dstFrame = nullptr; // the last frame
    AVFrame* _dstFrame2 = nullptr; // the second to the last frame
    AVFrame* _srcFrame = nullptr; // the src frame
    AVFrame* _srcFrame2 = nullptr; // the src frame
    int _curPos;
    SwsContext *_swsCtx = nullptr;
    AVPacket _packet;
	AVPixelFormat _pixelFmt;
	bool _atEnd;
    std::string _filename;
    bool _videoToolboxAccelerated; 
};
#endif // VIDEOREADER_H
