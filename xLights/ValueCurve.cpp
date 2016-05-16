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
    }
    _values.sort();
}
ValueCurve::ValueCurve(float min, float max, float parameter1, float parameter2, float parameter3)
{
    _min = min;
    _max = max;
    _parameter1 = parameter1;
    _parameter2 = parameter2;
    _parameter3 = parameter3;
}
ValueCurve::ValueCurve(std::string s)
{
    ValueCurve();
    Deserialise(s);
}
std::string ValueCurve::Serialise()
{
    std::string res = "";

    res += "vcType=" + _type + ",";
    res += "vcMin=" + std::string(wxString::Format("%f", _min).c_str()) + ",";
    res += "vcMax=" + std::string(wxString::Format("%f", _max).c_str()) + ",";
    res += "vcP1=" + std::string(wxString::Format("%f", _parameter1).c_str()) + ",";
    res += "vcP2=" + std::string(wxString::Format("%f", _parameter2).c_str()) + ",";
    res += "vcP3=" + std::string(wxString::Format("%f", _parameter3).c_str()) + ",";
    res += "vcValues=";
    for (auto it = _values.begin(); it != _values.end(); ++it)
    {
        res += "" + std::string(wxString::Format("%f", it->x).c_str()) + ":" + std::string(wxString::Format("%f", it->y).c_str());
        if (!(*it == _values.back()))
        {
            res += ";";
        }
    }
    res += ",";

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

void ValueCurve::Deserialise(std::string s)
{
    _values.clear();
    std::list<std::string> settings = tokenise(s, ',');

    for (auto setting = settings.begin(); setting != settings.end(); setting++)
    {
        std::list<std::string> setab = tokenise(*setting, '=');
        if (setab.front() == "vcType")
        {
            _type = setab.back();
        }
        else if (setab.front() == "vcMin")
        {
            _min = wxAtof(wxString(setab.back().c_str()));
        }
        else if (setab.front() == "vcMax")
        {
            _max = wxAtof(wxString(setab.back().c_str()));
        }
        else if (setab.front() == "vcP1")
        {
            _parameter1 = wxAtof(wxString(setab.back().c_str()));
        }
        else if (setab.front() == "vcP2")
        {
            _parameter2 = wxAtof(wxString(setab.back().c_str()));
        }
        else if (setab.front() == "vcP3")
        {
            _parameter3 = wxAtof(wxString(setab.back().c_str()));
        }
        else if (setab.front() == "vcValues")
        {
            std::list<std::string> points = tokenise(setab.back(), ';');
            for (auto p = points.begin(); p != points.end(); p++)
            {
                std::list<std::string> xy = tokenise(*p, ':');
                _values.push_back(vcSortablePoint(wxAtof(wxString(xy.front().c_str())), wxAtof(wxString(xy.back().c_str()))));
            }
        }
    }
    _values.sort();
}

void ValueCurve::SetType(std::string type)
{
    _type = type;
    RenderType();
}

float ValueCurve::GetValueAt(float offset)
{
    if (_values.size() == 0) return 1.0f;

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
