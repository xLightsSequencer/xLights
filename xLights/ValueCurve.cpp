#include "ValueCurve.h"
#include <wx/wx.h>
#include <wx/string.h>
#include <log4cpp/Category.hh>

void ValueCurve::SetUnscaledParameter1(float v)
{
    _parameter1 = SafeParameter(1, v * 100.0 / _max);
    RenderType();
}

float ValueCurve::SafeParameter(size_t p, float v)
{
    return std::min(100.0f, std::max(0.0f, v));
}

float ValueCurve::Safe01(float v)
{
    return std::min(1.0f, std::max(0.0f, v));
}

void ValueCurve::FixChangedScale(float oldmin, float oldmax)
{
    if (_type == "Custom")
    {
#pragma todo lots to do here    
    }
    else if (_type == "Flat")
    {
    }
    else if (_type == "Ramp")
    {
    }
    else if (_type == "Ramp Up/Down")
    {
    }
    else if (_type == "Ramp Up/Down Hold")
    {
    }
    else if (_type == "Saw Tooth")
    {
    }
    else if (_type == "Square")
    {
    }
    else if (_type == "Parabolic Down")
    {
    }
    else if (_type == "Parabolic Up")
    {
    }
    else if (_type == "Logarithmic Up")
    {
    }
    else if (_type == "Logarithmic Down")
    {
    }
    else if (_type == "Exponential Up")
    {
    }
    else if (_type == "Exponential Down")
    {
    }
    else if (_type == "Sine")
    {
    }
    else if (_type == "Abs Sine")
    {
    }
}

