#include "MatrixMapper.h"
#include <wx/string.h>
#include <wx/xml/xml.h>
#include <wx/wx.h>

MatrixMapper::MatrixMapper(int strings, int strandsPerString, int stringLength, MMORIENTATION orientation, MMSTARTLOCATION startLocation, size_t startChannel, const std::string& name)
{
    _lastSavedChangeCount = 0;
    _changeCount = 1;
    _name = name;
    _strings = strings;
    _strandsPerString = strandsPerString;
    _stringLength = stringLength;
    _orientation = orientation;
    _startLocation = startLocation;
    _startChannel = startChannel;
}

MatrixMapper::MatrixMapper(int strings, int strandsPerString, int stringLength, const std::string& orientation, const std::string& startLocation, size_t startChannel, const std::string& name)
{
    _lastSavedChangeCount = 0;
    _changeCount = 1;
    _name = name;
    _strings = strings;
    _strandsPerString = strandsPerString;
    _stringLength = stringLength;
    _orientation = MatrixMapper::EncodeOrientation(orientation);
    _startLocation = MatrixMapper::EncodeStartLocation(startLocation);
    _startChannel = startChannel;
}

MatrixMapper::MatrixMapper(wxXmlNode* n)
{
    _lastSavedChangeCount = 0;
    _changeCount = 0;
    _name = n->GetAttribute("Name", "");
    _strings = wxAtoi(n->GetAttribute("Strings", "1"));
    _strandsPerString = wxAtoi(n->GetAttribute("StrandsPerString", "1"));
    _stringLength = wxAtoi(n->GetAttribute("StringLength", "50"));
    _orientation = (MMORIENTATION)wxAtoi(n->GetAttribute("Orientation", "0"));
    _startLocation = (MMSTARTLOCATION)wxAtoi(n->GetAttribute("StartLocation", "0"));
    _startChannel = wxAtoi(n->GetAttribute("StartChannel", "1"));
}

wxXmlNode* MatrixMapper::Save()
{
    wxXmlNode* res = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "Matrix");

    res->AddAttribute("Name", _name);
    res->AddAttribute("Strings", wxString::Format(wxT("%i"), _strings));
    res->AddAttribute("StrandsPerString", wxString::Format(wxT("%i"), _strandsPerString));
    res->AddAttribute("StringLength", wxString::Format(wxT("%i"), _stringLength));
    res->AddAttribute("Orientation", wxString::Format(wxT("%i"), (int)_orientation));
    res->AddAttribute("StartLocation", wxString::Format(wxT("%i"), (int)_startLocation));
    res->AddAttribute("StartChannel", wxString::Format(wxT("%i"), (int)_startChannel));

    return res;
}

