/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include <wx/mediactrl.h>   //for wxMediaCtrl

#include "PlayListItemVideo.h"
#include "PlayListItemVideoPanel.h"
#include "../VideoCache.h"
#include "../../xLights/VideoReader.h"
#include "../xScheduleApp.h"
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"
#include "../../xLights/UtilFunctions.h"
#include "PlayerFrame.h"
#include "PlayerWindow.h"

#include <log4cpp/Category.hh>

PlayListItemVideo::PlayListItemVideo(wxXmlNode* node) : PlayListItem(node)
{
    _useMediaPlayer = false;
    _fadeInMS = 0;
    _fadeOutMS = 0;
    _cacheVideo = false;
    _loopVideo = false;
    _videoReader = nullptr;
    _cachedVideoReader = nullptr;
    _topMost = true;
    _suppressVirtualMatrix = false;
    _window = nullptr;
    _frame = nullptr;
    _videoFile = "";
    _origin.x = 0;
    _origin.y = 0;
    _size.SetWidth(300);
    _size.SetHeight(300);
    _durationMS = 0;
    PlayListItemVideo::Load(node);
}

PlayListItemVideo::~PlayListItemVideo()
{
    CloseFiles();

    if (_window != nullptr) {
        delete _window;
        _window = nullptr;
    }
    if (_frame != nullptr) {
        delete _frame;
        _frame = nullptr;
    }
}

void PlayListItemVideo::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _videoFile = node->GetAttribute("VideoFile", "");
    _videoFile = FixFile("", _videoFile);
    _origin = wxPoint(wxAtoi(node->GetAttribute("X", "0")), wxAtoi(node->GetAttribute("Y", "0")));
    _size = wxSize(wxAtoi(node->GetAttribute("W", "100")), wxAtoi(node->GetAttribute("H", "100")));
    _fadeInMS = wxAtoi(node->GetAttribute("FadeInMS", "0"));
    _fadeOutMS = wxAtoi(node->GetAttribute("FadeOutMS", "0"));
    _topMost = (node->GetAttribute("Topmost", "TRUE") == "TRUE");
    _cacheVideo = (node->GetAttribute("CacheVideo", "FALSE") == "TRUE");
    _loopVideo = (node->GetAttribute("LoopVideo", "FALSE") == "TRUE");
    _suppressVirtualMatrix = (node->GetAttribute("SuppressVM", "FALSE") == "TRUE");
    _durationMS = VideoReader::GetVideoLength(_videoFile);
    _useMediaPlayer = (node->GetAttribute("UseMediaPlayer", "FALSE") == "TRUE");
}

PlayListItemVideo::PlayListItemVideo() : PlayListItem()
{
    _type = "PLIVideo";
    _fadeInMS = 0;
    _fadeOutMS = 0;
    _cacheVideo = false;
    _loopVideo = false;
    _useMediaPlayer = false;
    _videoReader = nullptr;
    _cachedVideoReader = nullptr;
    _topMost = true;
    _suppressVirtualMatrix = false;
    _window = nullptr;
    _frame = nullptr;
    _videoFile = "";
    _origin.x = 0;
    _origin.y = 0;
    _durationMS = 0;
    _size.SetWidth(300);
    _size.SetHeight(300);
}

PlayListItem* PlayListItemVideo::Copy() const
{
    PlayListItemVideo* res = new PlayListItemVideo();
    res->_origin = _origin;
    res->_fadeInMS = _fadeInMS;
    res->_fadeOutMS = _fadeOutMS;
    res->_size = _size;
    res->_videoFile = _videoFile;
    res->_durationMS = _durationMS;
    res->_topMost = _topMost;
    res->_cacheVideo = _cacheVideo;
    res->_loopVideo = _loopVideo;
    res->_suppressVirtualMatrix = _suppressVirtualMatrix;
    res->_useMediaPlayer = _useMediaPlayer;
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemVideo::Save()
{
    wxXmlNode* node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, GetType());

    node->AddAttribute("VideoFile", _videoFile);
    node->AddAttribute("X", wxString::Format(wxT("%i"), _origin.x));
    node->AddAttribute("Y", wxString::Format(wxT("%i"), _origin.y));
    node->AddAttribute("W", wxString::Format(wxT("%i"), _size.GetWidth()));
    node->AddAttribute("H", wxString::Format(wxT("%i"), _size.GetHeight()));
    node->AddAttribute("FadeInMS", wxString::Format(wxT("%i"), _fadeInMS));
    node->AddAttribute("FadeOutMS", wxString::Format(wxT("%i"), _fadeOutMS));

    if (!_topMost) {
        node->AddAttribute("Topmost", "FALSE");
    }

    if (_cacheVideo) {
        node->AddAttribute("CacheVideo", "TRUE");
    }

    if (_useMediaPlayer) {
        node->AddAttribute("UseMediaPlayer", "TRUE");
    }

    if (_loopVideo) {
        node->AddAttribute("LoopVideo", "TRUE");
    }

    if (_suppressVirtualMatrix) {
        node->AddAttribute("SuppressVM", "TRUE");
    }

    PlayListItem::Save(node);

    return node;
}

