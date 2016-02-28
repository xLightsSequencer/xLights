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
#include <libswresample/swresample.h>
}

class VideoReader
{
public:
	VideoReader(std::string filename);
	VideoReader();
	int GetLengthMS() { return _length; };
	void Seek(UINT64 timestamp);
	wxImage GetNextFrame(UINT64& timestamp);
	bool IsValid() { return _valid; };
private:
	bool _valid;
	int _length;
};

class VideoEffect : public RenderableEffect
{
    public:
        VideoEffect(int id);
        virtual ~VideoEffect();
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);
        void Render(RenderBuffer &buffer,
					const std::string& filename, UINT64 starttime);
        virtual bool CanRenderOnBackgroundThread();
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
};

#endif // VIDEOEFFECT_H
