#include "../../include/video-16.xpm"
#include "../../include/video-24.xpm"
#include "../../include/video-32.xpm"
#include "../../include/video-48.xpm"
#include "../../include/video-64.xpm"

#include "VideoEffect.h"
#include "VideoPanel.h"
#include "../VideoReader.h"
#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../xLightsXmlFile.h"
#include "../xLightsMain.h" 
#include "../models/Model.h"
#include "../UtilFunctions.h"

#include <log4cpp/Category.hh>

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

    if (filename == "" || !wxFileExists(filename))
    {
        res.push_back(wxString::Format("    ERR: Video effect video file '%s' does not exist. Model '%s', Start %s", filename, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }
    else
    {
        if (!IsFileInShowDir(xLightsFrame::CurrentDir, filename.ToStdString()))
        {
            res.push_back(wxString::Format("    WARN: Video effect video file '%s' not under show directory. Model '%s', Start %s", filename, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
        }

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

bool VideoEffect::IsVideoFile(std::string filename)
{
    return VideoReader::IsVideoFile(filename);
}

wxPanel *VideoEffect::CreatePanel(wxWindow *parent) {
    return new VideoPanel(parent);
}

void VideoEffect::adjustSettings(const std::string &version, Effect *effect, bool removeDefaults)
{
    // give the base class a chance to adjust any settings
    if (RenderableEffect::needToAdjustSettings(version))
    {
        RenderableEffect::adjustSettings(version, effect, removeDefaults);
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
            settings["E_FILEPICKERCTRL_Video_Filename"] = FixFile("", file);
        }
    }

    if (settings.Contains("E_SLIDER_Video_Starttime"))
    {
        settings.erase("E_SLIDER_Video_Starttime");
        //long st = wxAtol(settings["E_SLIDER_Video_Starttime"]);
        //settings["E_SLIDER_Video_Starttime"] = wxString::Format(wxT("%i"), st / 10);
    }
}

void VideoEffect::SetDefaultParameters()
{
    VideoPanel *vp = (VideoPanel*)panel;
    if (vp == nullptr) {
        return;
    }

    vp->FilePicker_Video_Filename->SetFileName(wxFileName());
    SetSliderValue(vp->Slider_Video_Starttime, 0);
    SetSliderValue(vp->Slider_Video_CropBottom, 0);
    SetSliderValue(vp->Slider_Video_CropLeft, 0);
    SetSliderValue(vp->Slider_Video_CropRight, 100);
    SetSliderValue(vp->Slider_Video_CropTop, 100);
    SetCheckBoxValue(vp->CheckBox_Video_AspectRatio, false);
    SetChoiceValue(vp->Choice_Video_DurationTreatment, "Normal");
}

std::list<std::string> VideoEffect::GetFileReferences(const SettingsMap &SettingsMap)
{
    std::list<std::string> res;
    res.push_back(SettingsMap["E_FILEPICKERCTRL_Video_Filename"]);
    return res;
}

void VideoEffect::Render(Effect *effect, SettingsMap &SettingsMap, RenderBuffer &buffer) {
    float offset = buffer.GetEffectTimeIntervalPosition();

    Render(buffer,
		   SettingsMap["FILEPICKERCTRL_Video_Filename"],
		SettingsMap.GetDouble("TEXTCTRL_Video_Starttime", 0.0),
		std::min(SettingsMap.GetInt("TEXTCTRL_Video_CropLeft", 0), SettingsMap.GetInt("TEXTCTRL_Video_CropRight", 100)),
        std::max(SettingsMap.GetInt("TEXTCTRL_Video_CropLeft", 0), SettingsMap.GetInt("TEXTCTRL_Video_CropRight", 100)),
        std::max(SettingsMap.GetInt("TEXTCTRL_Video_CropTop", 100), SettingsMap.GetInt("TEXTCTRL_Video_CropBottom", 0)),
        std::min(SettingsMap.GetInt("TEXTCTRL_Video_CropTop", 100), SettingsMap.GetInt("TEXTCTRL_Video_CropBottom", 0)),
        SettingsMap.GetBool("CHECKBOX_Video_AspectRatio", false),
		SettingsMap.Get("CHOICE_Video_DurationTreatment", "Normal"),
        SettingsMap.GetBool("CHECKBOX_SynchroniseWithAudio", false),
        SettingsMap.GetBool("CHECKBOX_Video_TransparentBlack", false),
        SettingsMap.GetInt("TEXTCTRL_Video_TransparentBlack", 0),
        GetValueCurveDouble("Video_Speed", 1.0, SettingsMap, offset, VIDEO_SPEED_MIN, VIDEO_SPEED_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), VIDEO_SPEED_DIVISOR)
		);
}

class VideoRenderCache : public EffectRenderCache {
public:
    VideoRenderCache()
	{
		_videoframerate = -1;
		_videoreader = nullptr;
        _loops = 0;
        _frameMS = 50;
        _nextManualMS = 0;
	};
    virtual ~VideoRenderCache() {
		if (_videoreader != nullptr)
		{
			delete _videoreader;
			_videoreader = nullptr;
		}
	};

    VideoReader* _videoreader;
	int _videoframerate;
	int _loops;
    int _frameMS;
    int _nextManualMS = 0;
};

void VideoEffect::Render(RenderBuffer &buffer, std::string filename,
    double starttime, int cropLeft, int cropRight, int cropTop, int cropBottom, bool aspectratio, std::string durationTreatment, bool synchroniseAudio, bool transparentBlack, int transparentBlackLevel, double speed)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (cropLeft > cropRight)
    {
        auto temp = cropLeft;
        cropLeft = cropRight;
        cropRight = temp;
    }

    if (cropBottom > cropTop)
    {
        auto temp = cropBottom;
        cropBottom = cropTop;
        cropTop = temp;
    }

    if (cropLeft == cropRight)
    {
        if (cropLeft == 0)
        {
            cropRight++;
        }
        else
        {
            cropLeft--;
        }
    }
    if (cropBottom == cropTop)
    {
        if (cropBottom == 0)
        {
            cropTop++;
        }
        else
        {
            cropBottom--;
        }
    }

    VideoRenderCache *cache = (VideoRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new VideoRenderCache();
        buffer.infoCache[id] = cache;
    }

    int &_loops = cache->_loops;
    VideoReader* &_videoreader = cache->_videoreader;
    int& _frameMS = cache->_frameMS;
    int& _nextManualMS = cache->_nextManualMS;

    if (synchroniseAudio)
    {
        starttime = 0;
        durationTreatment = "Normal";
        if (buffer.GetMedia() != nullptr)
        {
            filename = buffer.GetMedia()->FileName();
            starttime = (double)buffer.curEffStartPer * (double)buffer.frameTimeInMs / 1000.0;
        }
    }

    // we always reopen video on first frame or if it is not open or if the filename has changed
    if (buffer.needToInit)
    {
        buffer.needToInit = false;

        _loops = 0;
        _nextManualMS = 0;
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
        else if (wxFileExists(filename))
        {
            // have to open the file
            int width = buffer.BufferWi * 100 / (cropRight - cropLeft);
            int height = buffer.BufferHt * 100 / (cropTop - cropBottom);
            _videoreader = new VideoReader(filename, width, height, aspectratio);

            if (_videoreader == nullptr)
            {
                logger_base.warn("VideoEffect: Failed to load video file %s.", (const char *)filename.c_str());
            }
            else
            {
                // extract the video length
                int videolen = _videoreader->GetLengthMS();

                if (videolen == 0)
                {
                    logger_base.warn("VideoEffect: Video %s was read as 0 length.", (const char *)filename.c_str());
                }

                VideoPanel *fp = static_cast<VideoPanel*>(panel);
                if (fp != nullptr)
                {
                    wxCommandEvent event(EVT_VIDEODETAILS);
                    event.SetInt(videolen);
                    event.SetString(filename);
                    wxPostEvent(fp, event);
                    //fp->addVideoTime(filename, videolen);
                }

                if (starttime != 0)
                {
                    logger_base.debug("Video effect initialising ... seeking to start location for the video %f.", (float)starttime);
                    _videoreader->Seek(starttime * 1000);
                }

                if (durationTreatment == "Slow/Accelerate")
                {
                    int effectFrames = buffer.curEffEndPer - buffer.curEffStartPer + 1;
                    int videoFrames = (videolen - (starttime * 1000)) / buffer.frameTimeInMs;
                    float speedFactor = (float)videoFrames / (float)effectFrames;
                    _frameMS = (int)((float)buffer.frameTimeInMs * speedFactor);
                }
                logger_base.debug("Video effect length: %d, video length: %d, startoffset: %f, duration treatment: %s.",
                    (buffer.curEffEndPer - buffer.curEffStartPer + 1) * _frameMS, videolen, (float)starttime,
                    (const char *)durationTreatment.c_str());
            }
        }
        else
        {
            if (buffer.curPeriod == buffer.curEffStartPer)
            {
                logger_base.warn("VideoEffect: Video file '%s' not found.", (const char *)filename.c_str());
            }
        }
    }

    if (_videoreader != nullptr && _videoreader->GetLengthMS() > 0)
    {
        long frame = 0;
        
        if (durationTreatment == "Manual")
        {
            frame = starttime * 1000 + _nextManualMS;
            _nextManualMS += speed * _frameMS;
        }
        else if (durationTreatment == "Manual and Loop")
        {
            frame = starttime * 1000 + _nextManualMS;

            while (frame < 0)
            {
                frame += _videoreader->GetLengthMS();
            }

            while (frame > _videoreader->GetLengthMS())
            {
                frame -= _videoreader->GetLengthMS();
            }

            _nextManualMS += speed * _frameMS;
        }
        else
        {
            frame = starttime * 1000 + (buffer.curPeriod - buffer.curEffStartPer) * _frameMS - _loops * (_videoreader->GetLengthMS() + _frameMS);
        }

        // get the image for the current frame
        AVFrame* image = _videoreader->GetNextFrame(frame);

        // if we have reached the end and we are to loop
        if (_videoreader->AtEnd() && durationTreatment == "Loop")
        {
            // jump back to start and try to read frame again
            _loops++;
            frame = starttime * 1000 + (buffer.curPeriod - buffer.curEffStartPer) * _frameMS - _loops * (_videoreader->GetLengthMS() + _frameMS);
            if (frame < 0)
            {
                frame = 0;
            }
            logger_base.debug("Video effect loop #%d at frame %d to video frame %d.", _loops, buffer.curPeriod - buffer.curEffStartPer, frame);

            _videoreader->Seek(0);
            image = _videoreader->GetNextFrame(frame);
        }

        int xoffset = cropLeft * _videoreader->GetWidth() / 100;
        int yoffset = cropBottom * _videoreader->GetHeight() / 100;
        int xtail = (100 - cropRight) * _videoreader->GetWidth() / 100;
        int ytail = (100 - cropTop) * _videoreader->GetHeight() / 100;
        int startx = (buffer.BufferWi - _videoreader->GetWidth() * (cropRight - cropLeft) / 100) / 2;
        int starty = (buffer.BufferHt - _videoreader->GetHeight() * (cropTop - cropBottom) / 100) / 2;

        //wxASSERT(xoffset + xtail + buffer.BufferWi == _videoreader->GetWidth());
        //wxASSERT(yoffset + ytail + buffer.BufferHt == _videoreader->GetHeight());

        // check it looks valid
        if (image != nullptr && frame >= 0)
        {
            // draw the image
            xlColor c;
            for (int y = 0; y < _videoreader->GetHeight() - yoffset - ytail; y++)
            {
                uint8_t* ptr = image->data[0] + (_videoreader->GetHeight() - 1 - y - yoffset) * _videoreader->GetWidth() * 3 + xoffset * 3;

                for (int x = 0; x < _videoreader->GetWidth() - xoffset - xtail; x++)
                {
                    try
                    {
                        c.Set(*(ptr),
                            *(ptr + 1),
                            *(ptr + 2), 255);
                    }
                    catch (...)
                    {
                        // this shouldnt happen so make it stand out
                        c = xlRED;
                    }

                    if (transparentBlack)
                    {
                        int level = c.Red() + c.Green() + c.Blue();
                        if (level > transparentBlackLevel)
                        {
                            buffer.SetPixel(x + startx, y + starty, c);
                        }
                    }
                    else
                    {
                        buffer.SetPixel(x + startx, y + starty, c);
                    }

                    ptr += 3;
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
