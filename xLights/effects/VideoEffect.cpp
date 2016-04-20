#include "VideoEffect.h"
#include "VideoPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../xLightsXmlFile.h"

#include "../../include/video-16.xpm"
#include "../../include/video-24.xpm"
#include "../../include/video-32.xpm"
#include "../../include/video-48.xpm"
#include "../../include/video-64.xpm"

#ifdef __WXMSW__
//#include "wx/msw/debughlp.h"
//wxString s;
//s.Printf("Seek to timestamp %d", timestampMS);
//wxDbgHelpDLL::LogError(s);
#endif

VideoReader::VideoReader(std::string filename, int maxwidth, int maxheight, bool keepaspectratio)
{
	_valid = false;
	_length = 0;
	_formatContext = NULL;
	_codecContext = NULL;
	_videoStream = NULL;
	_dstFrame = NULL;
    _srcFrame = NULL;
	_pixelFmt = AVPixelFormat::AV_PIX_FMT_RGB24;
	_currentframe = 0;
	_lastframe = 0;
	_atEnd = false;
	_swsCtx = NULL;

	av_register_all();
    
	int res = avformat_open_input(&_formatContext, filename.c_str(), NULL, NULL);
	if (res != 0)
	{
		std::cout << "Error opening the file" << std::endl;
		return;
	}

	if (avformat_find_stream_info(_formatContext, NULL) < 0)
	{
		std::cout << "Error finding the stream info" << std::endl;
		return;
	}

	// Find the video stream
    AVCodec* cdc;
	_streamIndex = av_find_best_stream(_formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, &cdc, 0);
	if (_streamIndex < 0)
	{
		std::cout << "Could not find any video stream in the file" << std::endl;
		return;
	}

	_videoStream = _formatContext->streams[_streamIndex];
    _videoStream->discard = AVDISCARD_NONE;
	_codecContext = _videoStream->codec;

    _codecContext->active_thread_type = FF_THREAD_FRAME;
    _codecContext->thread_count = 1;
	if (avcodec_open2(_codecContext, cdc, NULL) != 0)
	{
		std::cout << "Couldn't open the context with the decoder" << std::endl;
		return;
	}

	// at this point it is open and ready

	if (keepaspectratio)
	{
		// if > 0 then video will be shrunk
		// if < 0 then video will be stretched
		float shrink = std::min((float)maxwidth / (float)_codecContext->width, (float)maxheight / (float)_codecContext->height);
		_height = (int)((float)_codecContext->height * shrink);
		_width = (int)((float)_codecContext->width * shrink);
	}
	else
	{
		_height = maxheight;
		_width = maxwidth;
	}

	// get the video length in MS
	// Use the number of frames as the best possible way to calculate length
	_lastframe = _videoStream->nb_frames;
	if (_lastframe > 0)
	{
		_length = (int)(((uint64_t)_lastframe * (uint64_t)_videoStream->avg_frame_rate.den * 1000) / (uint64_t)_videoStream->avg_frame_rate.num);
	}
	
	// If it does not look right try to base if off the duration
	if (_length <= 0 || _lastframe <= 0)
	{
		_length = (int)((uint64_t)_formatContext->duration * (uint64_t)_videoStream->avg_frame_rate.num / (uint64_t)_videoStream->avg_frame_rate.den);
        _lastframe = _length  * (uint64_t)_videoStream->avg_frame_rate.num / (uint64_t)(_videoStream->avg_frame_rate.den) / 1000;
    }

	// If it still doesnt look right
	if (_length <= 0 || _lastframe <= 0)
	{
		// This seems to work for .asf, .mkv, .flv
		_length = _formatContext->duration / 1000;
		_lastframe = _length  * (uint64_t)_videoStream->avg_frame_rate.num / (uint64_t)(_videoStream->avg_frame_rate.den) / 1000;
	}

	if (_length <= 0 || _lastframe <= 0)
	{
		// This is bad ... it still does not look right
	}

	_dstFrame = av_frame_alloc();
	_dstFrame->width = _width;
	_dstFrame->height = _height;
	_dstFrame->linesize[0] = _width * 3;
	_dstFrame->data[0] = (uint8_t *)av_malloc(_width * _height * 3 * sizeof(uint8_t));

    _srcFrame = av_frame_alloc();
    _srcFrame->pkt_pts = 0;
    
    _swsCtx = sws_getContext(_codecContext->width, _codecContext->height,
                             _codecContext->pix_fmt, _width, _height, _pixelFmt, SWS_BICUBIC, NULL,
                             NULL, NULL);

    av_init_packet(&_packet);
	_valid = true;
}

static int GetFrameForTime(int ts, const AVStream *vs) {
    float tsf = ts;
    tsf *= (float)vs->avg_frame_rate.num;
    tsf /= (float)vs->avg_frame_rate.den;
    tsf /= 1000.0;
    return std::round(tsf);
}

