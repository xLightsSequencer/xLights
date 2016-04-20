#ifndef VIDEOEFFECT_H
#define VIDEOEFFECT_H

#include "RenderableEffect.h"
#include "../RenderBuffer.h"
#include <string>
#include <list>
#include <wx/gdicmn.h>
#include <wx/colour.h>
#include <wx/dcmemory.h>

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
	int GetLengthMS() { return _length; };
	void Seek(int timestampMS);
	AVFrame* GetNextFrame(int timestampMS);
	bool IsValid() { return _valid; };
	int GetWidth() { return _width; };
	int GetHeight() { return _height; };
	bool AtEnd() { return _atEnd; };

private:
	bool _valid;
	int _length;
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
	int _currentframe;
	int _lastframe;
	bool _atEnd;
};

class VideoEffect : public RenderableEffect
{
    public:
        VideoEffect(int id);
        virtual ~VideoEffect();
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);
        void Render(RenderBuffer &buffer,
					const std::string& filename, double starttime, bool keepaspectratio, bool loop);
        virtual bool CanBeRandom() {return false;}

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
        virtual bool needToAdjustSettings(const std::string& version) { return true; };
        virtual void adjustSettings(const std::string &version, Effect *effect);
};

#endif // VIDEOEFFECT_H
