#include "PlayListItemImage.h"
#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include "PlayListItemImagePanel.h"
#include "PlayerWindow.h"

PlayListItemImage::PlayListItemImage(wxXmlNode* node) : PlayListItem(node)
{
    _window = nullptr;
    _ImageFile = "";
    _origin.x = 0;
    _origin.y = 0;
    _size.SetWidth(100);
    _size.SetHeight(100);
    PlayListItemImage::Load(node);
}

PlayListItemImage::~PlayListItemImage()
{
    if (_window != nullptr)
    {
        delete _window;
        _window = nullptr;
    }
}

void PlayListItemImage::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _ImageFile = node->GetAttribute("ImageFile", "");
    _origin = wxPoint(wxAtoi(node->GetAttribute("X", "0")), wxAtoi(node->GetAttribute("Y", "0")));
    _size = wxSize(wxAtoi(node->GetAttribute("W", "100")), wxAtoi(node->GetAttribute("H", "100")));
}

PlayListItemImage::PlayListItemImage() : PlayListItem()
{
    _window = nullptr;
    _ImageFile = "";
    _origin.x = 0;
    _origin.y = 0;
    _size.SetWidth(100);
    _size.SetHeight(100);
}

PlayListItem* PlayListItemImage::Copy() const
{
    PlayListItemImage* res = new PlayListItemImage();
    res->_ImageFile = _ImageFile;
    res->_origin = _origin;
    res->_size= _size;
    PlayListItem::Copy(res);

    return res;
}


wxXmlNode* PlayListItemImage::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLIImage");

    node->AddAttribute("ImageFile", _ImageFile);
    node->AddAttribute("X", wxString::Format(wxT("%i"), _origin.x));
    node->AddAttribute("Y", wxString::Format(wxT("%i"), _origin.y));
    node->AddAttribute("W", wxString::Format(wxT("%i"), _size.GetWidth()));
    node->AddAttribute("H", wxString::Format(wxT("%i"), _size.GetHeight()));

    PlayListItem::Save(node);

    return node;
}

void PlayListItemImage::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemImagePanel(notebook, this), "Image", true);
}

std::string PlayListItemImage::GetName() const
{
    wxFileName fn(_ImageFile);
    if (fn.GetName() == "")
    {
        return "Image";
    }
    else
    {
        return fn.GetName().ToStdString();
    }
}

void PlayListItemImage::Frame(wxByte* buffer, size_t size, size_t ms, size_t framems)
{
    if (ms > _delay)
    {
        #pragma todo ... display the image
    }
}

void PlayListItemImage::Start()
{
    // reload the image file
    _image.LoadFile(_ImageFile);

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

    _window->SetImage(_image);
}

void PlayListItemImage::Stop()
{
    // destroy the window
    if (_window != nullptr)
    {
        delete _window;
        _window = nullptr;
    }
}

