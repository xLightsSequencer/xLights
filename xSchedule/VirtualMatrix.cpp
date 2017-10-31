#include "VirtualMatrix.h"
#include <wx/string.h>
#include <wx/xml/xml.h>
#include <wx/wx.h>
#include <log4cpp/Category.hh>
#include "xScheduleApp.h"

VirtualMatrix::VirtualMatrix(int width, int height, bool topMost, VMROTATION rotation, wxImageResizeQuality quality, size_t startChannel, const std::string& name, wxSize size, wxPoint loc)
{
    _lastSavedChangeCount = 0;
    _changeCount = 1;
    _name = name;
    _width = width;
    _height = height;
    _topMost = topMost;
    _rotation = rotation;
    _quality = quality;
    _size = size;
    _location = loc;
    _startChannel = startChannel;
    _image = nullptr;
    _window = nullptr;
}

VirtualMatrix::VirtualMatrix()
{
    _lastSavedChangeCount = 0;
    _changeCount = 1;
    _name = "";
    _width = 32;
    _height = 16;
    _topMost = true;
    _rotation = VMROTATION::VM_NORMAL;
    _quality = wxIMAGE_QUALITY_HIGH;
    _size = wxSize(300, 300);
    _location = wxPoint(0,0);
    _startChannel = 1;
    _image = nullptr;
    _window = nullptr;
}

VirtualMatrix::VirtualMatrix(int width, int height, bool topMost, const std::string& rotation, const std::string& quality, size_t startChannel, const std::string& name, wxSize size, wxPoint loc)
{
    _lastSavedChangeCount = 0;
    _changeCount = 1;
    _name = name;
    _width = width;
    _height = height;
    _topMost = topMost;
    _rotation = EncodeRotation(rotation);
    _quality = EncodeScalingQuality(quality);
    _size = size;
    _location = loc;
    _startChannel = startChannel;
    _image = nullptr;
    _window = nullptr;
}

VirtualMatrix::VirtualMatrix(wxXmlNode* n)
{
    _lastSavedChangeCount = 0;
    _changeCount = 0;
    _name = n->GetAttribute("Name", "");
    _width = wxAtoi(n->GetAttribute("Width", "32"));
    _height = wxAtoi(n->GetAttribute("Height", "16"));
    _topMost = (n->GetAttribute("TopMost", "TRUE") == "TRUE");
    _rotation = EncodeRotation(n->GetAttribute("Rotation", "None").ToStdString());
    _quality = EncodeScalingQuality(n->GetAttribute("Quality", "High").ToStdString());
    _size = wxSize(wxAtoi(n->GetAttribute("WW", "300")), wxAtoi(n->GetAttribute("WH", "300")));
    _location = wxPoint(wxAtoi(n->GetAttribute("X", "0")), wxAtoi(n->GetAttribute("Y", "0")));
    _startChannel = wxAtoi(n->GetAttribute("StartChannel", "1"));
    _image = nullptr;
    _window = nullptr;
}

wxXmlNode* VirtualMatrix::Save()
{
    wxXmlNode* res = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "VMatrix");

    res->AddAttribute("Name", _name);
    res->AddAttribute("Width", wxString::Format(wxT("%i"), (long)_width));
    res->AddAttribute("Height", wxString::Format(wxT("%i"), (long)_height));
    if (!_topMost)
    {
        res->AddAttribute("TopMost", "FALSE");
    }
    res->AddAttribute("Rotation", DecodeRotation(_rotation));
    res->AddAttribute("Quality", DecodeScalingQuality(_quality));
    res->AddAttribute("WW", wxString::Format(wxT("%i"), (long)_size.GetWidth()));
    res->AddAttribute("WH", wxString::Format(wxT("%i"), (long)_size.GetHeight()));
    res->AddAttribute("X", wxString::Format(wxT("%i"), (long)_location.x));
    res->AddAttribute("Y", wxString::Format(wxT("%i"), (long)_location.y));
    res->AddAttribute("StartChannel", wxString::Format(wxT("%i"), (int)_startChannel));

    return res;
}