void PlayListItemVideo::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemVideoPanel(notebook, this), GetTitle(), true);
}

std::string PlayListItemVideo::GetTitle() const
{
    return "Video";
}

std::string PlayListItemVideo::GetNameNoTime() const
{
    wxFileName fn(_videoFile);
    if (fn.GetName() == "") {
        return "Video";
    }
    else {
        return fn.GetName().ToStdString();
    }
}

size_t PlayListItemVideo::GetDurationMS() const
{
    return _delay + _durationMS;
}

void PlayListItemVideo::SetVideoFile(const std::string& videoFile)
{
    if (_videoFile != videoFile) {
        _videoFile = videoFile;
        OpenFiles(false);
        CloseFiles();
        _changeCount++;
    }
}

void PlayListItemVideo::CloseFiles()
{
    if (_videoReader != nullptr) {
        delete _videoReader;
        _videoReader = nullptr;
    }

    if (_cachedVideoReader != nullptr) {
        delete _cachedVideoReader;
        _cachedVideoReader = nullptr;
    }
}

void PlayListItemVideo::OpenFiles(bool doCache)
{
    CloseFiles();

    if (_cacheVideo && doCache && !_useMediaPlayer) {
        _cachedVideoReader = new CachedVideoReader(_videoFile, 0, GetFrameMS(), _size, false);
        _durationMS = _cachedVideoReader->GetLengthMS();
    }
    else {
        _videoReader = new VideoReader(_videoFile, _size.GetWidth(), _size.GetHeight(), false); // , true);
        _durationMS = _videoReader->GetLengthMS();
        if (_useMediaPlayer) {
            delete _videoReader;
            _videoReader = nullptr;
        }
    }
}

// Maximum milliseconds a media player file can be out of sync with the sequence
#define MAXMEDIAJITTER (3 * framems)

void PlayListItemVideo::Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    //logger_base.debug("Video rendering frame %ld for video %s.", (long)ms, (const char *)GetNameNoTime().c_str());

    if (ms < _delay) {
        // dont display anything
    }
    else {
        wxStopWatch sw;

        long adjustedMS = ms - _delay;

        if (_useMediaPlayer && _frame != nullptr) {
            bool videoOver = (!_loopVideo && adjustedMS > _durationMS);

            if (!videoOver && _frame->GetState() != wxMEDIASTATE_PLAYING) {
                _frame->Play();
            }

            long mediapos = adjustedMS;
            if (_loopVideo && _durationMS != 0) {
                mediapos = mediapos % _durationMS;
                // loop early to try and prevent black screen
                if (mediapos > _durationMS - framems) {
                    //logger_base.debug("Looping early");
                    mediapos = 0;
                }
            }

            if (videoOver) {
                _frame->Stop();
            }
            else {
                long videopos = _frame->Tell();
                long jitter = std::abs(videopos - mediapos);
                if (jitter > MAXMEDIAJITTER) {
                    _frame->Seek(mediapos);
                    //logger_base.debug("Sequence pos %ld, Desired video pos %ld, Video pos %ld, Jitter %ld, Length %ld, %s", adjustedMS, mediapos, videopos, jitter, (long)_durationMS, (const char*)_videoFile.c_str());
                }
            }
        }
        else {
            int brightness = 100;
            if (_fadeInMS != 0 && adjustedMS < _fadeInMS) {
                brightness = (float)adjustedMS * 100.0 / (float)_fadeInMS;
            }
            else if (_fadeOutMS != 0 && _stepLengthMS - adjustedMS < _fadeOutMS) {
                brightness = (float)(_stepLengthMS - adjustedMS) * 100.0 / (float)_fadeOutMS;
            }

            if (_cacheVideo) {
                if (_cachedVideoReader != nullptr) {
                    auto videoLength = _cachedVideoReader->GetLengthMS();
                    while (_loopVideo && adjustedMS > videoLength && videoLength > 0) {
                        adjustedMS -= videoLength;
                    }

                    if (_window != nullptr) _window->SetImage(CachedVideoReader::FadeImage(_cachedVideoReader->GetNextFrame(adjustedMS), brightness));
                }
            }
            else {
                if (_videoReader != nullptr) {
                    auto videoLength = _videoReader->GetLengthMS();
                    while (_loopVideo && adjustedMS > videoLength && videoLength > 0) {
                        adjustedMS -= videoLength;
                    }

                    AVFrame* img = _videoReader->GetNextFrame(adjustedMS, framems);
                    if (_window != nullptr) _window->SetImage(CachedVideoReader::FadeImage(CachedVideoReader::CreateImageFromFrame(img, _size), brightness));
                }
            }
        }
        if (sw.Time() > framems / 2) {
            logger_base.warn("   Getting frame %ld from video %s took more than half a frame: %ld.", (long)ms - _delay, (const char*)GetNameNoTime().c_str(), (long)sw.Time());
        }
        //logger_base.debug("   Done rendering frame %ld for video %s.", (long)ms - _delay, (const char *)GetNameNoTime().c_str());
    }
}