VideoReader::~VideoReader()
{
    if (_swsCtx != NULL) {
        sws_freeContext(_swsCtx);
        _swsCtx = NULL;
    }

    if (_srcFrame != NULL) {
        av_free(_srcFrame);
        _srcFrame = NULL;
    }
	if (_dstFrame != NULL)
	{
		if (_dstFrame->data[0] != NULL)
		{
			av_free(_dstFrame->data[0]);
		}
		av_free(_dstFrame);
		_dstFrame = NULL;
	}
	if (_codecContext != NULL)
	{
		avcodec_close(_codecContext);
		_codecContext = NULL;
	}
	if (_formatContext != NULL)
	{
		avformat_close_input(&_formatContext);
		_formatContext = NULL;
	}
}

void VideoReader::Seek(int timestampMS)
{
	// we have to be valid
	if (_valid)
	{
		// Seek about 5 secs prior to the desired timestamp ... to make sure we get a key frame
        int tgtframe = GetFrameForTime(timestampMS, _videoStream);

		if (tgtframe < _lastframe)
		{
			_atEnd = false;
		}
		else
		{
			// dont seek past the end of the file
			_atEnd = true;
			av_seek_frame(_formatContext, _streamIndex, _lastframe, AVSEEK_FLAG_FRAME);
			return;
		}

		int adj_timestamp = timestampMS - 5000;
		if (adj_timestamp < 0)
		{
			adj_timestamp = 0;
		}

		int flag = AVSEEK_FLAG_FRAME;
		int newframe = GetFrameForTime(adj_timestamp, _videoStream);
		if (newframe < _currentframe)
		{
			flag += AVSEEK_FLAG_BACKWARD;
		}
		av_seek_frame(_formatContext, _streamIndex, newframe, flag);
		_currentframe = newframe;

		while (_currentframe < tgtframe && av_read_frame(_formatContext, &_packet) >= 0)
		{

			// Is this a packet from the video stream?
			if (_packet.stream_index == _streamIndex)
			{
				// Decode video frame
				int frameFinished;
				avcodec_decode_video2(_codecContext, _srcFrame, &frameFinished,
					&_packet);

				// Did we get a video frame?
				if (frameFinished)
				{
					_currentframe++;
					sws_scale(_swsCtx, _srcFrame->data, _srcFrame->linesize, 0,
						_codecContext->height, _dstFrame->data,
						_dstFrame->linesize);
				}
			}

			// Free the packet that was allocated by av_read_frame
			av_packet_unref(&_packet);
		}
	}
}

AVFrame* VideoReader::GetNextFrame(int timestampMS)
{
	if (_valid && timestampMS <= _length)
	{
        int tgtframe = GetFrameForTime(timestampMS, _videoStream);
		AVPacket pkt2;

        int rc;
		while (_currentframe <= tgtframe && (rc = av_read_frame(_formatContext, &_packet)) >= 0 &&  _currentframe <= _lastframe)
		{
			// Is this a packet from the video stream?
			if (_packet.stream_index == _streamIndex)
			{
				// Decode video frame
                pkt2 = _packet;
                while (pkt2.size) {
                    int frameFinished = 0;
                    int ret = avcodec_decode_video2(_codecContext, _srcFrame, &frameFinished,
                                                &pkt2);
                    
                    // Did we get a video frame?
                    if (frameFinished)
                    {
                        _currentframe++;
                        sws_scale(_swsCtx, _srcFrame->data, _srcFrame->linesize, 0,
                                  _codecContext->height, _dstFrame->data,
                                  _dstFrame->linesize);
                    }
                    
                    if (ret >= 0) {
                        ret = FFMIN(ret, pkt2.size); /* guard against bogus return values */
                        pkt2.data += ret;
                        pkt2.size -= ret;
                    } else {
                        pkt2.size = 0;
                    }
                }

			}

			// Free the packet that was allocated by av_read_frame
			av_packet_unref(&_packet);
		}
	}
	else
	{
		_atEnd = true;
		return NULL;
	}

	if (_dstFrame->data[0] == NULL || _currentframe > _lastframe)
	{
		_atEnd = true;
		return NULL;
	}
	else
	{
		return _dstFrame;
	}
}

VideoEffect::VideoEffect(int id) : RenderableEffect(id, "Video", video_16, video_24, video_32, video_48, video_64)
{
}

VideoEffect::~VideoEffect()
{
}

wxPanel *VideoEffect::CreatePanel(wxWindow *parent) {
    return new VideoPanel(parent);
}