VMROTATION VirtualMatrix::EncodeRotation(const std::string rotation)
{
    if (wxString(rotation).Lower() == "none")
    {
        return VMROTATION::VM_NORMAL;
    }
    else if (wxString(rotation).Lower() == "90 cw")
    {
        return VMROTATION::VM_90;
    }
    else
    {
        return VMROTATION::VM_270;
    }
}

std::string VirtualMatrix::DecodeRotation(VMROTATION rotation)
{
    if (rotation == VMROTATION::VM_NORMAL)
    {
        return "None";
    }
    else if (rotation == VMROTATION::VM_90)
    {
        return "90 CW";
    }
    else
    {
        return "90 CCW";
    }
}

wxImageResizeQuality VirtualMatrix::EncodeScalingQuality(const std::string quality)
{
    if (wxString(quality).Lower() == "normal")
    {
        return wxIMAGE_QUALITY_NORMAL;
    }
    else if (wxString(quality).Lower() == "bicubic")
    {
        return wxIMAGE_QUALITY_BICUBIC;
    }
    else if (wxString(quality).Lower() == "bilinear")
    {
        return wxIMAGE_QUALITY_BILINEAR;
    }
    else if (wxString(quality).Lower() == "box average")
    {
        return wxIMAGE_QUALITY_BOX_AVERAGE;
    }
    else if (wxString(quality).Lower() == "high")
    {
        return wxIMAGE_QUALITY_HIGH;
    }
    return wxIMAGE_QUALITY_NORMAL;
}

std::string VirtualMatrix::DecodeScalingQuality(wxImageResizeQuality quality)
{
    switch (quality)
    {
    case wxIMAGE_QUALITY_NORMAL:
        return "Normal";
    case wxIMAGE_QUALITY_BICUBIC:
        return "Bicubic";
    case wxIMAGE_QUALITY_BILINEAR:
        return "Bilinear";
    case wxIMAGE_QUALITY_BOX_AVERAGE:
        return "Box Average";
    case wxIMAGE_QUALITY_HIGH:
        return "High";
    }

    return "Normal";
}

void VirtualMatrix::Frame(wxByte*buffer, size_t size)
{
    if (_image == nullptr) return;

    size_t end = _width * _height * 3 < size - (_startChannel - 1) ? _width * _height * 3 : size - (_startChannel - 1);

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
        _image->SetRGB((i / 3) % _width, i / 3 / _width, r, g, b);
    }

    if (_rotation == VMROTATION::VM_NORMAL)
    {
        _window->SetImage(*_image);
    }
    else if (_rotation == VMROTATION::VM_90)
    {
        wxImage rot = _image->Rotate90();
        _window->SetImage(rot);
    }
    else
    {
        wxImage rot = _image->Rotate90(false);
        _window->SetImage(rot);
    }
}

void VirtualMatrix::Start()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Virtual matrix started %s.", (const char *)_name.c_str());

    // create the window
    if (_window == nullptr)
    {
        _window = new PlayerWindow(wxGetApp().GetTopWindow(), _topMost, _quality, wxID_ANY, _location, _size);
    }
    else
    {
        _window->Move(_location);
        _window->SetSize(_size);
    }

    if (_suppress)
    {
        _window->Hide();
    }

    if (_image != nullptr)
    {
        delete _image;
        _image = nullptr;
    }

    _image = new wxImage(_width, _height);
}

void VirtualMatrix::Stop()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Virtual matrix stopped %s.", (const char *)_name.c_str());

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

void VirtualMatrix::Suppress(bool suppress)
{
    _suppress = suppress;

    if (_window != nullptr)
    {
        if (suppress)
        {
            _window->Hide();
        }
        else
        {
            _window->Show();
        }
    }
}