void PlayListItemVideo::Start(long stepLengthMS)
{
    PlayListItem::Start(stepLengthMS);

    if (_suppressVirtualMatrix) {
        xScheduleFrame::GetScheduleManager()->SuppressVM(true);
    }

    OpenFiles(true);

    if (_useMediaPlayer) {
        if (_frame == nullptr) {
            _frame = new PlayerFrame(wxGetApp().GetTopWindow(), _topMost, wxID_ANY, _origin, _size);
            bool res = _frame->Load(_videoFile);
            wxASSERT(res);
            _frame->Show();
        }
        else {
            _frame->Move(_origin);
            _frame->SetSize(_size);
            _frame->Show();
        }
    }
    else {
        // create the window
        if (_window == nullptr) {
            _window = new PlayerWindow(wxGetApp().GetTopWindow(), _topMost, wxIMAGE_QUALITY_BILINEAR /*wxIMAGE_QUALITY_HIGH*/, -1, wxID_ANY, _origin, _size);
        }
        else {
            _window->Move(_origin);
            _window->SetSize(_size);
        }
    }
}

void PlayListItemVideo::Stop()
{
    if (_suppressVirtualMatrix) {
        xScheduleFrame::GetScheduleManager()->SuppressVM(false);
    }

    CloseFiles();

    // destroy the window
    if (_window != nullptr) {
        delete _window;
        _window = nullptr;
    }

    if (_frame != nullptr) {
        _frame->Stop();
        delete _frame;
        _frame = nullptr;
    }
}

void PlayListItemVideo::Suspend(bool suspend)
{
    if (suspend) {
        if (_window != nullptr) _window->Hide();
        if (_frame != nullptr) {
            _frame->Pause();
            _frame->Hide();
        }
    }
    else {
        if (_window != nullptr) _window->Show();
        if (_frame != nullptr) {
            _frame->Play();
            _frame->Show();
        }
    }
}

void PlayListItemVideo::Pause(bool pause)
{
    if (_frame != nullptr) {
        if (pause) {
            _frame->Pause();
        }
        else {
            _frame->Play();
        }
    }
}

bool PlayListItemVideo::IsVideo(const std::string& ext)
{
    if (ext == "avi" ||
        ext == "mp4" ||
        ext == "mkv" ||
        ext == "mov" ||
        ext == "asf" ||
        ext == "flv" ||
        ext == "mpg" ||
        ext == "mpeg" ||
        ext == "m4v"
        ) {
        return true;
    }

    return false;
}

std::list<std::string> PlayListItemVideo::GetMissingFiles()
{
    std::list<std::string> res;
    if (!wxFile::Exists(GetVideoFile())) {
        res.push_back(GetVideoFile());
    }

    return res;
}