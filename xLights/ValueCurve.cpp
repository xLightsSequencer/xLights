#include "ValueCurve.h"
#include <sstream>
#include <iostream>

void ValueCurve::RenderType()
{
    if (_type != "Custom")
    {
        _values.clear();
    }
    else
    {
        if (_type == "Flat")
        {
            _values.push_back(vcSortablePoint(0.0f, _parameter1));
            _values.push_back(vcSortablePoint(1.0f, _parameter1));
        }
        else if (_type == "Ramp")
        {
            _values.push_back(vcSortablePoint(0.0f, _parameter1));
            _values.push_back(vcSortablePoint(1.0f, _parameter2));
        }
        else if (_type == "Ramp Up/Down")
        {
            _values.push_back(vcSortablePoint(0.0f, _parameter1));
            _values.push_back(vcSortablePoint(0.5f, _parameter2));
            _values.push_back(vcSortablePoint(1.0f, _parameter3));
        }
        else if (_type == "Ramp Up/Down Hold")
        {
            _values.push_back(vcSortablePoint(0.0f, _parameter1));
            _values.push_back(vcSortablePoint(0.5f - ((0.5f * (float)_parameter3) / 100.0), _parameter2));
            _values.push_back(vcSortablePoint(0.5f + ((0.5f * (float)_parameter3) / 100.0), _parameter2));
            _values.push_back(vcSortablePoint(1.0f, _parameter1));
        }
        else if (_type == "Saw Tooth")
        {
            int count = _parameter3;
            if (count < 1)
            {
                count = 1;
            }
            float per = 1.0f / count;
            _values.push_back(vcSortablePoint(0.0f, _parameter1));
            for (int i = 0; i < count; i++)
            {
                _values.push_back(vcSortablePoint(i * per + per / 2.0f, _parameter2));
                _values.push_back(vcSortablePoint((i + 1) * per, _parameter1));
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
            for (float i = 0.0f; i <= 1.0f; i+=0.05f)
            {
                _values.push_back(vcSortablePoint(i, a * (i - 0.5f) * (i - 0.5f) + _parameter2 * i + 0.0));
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
            for (float i = 0.0f; i <= 1.0f; i += 0.05f)
            {
                _values.push_back(vcSortablePoint(i, a * (i - 0.5f) * (i - 0.5f) + _parameter2 * i + _parameter3));
            }
        }
        else if (_type == "Logarithmic Up")
        {
            // p1 rate
            int a = _parameter1;
            if (_parameter1 == 0)
            {
                a = 1;
            }
            // y = log(ax - 1);
            for (float i = 0.0f; i <= 1.0f; i += 0.05f)
            {
                _values.push_back(vcSortablePoint(i, log(a*i-1)));
            }
        }
        else if (_type == "Logarithmic Down")
        {
            // p1 rate
            int a = _parameter1;
            if (_parameter1 == 0)
            {
                a = 1;
            }
            // y = 1.0+ -2^(ax-1.0);
            for (float i = 0.0f; i <= 1.0f; i += 0.05f)
            {
                _values.push_back(vcSortablePoint(i, 1.0f + -1 * exp2(a * i - 1.0f)));
            }
        }
        else if (_type == "Sine")
        {
            // p1 - offset in cycle
            // p2 - maxy
            // p3 - cycles
            // one cycle = 2* PI
            static const double pi2 = 6.283185307;
            float maxx = pi2 * std::max((float)_parameter3/10.0f, 0.1f);
            for (float i = 0.0f; i < 1.0f; i += 0.01f)
            {
                float r = i * maxx + (((float)_parameter1 * pi2) / 100.0f);
                _values.push_back(vcSortablePoint(i, (sin(r) * (std::max((float)_parameter2,1.0f) / 100.0f)) + 0.5f));
            }
        }
    }
    _values.sort();
}
ValueCurve::ValueCurve(const std::string& id, float min, float max, float parameter1, float parameter2, float parameter3)
{
    _id = id;
    _min = min;
    _max = max;
    _parameter1 = parameter1;
    _parameter2 = parameter2;
    _parameter3 = parameter3;
    _active = false;
}
std::string ValueCurve::Serialise()
{
    std::string res = "";

    res += _id + "_Type=" + _type + ",";
    res += _id + "_Min=" + std::string(wxString::Format("%f", _min).c_str()) + ",";
    res += _id + "_Max=" + std::string(wxString::Format("%f", _max).c_str()) + ",";
    res += _id + "_P1=" + std::string(wxString::Format("%f", _parameter1).c_str()) + ",";
    res += _id + "_P2=" + std::string(wxString::Format("%f", _parameter2).c_str()) + ",";
    res += _id + "_P3=" + std::string(wxString::Format("%f", _parameter3).c_str()) + ",";
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
        res += ",";
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

bool vcEndsWith(const std::string& in, const std::string& what)
{
    return (in.find_last_of(what) == (in.length() - what.length()));
}

void ValueCurve::SetSerialisedValue(std::string k, std::string s)
{
    _values.clear();
        if (vcEndsWith(k, "_Type"))
        {
            _type = s;
        }
        else if (vcEndsWith(k, "_Min"))
        {
            _min = wxAtof(wxString(s.c_str()));
        }
        else if (vcEndsWith(k, "_Max"))
        {
            _max = wxAtof(wxString(s.c_str()));
        }
        else if (vcEndsWith(k, "_P1"))
        {
            _parameter1 = wxAtof(wxString(s.c_str()));
        }
        else if (vcEndsWith(k, "_P2"))
        {
            _parameter2 = wxAtof(wxString(s.c_str()));
        }
        else if (vcEndsWith(k, "_P3"))
        {
            _parameter3 = wxAtof(wxString(s.c_str()));
        }
        else if (vcEndsWith(k, "_Values"))
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

float ValueCurve::GetValueAt(float offset)
{
    if (_values.size() == 0) return 1.0f;
    if (!_active) return 1.0f;

    vcSortablePoint last = _values.front();
    auto it = _values.begin();
    while (*it < offset && it != _values.end())
    {
        last = *it;
        ++it;
    }

    if (it == _values.end())
    {
        return _values.back().y;
    }

    // straight line interpolation between last and *it
    return last.y + (it->y - last.y) * (offset - last.x) / (it->x - last.x);
}

void ValueCurve::SetValueAt(float offset, float value)
{
    auto it = _values.begin();
    while (it != _values.end() && *it <= offset)
    {
        if (*it == offset)
        {
            _values.remove(*it);
        }
    }
    _values.push_back(vcSortablePoint(offset, value));
    _values.sort();
}
