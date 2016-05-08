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
	VideoReader(std::string filename, int width, int height, bool keepaspectratio);
	~VideoReader();
	int GetLengthMS() { return _lengthMS; };
	void Seek(int timestampMS);
	AVFrame* GetNextFrame(int timestampMS);
	bool IsValid() { return _valid; };
	int GetWidth() { return _width; };
	int GetHeight() { return _height; };
	bool AtEnd() { return _atEnd; };
    int GetPos();

private:
	bool _valid;
    int _lengthMS;
    int _dtspersec;
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
};
#endif // VIDEOREADER_H
