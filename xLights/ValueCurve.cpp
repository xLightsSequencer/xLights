#include "ValueCurve.h"
#include <sstream>
#include <iostream>
#include <wx/wx.h>
#include <wx/string.h>

float ValueCurve::SafeParameter(size_t p, float v)
{
    return std::min(100.0f, std::max(0.0f, v));
}

float ValueCurve::Safe01(float v)
{
    return std::min(1.0f, std::max(0.0f, v));
}

void ValueCurve::RenderType()
{
    if (_type != "Custom")
    {
        _values.clear();
    }
    if (_type == "Flat")
    {
        _values.push_back(vcSortablePoint(0.0f, (float)_parameter1 / 100.0));
        _values.push_back(vcSortablePoint(1.0f, (float)_parameter1 / 100.0));
    }
    else if (_type == "Ramp")
    {
        _values.push_back(vcSortablePoint(0.0f, (float)_parameter1 / 100.0));
        _values.push_back(vcSortablePoint(1.0f, (float)_parameter2 / 100.0));
    }
    else if (_type == "Ramp Up/Down")
    {
        _values.push_back(vcSortablePoint(0.0f, (float)_parameter1 / 100.0));
        _values.push_back(vcSortablePoint(0.5f, (float)_parameter2 / 100.0));
        _values.push_back(vcSortablePoint(1.0f, (float)_parameter3 / 100.0));
    }
    else if (_type == "Ramp Up/Down Hold")
    {
        _values.push_back(vcSortablePoint(0.0f, (float)_parameter1 / 100.0));
        _values.push_back(vcSortablePoint(0.5f - ((0.5f * (float)_parameter3) / 100.0), (float)_parameter2 / 100.0));
        _values.push_back(vcSortablePoint(0.5f + ((0.5f * (float)_parameter3) / 100.0), (float)_parameter2 / 100.0));
        _values.push_back(vcSortablePoint(1.0f, (float)_parameter1 / 100.0));
    }
    else if (_type == "Saw Tooth")
    {
        int count = _parameter3;
        if (count < 1)
        {
            count = 1;
        }
        float per = 1.0f / count;
        _values.push_back(vcSortablePoint(0.0f, (float)_parameter1 / 100.0));
        for (int i = 0; i < count; i++)
        {
            _values.push_back(vcSortablePoint(i * per + per / 2.0f, (float)_parameter2 / 100.0));
            _values.push_back(vcSortablePoint((i + 1) * per, (float)_parameter1 / 100.0));
        }
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
            _values.push_back(vcSortablePoint(i, Safe01(a * (i - 0.5f) * (i - 0.5f) + (float)_parameter2 / 100.0)));
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
            _values.push_back(vcSortablePoint(i, Safe01(a * (i - 0.5f) * (i - 0.5f) + (float)_parameter2 / 100.0)));
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
            _values.push_back(vcSortablePoint(i, Safe01(((float)_parameter2 - 50.0) / 50.0 + log(a*i)+1.0f)));
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
            _values.push_back(vcSortablePoint(i, Safe01(((float)_parameter2 - 50.0) / 50.0 + 1.5f + -1 * exp2(a * i - 1.0f))));
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
            _values.push_back(vcSortablePoint(i, Safe01(((float)_parameter4 - 50.0) / 50.0 + (sin(r) * (std::max((float)_parameter2, 1.0f) / 200.0f)) + 0.5f)));
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
            _values.push_back(vcSortablePoint(i, Safe01(((float)_parameter4 - 50.0) / 50.0 + (abs(sin(r) * (std::max((float)_parameter2, 1.0f) / 100.0f))))));
        }
    }
    _values.sort();
}

ValueCurve::ValueCurve(const std::string& id, float min, float max, const std::string type, float parameter1, float parameter2, float parameter3, float parameter4)
{
    _type = type;
    _id = id;
    _min = min;
    _max = max;
    _parameter1 = SafeParameter(1, parameter1);
    _parameter2 = SafeParameter(2, parameter2);
    _parameter3 = SafeParameter(3, parameter3);
    _parameter4 = SafeParameter(4, parameter4);
    _active = false;
    RenderType();
}

void ValueCurve::SetDefault(float min, float max)
{
    _type = "Flat";
    _min = min;
    _max = max;
    _parameter1 = SafeParameter(1, 0);
    _parameter2 = SafeParameter(2, 0);
    _parameter3 = SafeParameter(3, 0);
    _parameter4 = SafeParameter(4, 0);
    _active = false;
    RenderType();
}