void VideoEffect::adjustSettings(const std::string &version, Effect *effect)
{
    // give the base class a chance to adjust any settings
    if (RenderableEffect::needToAdjustSettings(version))
    {
        RenderableEffect::adjustSettings(version, effect);
    }

    SettingsMap &settings = effect->GetSettings();
    std::string file = settings["FILEPICKERCTRL_Video_Filename"];

    if (file != "")
    {
        if (!wxFile::Exists(file))
        {
            settings["FILEPICKERCTRL_Video_Filename"] = xLightsXmlFile::FixFile("", file);
        }
    }
}

void VideoEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    Render(buffer,
		   SettingsMap["FILEPICKERCTRL_Video_Filename"],
		SettingsMap.GetDouble("TEXTCTRL_Video_Starttime", 0.0),
		SettingsMap.GetBool("CHECKBOX_Video_AspectRatio", false),
		SettingsMap.GetBool("CHECKBOX_Video_Loop", false)
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
	int _starttime;
    VideoReader* _videoreader;
	int _videoframerate;
	bool _aspectratio;
	bool _loop;
	int _loops;
};

void VideoEffect::Render(RenderBuffer &buffer, const std::string& filename,
	double starttime, bool aspectratio, bool loop)
{
	VideoRenderCache *cache = (VideoRenderCache*)buffer.infoCache[id];
	if (cache == nullptr) {
		cache = new VideoRenderCache();
		buffer.infoCache[id] = cache;
	}

	std::string &_filename = cache->_filename;
	int &_starttime = cache->_starttime;
	bool &_aspectratio = cache->_aspectratio;
	bool &_loop = cache->_loop;
	int &_loops = cache->_loops;
	VideoReader* &_videoreader = cache->_videoreader;

	if (_starttime != starttime)
	{
		_starttime = starttime;
	}
    
	// we always reopen video on first frame or if it is not open or if the filename has changed
	if (buffer.needToInit || _videoreader == NULL || _filename != filename || _aspectratio != aspectratio || _loop != loop)
	{
        buffer.needToInit = false;
		_filename = filename;
		_aspectratio = aspectratio;
		_loop = loop;
		_loops = 0;
		if (_videoreader != NULL)
		{
			delete _videoreader;
			_videoreader = NULL;
		}

		if (wxFileExists(_filename))
		{
			// have to open the file
			_videoreader = new VideoReader(_filename, buffer.BufferWi, buffer.BufferHt, _aspectratio);

			// extract the video length
			int videolen = _videoreader->GetLengthMS();

			VideoPanel *fp = (VideoPanel*)panel;
			if (fp != nullptr)
			{
                fp->addVideoTime(filename, videolen);
			}

			if (_starttime != 0)
			{
				_videoreader->Seek(_starttime * 1000);
			}
		}
	}

	if (_videoreader != NULL)
	{
		// get the image for the current frame
		AVFrame* image = _videoreader->GetNextFrame(_starttime * 1000 + (buffer.curPeriod - buffer.curEffStartPer) * buffer.frameTimeInMs - _loops * _videoreader->GetLengthMS());
		
		// if we have reached the end and we are to loop
		if (_videoreader->AtEnd() && _loop)
		{
			_loops++;
			// jump back to start and try to read frame again
			_videoreader->Seek(0);
			image = _videoreader->GetNextFrame(_starttime * 1000 + (buffer.curPeriod - buffer.curEffStartPer) * buffer.frameTimeInMs - _loops * _videoreader->GetLengthMS());
		}

		int startx = (buffer.BufferWi - _videoreader->GetWidth()) / 2;
		int starty = (buffer.BufferHt - _videoreader->GetHeight()) / 2;

		// check it looks valid
		if (image != NULL)
		{
			// draw the image
			xlColor c;
			for (int y = 0; y < _videoreader->GetHeight(); y++)
			{
				for (int x = 0; x < _videoreader->GetWidth(); x++)
				{
					try
					{
						c.Set(*(image->data[0] + (_videoreader->GetHeight() - 1 - y) * _videoreader->GetWidth() * 3 + x * 3),
							  *(image->data[0] + (_videoreader->GetHeight() - 1 - y) * _videoreader->GetWidth() * 3 + x * 3 + 1),
							  *(image->data[0] + (_videoreader->GetHeight() - 1 - y) * _videoreader->GetWidth() * 3 + x * 3 + 2), 255);
					}
					catch (...)
					{
						// this shouldnt happen so make it stand out
						c = xlRED;
					}
					buffer.SetPixel(x + startx, y+starty, c);
				}
			}
		}
		else
		{
			// display a blue background to show we have gone past end of video
			for (int y = 0; y < _videoreader->GetHeight(); y++)
			{
				for (int x = 0; x < _videoreader->GetWidth(); x++)
				{
					buffer.SetPixel(x+startx, y+starty, xlBLUE);
				}
			}
		}
	}
}
