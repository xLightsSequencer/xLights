#include "PlayListItemVideo.h"
#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include "PlayListItemVideoPanel.h"
#include "../../xLights/VideoReader.h"
#include "PlayerWindow.h"

PlayListItemVideo::PlayListItemVideo(wxXmlNode* node) : PlayListItem(node)
{
    _window = nullptr;
    _videoFile = "";
    _origin.x = 0;
    _origin.y = 0;
    _size.SetWidth(100);
    _size.SetHeight(100);
    _videoReader = nullptr;
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
    _origin = wxPoint(wxAtoi(node->GetAttribute("X", "0")), wxAtoi(node->GetAttribute("Y", "0")));
    _size = wxSize(wxAtoi(node->GetAttribute("W", "100")), wxAtoi(node->GetAttribute("H", "100")));
    OpenFiles();
    CloseFiles();
}

PlayListItemVideo::PlayListItemVideo() : PlayListItem()
{
    _window = nullptr;
    _videoFile = "";
    _origin.x = 0;
    _origin.y = 0;
    _durationMS = 0;
    _size.SetWidth(100);
    _size.SetHeight(100);
    _videoReader = nullptr;
}

PlayListItem* PlayListItemVideo::Copy() const
{
    PlayListItemVideo* res = new PlayListItemVideo();
    res->_origin = _origin;
    res->_size = _size;
    res->_videoFile = _videoFile;
    res->_durationMS = _durationMS;
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

    PlayListItem::Save(node);

    return node;
}

void PlayListItemVideo::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemVideoPanel(notebook, this), "Video", true);
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
}

void PlayListItemVideo::OpenFiles()
{
    CloseFiles();
    _videoReader = new VideoReader(_videoFile, _size.GetWidth(), _size.GetHeight(), false);
    _durationMS = _videoReader->GetLengthMS();
}

wxImage PlayListItemVideo::CreateImageFromFrame(AVFrame* frame)
{
    if (frame != NULL)
    {
        wxImage img(frame->width, frame->height, (unsigned char *)frame->data[0], true);
        img.SetType(wxBitmapType::wxBITMAP_TYPE_BMP);
        return img;
    }
    else
    {
        wxImage img(_size.x, _size.y, true);
        return img;
    }
}

void PlayListItemVideo::Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    AVFrame* img = _videoReader->GetNextFrame(ms);
    _window->SetImage(CreateImageFromFrame(img));
}

void PlayListItemVideo::Start()
{
    OpenFiles();

    // create the window
    if (_window == nullptr)
    {
        _window = new PlayerWindow(nullptr, wxID_ANY, _origin, _size);
    }
    else
    {
        _window->Move(_origin);
        _window->SetSize(_size);
    }
}

void PlayListItemVideo::Stop()
{
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