void ValueCurve::RenderType()
{
    if (_type != "Custom")
    {
        _values.clear();
    }
    if (_type == "Flat")
    {
        _values.push_back(vcSortablePoint(0.0f, (float)_parameter1 / 100.0, false));
        _values.push_back(vcSortablePoint(1.0f, (float)_parameter1 / 100.0, false));
    }
    else if (_type == "Ramp")
    {
        _values.push_back(vcSortablePoint(0.0f, (float)_parameter1 / 100.0, false));
        _values.push_back(vcSortablePoint(1.0f, (float)_parameter2 / 100.0, false));
    }
    else if (_type == "Ramp Up/Down")
    {
        _values.push_back(vcSortablePoint(0.0f, (float)_parameter1 / 100.0, false));
        _values.push_back(vcSortablePoint(0.5f, (float)_parameter2 / 100.0, false));
        _values.push_back(vcSortablePoint(1.0f, (float)_parameter3 / 100.0, false));
    }
    else if (_type == "Ramp Up/Down Hold")
    {
        _values.push_back(vcSortablePoint(0.0f, (float)_parameter1 / 100.0, false));
        _values.push_back(vcSortablePoint(0.5f - ((0.5f * (float)_parameter3) / 100.0), (float)_parameter2 / 100.0, false));
        _values.push_back(vcSortablePoint(0.5f + ((0.5f * (float)_parameter3) / 100.0), (float)_parameter2 / 100.0, false));
        _values.push_back(vcSortablePoint(1.0f, (float)_parameter1 / 100.0, false));
    }
    else if (_type == "Saw Tooth")
    {
        int count = _parameter3;
        if (count < 1)
        {
            count = 1;
        }
        float per = 1.0f / count;
        _values.push_back(vcSortablePoint(0.0f, (float)_parameter1 / 100.0, false));
        for (int i = 0; i < count; i++)
        {
            _values.push_back(vcSortablePoint(i * per + per / 2.0f, (float)_parameter2 / 100.0, false));
            _values.push_back(vcSortablePoint((i + 1) * per, (float)_parameter1 / 100.0, false));
        }
    }
    else if (_type == "Square")
    {
        int count = _parameter3;
        if (count < 1)
        {
            count = 1;
        }
        float per = 1.0f / (2 * count);
        bool low = true;

        for (int i = 0; i < 2 * count; i++)
        {
            if (low)
            {
                if (i != 0)
                {
                    _values.push_back(vcSortablePoint((float)i * per - 0.01f, (float)_parameter2 / 100.0, false));
                }
                _values.push_back(vcSortablePoint((float)i * per, (float)_parameter1 / 100.0, false));
            }
            else
            {
                _values.push_back(vcSortablePoint((float)i * per - 0.01f, (float)_parameter1 / 100.0, false));
                _values.push_back(vcSortablePoint((float)i * per, (float)_parameter2 / 100.0, false));
            }
            low = !low;
        }
        _values.push_back(vcSortablePoint(1.0f, (float)_parameter2 / 100.0, false));
    }
    else if (_type == "Parabolic Down")
    {
        int a = _parameter1;
        if (_parameter1 == 0)
        {
            a = 1;
        }
        // y = a(x-h)^2 + bx + k
        // h = 0.5
        // k = 0.0
        for (double i = 0.0; i <= 1.01; i += 0.05)
        {
            if (i > 1.0) i = 1.0;
            float y = a * (i - 0.5f) * (i - 0.5f) + (float)_parameter2 / 100.0;
            bool wrapped = false;
            if (_wrap)
            {
                while (y > 1.0f)
                {
                    wrapped = true;
                    y -= 1.0f;
                }
                while (y < 0.0f)
                {
                    wrapped = true;
                    y += 1.0f;
                }
            }
            bool lastwrapped = false;
            if (_values.size() > 0) _values.back().IsWrapped();
            _values.push_back(vcSortablePoint(i, Safe01(y), (lastwrapped != wrapped)));
        }
    }
    else if (_type == "Parabolic Up")
    {
        int a = -1 * _parameter1;
        if (_parameter1 == 0)
        {
            a = -1;
        }
        // y = -1 * a(x-h)^2 + bx + k
        // h = 0.5
        // k = c
        for (double i = 0.0; i <= 1.01; i += 0.05)
        {
            if (i > 1.0) i = 1.0;
            float y = a * (i - 0.5f) * (i - 0.5f) + (float)_parameter2 / 100.0;
            bool wrapped = false;
            if (_wrap)
            {
                while (y > 1.0f)
                {
                    wrapped = true;
                    y -= 1.0f;
                }
                while (y < 0.0f)
                {
                    wrapped = true;
                    y += 1.0f;
                }
            }
            bool lastwrapped = false;
            if (_values.size() > 0) _values.back().IsWrapped();
            _values.push_back(vcSortablePoint(i, Safe01(y), (lastwrapped != wrapped)));
        }
    }
    else if (_type == "Logarithmic Up")
    {
        // p1 rate
        float a = (float)_parameter1 / 25.0f;
        if (_parameter1 == 0)
        {
            a = 0.04f;
        }
        // y = log(ax - 1);
        for (double i = 0.0; i <= 1.01; i += 0.05)
        {
            if (i > 1.0) i = 1.0;
            float y = ((float)_parameter2 - 50.0) / 50.0 + log(a + a*i) + 1.0f;
            bool wrapped = false;
            if (_wrap)
            {
                while (y > 1.0f)
                {
                    wrapped = true;
                    y -= 1.0f;
                }
                while (y < 0.0f)
                {
                    wrapped = true;
                    y += 1.0f;
                }
            }
            bool lastwrapped = false;
            if (_values.size() > 0) _values.back().IsWrapped();
            _values.push_back(vcSortablePoint(i, Safe01(y), (lastwrapped != wrapped)));
        }
    }
    else if (_type == "Logarithmic Down")
    {
        // p1 rate
        float a = (float)_parameter1 / 10.0f;
        if (_parameter1 == 0)
        {
            a = 0.1f;
        }
        // y = 1.0+ -2^(ax-1.0);
        for (double i = 0.0; i <= 1.01; i += 0.05)
        {
            if (i > 1.0) i = 1.0;
            float y = ((float)_parameter2 - 50.0) / 50.0 + 1.5f + -1 * exp2(a * i - 1.0f);
            bool wrapped = false;
            if (_wrap)
            {
                while (y > 1.0f)
                {
                    wrapped = true;
                    y -= 1.0f;
                }
                while (y < 0.0f)
                {
                    wrapped = true;
                    y += 1.0f;
                }
            }
            bool lastwrapped = false;
            if (_values.size() > 0) _values.back().IsWrapped();
            _values.push_back(vcSortablePoint(i, Safe01(y), (lastwrapped != wrapped)));
        }
    }
    else if (_type == "Exponential Up")
    {
        // p1 rate
        float a = (float)_parameter1 / 10.0f;
        if (_parameter1 == 0)
        {
            a = 0.1f;
        }
        // y = 1.0+ -2^(ax-1.0);
        for (double i = 0.0; i <= 1.01; i += 0.05)
        {
            if (i > 1.0) i = 1.0;
            float y = ((float)_parameter2 - 50.0) / 50.0 + (exp(a*i) - 1.0) / (exp(a) - 1.0);
            bool wrapped = false;
            if (_wrap)
            {
                while (y > 1.0f)
                {
                    wrapped = true;
                    y -= 1.0f;
                }
                while (y < 0.0f)
                {
                    wrapped = true;
                    y += 1.0f;
                }
            }
            bool lastwrapped = false;
            if (_values.size() > 0) _values.back().IsWrapped();
            _values.push_back(vcSortablePoint(i, Safe01(y), (lastwrapped != wrapped)));
        }
    }
    else if (_type == "Exponential Down")
    {
        // p1 rate
        float a = (float)_parameter1 / 10.0f;
        if (_parameter1 == 0)
        {
            a = 0.1f;
        }
        // y = 1.0+ -2^(ax-1.0);
        for (double i = 0.0; i <= 1.01; i += 0.05)
        {
            if (i > 1.0) i = 1.0;
            float y = ((float)_parameter2 - 50.0) / 50.0 + 1.0 - (exp(a*i) - 1.0) / (exp(a) - 1.0);
            bool wrapped = false;
            if (_wrap)
            {
                while (y > 1.0f)
                {
                    wrapped = true;
                    y -= 1.0f;
                }
                while (y < 0.0f)
                {
                    wrapped = true;
                    y += 1.0f;
                }
            }
            bool lastwrapped = false;
            if (_values.size() > 0) _values.back().IsWrapped();
            _values.push_back(vcSortablePoint(i, Safe01(y), (lastwrapped != wrapped)));
        }
    }
    else if (_type == "Sine")
    {
        // p1 - offset in cycle
        // p2 - maxy
        // p3 - cycles
        // one cycle = 2* PI
        static const double pi2 = 6.283185307;
        float maxx = pi2 * std::max((float)_parameter3 / 10.0f, 0.1f);
        for (double i = 0.0; i <= 1.01; i += 0.025)
        {
            if (i > 1.0) i = 1.0;
            float r = i * maxx + (((float)_parameter1 * pi2) / 100.0f);
            float y = ((float)_parameter4 - 50.0) / 50.0 + (sin(r) * (std::max((float)_parameter2, 1.0f) / 200.0f)) + 0.5f;
            bool wrapped = false;
            if (_wrap)
            {
                while (y > 1.0f)
                {
                    wrapped = true;
                    y -= 1.0f;
                }
                while (y < 0.0f)
                {
                    wrapped = true;
                    y += 1.0f;
                }
            }
            bool lastwrapped = false;
            if (_values.size() > 0) _values.back().IsWrapped();
            _values.push_back(vcSortablePoint(i, Safe01(y), (lastwrapped != wrapped)));
        }
    }
    else if (_type == "Abs Sine")
    {
        // p1 - offset in cycle
        // p2 - maxy
        // p3 - cycles
        // one cycle = 2* PI
        static const double pi2 = 6.283185307;
        float maxx = pi2 * std::max((float)_parameter3 / 10.0f, 0.1f);
        for (double i = 0.0; i <= 1.01; i += 0.025)
        {
            if (i > 1.0) i = 1.0;
            float r = i * maxx + (((float)_parameter1 * pi2) / 100.0f);
            float y = ((float)_parameter4 - 50.0) / 50.0 + (std::abs(sin(r) * (std::max((float)_parameter2, 1.0f) / 100.0f)));
            bool wrapped = false;
            if (_wrap)
            {
                while (y > 1.0f)
                {
                    y -= 1.0f;
                    wrapped = true;
                }
                while (y < 0.0f)
                {
                    wrapped = true;
                    y += 1.0f;
                }
            }
            bool lastwrapped = false;
            if (_values.size() > 0) _values.back().IsWrapped();
            _values.push_back(vcSortablePoint(i, Safe01(y), (lastwrapped != wrapped)));
        }
    }
    _values.sort();
}