size_t MatrixMapper::Map(int x, int y) const
{
    size_t loc = _startChannel;

    if (_orientation == MMORIENTATION::VERTICAL)
    {
        switch(_startLocation)
        {
        case MMSTARTLOCATION::BOTTOM_LEFT:
            loc += x * GetHeight() * 3;
            if (_strandsPerString % 2 == 0)
            {
                if (x % 2 == 0)
                {
                    loc += y * 3;
                }
                else
                {
                    loc += (GetHeight() - y - 1) * 3;
                }
            }
            else
            {
                if ((x - ((x / _strandsPerString) * _strandsPerString)) % 2 == 0)
                {
                    loc += y * 3;
                }
                else
                {
                    loc += (GetHeight() - y - 1) * 3;
                }
            }
            break;
        case MMSTARTLOCATION::BOTTOM_RIGHT:
            loc += (GetWidth() - x - 1) * GetHeight() * 3;
            if (_strandsPerString % 2 == 0)
            {
                if (x % 2 == 0)
                {
                    loc += y * 3;
                }
                else
                {
                    loc += (GetHeight() - y - 1) * 3;
                }
            }
            else
            {
                if (((GetWidth() - x - 1) - (((GetWidth() - x - 1) / _strandsPerString) * _strandsPerString)) % 2 == 0)
                {
                    loc += y * 3;
                }
                else
                {
                    loc += (GetHeight() - y - 1) * 3;
                }
            }
            break;
        case MMSTARTLOCATION::TOP_LEFT:
            loc += x * GetHeight() * 3;
            if (_strandsPerString % 2 == 0)
            {
                if (x % 2 == 1)
                {
                    loc += y * 3;
                }
                else
                {
                    loc += (GetHeight() - y - 1) * 3;
                }
            }
            else
            {
                if ((x - ((x / _strandsPerString) * _strandsPerString)) % 2 == 1)
                {
                    loc += y * 3;
                }
                else
                {
                    loc += (GetHeight() - y - 1) * 3;
                }
            }
            break;
        case MMSTARTLOCATION::TOP_RIGHT:
            loc += (GetWidth() - x - 1) * GetHeight() * 3;
            if (_strandsPerString % 2 == 0)
            {
                if (x % 2 == 1)
                {
                    loc += y * 3;
                }
                else
                {
                    loc += (GetHeight() - y - 1) * 3;
                }
            }
            else
            {
                if (((GetWidth() - x - 1) - (((GetWidth() - x - 1) / _strandsPerString) * _strandsPerString)) % 2 == 1)
                {
                    loc += y * 3;
                }
                else
                {
                    loc += (GetHeight() - y - 1) * 3;
                }
            }
            break;
        }
    }
    else
    {
        switch(_startLocation)
        {
        case MMSTARTLOCATION::BOTTOM_LEFT:
            loc += y * GetWidth() * 3;
            if (_strandsPerString % 2 == 0)
            {
                if (y % 2 == 0)
                {
                    loc += x * 3;
                }
                else
                {
                    loc += (GetWidth() - x - 1) * 3;
                }
            }
            else
            {
                if ((y - ((y / _strandsPerString) * _strandsPerString)) % 2 == 0)
                {
                    loc += x * 3;
                }
                else
                {
                    loc += (GetWidth() - x - 1) * 3;
                }
            }
            break;
        case MMSTARTLOCATION::BOTTOM_RIGHT:
            loc += y * GetWidth() * 3;
            if (_strandsPerString % 2 == 0)
            {
                if (y % 2 == 1)
                {
                    loc += x * 3;
                }
                else
                {
                    loc += (GetWidth() - x - 1) * 3;
                }
            }
            else
            {
                if ((y - ((y / _strandsPerString) * _strandsPerString)) % 2 == 1)
                {
                    loc += x * 3;
                }
                else
                {
                    loc += (GetWidth() - x - 1) * 3;
                }
            }
            break;
        case MMSTARTLOCATION::TOP_LEFT:
            loc += (GetHeight() - y - 1) * GetWidth() * 3;
            if (_strandsPerString % 2 == 0)
            {
                if (y % 2 == 0)
                {
                    loc += x * 3;
                }
                else
                {
                    loc += (GetWidth() - x - 1) * 3;
                }
            }
            else
            {
                if (((GetHeight() - y - 1) - (((GetHeight() - y - 1) / _strandsPerString) * _strandsPerString)) % 2 == 0)
                {
                    loc += x * 3;
                }
                else
                {
                    loc += (GetWidth() - x - 1) * 3;
                }
            }
            break;
        case MMSTARTLOCATION::TOP_RIGHT:
            loc += (GetHeight() - y - 1) * GetWidth() * 3;
            if (_strandsPerString % 2 == 0)
            {
                if (y % 2 == 1)
                {
                    loc += x * 3;
                }
                else
                {
                    loc += (GetWidth() - x - 1) * 3;
                }
            }
            else
            {
                if (((GetHeight() - y - 1) - (((GetHeight() - y - 1) / _strandsPerString) * _strandsPerString)) % 2 == 1)
                {
                    loc += x * 3;
                }
                else
                {
                    loc += (GetWidth() - x - 1) * 3;
                }
            }
            break;
        }
    }

    // make sure the value is within the range expected ... until i know my code is right
    wxASSERT(loc >= _startChannel && loc < _startChannel + GetChannels());

    return loc;
}

