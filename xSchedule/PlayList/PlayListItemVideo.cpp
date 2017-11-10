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

PlayListItemVideo::PlayListItemVideo(wxXmlNode* node) : PlayListItem(node)
{
    _cacheVideo = false;
    _videoReader = nullptr;
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
    CloseFiles(true);

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
    _origin = wxPoint(wxAtoi(node->GetAttribute("X", "0")), wxAtoi(node->GetAttribute("Y", "0")));
    _size = wxSize(wxAtoi(node->GetAttribute("W", "100")), wxAtoi(node->GetAttribute("H", "100")));
    _topMost = (node->GetAttribute("Topmost", "TRUE") == "TRUE");
    _cacheVideo = (node->GetAttribute("CacheVideo", "FALSE") == "TRUE");
    _suppressVirtualMatrix = (node->GetAttribute("SuppressVM", "FALSE") == "TRUE");
    OpenFiles();
    CloseFiles(false);
}

PlayListItemVideo::PlayListItemVideo() : PlayListItem()
{
    _cacheVideo = false;
    _videoReader = nullptr;
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
    res->_size = _size;
    res->_videoFile = _videoFile;
    res->_durationMS = _durationMS;
    res->_topMost = _topMost;
    res->_cacheVideo = _cacheVideo;
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

    if (!_topMost)
    {
        node->AddAttribute("Topmost", "FALSE");
    }

    if (_cacheVideo)
    {
        node->AddAttribute("CacheVideo", "TRUE");
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
        OpenFiles();
        CloseFiles(false);
        _changeCount++;
    }
}

void PlayListItemVideo::CloseFiles(bool purgeCache)
{
    if (_videoReader != nullptr)
    {
        delete _videoReader;
        _videoReader = nullptr;
    }

    if (_cacheVideo && purgeCache)
    {
        VideoCache::GetVideoCache()->PurgeVideo(_videoFile, _size);
    }
}

void PlayListItemVideo::OpenFiles()
{
    CloseFiles(false);

    _videoReader = new VideoReader(_videoFile, _size.GetWidth(), _size.GetHeight(), false);
    _durationMS = _videoReader->GetLengthMS();
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

        if (_cacheVideo)
        {
            _window->SetImage(VideoCache::GetVideoCache()->GetImage(_videoFile, ms - _delay, framems, _size));
        }
        else
        {
            AVFrame* img = _videoReader->GetNextFrame(ms - _delay, framems);
            _window->SetImage(VideoCache::CreateImageFromFrame(img, _size));
        }

        if (sw.Time() > framems / 2)
        {
            logger_base.warn("   Getting frame %ld from video %s took more than half a frame: %ld.", (long)ms - _delay, (const char *)GetNameNoTime().c_str(), (long)sw.Time());
        }

        //logger_base.debug("   Done rendering frame %ld for video %s.", (long)ms - _delay, (const char *)GetNameNoTime().c_str());
    }
}

void PlayListItemVideo::Start()
{
    if (_suppressVirtualMatrix)
    {
        xScheduleFrame::GetScheduleManager()->SuppressVM(true);
    }

    if (_cacheVideo)
    {
        VideoCache::GetVideoCache()->Cache(_videoFile, 0, 999999999, GetFrameMS(), _size, false);
        _durationMS = VideoCache::GetVideoCache()->GetLengthMS(_videoFile);
    }

    OpenFiles();

    // create the window
    if (_window == nullptr)
    {
        _window = new PlayerWindow(wxGetApp().GetTopWindow(), _topMost, wxIMAGE_QUALITY_HIGH, wxID_ANY, _origin, _size);
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

    CloseFiles(true);

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

std::list<std::string> PlayListItemVideo::GetMissingFiles() const
{
    std::list<std::string> res;
    if (!wxFile::Exists(GetVideoFile()))
    {
        res.push_back(GetVideoFile());
    }

    return res;
}