ValueCurve::ValueCurve(const std::string& id, float min, float max, const std::string type, float parameter1, float parameter2, float parameter3, float parameter4, bool wrap, float divisor)
{
    _type = type;
    _id = id;
    _min = min;
    _max = max;
    _wrap = wrap;
    _divisor = divisor;
    _parameter1 = SafeParameter(1, parameter1);
    _parameter2 = SafeParameter(2, parameter2);
    _parameter3 = SafeParameter(3, parameter3);
    _parameter4 = SafeParameter(4, parameter4);
    _active = false;
    RenderType();
}

void ValueCurve::SetDefault(float min, float max, int divisor)
{
    _type = "Flat";
    if (min != -9.1234f)
    {
        _min = min;
    }
    if (max != -9.1234f)
    {
        _max = max;
    }
    _parameter1 = SafeParameter(1, 0);
    _parameter2 = SafeParameter(2, 0);
    _parameter3 = SafeParameter(3, 0);
    _parameter4 = SafeParameter(4, 0);
    _active = false;
    _wrap = false;
    if (divisor != 91234)
    {
        _divisor = divisor;
    }
    RenderType();
}

ValueCurve::ValueCurve(const std::string& s)
{
    _min = -9.1234f;
    _max = -9.1234f;
    SetDefault();
    _divisor = 1;
    Deserialise(s);
}

