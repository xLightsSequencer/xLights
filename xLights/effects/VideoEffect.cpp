#include "VideoEffect.h"
#include "VideoPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

#include "../../include/video-16.xpm"
#include "../../include/video-24.xpm"
#include "../../include/video-32.xpm"
#include "../../include/video-48.xpm"
#include "../../include/video-64.xpm"

VideoReader::VideoReader(std::string filename)
{
	_valid = false;
	_length = 0;

	av_register_all();

	AVFormatContext* formatContext = NULL;
	int res = avformat_open_input(&formatContext, filename.c_str(), NULL, NULL);
	if (res != 0)
	{
		std::cout << "Error opening the file" << std::endl;
		return;
	}

	if (avformat_find_stream_info(formatContext, NULL) < 0)
	{
		avformat_close_input(&formatContext);
		std::cout << "Error finding the stream info" << std::endl;
		return;
	}

	// Find the video stream
	AVCodec* cdc = nullptr;
	int streamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, &cdc, 0);
	if (streamIndex < 0)
	{
		avformat_close_input(&formatContext);
		std::cout << "Could not find any video stream in the file" << std::endl;
		return;
	}

	AVStream* videoStream = formatContext->streams[streamIndex];
	AVCodecContext* codecContext = videoStream->codec;
	codecContext->codec = cdc;

	if (avcodec_open2(codecContext, codecContext->codec, NULL) != 0)
	{
		avformat_close_input(&formatContext);
		std::cout << "Couldn't open the context with the decoder" << std::endl;
		return;
	}

	// at this point it is open and ready

	// get the video length



	_valid = true;

	avcodec_close(codecContext);
	avformat_close_input(&formatContext);
}

VideoEffect::VideoEffect(int id) : RenderableEffect(id, "Video", video_16, video_24, video_32, video_48, video_64)
{
}

VideoEffect::~VideoEffect()
{
}

bool VideoEffect::CanRenderOnBackgroundThread() {
    return true;
}
wxPanel *VideoEffect::CreatePanel(wxWindow *parent) {
    return new VideoPanel(parent);
}

void VideoEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    Render(buffer,
		   SettingsMap["FILEPICKER_Pictures_Filename"],
		   (UINT64)(SettingsMap.GetDouble("TEXTCTRL_Video_Starttime", 0.0))
          );
}

class VideoRenderCache : public EffectRenderCache {
public:
    VideoRenderCache() 
	{ 
		_videoframerate = -1;
		_videoreader = NULL; 
	};
    virtual ~VideoRenderCache() {
		if (_videoreader != NULL)
		{
			delete _videoreader;
			_videoreader = NULL;
		}
	};

	std::string _filename;
	UINT64 _starttime;
    VideoReader* _videoreader;
	int _videoframerate;
};

void VideoEffect::Render(RenderBuffer &buffer, const std::string& filename,
	UINT64 starttime)
{
	buffer.drawingContext->Clear();

	VideoRenderCache *cache = (VideoRenderCache*)buffer.infoCache[id];
	if (cache == nullptr) {
		cache = new VideoRenderCache();
		buffer.infoCache[id] = cache;
	}

	std::string &_filename = cache->_filename;
	UINT64 &_starttime = cache->_starttime;
	VideoReader* &_videoreader = cache->_videoreader;

	if (_videoreader == NULL ||
		_filename != filename ||
		_starttime != starttime)
	{
		_starttime = starttime;
		if (filename == "")
		{
			_filename = filename;
			if (_videoreader != NULL)
			{
				delete _videoreader;
				_videoreader = NULL;
			}
		}
		else
		{
			if (_videoreader == NULL || _filename != filename)
			{
				_filename = filename;
				if (_videoreader != NULL)
				{
					delete _videoreader;
					_videoreader = NULL;
				}

				// have to open the file			
				_videoreader = new VideoReader(_filename);

				// extract the video length
				int videolen = _videoreader->GetLengthMS();

				VideoPanel *fp = (VideoPanel*)panel;
				if (fp != nullptr)
				{
					fp->Slider_Video_Starttime->SetMax(videolen);
				}
				else
				{
					fp->Slider_Video_Starttime->SetMax(0);
				}
			}
		}
	}

	int in_millisecs = starttime + ((buffer.curPeriod - buffer.curEffStartPer) * 50);
	
	// get the image for the current frame
    wxImage * image = buffer.drawingContext->FlushAndGetImage();

	// then we need to shrink it to our dimensions

	// draw the image
    xlColor c;
    for(int y=0; y<buffer.BufferHt; y++)
    {
        for(int x=0; x< buffer.BufferWi; x++)
        {
			c.Set(image->GetRed(x,y),image->GetGreen(x,y),image->GetBlue(x,y), 255);
            buffer.SetPixel(x, y, c);
        }
    }
}