int MatrixMapper::GetChannels() const
{
    return _stringLength * _strings * 3;
}

int MatrixMapper::GetWidth() const
{
    if (_orientation == MMORIENTATION::VERTICAL)
    {
        return _strings * _strandsPerString;
    }
    else
    {
        return _stringLength / _strandsPerString;
    }
}

int MatrixMapper::GetHeight() const
{
	if (_orientation == MMORIENTATION::VERTICAL)
	{
        return _stringLength / _strandsPerString;
	}
    else
    {
        return _strings * _strandsPerString;
    }
}

MMORIENTATION MatrixMapper::EncodeOrientation(const std::string orientation)
{
    if (wxString(orientation).Lower() == "horizontal")
    {
        return MMORIENTATION::HORIZONTAL;
    }
    else
    {
        return MMORIENTATION::VERTICAL;
    }
}

std::string MatrixMapper::DecodeOrientation(MMORIENTATION orientation)
{
    if (orientation == MMORIENTATION::HORIZONTAL)
    {
        return "Horizontal";
    }
    else
    {
        return "Vertical";
    }
}

MMSTARTLOCATION MatrixMapper::EncodeStartLocation(const std::string startLocation)
{
    wxString sl = wxString(startLocation).Lower();

    if (sl == "bottom left")
    {
        return MMSTARTLOCATION::BOTTOM_LEFT;
    }
    else if (sl == "bottom right")
    {
        return MMSTARTLOCATION::BOTTOM_RIGHT;
    }
    else if (sl == "top left")
    {
        return MMSTARTLOCATION::TOP_LEFT;
    }
    else
    {
        return MMSTARTLOCATION::TOP_RIGHT;
    }
}

std::string MatrixMapper::DecodeStartLocation(MMSTARTLOCATION startLocation)
{
    switch(startLocation)
    {
    case MMSTARTLOCATION::BOTTOM_LEFT:
        return "Bottom Left";
    case MMSTARTLOCATION::BOTTOM_RIGHT:
        return "Bottom Right";
    case MMSTARTLOCATION::TOP_LEFT:
        return "Top Left";
    default:
        break;
    }

    return "Top Right";
}

void MatrixMapper::Test()
{
    MatrixMapper h_bl_e(2, 4, 200, "Horizontal", "Bottom Left", 1, "Test");
    wxASSERT(h_bl_e.Map(0, 0) == 1);
    wxASSERT(h_bl_e.Map(49, 0) == 148);
    wxASSERT(h_bl_e.Map(0, 3) == 598);

    MatrixMapper h_br_e(2, 4, 200, "Horizontal", "Bottom Right", 1, "Test");
    wxASSERT(h_br_e.Map(0, 0) == 148);
    wxASSERT(h_br_e.Map(49, 0) == 1);
    wxASSERT(h_br_e.Map(0, 3) == 451);

    MatrixMapper h_bl_o(2, 3, 150, "Horizontal", "Bottom Left", 1, "Test");
    wxASSERT(h_bl_o.Map(0, 0) == 1);
    wxASSERT(h_bl_o.Map(49, 0) == 148);
    wxASSERT(h_bl_o.Map(0, 3) == 451);

    MatrixMapper h_br_o(2, 3, 150, "Horizontal", "Bottom Right", 1, "Test");
    wxASSERT(h_br_o.Map(0, 0) == 148);
    wxASSERT(h_br_o.Map(49, 0) == 1);
    wxASSERT(h_br_o.Map(0, 3) == 598);
}

MatrixMapper::MatrixMapper()
{
    _lastSavedChangeCount = 0;
    _changeCount = 1;
    _name = "";
    _strings = 1;
    _strandsPerString = 1;
    _stringLength = 50;
    _orientation = MMORIENTATION::HORIZONTAL;
    _startLocation = MMSTARTLOCATION::BOTTOM_LEFT;
    _startChannel = 1;
}