void ValueCurve::Deserialise(std::string s)
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
        RenderType();
    }
}
std::string ValueCurve::Serialise()
{
    std::string res = "";

    if (IsActive())
    {
        res += _id + "_Id=" + _id + "|";
        res += _id + "_Type=" + _type + "|";
        res += _id + "_Min=" + std::string(wxString::Format("%f", _min).c_str()) + "|";
        res += _id + "_Max=" + std::string(wxString::Format("%f", _max).c_str()) + "|";
        res += _id + "_P1=" + std::string(wxString::Format("%f", _parameter1).c_str()) + "|";
        res += _id + "_P2=" + std::string(wxString::Format("%f", _parameter2).c_str()) + "|";
        res += _id + "_P3=" + std::string(wxString::Format("%f", _parameter3).c_str()) + "|";
        res += _id + "_P4=" + std::string(wxString::Format("%f", _parameter4).c_str()) + "|";
        if (_type == "Custom")
        {
            res += _id + "_Values=";
            for (auto it = _values.begin(); it != _values.end(); ++it)
            {
                res += "" + std::string(wxString::Format("%f", it->x).c_str()) + ":" + std::string(wxString::Format("%f", it->y).c_str());
                if (!(*it == _values.back()))
                {
                    res += ";";
                }
            }
        }
    }
    return res;
}

std::list<std::string> tokenise(std::string in, char token)
{
    std::list<std::string> res;
    std::istringstream f(in);
    std::string s;
    while (getline(f, s, token)) {
        res.push_back(s);
    }
    return res;
}

bool vcEndsWith(const wxString& in, const wxString& what)
{
    return in.EndsWith(what);
}

void ValueCurve::SetSerialisedValue(std::string k, std::string s)
{
    wxString kk = wxString(k.c_str());
    _values.clear();
    if (vcEndsWith(kk, "_Id"))
    {
        _id = s;
    }
    else if (vcEndsWith(kk, "_Type"))
    {
        _type = s;
    }
    else if (vcEndsWith(kk, "_Min"))
        {
            _min = wxAtof(wxString(s.c_str()));
        }
        else if (vcEndsWith(kk, "_Max"))
        {
            _max = wxAtof(wxString(s.c_str()));
        }
        else if (vcEndsWith(kk, "_P1"))
        {
            _parameter1 = wxAtof(wxString(s.c_str()));
        }
        else if (vcEndsWith(kk, "_P2"))
        {
            _parameter2 = wxAtof(wxString(s.c_str()));
        }
        else if (vcEndsWith(kk, "_P3"))
        {
            _parameter3 = wxAtof(wxString(s.c_str()));
        }
        else if (vcEndsWith(kk, "_P4"))
        {
            _parameter4 = wxAtof(wxString(s.c_str()));
        }
        else if (vcEndsWith(kk, "_Values"))
        {
            std::list<std::string> points = tokenise(s, ';');
            for (auto p = points.begin(); p != points.end(); p++)
            {
                std::list<std::string> xy = tokenise(*p, ':');
                _values.push_back(vcSortablePoint(wxAtof(wxString(xy.front().c_str())), wxAtof(wxString(xy.back().c_str()))));
            }
        }
    
    _values.sort();
    _active = true;
}

void ValueCurve::SetType(std::string type)
{
    _type = type;
    RenderType();
}

float ValueCurve::GetOutputValueAt(float offset)
{
    return _min + (_max - _min) * GetValueAt(offset);
}

float ValueCurve::GetValueAt(float offset)
{
    if (_values.size() < 2) return 1.0f;
    if (!_active) return 1.0f;

    vcSortablePoint last = _values.front();
    auto it = _values.begin();
    it++;
    while (it != _values.end() && *it < offset)
    {
        last = *it;
        ++it;
    }

    if (it == _values.end())
    {
        return _values.back().y;
    }

    // straight line interpolation between last and *it
    if (it->x == last.x)
    {
        // this should not be possible
        return it->y;
    }
    else
    {
        return last.y + (it->y - last.y) * (offset - last.x) / (it->x - last.x);
    }
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
    _values.push_back(vcSortablePoint(offset, value));
    _values.sort();
}