void ValueCurve::Deserialise(const std::string& s)
{
    if (s == "")
    {
        SetDefault(0, 100);
        _active = false;
    }
    else if (s.find('|') == std::string::npos)
    {
        SetDefault(0, 100);
        _active = false;
        _id = s;
    }
    else
    {
        _active = true;
        _values.clear();
        _type = "Flat";
        _parameter1 = 0.0f;
        _parameter2 = 0.0f;
        _parameter3 = 0.0f;
        _parameter4 = 0.0f;
        _wrap = false;

        float oldmin = _min;
        float oldmax = _max;

        wxArrayString v = wxSplit(wxString(s.c_str()), '|');
        for (auto vs = v.begin(); vs != v.end(); vs++)
        {
            wxArrayString v1 = wxSplit(*vs, '=');
            if (v1.size() == 2)
            {
                SetSerialisedValue(v1[0].ToStdString(), v1[1].ToStdString());
            }
        }

        if ((oldmin != -9.1234f && oldmin != _min) || (oldmax != -9.1234f && oldmax != _max))
        {
            FixChangedScale(oldmin, oldmax);
        }

        RenderType();
    }
}
std::string ValueCurve::Serialise()
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
        res += "Min=" + std::string(wxString::Format("%.2f", _min).c_str()) + "|";
        res += "Max=" + std::string(wxString::Format("%.2f", _max).c_str()) + "|";
        if (_parameter1 != 0)
        {
            res += "P1=" + std::string(wxString::Format("%d", (int)_parameter1).c_str()) + "|";
        }
        if (_parameter2 != 0)
        {
            res += "P2=" + std::string(wxString::Format("%d", (int)_parameter2).c_str()) + "|";
        }
        if (_parameter3 != 0)
        {
            res += "P3=" + std::string(wxString::Format("%d", (int)_parameter3).c_str()) + "|";
        }
        if (_parameter4 != 0)
        {
            res += "P4=" + std::string(wxString::Format("%d", (int)_parameter4).c_str()) + "|";
        }
        if (_wrap)
        {
            res += "WRAP=TRUE|";
        }
        if (_type == "Custom")
        {
            RemoveExcessCustomPoints();
            res += "Values=";
            for (auto it = _values.begin(); it != _values.end(); ++it)
            {
                res += std::string(wxString::Format("%.2f", it->x).c_str()) + ":" + std::string(wxString::Format("%.2f", it->y).c_str());
                if (!(*it == _values.back()))
                {
                    res += ";";
                }
            }
            res += "|";
        }
    }
    else
    {
        res += "Active=FALSE|";
    }
    return res;
}

void ValueCurve::SetSerialisedValue(std::string k, std::string s)
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
    else if (kk == "Min")
        {
            _min = wxAtof(wxString(s.c_str()));
        }
        else if (kk == "Max")
        {
            _max = wxAtof(wxString(s.c_str()));
        }
        else if (kk == "P1")
        {
            _parameter1 = (float)wxAtoi(wxString(s.c_str()));
        }
        else if (kk == "WRAP")
        {
            _wrap = true;
        }
        else if (kk == "P2")
        {
            _parameter2 = (float)wxAtoi(wxString(s.c_str()));
        }
        else if (kk == "P3")
        {
            _parameter3 = (float)wxAtoi(wxString(s.c_str()));
        }
        else if (kk == "P4")
        {
            _parameter4 = (float)wxAtoi(wxString(s.c_str()));
        }
        else if (kk == "Values")
        {
            wxArrayString points = wxSplit(s, ';');

            for (auto p = points.begin(); p != points.end(); p++)
            {
                wxArrayString xy = wxSplit(*p, ':');
                _values.push_back(vcSortablePoint(wxAtof(wxString(xy.front().c_str())), wxAtof(wxString(xy.back().c_str())), false));
            }
        }
    
    _values.sort();
    //_active = true;
}

