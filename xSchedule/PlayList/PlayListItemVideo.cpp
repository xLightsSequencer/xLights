#include "PlayListItemVideo.h"
#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include "PlayListItemVideoPanel.h"
#include "../VideoCache.h"
#include "../../xLights/VideoReader.h"
#include "PlayerWindow.h"
#include <log4cpp/Category.hh>
#include "../xScheduleApp.h"
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"
#include "../../xLights/UtilFunctions.h"

PlayListItemVideo::PlayListItemVideo(wxXmlNode* node) : PlayListItem(node)
{
    _fadeInMS = 0;
    _fadeOutMS = 0;
    _cacheVideo = false;
    _loopVideo = false;
    _videoReader = nullptr;
    _cachedVideoReader = nullptr;
    _topMost = true;
    _suppressVirtualMatrix = false;
    _window = nullptr;
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

    if (_window != nullptr)
    {
        delete _window;
        _window = nullptr;
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
    OpenFiles(false);
    CloseFiles();
}

PlayListItemVideo::PlayListItemVideo() : PlayListItem()
{
    _fadeInMS = 0;
    _fadeOutMS = 0;
    _cacheVideo = false;
    _loopVideo = false;
    _videoReader = nullptr;
    _cachedVideoReader = nullptr;
    _topMost = true;
    _suppressVirtualMatrix = false;
    _window = nullptr;
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
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemVideo::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLIVideo");

    node->AddAttribute("VideoFile", _videoFile);
    node->AddAttribute("X", wxString::Format(wxT("%i"), _origin.x));
    node->AddAttribute("Y", wxString::Format(wxT("%i"), _origin.y));
    node->AddAttribute("W", wxString::Format(wxT("%i"), _size.GetWidth()));
    node->AddAttribute("H", wxString::Format(wxT("%i"), _size.GetHeight()));
    node->AddAttribute("FadeInMS", wxString::Format(wxT("%i"), _fadeInMS));
    node->AddAttribute("FadeOutMS", wxString::Format(wxT("%i"), _fadeOutMS));

    if (!_topMost)
    {
        node->AddAttribute("Topmost", "FALSE");
    }

    if (_cacheVideo)
    {
        node->AddAttribute("CacheVideo", "TRUE");
    }

    if (_loopVideo)
    {
        node->AddAttribute("LoopVideo", "TRUE");
    }

    if (_suppressVirtualMatrix)
    {
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
    if (fn.GetName() == "")
    {
        return "Video";
    }
    else
    {
        return fn.GetName().ToStdString();
    }
}

size_t PlayListItemVideo::GetDurationMS() const
{
    return _delay + _durationMS;
}

void PlayListItemVideo::SetVideoFile(const std::string& videoFile) 
{ 
    if (_videoFile != videoFile)
    {
        _videoFile = videoFile;
        OpenFiles(false);
        CloseFiles();
        _changeCount++;
    }
}

void PlayListItemVideo::CloseFiles()
{
    if (_videoReader != nullptr)
    {
        delete _videoReader;
        _videoReader = nullptr;
    }

    if (_cachedVideoReader != nullptr)
    {
        delete _cachedVideoReader;
        _cachedVideoReader = nullptr;
    }
}

void PlayListItemVideo::OpenFiles(bool doCache)
{
    CloseFiles();

    if (_cacheVideo && doCache)
    {
        _cachedVideoReader = new CachedVideoReader(_videoFile, 0, GetFrameMS(), _size, false);
        _durationMS = _cachedVideoReader->GetLengthMS();
    }
    else
    {
        _videoReader = new VideoReader(_videoFile, _size.GetWidth(), _size.GetHeight(), false);
        _durationMS = _videoReader->GetLengthMS();
    }
}

void PlayListItemVideo::Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    //logger_base.debug("Video rendering frame %ld for video %s.", (long)ms, (const char *)GetNameNoTime().c_str());

    if (ms < _delay)
    {
        // dont display anything
    }
    else
    {
        wxStopWatch sw;

        long adjustedMS = ms - _delay;

        int brightness = 100;
        if (_fadeInMS != 0 && adjustedMS < _fadeInMS)
        {
            brightness = (float)adjustedMS * 100.0 / (float)_fadeInMS;
        }
        else if (_fadeOutMS != 0 && _stepLengthMS - adjustedMS < _fadeOutMS)
        {
            brightness = (float)(_stepLengthMS - adjustedMS) * 100.0 / (float)_fadeOutMS;
        }

        if (_cacheVideo)
        {
            if (_cachedVideoReader != nullptr)
            {
                while (_loopVideo && adjustedMS > _cachedVideoReader->GetLengthMS())
                {
                    adjustedMS -= _cachedVideoReader->GetLengthMS();
                }

                _window->SetImage(CachedVideoReader::FadeImage(_cachedVideoReader->GetNextFrame(adjustedMS), brightness));
            }
        }
        else
        {
            if (_videoReader != nullptr)
            {
                while (_loopVideo && adjustedMS > _videoReader->GetLengthMS())
                {
                    adjustedMS -= _videoReader->GetLengthMS();
                }

                AVFrame* img = _videoReader->GetNextFrame(adjustedMS, framems);
                _window->SetImage(CachedVideoReader::FadeImage(CachedVideoReader::CreateImageFromFrame(img, _size), brightness));
            }
        }

        if (sw.Time() > framems / 2)
        {
            logger_base.warn("   Getting frame %ld from video %s took more than half a frame: %ld.", (long)ms - _delay, (const char *)GetNameNoTime().c_str(), (long)sw.Time());
        }

        //logger_base.debug("   Done rendering frame %ld for video %s.", (long)ms - _delay, (const char *)GetNameNoTime().c_str());
    }
}

void PlayListItemVideo::Start(long stepLengthMS)
{
    PlayListItem::Start(stepLengthMS);

    if (_suppressVirtualMatrix)
    {
        xScheduleFrame::GetScheduleManager()->SuppressVM(true);
    }

    OpenFiles(true);

    // create the window
    if (_window == nullptr)
    {
        _window = new PlayerWindow(wxGetApp().GetTopWindow(), _topMost, wxIMAGE_QUALITY_HIGH, -1, wxID_ANY, _origin, _size);
    }
    else
    {
        _window->Move(_origin);
        _window->SetSize(_size);
    }
}

void PlayListItemVideo::Stop()
{
    if (_suppressVirtualMatrix)
    {
        xScheduleFrame::GetScheduleManager()->SuppressVM(false);
    }

    CloseFiles();

    // destroy the window
    if (_window != nullptr)
    {
        delete _window;
        _window = nullptr;
    }
}

void PlayListItemVideo::Suspend(bool suspend)
{
    if (suspend)
    {
        if (_window != nullptr) _window->Hide();
    }
    else
    {
        if (_window != nullptr) _window->Show();
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
        )
    {
        return true;
    }

    return false;
}

std::list<std::string> PlayListItemVideo::GetMissingFiles() 
{
    std::list<std::string> res;
    if (!wxFile::Exists(GetVideoFile()))
    {
        res.push_back(GetVideoFile());
    }

    return res;
}