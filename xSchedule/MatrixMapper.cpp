#include "MatrixMapper.h"
#include <wx/string.h>
#include <wx/xml/xml.h>
#include <wx/wx.h>
#include "../xLights/outputs/OutputManager.h"
#include <log4cpp/Category.hh>

MatrixMapper::MatrixMapper(OutputManager* outputManager, int strings, int strandsPerString, int stringLength, MMORIENTATION orientation, MMSTARTLOCATION startLocation, const std::string& startChannel, const std::string& name)
{
    _outputManager = outputManager;
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

MatrixMapper::MatrixMapper(OutputManager* outputManager, int strings, int strandsPerString, int stringLength, const std::string& orientation, const std::string& startLocation, const std::string& startChannel, const std::string& name)
{
    _outputManager = outputManager;
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

MatrixMapper::MatrixMapper(OutputManager* outputManager, wxXmlNode* n)
{
    _outputManager = outputManager;
    _lastSavedChangeCount = 0;
    _changeCount = 0;
    _name = n->GetAttribute("Name", "");
    _strings = wxAtoi(n->GetAttribute("Strings", "1"));
    _strandsPerString = wxAtoi(n->GetAttribute("StrandsPerString", "1"));
    _stringLength = wxAtoi(n->GetAttribute("StringLength", "50"));
    _orientation = (MMORIENTATION)wxAtoi(n->GetAttribute("Orientation", "0"));
    _startLocation = (MMSTARTLOCATION)wxAtoi(n->GetAttribute("StartLocation", "0"));
    _startChannel = n->GetAttribute("StartChannel", "1").ToStdString();
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
    res->AddAttribute("StartChannel", _startChannel);

    return res;
}

size_t MatrixMapper::Map(int x, int y) const
{
    wxASSERT(x >= 0 && x < GetWidth());
    wxASSERT(y >= 0 && y < GetHeight());

    long startChannel = GetStartChannelAsNumber();
    size_t loc = startChannel;

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
                if (_strandsPerString == 1 || ((x - ((x / _strandsPerString) * _strandsPerString)) % 2 == 0))
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
                    loc += (GetHeight() - y - 1) * 3;
                }
                else
                {
                    loc += y * 3;
                }
            }
            else
            {
                if (_strandsPerString == 1 || (((GetWidth() - x - 1) - (((GetWidth() - x - 1) / _strandsPerString) * _strandsPerString)) % 2 == 0))
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
                if (_strandsPerString != 1 && (x - ((x / _strandsPerString) * _strandsPerString)) % 2 == 1)
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
                    loc += (GetHeight() - y - 1) * 3;
                }
                else
                {
                    loc += y * 3;
                }
            }
            else
            {
                if (_strandsPerString != 1 && (((GetWidth() - x - 1) - (((GetWidth() - x - 1) / _strandsPerString) * _strandsPerString)) % 2 == 1))
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
                if (_strandsPerString == 1 || ((y - ((y / _strandsPerString) * _strandsPerString)) % 2 == 0))
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
                if (_strandsPerString != 1 && ((y - ((y / _strandsPerString) * _strandsPerString)) % 2 == 1))
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
                if (_strandsPerString == 1 || (((GetHeight() - y - 1) - (((GetHeight() - y - 1) / _strandsPerString) * _strandsPerString)) % 2 == 0))
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
    if (loc < startChannel || loc >= startChannel + GetChannels())
    {
        // location out of range ... this can happen if the user tampers with the matrix while it is in use
        // force it to a valid value
        loc = startChannel;
        wxASSERT(false);
    }

    return loc;
}

size_t MatrixMapper::GetChannels() const
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

long MatrixMapper::GetStartChannelAsNumber() const
{
    return _outputManager->DecodeStartChannel(_startChannel);
}

MMORIENTATION MatrixMapper::EncodeOrientation(const std::string orientation)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxString o = wxString(orientation).Lower();

    if (o == "horizontal")
    {
        return MMORIENTATION::HORIZONTAL;
    }
    else if (o == "vertical")
    {
        return MMORIENTATION::VERTICAL;
    }

    logger_base.error("Unknown orientation %s", (const char *)o.c_str());
    return MMORIENTATION::VERTICAL;
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
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

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
    else if (sl == "top right")
    {
        return MMSTARTLOCATION::TOP_RIGHT;
    }

    logger_base.error("Unknown start location %s", (const char *)sl.c_str());

    return MMSTARTLOCATION::TOP_LEFT;
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

