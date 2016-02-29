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
	VideoReader(std::string filename, int width, int height);
	~VideoReader();
	int GetLengthMS() { return _length; };
	void Seek(int timestampMS);
	AVPicture* GetNextFrame(int timestampMS);
	bool IsValid() { return _valid; };
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
	AVPixelFormat _pixelFmt;
	int _currentframe;
};

class VideoEffect : public RenderableEffect
{
    public:
        VideoEffect(int id);
        virtual ~VideoEffect();
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);
        void Render(RenderBuffer &buffer,
					const std::string& filename, double starttime);
        virtual bool CanRenderOnBackgroundThread();
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
};

#endif // VIDEOEFFECT_H
