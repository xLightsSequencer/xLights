#include "RotoZoom.h"
#include <wx/wx.h>
#include <wx/string.h>

bool rzEndsWith(const wxString& in, const wxString& what)
{
    return in.EndsWith(what);
}

void RotoZoomParms::SetSerialisedValue(std::string k, std::string s)
{
    wxString kk = wxString(k.c_str());
    if (rzEndsWith(kk, "_Id"))
    {
        _id = s;
    }
    else if (rzEndsWith(kk, "_Rotations"))
    {
        _rotations = wxAtof(wxString(s.c_str()));
    }
    else if (rzEndsWith(kk, "_Zooms"))
    {
        _zooms = wxAtof(wxString(s.c_str()));
    }
    else if (rzEndsWith(kk, "_ZoomMaximum"))
    {
        _zoommaximum = wxAtof(wxString(s.c_str()));
    }
    else if (rzEndsWith(kk, "_XCenter"))
    {
        _xcenter = wxAtoi(wxString(s.c_str()));
    }
    else if (rzEndsWith(kk, "_YCenter"))
    {
        _ycenter = wxAtoi(wxString(s.c_str()));
    }
    _active = true;
}

RotoZoomParms::RotoZoomParms(const std::string& id, float rotations, float zooms, float zoommaximum, int x, int y)
{
    _active = false;
    _id = id;
    ApplySettings(rotations, zooms, zoommaximum, x, y);
}

void RotoZoomParms::ApplySettings(float rotations, float zooms, float zoommaximum, int x, int y)
{
    _rotations = rotations;
    _zooms = zooms;
    _zoommaximum = zoommaximum;
    _xcenter = x;
    _ycenter = y;
}

void RotoZoomParms::SetDefault(wxSize size)
{
    _rotations = 1;
    _zooms = 1;
    _xcenter = size.GetWidth() / 2;
    _ycenter = size.GetHeight() / 2;
    _active = false;
}

std::string RotoZoomParms::Serialise()
{
    std::string res = "";

    if (IsActive())
    {
        res += _id + "_Id=" + _id + "|";
        res += _id + "_Rotations=" + std::string(wxString::Format("%f", _rotations).c_str()) + "|";
        res += _id + "_Zooms=" + std::string(wxString::Format("%f", _zooms).c_str()) + "|";
        res += _id + "_ZoomMaximum=" + std::string(wxString::Format("%f", _zoommaximum).c_str()) + "|";
        res += _id + "_XCenter=" + std::string(wxString::Format("%d", _xcenter).c_str()) + "|";
        res += _id + "_YCenter=" + std::string(wxString::Format("%d", _ycenter).c_str()) + "|";
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

wxPoint RotoZoomParms::GetTransform(int x, int y, float offset)
{
    float PI_2 = 6.283185307179586476925286766559f;
    float angle = PI_2 * _rotations / 10.0f * offset;
    float scale = sin(PI_2 * (_zooms / 10.0f) * offset) * _zoommaximum / 5.0f;

    if (scale < 0.3f)
    {
        scale = 0.3f;
    }

    float u = cos(-angle) * (x + _xcenter) * (1.0f / scale)
        + sin(-angle) * (y + _ycenter) * (1.0f / scale);
    float v = -sin(-angle) * (x + _xcenter) * (1.0f / scale)
        + cos(-angle) * (y + _ycenter) * (1.0f / scale);

    return wxPoint(u, v);
}