void MatrixMapper::Test(OutputManager* outputManager)
{
    MatrixMapper h_bl_e(outputManager, 2, 4, 200, "Horizontal", "Bottom Left", "1", "Test");
    wxASSERT(h_bl_e.Map(0, 0) == 1);
    wxASSERT(h_bl_e.Map(49, 0) == 148);
    wxASSERT(h_bl_e.Map(0, 3) == 598);

    MatrixMapper h_br_e(outputManager, 2, 4, 200, "Horizontal", "Bottom Right", "1", "Test");
    wxASSERT(h_br_e.Map(0, 0) == 148);
    wxASSERT(h_br_e.Map(49, 0) == 1);
    wxASSERT(h_br_e.Map(0, 3) == 451);

    MatrixMapper h_bl_o(outputManager, 2, 3, 150, "Horizontal", "Bottom Left", "1", "Test");
    wxASSERT(h_bl_o.Map(0, 0) == 1);
    wxASSERT(h_bl_o.Map(49, 0) == 148);
    wxASSERT(h_bl_o.Map(0, 3) == 451);

    MatrixMapper h_br_o(outputManager, 2, 3, 150, "Horizontal", "Bottom Right", "1", "Test");
    wxASSERT(h_br_o.Map(0, 0) == 148);
    wxASSERT(h_br_o.Map(49, 0) == 1);
    wxASSERT(h_br_o.Map(0, 3) == 598);

    MatrixMapper h_tl_o(outputManager, 40, 1, 150, "Horizontal", "Top Left", "1", "Test");
    wxASSERT(h_tl_o.Map(0, 0) == 17551);
    wxASSERT(h_tl_o.Map(149, 0) == 17998);
    wxASSERT(h_tl_o.Map(0, 3) == 16201);

    MatrixMapper v_tl_e(outputManager, 4, 4, 600, "Vertical", "Top Left", "1", "Test");
    wxASSERT(v_tl_e.Map(0, 0) == 448);
    wxASSERT(v_tl_e.Map(15, 149) == 7198);
    wxASSERT(v_tl_e.Map(0, 3) == 439);

    MatrixMapper v_tr_o(outputManager, 4, 3, 450, "Vertical", "Top Right", "1", "Test");
    wxASSERT(v_tr_o.Map(0, 0) == 5398);
    wxASSERT(v_tr_o.Map(11, 149) == 1);
    wxASSERT(v_tr_o.Map(0, 3) == 5389);

    MatrixMapper v_tl_o(outputManager, 4, 3, 450, "Vertical", "Top Left", "1", "Test");
    wxASSERT(v_tl_o.Map(0, 0) == 448);
    wxASSERT(v_tl_o.Map(11, 149) == 4951);
    wxASSERT(v_tl_o.Map(0, 3) == 439);

    MatrixMapper v_tl_o2(outputManager, 1, 30, 210, "Vertical", "Top Left", "1", "Test");
    wxASSERT(v_tl_o2.Map(0, 0) == 19);
    wxASSERT(v_tl_o2.Map(29, 6) == 209 * 3 + 1);
    wxASSERT(v_tl_o2.Map(0, 3) == 10);
    wxASSERT(v_tl_o2.Map(10, 3) == 220);

    MatrixMapper v_tl_o3(outputManager, 30, 1, 7, "Vertical", "Top Left", "1", "Test");
    wxASSERT(v_tl_o3.Map(0, 0) == 19);
    wxASSERT(v_tl_o3.Map(29, 6) == 610);
    wxASSERT(v_tl_o3.Map(0, 3) == 10);
    wxASSERT(v_tl_o3.Map(10, 3) == 220);

    MatrixMapper v_tr_o2(outputManager, 1, 75, 525, "Vertical", "Top Right", "1", "Test");
    wxASSERT(v_tr_o2.Map(0, 0) == 1573);
    wxASSERT(v_tr_o2.Map(74, 6) == 1);
    wxASSERT(v_tr_o2.Map(0, 3) == 1564);
    wxASSERT(v_tr_o2.Map(65, 4) == 202);

    MatrixMapper v_tr_o3(outputManager, 75, 1, 7, "Vertical", "Top Right", "1", "Test");
    wxASSERT(v_tr_o3.Map(0, 0) == 1573);
    wxASSERT(v_tr_o3.Map(74, 6) == 1);
    wxASSERT(v_tr_o3.Map(0, 3) == 1564);
    wxASSERT(v_tr_o3.Map(65, 4) == 196);

    MatrixMapper v_bl_e(outputManager, 4, 4, 600, "Vertical", "Bottom Left", "1", "Test");
    wxASSERT(v_bl_e.Map(0, 0) == 1);
    wxASSERT(v_bl_e.Map(15, 149) == 6751);
    wxASSERT(v_bl_e.Map(0, 3) == 10);

    MatrixMapper v_bl_o(outputManager, 4, 3, 450, "Vertical", "Bottom Left", "1", "Test");
    wxASSERT(v_bl_o.Map(0, 0) == 1);
    wxASSERT(v_bl_o.Map(11, 149) == 5398);
    wxASSERT(v_bl_o.Map(0, 3) == 10);

    MatrixMapper v_br_e(outputManager, 4, 4, 600, "Vertical", "Bottom Right", "1", "Test");
    wxASSERT(v_br_e.Map(0, 0) == 7198);
    wxASSERT(v_br_e.Map(15, 149) == 448);
    wxASSERT(v_br_e.Map(0, 3) == 7189);

    MatrixMapper v_tr_e(outputManager, 4, 4, 600, "Vertical", "Top Right", "1", "Test");
    wxASSERT(v_tr_e.Map(0, 0) == 6751);
    wxASSERT(v_tr_e.Map(15, 149) == 1);
    wxASSERT(v_tr_e.Map(0, 3) == 6760);

    MatrixMapper v_br_o(outputManager, 4, 3, 450, "Vertical", "Bottom Right", "1", "Test");
    wxASSERT(v_br_o.Map(0, 0) == 4951);
    wxASSERT(v_br_o.Map(11, 149) == 448);
    wxASSERT(v_br_o.Map(0, 3) == 4960);

    MatrixMapper v_br_e2(outputManager, 10, 1, 10, "Vertical", "Bottom Right", "1", "Test");
    wxASSERT(v_br_e2.Map(0, 0) == 271);
    wxASSERT(v_br_e2.Map(9, 9) == 28);
    wxASSERT(v_br_e2.Map(0, 3) == 280);

    MatrixMapper v_bl_e2(outputManager, 10, 1, 10, "Vertical", "Bottom Left", "1", "Test");
    wxASSERT(v_bl_e2.Map(0, 0) == 1);
    wxASSERT(v_bl_e2.Map(9, 9) == 298);
    wxASSERT(v_bl_e2.Map(0, 3) == 10);

    MatrixMapper v_tr_e2(outputManager, 10, 1, 10, "Vertical", "Top Right", "1", "Test");
    wxASSERT(v_tr_e2.Map(0, 0) == 298);
    wxASSERT(v_tr_e2.Map(9, 9) == 1);
    wxASSERT(v_tr_e2.Map(0, 3) == 289);

    MatrixMapper v_tl_e2(outputManager, 10, 1, 10, "Vertical", "Top Left", "1", "Test");
    wxASSERT(v_tl_e2.Map(0, 0) == 28);
    wxASSERT(v_tl_e2.Map(9, 9) == 271);
    wxASSERT(v_tl_e2.Map(0, 3) == 19);

    MatrixMapper v_br_o2(outputManager, 9, 1, 10, "Vertical", "Bottom Right", "1", "Test");
    wxASSERT(v_br_o2.Map(0, 0) == 241);
    wxASSERT(v_br_o2.Map(8, 9) == 28);
    wxASSERT(v_br_o2.Map(0, 3) == 250);

    MatrixMapper v_bl_o2(outputManager, 9, 1, 10, "Vertical", "Bottom Left", "1", "Test");
    wxASSERT(v_bl_o2.Map(0, 0) == 1);
    wxASSERT(v_bl_o2.Map(8, 9) == 268);
    wxASSERT(v_bl_o2.Map(0, 3) == 10);

    MatrixMapper v_tr_o4(outputManager, 9, 1, 10, "Vertical", "Top Right", "1", "Test");
    wxASSERT(v_tr_o4.Map(0, 0) == 268);
    wxASSERT(v_tr_o4.Map(8, 9) == 1);
    wxASSERT(v_tr_o4.Map(0, 3) == 259);

    MatrixMapper v_tl_o4(outputManager, 9, 1, 10, "Vertical", "Top Left", "1", "Test");
    wxASSERT(v_tl_o4.Map(0, 0) == 28);
    wxASSERT(v_tl_o4.Map(8, 9) == 241);
    wxASSERT(v_tl_o4.Map(0, 3) == 19);
}

std::string MatrixMapper::GetConfigDescription() const
{
    return wxString::Format("Orientation: %s, Start: %s, Strings: %d, Strands/String: %d, Nodes Per String: %d, StartChannel:%s:%ld.",
        DecodeOrientation(_orientation),
        DecodeStartLocation(_startLocation),
        _strings,
        _strandsPerString,
        _stringLength,
        _startChannel,
        GetStartChannelAsNumber()).ToStdString();
}

MatrixMapper::MatrixMapper(OutputManager* outputManager)
{
    _outputManager = outputManager;
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