void ValueCurve::SetType(std::string type)
{
    _type = type;
    RenderType();
}

float ValueCurve::GetOutputValue(float offset)
{
    return (_min + (_max - _min) * offset) / _divisor;
}

float ValueCurve::GetOutputValueAt(float offset)
{
    return _min + (_max - _min) * GetValueAt(offset);
}

float ValueCurve::GetValueAt(float offset)
{
    float res = 0.0f;

    if (_values.size() < 2) return 1.0f;
    if (!_active) return 1.0f;

    if (offset < 0.0f) offset = 0.0;
    if (offset > 1.0f) offset = 1.0;

    vcSortablePoint last = _values.front();
    auto it = _values.begin();
    it++;

    while (it != _values.end() && it->x < offset)
    {
        last = *it;
        ++it;
    }

    if (it == _values.end())
    {
        res = _values.back().y;
    }
    else if (it->x == last.x)
    {
        // this should not be possible
        res = it->y;
    }
    else
    {
        if (it->x == offset)
        {
            res = it->y;
        }
        else if (it->IsWrapped())
        {
            res = it->y;
        }
        else
        {
            res = last.y + (it->y - last.y) * (offset - last.x) / (it->x - last.x);
        }
    }

    if (res < 0.0f)
    {
        res = 0.0f;
    }
    if (res > 1.0f)
    {
        res = 1.0f;
    }

    return res;
}

bool ValueCurve::IsSetPoint(float offset)
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

void ValueCurve::DeletePoint(float offset)
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

void ValueCurve::RemoveExcessCustomPoints()
{
    // go through list and remove middle points where 3 in a row have the same value
    auto it1 = _values.begin();
    auto it2 = it1;
    it2++;
    auto it3 = it2;
    it3++;

    while (it1 != _values.end() && it2 != _values.end() && it3 != _values.end())
    {
        if (it1->y == it2->y && it2->y == it3->y)
        {
            _values.remove(*it2);
            it2 = it1;
            it2++;
            it3 = it2;
            it3++;
        }
        else
        {
            it1++;
            it2++;
            it3++;
        }
    }
}

void ValueCurve::SetValueAt(float offset, float value)
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
    _values.push_back(vcSortablePoint(offset, value, false));
    _values.sort();
}

void ValueCurve::SetWrap(bool wrap) 
{ 
    _wrap = wrap; 

    if (!_wrap)
    {
        for (auto it = _values.begin(); it != _values.end(); it++)
        {
            it->ClearWrap();
        }
    }

    RenderType(); 
}

float ValueCurve::FindMinPointLessThan(float point)
{
    float res = 0.0;

    for (auto it = _values.begin(); it != _values.end(); it++)
    {
        if (it->x < point)
        {
            res = it->x + 0.025;
        }
    }

    return vcSortablePoint::Normalise(res);
}
float ValueCurve::FindMaxPointGreaterThan(float point)
{
    float res = 1.0;

    for (auto it = _values.begin(); it != _values.end(); it++)
    {
        if (it->x > point)
        {
            res = it->x - 0.025;
            break;
        }
    }

    return vcSortablePoint::Normalise(res);
}

bool ValueCurve::NearCustomPoint(float x, float y)
{
    for (auto it = _values.begin(); it != _values.end(); it++)
    {
        if (it->IsNear(x, y))
        {
            return true;
        }
    }

    return false;
}

wxBitmap ValueCurve::GetImage(int width, int height)
{
    wxBitmap bmp(width, height);

    wxMemoryDC dc(bmp);
    dc.SetBrush(*wxLIGHT_GREY_BRUSH);
    dc.DrawRectangle(0, 0, width - 1, height - 1);
    dc.SetPen(*wxBLACK_PEN);
    float lastY = height - 1 - (GetValueAt(0)) * height;

    for (int x = 1; x < width; x++) {
        float x1 = x;
        x1 /= width;
        float y = (GetValueAt(x1)) * width;
        y = height - 1 - y;
        dc.DrawLine(x - 1, lastY, x, y);
        lastY = y;
    }

    return bmp;
}