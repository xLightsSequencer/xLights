/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

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
#include "../ExternalHooks.h"

#include <log4cpp/Category.hh>

VideoEffect::VideoEffect(int id) : RenderableEffect(id, "Video", video_16, video_24, video_32, video_48, video_64)
{
}

VideoEffect::~VideoEffect()
{
}

std::list<std::string> VideoEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res;

    wxString filename = settings.Get("E_FILEPICKERCTRL_Video_Filename", "");

    if (settings.GetBool("E_CHECKBOX_SynchroniseWithAudio", 0) == 1) {
        if (media != nullptr) {
            filename = media->FileName();
        }
    }

    if (filename == "" || !FileExists(filename))
    {
        res.push_back(wxString::Format("    ERR: Video effect video file '%s' does not exist. Model '%s', Start %s", filename, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }
    else
    {
        if (!IsFileInShowDir(xLightsFrame::CurrentDir, filename.ToStdString()))
        {
            res.push_back(wxString::Format("    WARN: Video effect video file '%s' not under show directory. Model '%s', Start %s", filename, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
        }

        VideoReader* videoreader = new VideoReader(filename.ToStdString(), 100, 100, false, true, true);
        if (videoreader == nullptr || videoreader->GetLengthMS() == 0)
        {
            res.push_back(wxString::Format("    ERR: Video effect video file '%s' could not be understood. Format may not be supported. Model '%s', Start %s", filename, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
        }
        else if (videoreader != nullptr)
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

            if (!renderCache)
            {
                int vh = videoreader->GetHeight();
                int vw = videoreader->GetHeight();

#define VIDEOSIZETHRESHOLD 10
                if (vh > VIDEOSIZETHRESHOLD * model->GetDefaultBufferHt() || vw > VIDEOSIZETHRESHOLD * model->GetDefaultBufferWi())
                {
                    float scale = std::max((float)vh / model->GetDefaultBufferHt(), (float)vw / model->GetDefaultBufferWi());
                    res.push_back(wxString::Format("    WARN: Video effect video file '%s' is %.1f times the height or width of the model ... xLights is going to need to do lots of work to resize the video. Model '%s', Start %s", filename, scale, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
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
    wxString camera = settings.Get("B_CHOICE_PerPreviewCamera", "2D");
    int w, h;
    model->GetBufferSize(bufferstyle.ToStdString(), camera.ToStdString(), transform.ToStdString(), w, h, settings.GetInt("B_SPINCTRL_BufferStagger", 0));

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

xlEffectPanel *VideoEffect::CreatePanel(wxWindow *parent) {
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

    if (file != "") {
        if (!FileExists(file, false)) {
            settings["E_FILEPICKERCTRL_Video_Filename"] = FixFile("", file);
        }
    }

    if (settings.Contains("E_SLIDER_Video_Starttime")) {
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
    vp->BitmapButton_Video_CropLeftVC->SetActive(false);
    vp->BitmapButton_Video_CropRightVC->SetActive(false);
    vp->BitmapButton_Video_CropTopVC->SetActive(false);
    vp->BitmapButton_Video_CropBottomVC->SetActive(false);
    vp->BitmapButton_Video_Speed->SetActive(false);
    SetCheckBoxValue(vp->CheckBox_Video_AspectRatio, false);
    SetChoiceValue(vp->Choice_Video_DurationTreatment, "Normal");

    SetCheckBoxValue(vp->CheckBox_TransparentBlack, false);
    SetSliderValue(vp->Slider1, 0);
}

std::list<std::string> VideoEffect::GetFileReferences(Model* model, const SettingsMap &SettingsMap) const
{
    std::list<std::string> res;
    if (SettingsMap["E_FILEPICKERCTRL_Video_Filename"] != "") {
        res.push_back(SettingsMap["E_FILEPICKERCTRL_Video_Filename"]);
    }
    return res;
}

bool VideoEffect::CleanupFileLocations(xLightsFrame* frame, SettingsMap &SettingsMap)
{
    bool rc = false;
    wxString file = SettingsMap["E_FILEPICKERCTRL_Video_Filename"];
    if (FileExists(file))
    {
        if (!frame->IsInShowFolder(file))
        {
            SettingsMap["E_FILEPICKERCTRL_Video_Filename"] = frame->MoveToShowFolder(file, wxString(wxFileName::GetPathSeparator()) + "Videos");
            rc = true;
        }
    }

    return rc;
}

void VideoEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer)
{
    float offset = buffer.GetEffectTimeIntervalPosition();

    int cl = GetValueCurveInt("Video_CropLeft", 0, SettingsMap, offset, VIDEO_CROP_MIN, VIDEO_CROP_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int cr = GetValueCurveInt("Video_CropRight", 100, SettingsMap, offset, VIDEO_CROP_MIN, VIDEO_CROP_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int ct = GetValueCurveInt("Video_CropTop", 100, SettingsMap, offset, VIDEO_CROP_MIN, VIDEO_CROP_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int cb = GetValueCurveInt("Video_CropBottom", 0, SettingsMap, offset, VIDEO_CROP_MIN, VIDEO_CROP_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

    Render(buffer,
           SettingsMap["FILEPICKERCTRL_Video_Filename"],
           SettingsMap.GetDouble("TEXTCTRL_Video_Starttime", 0.0),
           std::min(cl, cr),
           std::max(cl, cr),
           std::max(ct, cb),
           std::min(ct, cb),
           SettingsMap.GetBool("CHECKBOX_Video_AspectRatio", false),
           SettingsMap.Get("CHOICE_Video_DurationTreatment", "Normal"),
           SettingsMap.GetBool("CHECKBOX_SynchroniseWithAudio", false),
           SettingsMap.GetBool("CHECKBOX_Video_TransparentBlack", false),
           SettingsMap.GetInt("TEXTCTRL_Video_TransparentBlack", 0),
           GetValueCurveDouble("Video_Speed", 1.0, SettingsMap, offset, VIDEO_SPEED_MIN, VIDEO_SPEED_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), VIDEO_SPEED_DIVISOR),
           SettingsMap.GetInt("TEXTCTRL_SampleSpacing", 0));
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
    double starttime, int cropLeft, int cropRight, int cropTop, int cropBottom, bool aspectratio, std::string durationTreatment, bool synchroniseAudio, bool transparentBlack, int transparentBlackLevel, double speed, uint32_t sampleSpacing)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (cropLeft > cropRight)
    {
        std::swap(cropLeft, cropRight);
    }

    if (cropBottom > cropTop)
    {
        std::swap(cropTop, cropBottom);
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
        else if (FileExists(filename))
        {
            // have to open the file
            int width = buffer.BufferWi * 100 / (cropRight - cropLeft);
            int height = buffer.BufferHt * 100 / (cropTop - cropBottom);

            bool useNativeResolution = (sampleSpacing > 0);

            _videoreader = new VideoReader(filename, width, height, aspectratio, useNativeResolution, true);

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

                // read the first frame ... if i dont it thinks the first frame i read is the first frame
                _videoreader->GetNextFrame(0);

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

    if (_videoreader != nullptr && sampleSpacing == 0) {
        int width = buffer.BufferWi * 100 / (cropRight - cropLeft);
        int height = buffer.BufferHt * 100 / (cropTop - cropBottom);
        bool vwidthEq = width == _videoreader->GetWidth();
        bool vheightEq = height == _videoreader->GetHeight();
        if (aspectratio) {
            // if aspect ratio scaling, then only one or the other will be equal
            vwidthEq = vheightEq | vwidthEq;
            vheightEq = vwidthEq;
        }
        if (!vwidthEq || !vheightEq) {
            // need to close and reopen video reader to the new size ... this is inefficient ... but lots of work to do to change video reader size dynamically
            delete _videoreader;
            _videoreader = new VideoReader(filename, width, height, aspectratio, false, true);
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

            int ch = _videoreader->GetPixelChannels();

            // check it looks valid
            if (image != nullptr && frame >= 0) {

                // This handles normal scaling of videos
                if (sampleSpacing == 0) {
                    int xoffset = cropLeft * _videoreader->GetWidth() / 100;
                    int yoffset = cropBottom * _videoreader->GetHeight() / 100;
                    int xtail = (100 - cropRight) * _videoreader->GetWidth() / 100;
                    int ytail = (100 - cropTop) * _videoreader->GetHeight() / 100;
                    int startx = (buffer.BufferWi - _videoreader->GetWidth() * (cropRight - cropLeft) / 100) / 2;
                    int starty = (buffer.BufferHt - _videoreader->GetHeight() * (cropTop - cropBottom) / 100) / 2;

                    // wxASSERT(xoffset + xtail + buffer.BufferWi == _videoreader->GetWidth());
                    // wxASSERT(yoffset + ytail + buffer.BufferHt == _videoreader->GetHeight());

                    // draw the image
                    xlColor c;
                    for (int y = 0; y < _videoreader->GetHeight() - yoffset - ytail; y++) {
                        uint8_t* ptr = image->data[0] + (_videoreader->GetHeight() - 1 - y - yoffset) * _videoreader->GetWidth() * ch + xoffset * ch;

                        for (int x = 0; x < _videoreader->GetWidth() - xoffset - xtail; x++) {
                            try {
                                c.Set(*(ptr),
                                      *(ptr + 1),
                                      *(ptr + 2), ch == 3 ? 255 : *(ptr + 3));
                            } catch (...) {
                                // this shouldnt happen so make it stand out
                                c = xlRED;
                            }

                            if (transparentBlack) {
                                int level = c.Red() + c.Green() + c.Blue();
                                if (level > transparentBlackLevel) {
                                    buffer.SetPixel(x + startx, y + starty, c);
                                }
                            } else {
                                buffer.SetPixel(x + startx, y + starty, c);
                            }

                            ptr += ch;
                        }
                    }
                    // logger_base.debug("Video render %s frame %d timestamp %ldms took %ldms.", (const char *)filename.c_str(), buffer.curPeriod, frame, sw.Time());
                }
                else
                {
                    // this handles video sampling where we sample pixels from the image rather than scaling it and thus washing out the colours
                    int xneeded = buffer.BufferWi * sampleSpacing;
                    int yneeded = buffer.BufferHt * sampleSpacing;
                    int imageWidthAfterCropping = ((100 - cropLeft + cropRight) * image->width) / 200;
                    int imageHeightAferCropping = ((cropTop + 100 - cropBottom) * image->width) / 200;
                    int startx = imageWidthAfterCropping / 2 - xneeded / 2 + (cropLeft * image->width) / 100;
                    int starty = imageHeightAferCropping / 2 - yneeded / 2 + ((100 - cropTop) * image->height) / 100;

                    int cury = starty;
                    xlColor c;
                    for (int y = 0; y < buffer.BufferHt; ++y)
                    {
                        if (cury >= 0 && cury < image->height)
                        {
                            int curx = startx;
                            for (int x = 0; x < buffer.BufferWi; ++x) {
                                if (curx >= 0 && curx < image->width) {
                                    uint8_t* ptr = image->data[0] + (_videoreader->GetHeight() - 1 - cury) * _videoreader->GetWidth() * ch + curx * ch;
                                    try {
                                        c.Set(*(ptr),
                                              *(ptr + 1),
                                              *(ptr + 2), ch == 3 ? 255 : *(ptr + 3));
                                    } catch (...) {
                                        // this shouldnt happen so make it stand out
                                        c = xlRED;
                                    }

                                    buffer.SetPixel(x, y, c);
                                }
                                curx += sampleSpacing;
                            }
                        }
                        cury += sampleSpacing;
                    }
                }
            } else {
                if (durationTreatment == "Normal") {
                    // display a blue background to show we have gone past end of video
                    for (int y = 0; y < buffer.BufferHt; y++) {
                        for (int x = 0; x < buffer.BufferWi; x++) {
                            buffer.SetPixel(x, y, xlBLUE);
                        }
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
