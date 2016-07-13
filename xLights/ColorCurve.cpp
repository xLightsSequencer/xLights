#include "ColorCurve.h"
#include <sstream>
#include <iostream>
#include <wx/wx.h>
#include <wx/string.h>
#include <log4cpp/Category.hh>
#include <wx/bitmap.h>

#include <wx/graphics.h>
#if wxUSE_GRAPHICS_CONTEXT == 0
#error Please refer to README.windows to make necessary changes to wxWidgets setup.h file.
#error You will also need to rebuild wxWidgets once the change is made.
#endif

float ColorCurve::Safe01(float v)
{
    return std::min(1.0f, std::max(0.0f, v));
}

ColorCurve::ColorCurve(const std::string& id, const std::string type, xlColor c)
{
    _type = type;
    _id = id;
    _values.push_back(ccSortableColorPoint(0, c, 0, 0));
    _active = false;
}

ColorCurve::ColorCurve(const std::string& s)
{
    Deserialise(s);
}

void ColorCurve::Deserialise(const std::string& s)
{
    if (s == "")
    {
        _active = false;
        _values.clear();
    }
    else if (s.find('|') == std::string::npos)
    {
        _active = false;
        _id = s;
        _values.clear();
    }
    else
    {
        _active = true;
        _values.clear();
        _type = "Gradient";
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

    if (_values.size() == 0)
    {
        _values.push_back(ccSortableColorPoint(0, xlBLACK, 0, 0));
    }
}
std::string ColorCurve::Serialise()
{
    std::string res = "";

    if (IsActive())
    {
        res += "Active=TRUE|";
        res += "Id=" + _id + "|";
        if (_type != "Flat")
        {
            res += "Type=" + _type + "|";
        }
        res += "Values=";
        for (auto it = _values.begin(); it != _values.end(); ++it)
        {
            res += it->Serialise();
            if (!(*it == _values.back()))
            {
                res += ";";
            }
        }
        res += "|";
    }
    else
    {
        res += "Active=FALSE|";
    }
    return res;
}

void ColorCurve::SetSerialisedValue(std::string k, std::string s)
{
    wxString kk = wxString(k.c_str());
    if (kk == "Id")
    {
        _id = s;
    }
    else if (kk == "Active")
    {
        if (s == "FALSE")
        {
            _active = false;
        }
        else
        {
            // it should already be true
            wxASSERT(_active == true);
        }
    }
    else if (kk == "Type")
    {
        _type = s;
    }
        else if (kk == "Values")
        {
            wxArrayString points = wxSplit(s, ';');

            for (auto p = points.begin(); p != points.end(); p++)
            {
                _values.push_back(ccSortableColorPoint(p->ToStdString()));
            }
        }
    
    _values.sort();
    //_active = true;
}

void ColorCurve::SetType(std::string type)
{
    _type = type;
}

uint8_t ChannelBlend(uint8_t c1, uint8_t c2, float ratio)
{
    return c1 + floor(ratio*(c2 - c1) + 0.5);
}

xlColor GetGradientColor(float ratio, xlColor& c1, xlColor& c2)
{
    return xlColor(ChannelBlend(c1.Red(), c2.Red(), ratio), ChannelBlend(c1.Green(), c2.Green(), ratio), ChannelBlend(c1.Blue(), c2.Blue(), ratio));
}

xlColor ColorCurve::GetValueAt(float offset)
{
    if (_type == "Gradient")
    {
        float start = 0;
        float end = 1;
        xlColor startc = xlBLACK;
        xlColor endc = xlBLACK;

        // find the value before the offset
        float d = 0;
        ccSortableColorPoint* pt = GetActivePoint(offset, d);

        if (offset - pt->x < 0.5 * d)
        {
            end = pt->x + 0.5 *d;
            endc = pt->color;

            float dp = 0;
            ccSortableColorPoint* ptp = GetPriorActivePoint(offset, dp);

            if (ptp == NULL)
            {
                start = 0;
                startc = pt->color;
            }
            else
            {
                start = ptp->x + dp * 0.5;
                startc = ptp->color;
            }
        }
        else
        {
            start = pt->x + 0.5 * d;
            startc = pt->color;

            float dn = 0;
            ccSortableColorPoint* ptn = GetNextActivePoint(offset, dn);

            if (ptn == NULL)
            {
                end = 1.0;
                endc = pt->color;
            }
            else
            {
                end = ptn->x + dn * 0.5;
                endc = ptn->color;
            }
        }

        return GetGradientColor((offset - start) / (end - start), startc, endc);
    }
    else if (_type == "Sharp")
    {
        // find the value immediately before the offset ... that is the color to return
        float d = 0;
        ccSortableColorPoint* pt = GetActivePoint(offset, d);
        return pt->color;
    }
    else if (_type == "Fade")
    {
        // find the value immediately before the offset
        float d = 0;
        ccSortableColorPoint* pt = GetActivePoint(offset, d);

        if (d * pt->fadein > offset - pt->x)
        {
            // in fade in
            xlColor lastc = xlBLACK;
            float dp = 0;
            ccSortableColorPoint* ptp = GetPriorActivePoint(offset, dp);
            if (ptp == NULL)
            {
                lastc = pt->color;
            }
            else
            {
                if (ptp->fadeout == 0)
                {
                    lastc = ptp->color;
                }
            }
            return xlColor(lastc.red + pt->color.red * (offset - pt->x) / (d * pt->fadein),
                lastc.green + pt->color.green * (offset - pt->x) / (d * pt->fadein),
                lastc.blue + pt->color.blue * (offset - pt->x) / (d * pt->fadein)
                );
        }
        else if (d * pt->fadeout < offset - pt->x)
        {
            // in fade out
            xlColor nextc = xlBLACK;
            float dn = 0;
            ccSortableColorPoint* ptn = GetNextActivePoint(offset, dn);
            if (ptn == NULL)
            {
                nextc = pt->color;
            }
            else
            {
                if (ptn->fadein == 0)
                {
                    nextc = ptn->color;
                }
            }
            return xlColor(nextc.red + pt->color.red * (offset - pt->x) / (d - d * pt->fadeout),
                nextc.green + pt->color.green * (offset - pt->x) / (d - d * pt->fadeout),
                nextc.blue + pt->color.blue * (offset - pt->x) / (d - d * pt->fadeout)
            );
        }
    }

    return xlBLACK;
}

bool ColorCurve::IsSetPoint(float offset)
{
    auto it = _values.begin();
    while (it != _values.end() && *it <= offset)
    {
        if (*it == offset)
        {
            return true;
        }
        it++;
    }

    return false;
}

void ColorCurve::DeletePoint(float offset)
{
    if (GetPointCount() > 2)
    {
        auto it = _values.begin();
        while (it != _values.end() && *it <= offset)
        {
            if (*it == offset)
            {
                _values.remove(*it);
                break;
            }
            it++;
        }
    }
}

void ColorCurve::SetValueAt(float offset, xlColor c, float fadein, float fadeout)
{
    auto it = _values.begin();
    while (it != _values.end() && *it <= offset)
    {
        if (*it == offset)
        {
            _values.remove(*it);
            break;
        }
        it++;
    }

    _values.push_back(ccSortableColorPoint(offset, c, fadein, fadeout));
    _values.sort();
}

wxImage ColorCurve::GetImage(int x, int y)
{
    wxImage bmp(x, y);
    wxBitmap b(bmp);
    wxMemoryDC dc(b);

    for (int i = 0; i < x; i++)
    {
        dc.SetPen(wxPen(GetValueAt((float)i / (float)x).asWxColor(), 1, wxPENSTYLE_SOLID));
        dc.DrawLine(wxPoint(i, 0), wxPoint(i, y));
    }
    return bmp;
}

bool ColorCurve::NearPoint(float x)
{
    for (auto it = _values.begin(); it != _values.end(); it++)
    {
        if (it->IsNear(x))
        {
            return true;
        }
    }

    return false;
}

ccSortableColorPoint* ColorCurve::GetActivePoint(float x, float& duration)
{
    ccSortableColorPoint* candidate = NULL;
    for (auto it = _values.begin(); it != _values.end(); it++)
    {
        if (*it <= x)
        {
            candidate = &(*it);
        }
        else
        {
            return candidate;
        }
    }

    return candidate;
}

ccSortableColorPoint* ColorCurve::GetPriorActivePoint(float x, float& duration)
{
    ccSortableColorPoint* candidate = NULL;
    ccSortableColorPoint* last = NULL;
    for (auto it = _values.begin(); it != _values.end(); it++)
    {
        if (*it <= x)
        {
            candidate = last;
            last = &(*it);
        }
        else
        {
            return candidate;
        }
    }

    return candidate;
}

ccSortableColorPoint* ColorCurve::GetNextActivePoint(float x, float& duration)
{
    for (auto it = _values.begin(); it != _values.end(); it++)
    {
        if (!(*it <= x))
        {
            return &(*it);
        }
    }

    return NULL;
}
