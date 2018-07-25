#ifndef VIDEOREADER_H
#define VIDEOREADER_H

#include <string>
#include <list>

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
	VideoReader(const std::string& filename, int width, int height, bool keepaspectratio, bool usenativeresolution=false);
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

private:
	bool _valid;
    double _lengthMS;
    double _dtspersec;
    long _frames;
    int _frameMS;
	AVFormatContext* _formatContext;
	AVCodecContext* _codecContext;
	AVStream* _videoStream;
	int _streamIndex;
	int _width;
	int _height;
	AVFrame* _dstFrame; // the last frame
    AVFrame* _srcFrame;
    SwsContext *_swsCtx;
    AVPacket _packet;
	AVPixelFormat _pixelFmt;
	bool _atEnd;
    std::string _filename;
};
#endif // VIDEOREADER_H
