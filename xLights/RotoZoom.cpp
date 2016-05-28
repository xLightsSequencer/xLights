#include "RotoZoom.h"
#include <wx/wx.h>
#include <wx/string.h>

void RotoZoomParms::SetSerialisedValue(std::string k, std::string s)
{
    wxString kk = wxString(k.c_str());
    if (kk == "Id")
    {
        _id = s;
    }
    else if (kk == "Rots")
    {
        _rotations = (float)wxAtoi(wxString(s.c_str()));
    }
    else if (kk == "Zooms")
    {
        _zooms = (float)wxAtoi(wxString(s.c_str()));
    }
    else if (kk == "Start")
    {
        _start = wxAtoi(wxString(s.c_str()));
    }
    else if (kk == "ZMax")
    {
        _zoommaximum = (float)wxAtoi(wxString(s.c_str()));
    }
    else if (kk == "ZMin")
    {
        _zoomminimum = (float)wxAtoi(wxString(s.c_str()));
    }
    else if (kk == "Quality")
    {
        _quality = wxAtoi(wxString(s.c_str()));
    }
    else if (kk == "X")
    {
        _xcenter = wxAtoi(wxString(s.c_str()));
    }
    else if (kk == "Y")
    {
        _ycenter = wxAtoi(wxString(s.c_str()));
    }
    _active = true;
}

RotoZoomParms::RotoZoomParms(const std::string& id, float rotations, float zooms, int start, float zoomminimum, float zoommaximum, int quality, int x, int y)
{
    _active = false;
    _id = id;
    ApplySettings(rotations, zooms, start, zoomminimum, zoommaximum, quality, x, y);
}

void RotoZoomParms::ApplySettings(float rotations, float zooms, int start, float zoomminimum, float zoommaximum, int quality, int x, int y)
{
    _rotations = rotations;
    _zooms = zooms;
    _start = start;
    _zoomminimum = zoomminimum;
    _zoommaximum = zoommaximum;
    _quality = quality;
    _xcenter = x;
    _ycenter = y;
}

void RotoZoomParms::SetDefault(wxSize size)
{
    _rotations = 1;
    _zooms = 1;
    _start = 0;
    _zoomminimum = 10;
    _zoommaximum = 20;
    _quality = 1;
    _xcenter = size.GetWidth() / 2;
    _ycenter = size.GetHeight() / 2;
    _active = false;
}

std::string RotoZoomParms::Serialise()
{
    std::string res = "";

    if (IsActive())
    {
        res += "Id=" + _id + "|";
        if (_rotations != 10.0f)
        {
            res += "Rots=" + std::string(wxString::Format("%d", (int)_rotations).c_str()) + "|";
        }
        if (_zooms != 10.0f)
        {
            res += "Zooms=" + std::string(wxString::Format("%d", (int)_zooms).c_str()) + "|";
        }
        if (_start != 0)
        {
            res += "Start=" + std::string(wxString::Format("%d", _start).c_str()) + "|";
        }
        if (_zoomminimum != 10.0f)
        {
            res += "ZMin=" + std::string(wxString::Format("%d", (int)_zoomminimum).c_str()) + "|";
        }
        if (_zoommaximum != 20.0f)
        {
            res += "ZMax=" + std::string(wxString::Format("%d", (int)_zoommaximum).c_str()) + "|";
        }
        if (_quality != 1)
        {
            res += "Quality=" + std::string(wxString::Format("%d", _quality).c_str()) + "|";
        }
        if (_xcenter != 50)
        {
            res += "X=" + std::string(wxString::Format("%d", _xcenter).c_str()) + "|";
        }
        if (_ycenter != 50)
        {
            res += "Y=" + std::string(wxString::Format("%d", _ycenter).c_str()) + "|";
        }
    }
    return res;
}

void RotoZoomParms::Deserialise(std::string s)
{
    if (s == "")
    {
        _active = false;
    }
    else
    {
        _active = true;
        _rotations = 10.0f;
        _zooms = 10.0f;
        _start = 0;
        _zoomminimum = 10.0f;
        _zoommaximum = 20.0f;
        _quality = 1;
        _xcenter = 50;
        _ycenter = 50;
        wxArrayString v = wxSplit(wxString(s.c_str()), '|');
        for (auto vs = v.begin(); vs != v.end(); vs++)
        {
            wxArrayString v1 = wxSplit(*vs, '=');
            if (v1.size() == 2)
            {
                SetSerialisedValue(v1[0].ToStdString(), v1[1].ToStdString());
            }
        }
    }
}

wxPoint RotoZoomParms::GetTransform(float x, float y, float offset, wxSize size)
{
    const float PI_2 = 6.283185307f;
    float start = PI_2 * (float)_start / 100.0;
    float angle = PI_2 * _rotations / 10.0f * offset;
    float scale = 1.0f;
    float spread = (_zoommaximum - _zoomminimum) / 10.0f;
    
    if (_zooms != 0)
    {
        if (spread == 0.0)
        {
            scale = _zoomminimum / 10.0f;
        }
        else
        {
            scale = _zoomminimum + (sin(start + PI_2 * (_zooms / 10.0f) * offset) + 1.0f) / 2.0f * spread;
            if (scale < 0.01f)
            {
                scale = 0.01f;
            }
        }
    }

    float xoff = (_xcenter * size.GetWidth()) / 100.0;
    float yoff = (_ycenter * size.GetHeight()) / 100.0;
    float c = cos(-angle);
    float s = sin(-angle);

    float u = xoff + c * (x - xoff) * scale + s * (y - yoff) * scale; 
    float v = yoff + -s * (x - xoff) * scale + c * (y - yoff) * scale;
    
    return wxPoint(u, v);
}