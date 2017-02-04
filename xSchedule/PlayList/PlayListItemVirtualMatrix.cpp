#include "PlayListItemVirtualMatrix.h"
#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include "PlayListItemVirtualMatrixPanel.h"
#include "PlayerWindow.h"

PlayListItemVirtualMatrix::PlayListItemVirtualMatrix(wxXmlNode* node) : PlayListItem(node)
{
    _topMost = true;
    _quality = wxIMAGE_QUALITY_HIGH;
    _image = nullptr;
    _priority = 20;
    _window = nullptr;
    _startChannel = 1;
    _matrixWidth = 100;
    _matrixHeight = 100;
    _origin.x = 0;
    _origin.y = 0;
    _size.SetWidth(300);
    _size.SetHeight(300);
    PlayListItemVirtualMatrix::Load(node);
}

PlayListItemVirtualMatrix::~PlayListItemVirtualMatrix()
{
    if (_window != nullptr)
    {
        delete _window;
        _window = nullptr;
    }
}

void PlayListItemVirtualMatrix::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _origin = wxPoint(wxAtoi(node->GetAttribute("X", "0")), wxAtoi(node->GetAttribute("Y", "0")));
    _size = wxSize(wxAtoi(node->GetAttribute("W", "100")), wxAtoi(node->GetAttribute("H", "100")));
    _matrixWidth = wxAtoi(node->GetAttribute("MatrixWidth", "100"));
    _matrixHeight = wxAtoi(node->GetAttribute("MatrixHeight", "100"));
    _startChannel = wxAtol(node->GetAttribute("StartChannel", "1"));
    _topMost = (node->GetAttribute("Topmost", "TRUE") == "TRUE");
}

PlayListItemVirtualMatrix::PlayListItemVirtualMatrix() : PlayListItem()
{
    _quality = wxIMAGE_QUALITY_HIGH;
    _topMost = true;
    _image = nullptr;
    _priority = 20;
    _window = nullptr;
    _startChannel = 1;
    _matrixWidth = 100;
    _matrixHeight = 100;
    _origin.x = 0;
    _origin.y = 0;
    _size.SetWidth(300);
    _size.SetHeight(300);
}

PlayListItem* PlayListItemVirtualMatrix::Copy() const
{
    PlayListItemVirtualMatrix* res = new PlayListItemVirtualMatrix();
    res->_origin = _origin;
    res->_size = _size;
    res->_matrixWidth = _matrixWidth;
    res->_matrixHeight = _matrixHeight;
    res->_startChannel = _startChannel;
    res->_topMost = _topMost;
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemVirtualMatrix::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLIVirtualMatrix");

    node->AddAttribute("X", wxString::Format(wxT("%i"), _origin.x));
    node->AddAttribute("Y", wxString::Format(wxT("%i"), _origin.y));
    node->AddAttribute("W", wxString::Format(wxT("%i"), _size.GetWidth()));
    node->AddAttribute("H", wxString::Format(wxT("%i"), _size.GetHeight()));
    node->AddAttribute("MatrixWidth", wxString::Format(wxT("%i"), _matrixWidth));
    node->AddAttribute("MatrixHeight", wxString::Format(wxT("%i"), _matrixHeight));
    node->AddAttribute("StartChannel", wxString::Format(wxT("%i"), _startChannel));

    if (!_topMost)
    {
        node->AddAttribute("Topmost", "FALSE");
    }

    PlayListItem::Save(node);

    return node;
}

void PlayListItemVirtualMatrix::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemVirtualMatrixPanel(notebook, this), GetTitle(), true);
}

std::string PlayListItemVirtualMatrix::GetTitle() const
{
    return "Virtual Matrix";
}

std::string PlayListItemVirtualMatrix::GetNameNoTime() const
{
    if (_name == "")
    {
        return "Virtual Matrix";
    }
    else
    {
        return _name;
    }
}

void PlayListItemVirtualMatrix::Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    if (outputframe)
    {
        size_t end = _matrixWidth*_matrixHeight * 3 < size - (_startChannel - 1) ? _matrixWidth*_matrixHeight * 3 : size - (_startChannel - 1);

        for (size_t i = 0; i < end; i += 3)
        {
            wxByte* pb = buffer + (_startChannel - 1) + i;
            wxByte r = *pb;
            wxByte g = 0;
            if (i + 1 < end)
            {
                g = *(pb + 1);
            }
            wxByte b = 0;
            if (i + 2 < end)
            {
                b = *(pb + 2);
            }
            _image->SetRGB((i / 3) % _matrixWidth, i / 3 / _matrixWidth, r, g, b);
        }

        _window->SetImage(*_image);
    }
}

void PlayListItemVirtualMatrix::Start()
{
    // create the window
    if (_window == nullptr)
    {
        _window = new PlayerWindow(nullptr, _topMost, _quality, wxID_ANY, _origin, _size);
    }
    else
    {
        _window->Move(_origin);
        _window->SetSize(_size);
    }

    if (_image != nullptr)
    {
        delete _image;
        _image = nullptr;
    }

    _image = new wxImage(_matrixWidth, _matrixHeight);
}

void PlayListItemVirtualMatrix::Stop()
{
    if (_image != nullptr)
    {
        delete _image;
        _image = nullptr;
    }

    // destroy the window
    if (_window != nullptr)
    {
        delete _window;
        _window = nullptr;
    }
}

