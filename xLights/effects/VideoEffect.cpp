#include "VideoEffect.h"
#include "VideoPanel.h"
#include "../VideoReader.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../xLightsXmlFile.h"
#include "../models/Model.h"
#include <log4cpp/Category.hh>
#include "../SequenceCheck.h"

#include "../../include/video-16.xpm"
#include "../../include/video-24.xpm"
#include "../../include/video-32.xpm"
#include "../../include/video-48.xpm"
#include "../../include/video-64.xpm"

VideoEffect::VideoEffect(int id) : RenderableEffect(id, "Video", video_16, video_24, video_32, video_48, video_64)
{
}

VideoEffect::~VideoEffect()
{
}

std::list<std::string> VideoEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff)
{
    std::list<std::string> res;

    wxString filename = settings.Get("E_FILEPICKERCTRL_Video_Filename", "");

    if (!wxFileExists(filename))
    {
        res.push_back(wxString::Format("    ERR: Video effect video file '%s' does not exist. Model '%s', Start %s", filename, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }
    else
    {
        VideoReader* videoreader = new VideoReader(filename.ToStdString(), 100, 100, false);

        if (videoreader == nullptr || videoreader->GetLengthMS() == 0)
        {
            res.push_back(wxString::Format("    ERR: Video effect video file '%s' could not be understood. Format may not be supported. Model '%s', Start %s", filename, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
        }
        else
        {
            double starttime = settings.GetDouble("E_TEXTCTRL_Video_Starttime", 0.0);
            wxString treatment = settings.Get("E_CHOICE_Video_DurationTreatment", "Normal");

            if (treatment == "Normal")
            {
                int videoduration = videoreader->GetLengthMS() - starttime;
                int effectduration = eff->GetEndTimeMS() - eff->GetStartTimeMS();
                if (videoduration < effectduration)
                {
                    res.push_back(wxString::Format("    WARN: Video effect video file '%s' is shorter %s than effect duration %s. Model '%s', Start %s", filename, FORMATTIME(videoduration), FORMATTIME(effectduration), model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
                }
            }
        }

        if (videoreader != nullptr)
        {
            delete videoreader;
        }
    }

    wxString bufferstyle = settings.Get("B_CHOICE_BufferStyle", "Default");
    wxString transform = settings.Get("B_CHOICE_BufferTransform", "None");
    int w, h;
    model->GetBufferSize(bufferstyle.ToStdString(), transform.ToStdString(), w, h);

    if (w < 2 || h < 2)
    {
        res.push_back(wxString::Format("    ERR: Video effect video file '%s' cannot render onto model as it is not high or wide enough (%d,%d). Model '%s', Start %s", filename, w, h, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }

    return res;
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

    // if the old loop setting is prsent then clear it and change the duration treatment
    bool loop = settings.GetBool("E_CHECKBOX_Video_Loop", false);
    if (loop)
    {
        settings["E_CHOICE_Video_DurationTreatment"] = "Loop";
        settings.erase("E_CHECKBOX_Video_Loop");
    }

    std::string file = settings["E_FILEPICKERCTRL_Video_Filename"];

    if (file != "")
    {
        if (!wxFile::Exists(file))
        {
            settings["E_FILEPICKERCTRL_Video_Filename"] = xLightsXmlFile::FixFile("", file);
        }
    }
}

void VideoEffect::SetDefaultParameters(Model *cls)
{
    VideoPanel *vp = (VideoPanel*)panel;
    if (vp == nullptr) {
        return;
    }

    vp->FilePicker_Video_Filename->SetFileName(wxFileName());
    SetSliderValue(vp->Slider_Video_Starttime, 0);
    SetCheckBoxValue(vp->CheckBox_Video_AspectRatio, false);
    SetChoiceValue(vp->Choice_Video_DurationTreatment, "Normal");
}

void VideoEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    Render(buffer,
		   SettingsMap["FILEPICKERCTRL_Video_Filename"],
		SettingsMap.GetDouble("TEXTCTRL_Video_Starttime", 0.0),
		SettingsMap.GetBool("CHECKBOX_Video_AspectRatio", false),
		SettingsMap.Get("CHOICE_Video_DurationTreatment", "Normal")
		);
}

class VideoRenderCache : public EffectRenderCache {
public:
    VideoRenderCache()
	{
		_videoframerate = -1;
		_videoreader = nullptr;
	};
    virtual ~VideoRenderCache() {
		if (_videoreader != nullptr)
		{
			delete _videoreader;
			_videoreader = nullptr;
		}
	};

	std::string _filename;
	double _starttime;
    VideoReader* _videoreader;
	int _videoframerate;
	bool _aspectratio;
	std::string _durationTreatment;
	int _loops;
    int _frameMS;
};

void VideoEffect::Render(RenderBuffer &buffer, const std::string& filename,
	double starttime, bool aspectratio, std::string durationTreatment)
{
    wxStopWatch sw;
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    VideoRenderCache *cache = (VideoRenderCache*)buffer.infoCache[id];
	if (cache == nullptr) {
		cache = new VideoRenderCache();
		buffer.infoCache[id] = cache;
	}

	std::string &_filename = cache->_filename;
	double &_starttime = cache->_starttime;
	bool &_aspectratio = cache->_aspectratio;
	std::string &_durationTreatment = cache->_durationTreatment;
	int &_loops = cache->_loops;
    int &_frameMS = cache->_frameMS;
	VideoReader* &_videoreader = cache->_videoreader;

	if (_starttime != starttime)
	{
		_starttime = starttime;
	}
    
	// we always reopen video on first frame or if it is not open or if the filename has changed
	if (buffer.needToInit)
	{
        buffer.needToInit = false;
		_filename = filename;
		_aspectratio = aspectratio;
		_durationTreatment = durationTreatment;
		_loops = 0;
        _frameMS = buffer.frameTimeInMs;
		if (_videoreader != nullptr)
		{
			delete _videoreader;
			_videoreader = nullptr;
		}

        if (buffer.BufferHt == 1)
        {
            logger_base.warn("VideoEffect::Cannot render video onto a 1 pixel high model. Have you set it to single line?");
        }
        else if (wxFileExists(_filename))
		{
			// have to open the file
			_videoreader = new VideoReader(_filename, buffer.BufferWi, buffer.BufferHt, _aspectratio);

            if (_videoreader == nullptr)
            {
                logger_base.warn("VideoEffect: Failed to load video file %s.", (const char *)_filename.c_str());
            }
            else
            {
                // extract the video length
                int videolen = _videoreader->GetLengthMS();

                VideoPanel *fp = static_cast<VideoPanel*>(panel);
                if (fp != nullptr)
                {
                    fp->addVideoTime(filename, videolen);
                }

                if (_starttime != 0)
                {
                    _videoreader->Seek(_starttime * 1000);
                }

                if (_durationTreatment == "Slow/Accelerate")
                {
                    int effectFrames = buffer.curEffEndPer - buffer.curEffStartPer + 1;
                    int videoFrames = (videolen - (_starttime * 1000)) / buffer.frameTimeInMs;
                    float speedFactor = (float)videoFrames / (float)effectFrames;
                    _frameMS = (int)((float)buffer.frameTimeInMs * speedFactor);
                }
                logger_base.debug("Video effect length: %d, video length: %d, startoffset: %f, duration treatment: %s.",
                                  (buffer.curEffEndPer - buffer.curEffStartPer + 1) * _frameMS, videolen, (float)_starttime,
                                  (const char *)_durationTreatment.c_str());
            }
		}
        else
        {
            if (buffer.curPeriod == buffer.curEffStartPer)
            {
                logger_base.warn("VideoEffect: Video file '%s' not found.", (const char *)_filename.c_str());
            }
        }
	}


	if (_videoreader != nullptr)
	{
        long frame = _starttime * 1000 + (buffer.curPeriod - buffer.curEffStartPer) * _frameMS - _loops * (_videoreader->GetLengthMS() + _frameMS);
        // get the image for the current frame
		AVFrame* image = _videoreader->GetNextFrame(frame);
		
		// if we have reached the end and we are to loop
		if (_videoreader->AtEnd() && _durationTreatment == "Loop")
		{
            // jump back to start and try to read frame again
            _loops++;
            frame = _starttime * 1000 + (buffer.curPeriod - buffer.curEffStartPer) * _frameMS - _loops * (_videoreader->GetLengthMS() + _frameMS);
            if (frame < 0)
            {
                frame = 0;
            }
            logger_base.debug("Video effect loop #%d at frame %d to video frame %d.", _loops, buffer.curPeriod - buffer.curEffStartPer, frame);

            _videoreader->Seek(0);
			image = _videoreader->GetNextFrame(frame);
		}

		int startx = (buffer.BufferWi - _videoreader->GetWidth()) / 2;
		int starty = (buffer.BufferHt - _videoreader->GetHeight()) / 2;

		// check it looks valid
		if (image != nullptr)
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
            //logger_base.debug("Video render %s frame %d timestamp %ldms took %ldms.", (const char *)filename.c_str(), buffer.curPeriod, frame, sw.Time());
        }
		else
		{
			// display a blue background to show we have gone past end of video
			for (int y = 0; y < buffer.BufferHt; y++)
			{
				for (int x = 0; x < buffer.BufferWi; x++)
				{
					buffer.SetPixel(x, y, xlBLUE);
				}
			}
		}
	}
    else
    {
        // display a red background to show we have a problem
        for (int y = 0; y < buffer.BufferHt; y++)
        {
            for (int x = 0; x < buffer.BufferWi; x++)
            {
                buffer.SetPixel(x, y, xlRED);
            }
        }
    }
}
