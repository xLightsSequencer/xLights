#include "ValueCurve.h"
#include <wx/wx.h>
#include <wx/string.h>
#include "xLightsVersion.h"
#include "xLightsMain.h"
#include "xLightsXmlFile.h"
#include <wx/msgdlg.h>
#include "UtilFunctions.h"
#include <log4cpp/Category.hh>

float ValueCurve::SafeParameter(size_t p, float v)
{
    float low;
    float high;
    ValueCurve::GetRangeParm(p, _type, low, high);

    if (low == MINVOID) low = _min;
    if (high == MAXVOID) high = _max;

    wxASSERT(_min != MINVOIDF);
    wxASSERT(_max != MAXVOIDF);

    return std::min(high, std::max(low, v));
}

float ValueCurve::Safe01(float v)
{
    return std::min(1.0f, std::max(0.0f, v));
}

void ValueCurve::GetRangeParm(int parm, const std::string& type, float& low, float &high)
{
    switch (parm)
    {
    case 1:
        ValueCurve::GetRangeParm1(type, low, high);
        break;
    case 2:
        ValueCurve::GetRangeParm2(type, low, high);
        break;
    case 3:
        ValueCurve::GetRangeParm3(type, low, high);
        break;
    case 4:
        ValueCurve::GetRangeParm4(type, low, high);
        break;
    default:
        break;
    }
}

float ValueCurve::Denormalise(int parm, float value)
{
    float low;
    float high;
    ValueCurve::GetRangeParm(parm, _type, low, high);

    if (low == MINVOID)
    {
        wxASSERT(_min != MINVOIDF);
        low = _min;
    }
    if (high == MAXVOID)
    {
        wxASSERT(_max != MAXVOIDF);
        high = _max;
    }

    float res = value;

    if (low != 0 || high != 100)
    {
        float range = high - low;
        res = value * range / 100.0 + low;
    }

    if (res < low) res = low;
    if (res > high) res = high;

    return res;
}

void ValueCurve::ConvertToRealValues(float oldmin, float oldmax)
{
    float min = _min;
    _min = oldmin;
    float max = _max;
    _max = oldmax;

    _parameter1 = Denormalise(1, _parameter1);
    _parameter2 = Denormalise(2, _parameter2);
    _parameter3 = Denormalise(3, _parameter3);
    _parameter4 = Denormalise(4, _parameter4);

    _min = min;
    _max = max;
}

void ValueCurve::GetRangeParm1(const std::string& type, float& low, float &high)
{
    low = 0;
    high = 100;

    if (type == "Custom")
    {
    }
    else if (type == "Flat")
    {
        low = MINVOID;
        high = MAXVOID;
    }
    else if (type == "Random")
    {
        low = MINVOID;
        high = MAXVOID;
    }
    else if (type == "Ramp")
    {
        low = MINVOID;
        high = MAXVOID;
    }
    else if (type == "Ramp Up/Down")
    {
        low = MINVOID;
        high = MAXVOID;
    }
    else if (type == "Ramp Up/Down Hold")
    {
        low = MINVOID;
        high = MAXVOID;
    }
    else if (type == "Saw Tooth")
    {
        low = MINVOID;
        high = MAXVOID;
    }
    else if (type == "Square")
    {
        low = MINVOID;
        high = MAXVOID;
    }
    else if (type == "Parabolic Down")
    {
    }
    else if (type == "Parabolic Up")
    {
    }
    else if (type == "Logarithmic Up")
    {
    }
    else if (type == "Logarithmic Down")
    {
    }
    else if (type == "Exponential Up")
    {
    }
    else if (type == "Exponential Down")
    {
    }
    else if (type == "Sine")
    {
    }
    else if (type == "Decaying Sine")
    {
    }
    else if (type == "Abs Sine")
    {
    }
}

void ValueCurve::GetRangeParm2(const std::string& type, float& low, float &high)
{
    low = 0;
    high = 100;

    if (type == "Custom")
    {
    }
    else if (type == "Flat")
    {
    }
    else if (type == "Random")
    {
        low = MINVOID;
        high = MAXVOID;
    }
    else if (type == "Ramp")
    {
        low = MINVOID;
        high = MAXVOID;
    }
    else if (type == "Ramp Up/Down")
    {
        low = MINVOID;
        high = MAXVOID;
    }
    else if (type == "Ramp Up/Down Hold")
    {
        low = MINVOID;
        high = MAXVOID;
    }
    else if (type == "Saw Tooth")
    {
        low = MINVOID;
        high = MAXVOID;
    }
    else if (type == "Square")
    {
        low = MINVOID;
        high = MAXVOID;
    }
    else if (type == "Parabolic Down")
    {
        low = MINVOID;
        high = MAXVOID;
    }
    else if (type == "Parabolic Up")
    {
        low = MINVOID;
        high = MAXVOID;
    }
    else if (type == "Logarithmic Up")
    {
        low = MINVOID;
        high = MAXVOID;
    }
    else if (type == "Logarithmic Down")
    {
        low = MINVOID;
        high = MAXVOID;
    }
    else if (type == "Exponential Up")
    {
        low = MINVOID;
        high = MAXVOID;
    }
    else if (type == "Exponential Down")
    {
        low = MINVOID;
        high = MAXVOID;
    }
    else if (type == "Sine")
    {
        low = MINVOID;
        high = MAXVOID;
    }
    else if (type == "Decaying Sine")
    {
        //low = MINVOID;
        //high = MAXVOID;
    }
    else if (type == "Abs Sine")
    {
        low = MINVOID;
        high = MAXVOID;
    }
}

void ValueCurve::GetRangeParm3(const std::string& type, float& low, float &high)
{
    low = 0;
    high = 100;

    if (type == "Custom")
    {
    }
    else if (type == "Flat")
    {
    }
    else if (type == "Ramp")
    {
    }
    else if (type == "Ramp Up/Down")
    {
        low = MINVOID;
        high = MAXVOID;
    }
    else if (type == "Random")
    {
        low = 1;
        high = VC_X_POINTS;
    }
    else if (type == "Ramp Up/Down Hold")
    {
    }
    else if (type == "Saw Tooth")
    {
    }
    else if (type == "Square")
    {
    }
    else if (type == "Parabolic Down")
    {
    }
    else if (type == "Parabolic Up")
    {
    }
    else if (type == "Logarithmic Up")
    {
    }
    else if (type == "Logarithmic Down")
    {
    }
    else if (type == "Exponential Up")
    {
    }
    else if (type == "Exponential Down")
    {
    }
    else if (type == "Sine")
    {
    }
    else if (type == "Decaying Sine")
    {
    }
    else if (type == "Abs Sine")
    {
    }
}

void ValueCurve::GetRangeParm4(const std::string& type, float& low, float &high)
{
    low = 0;
    high = 100;

    if (type == "Custom")
    {
    }
    else if (type == "Flat")
    {
    }
    else if (type == "Ramp")
    {
    }
    else if (type == "Random")
    {
    }
    else if (type == "Ramp Up/Down")
    {
    }
    else if (type == "Ramp Up/Down Hold")
    {
    }
    else if (type == "Saw Tooth")
    {
    }
    else if (type == "Square")
    {
    }
    else if (type == "Parabolic Down")
    {
    }
    else if (type == "Parabolic Up")
    {
    }
    else if (type == "Logarithmic Up")
    {
    }
    else if (type == "Logarithmic Down")
    {
    }
    else if (type == "Exponential Up")
    {
    }
    else if (type == "Exponential Down")
    {
    }
    else if (type == "Sine")
    {
        low = MINVOID;
        high = MAXVOID;
    }
    else if (type == "Decaying Sine")
    {
    }
    else if (type == "Abs Sine")
    {
        low = MINVOID;
        high = MAXVOID;
    }
}

void ValueCurve::Reverse()
{
    _timeOffset = 100 - _timeOffset;
    if (_type == "Custom")
    {
        for (auto it = _values.begin(); it != _values.end(); ++it)
        {
            it->x = 1.0 - it->x;
        }
    }
    else if (_type == "Ramp")
    {
        float p1 = GetParameter1();
        SetParameter1(GetParameter2());
        SetParameter2(p1);
    }
    else if (_type == "Ramp Up/Down")
    {
        float p1 = GetParameter1();
        SetParameter1(GetParameter3());
        SetParameter3(p1);
    }
    //else if (_type == "Logarithmic Up")
    //{
    //    float p1 = GetParameter1();
    //    float p2 = GetParameter2();
    //    SetType("Logarithmic Down");
    //    SetParameter1(100.0 - p1);
    //    SetParameter2(p2);
    //}
    //else if (_type == "Logarithmic Down")
    //{
    //    float p1 = GetParameter1();
    //    float p2 = GetParameter2();
    //    SetType("Logarithmic Up");
    //    SetParameter1(100.0 - p1);
    //    SetParameter2(p2);
    //}
    //else if (_type == "Exponential Up")
    //{
    //    float p1 = GetParameter1();
    //    float p2 = GetParameter2();
    //    SetType("Exponential Down");
    //    SetParameter1(100.0 - p1);
    //    SetParameter2(p2);
    //}
    //else if (_type == "Exponential Down")
    //{
    //    float p1 = GetParameter1();
    //    float p2 = GetParameter2();
    //    SetType("Exponential Up");
    //    SetParameter1(100.0 - p1);
    //    SetParameter2(p2);
    //}
    else if (_type == "Sine")   
    {
        _timeOffset = 100 - _timeOffset;
        _timeOffset += 0.5 * 100 * 10 / _parameter3;
        _timeOffset %= 100;
    }
    else if (_type == "Abs Sine")
    {
        _timeOffset = 100 - _timeOffset;
        _timeOffset += 0.5 * 100 * 10 / _parameter3;
        _timeOffset %= 100;
    }
    else if (_type == "Square")
    {
        float p1 = GetParameter1();
        SetParameter1(GetParameter2());
        SetParameter2(p1);
    }
}

float ValueCurve::Normalise(int parm, float value)
{
    float low;
    float high;
    ValueCurve::GetRangeParm(parm, _type, low, high);

    if (low == MINVOID)
    {
        wxASSERT(_min != MINVOIDF);
        low = _min;
    }
    if (high == MAXVOID)
    {
        wxASSERT(_max != MAXVOIDF);
        high = _max;
    }

    float res = value;
    if (low != 0 || high != 100)
    {
        float range = high - low;
        res = (value - low) * 100 / range;
    }

    if (res < 0) res = 0;
    if (res > 100) res = 100;

    return res;
}

// unfixes the changed scale from whatever it is now to 0-100
void ValueCurve::UnFixChangedScale(float newmin, float newmax)
{
    if (newmin == 0 && newmax == 100) return;

    float oldrange = newmax - newmin;
    float newrange = 100 - 0;
    float mindiff = 0 - newmin;

    float min, max;
    GetRangeParm(1, _type, min, max);
    if (min == MINVOID)
    {
        _parameter1 = _parameter1 * newrange / oldrange + mindiff;
    }

    GetRangeParm(2, _type, min, max);
    if (min == MINVOID)
    {
        _parameter2 = _parameter2 * newrange / oldrange + mindiff;
    }

    GetRangeParm(3, _type, min, max);
    if (min == MINVOID)
    {
        _parameter3 = _parameter3 * newrange / oldrange + mindiff;
    }

    GetRangeParm(4, _type, min, max);
    if (min == MINVOID)
    {
        _parameter4 = _parameter4 * newrange / oldrange + mindiff;
    }
}

// fixes curves that were saved with the wrong scale
void ValueCurve::FixScale(int scale)
{
    float min, max;
    GetRangeParm(1, _type, min, max);
    if (min == MINVOID)
    {
        _parameter1 *= scale;
    }

    GetRangeParm(2, _type, min, max);
    if (min == MINVOID)
    {
        _parameter2 *= scale;
    }

    GetRangeParm(3, _type, min, max);
    if (min == MINVOID)
    {
        _parameter3 *= scale;
    }

    GetRangeParm(4, _type, min, max);
    if (min == MINVOID)
    {
        _parameter4 *= scale;
    }
}

// fixes the changed scale from 0-100 to whatever it is now
void ValueCurve::FixChangedScale(float newmin, float newmax, int divisor)
{
    if (newmin == 0 && newmax == 100 && divisor == 1) return;

    float newrange = newmax - newmin;
    float oldrange = 100 - 0;
    float mindiff = newmin - 0;

    float min, max;
    GetRangeParm(1, _type, min, max);
    if (min == MINVOID)
    {
        _parameter1 = (_parameter1 * newrange / oldrange + mindiff); // / divisor;
    }

    GetRangeParm(2, _type, min, max);
    if (min == MINVOID)
    {
        _parameter2 = (_parameter2 * newrange / oldrange + mindiff); // / divisor;
    }

    GetRangeParm(3, _type, min, max);
    if (min == MINVOID)
    {
        _parameter3 = (_parameter3 * newrange / oldrange + mindiff); // / divisor;
    }

    GetRangeParm(4, _type, min, max);
    if (min == MINVOID)
    {
        _parameter4 = (_parameter4 * newrange / oldrange + mindiff); // / divisor;
    }
}

void ValueCurve::ConvertChangedScale(float newmin, float newmax)
{
    if (newmin == _min && newmax == _max) return;

    float newrange = newmax - newmin;
    float oldrange = _max - _min;

    if (newrange > oldrange)
    {
        // this is suspicious ... generally ranges increase with versions not decrease so I am going to ignore this request
        return;
    }

    // now handle custom
    if (_type == "Custom")
    {
        wxASSERT(_min != MINVOIDF);
        wxASSERT(_max != MAXVOIDF);

        for (auto it = _values.begin(); it != _values.end(); ++it)
        {
            it->y = it->y * newrange / oldrange;
        }
    }
}

void ValueCurve::RenderType()
{
    // dont render if we dont know our limits
    if (_min == MINVOIDF || _max == MAXVOIDF || _divisor == MAXVOID) return;

    float parameter1 = Normalise(1, _parameter1);
    float parameter2 = Normalise(2, _parameter2);
    float parameter3 = Normalise(3, _parameter3);
    float parameter4 = Normalise(4, _parameter4);

    if (_type != "Custom")
    {
        _values.clear();
    }
    if (_type == "Flat")
    {
        _values.push_back(vcSortablePoint(0.0f, parameter1 / 100.0, false));
        _values.push_back(vcSortablePoint(1.0f, parameter1 / 100.0, false));
    }
    else if (_type == "Ramp")
    {
        _values.push_back(vcSortablePoint(0.0f, parameter1 / 100.0, false));
        _values.push_back(vcSortablePoint(1.0f, parameter2 / 100.0, false));
    }
    else if (_type == "Ramp Up/Down")
    {
        _values.push_back(vcSortablePoint(0.0f, parameter1 / 100.0, false));
        _values.push_back(vcSortablePoint(0.5f, parameter2 / 100.0, false));
        _values.push_back(vcSortablePoint(1.0f, parameter3 / 100.0, false));
    }
    else if (_type == "Ramp Up/Down Hold")
    {
        _values.push_back(vcSortablePoint(0.0f, parameter1 / 100.0, false));
        _values.push_back(vcSortablePoint(0.5f - ((0.5f * parameter3) / 100.0), parameter2 / 100.0, false));
        _values.push_back(vcSortablePoint(0.5f + ((0.5f * parameter3) / 100.0), parameter2 / 100.0, false));
        _values.push_back(vcSortablePoint(1.0f, parameter1 / 100.0, false));
    }
    else if (_type == "Saw Tooth")
    {
        int count = _parameter3;
        if (count < 1)
        {
            count = 1;
        }
        float per = 1.0f / count;
        _values.push_back(vcSortablePoint(0.0f, parameter1 / 100.0, false));
        for (int i = 0; i < count; i++)
        {
            _values.push_back(vcSortablePoint(i * per + per / 2.0f, parameter2 / 100.0, false));
            _values.push_back(vcSortablePoint((i + 1) * per, parameter1 / 100.0, false));
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
            float f1 = (float)i * per - 0.0001f;
            float f2 = (float)i * per;
            if (vcSortablePoint::Normalise(f1) != vcSortablePoint::Normalise(f2))
            {
                f1 = (float)i * per;
                f2 = (float)i * per + 0.0001f;
            }
            if (low)
            {
                if (i != 0)
                {
                    _values.push_back(vcSortablePoint(f1, (float)parameter2 / 100.0, false));
                }
                _values.push_back(vcSortablePoint(f2, (float)parameter1 / 100.0, false));
            }
            else
            {
                _values.push_back(vcSortablePoint(f1, (float)parameter1 / 100.0, false));
                _values.push_back(vcSortablePoint(f2, (float)parameter2 / 100.0, false));
            }
            low = !low;
        }
        _values.push_back(vcSortablePoint(1.0f, parameter2 / 100.0, false));
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
            float y = a * (i - 0.5f) * (i - 0.5f) + parameter2 / 100.0;
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
            float y = a * (i - 0.5f) * (i - 0.5f) + parameter2 / 100.0;
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
        float a = parameter1 / 25.0f;
        if (_parameter1 == 0)
        {
            a = 0.04f;
        }
        // y = log(ax - 1);
        for (double i = 0.0; i <= 1.01; i += 0.05)
        {
            if (i > 1.0) i = 1.0;
            float y = (parameter2 - 50.0) / 50.0 + log(a + a*i) + 1.0f;
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
        float a = parameter1 / 10.0f;
        if (_parameter1 == 0)
        {
            a = 0.1f;
        }
        // y = 1.0+ -2^(ax-1.0);
        for (double i = 0.0; i <= 1.01; i += 0.05)
        {
            if (i > 1.0) i = 1.0;
            float y = (parameter2 - 50.0) / 50.0 + 1.5f + -1 * exp2(a * i - 1.0f);
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
        float a = parameter1 / 10.0f;
        if (_parameter1 == 0)
        {
            a = 0.1f;
        }
        // y = 1.0+ -2^(ax-1.0);
        for (double i = 0.0; i <= 1.01; i += 0.05)
        {
            if (i > 1.0) i = 1.0;
            float y = (parameter2 - 50.0) / 50.0 + (exp(a*i) - 1.0) / (exp(a) - 1.0);
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
        float a = parameter1 / 10.0f;
        if (_parameter1 == 0)
        {
            a = 0.1f;
        }
        // y = 1.0+ -2^(ax-1.0);
        for (double i = 0.0; i <= 1.01; i += 0.05)
        {
            if (i > 1.0) i = 1.0;
            float y = (parameter2 - 50.0) / 50.0 + 1.0 - (exp(a*i) - 1.0) / (exp(a) - 1.0);
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
        // p1 - offset in cycle - deprecated
        // p2 - maxy
        // p3 - cycles
        // one cycle = 2* PI
        static const double pi2 = 6.283185307;
        float maxx = pi2 * std::max(parameter3 / 10.0f, 0.1f);
        for (double i = 0.0; i <= 1.01; i += 0.025)
        {
            if (i > 1.0) i = 1.0;
            float r = i * maxx; // +((parameter1 * pi2) / 100.0f);
            float y = (parameter4 - 50.0) / 50.0 + (sin(r) * (std::max(parameter2, 1.0f) / 200.0f)) + 0.5f;
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
    else if (_type == "Decaying Sine")
    {
        // p1 - offset in cycle - deprecated
        // p2 - maxy
        // p3 - cycles
        // one cycle = 2* PI
        static const double pi2 = 6.283185307;
        float maxx = pi2 * std::max((float)_parameter3 / 10.0f, 0.1f);
        for (double i = 0.0; i <= 1.01; i += 0.025)
        {
            if (i > 1.0) i = 1.0;

            float r = i * maxx; // +(((float)_parameter1 * pi2) / 100.0f);
            float exponent = exp(-0.1 * i * maxx);
            float y = ((float)_parameter4 - 50.0) / 50.0 + (exponent * cos(r) * (std::max((float)_parameter2, 1.0f) / 200.0f)) + 0.5f;
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
    else if (_type == "Random")
    {
        // p1 - minimum
        // p2 - maximum
        // p3 - points
        float min = parameter1 / 100.0;
        float max = parameter2 / 100.0;
        int points = std::round(Denormalise(3, parameter3));

        if (points == 1)
        {
            float value = rand01() * (max - min) + min;
            _values.push_back(vcSortablePoint(0.0f, value, false));
            _values.push_back(vcSortablePoint(1.0f, value, false));
        }
        else
        {
            float value = rand01() * (max - min) + min;
            _values.push_back(vcSortablePoint(0.0f, value, false));
            value = rand01() * (max - min) + min;
            _values.push_back(vcSortablePoint(1.0f, value, false));

            for (int i = 2; i < points; i++)
            {
                float x = vcSortablePoint::Normalise(rand01());
                while (IsSetPoint(x))
                {
                    x = vcSortablePoint::Normalise(rand01());
                }
                value = rand01() * (max - min) + min;
                _values.push_back(vcSortablePoint(x, value, false));
            }
        }
    }
    else if (_type == "Abs Sine")
    {
        // p1 - offset in cycle - deprecated
        // p2 - maxy
        // p3 - cycles
        // one cycle = 2* PI
        static const double pi2 = 6.283185307;
        float maxx = pi2 * std::max(parameter3 / 10.0f, 0.1f);
        for (double i = 0.0; i <= 1.01; i += 0.025)
        {
            if (i > 1.0) i = 1.0;
            float r = i * maxx; // +((parameter1 * pi2) / 100.0f);
            float y = (parameter4 - 50.0) / 50.0 + (std::abs(sin(r) * (std::max(parameter2, 1.0f) / 100.0f)));
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
    _realValues = true;
    _divisor = divisor;
    _timeOffset = 0;
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
    if (min != MINVOIDF)
    {
        _min = min;
    }
    if (max != MAXVOIDF)
    {
        _max = max;
    }
    _parameter1 = 0;
    _parameter2 = 0;
    _parameter3 = 0;
    _parameter4 = 0;
    _timeOffset = 0;
    _active = false;
    _wrap = false;
    _realValues = true;
    if (divisor != MAXVOID)
    {
        _divisor = divisor;
    }

    RenderType();
}

ValueCurve::ValueCurve(const std::string& s)
{
    _min = MINVOIDF;
    _max = MAXVOIDF;
    SetDefault();
    _divisor = 1;
    Deserialise(s);
}

void ValueCurve::Deserialise(const std::string& s, bool holdminmax)
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
        _realValues = false;
        _active = true;
        _values.clear();
        _type = "Flat";
        _parameter1 = 0.0f;
        _parameter2 = 0.0f;
        _parameter3 = 0.0f;
        _parameter4 = 0.0f;
        _timeOffset = 0;
        _wrap = false;

        float oldmin = _min;
        float oldmax = _max;

        wxArrayString v = wxSplit(wxString(s.c_str()), '|');
        for (auto vs = v.begin(); vs != v.end(); ++vs)
        {
            wxArrayString v1 = wxSplit(*vs, '=');
            if (v1.size() == 2)
            {
                SetSerialisedValue(v1[0].ToStdString(), v1[1].ToStdString());
            }
        }

        // if we are asked to hold min max then we are just going to use the ones we deserialised and assume parms are all ok
        // this is generally only done when loading an xvc file ... particularly an old one
        if (!holdminmax)
        {
            // This converts curves from the 0-100 to the real scale
            if (_active && !_realValues && oldmin != MINVOIDF)
            {
                if (_min != 0 || _max != 100)
                {
                    // use the scale in the file if it wasnt 0-100
                    FixChangedScale(_min, _max, _divisor);
                }
                else
                {
                    if (oldmin != 0 || oldmax != 100 || _divisor != 1)
                    {
                        // otherwise use the scale of this VC ... this is not great ... if the VC range has been expanded then
                        // it isnt going to convert correctly

                        // this is actually checking for something .25 or newer
                        // this should be updated every release by 1 until we decide to change a slider range for the first time
                        // at that point we are going to need to force people to go back to a version after .24 but before the
                        // first version with the change
                        if (!::IsVersionOlder("2018.1", xlights_version_string.ToStdString()))
                        {
                            static std::string warnedfile = "";

                            if (xLightsFrame::CurrentSeqXmlFile != nullptr && warnedfile != xLightsFrame::CurrentSeqXmlFile->GetFullName().ToStdString())
                            {
                                warnedfile = xLightsFrame::CurrentSeqXmlFile->GetFullName().ToStdString();
                                wxMessageBox("Sequence contains value curves that cannot be converted automatically. Please open and save this sequence in v2017.45 before proceeding.");
                            }
                        }
                        FixChangedScale(oldmin, oldmax, _divisor);
                    }
                }
                _realValues = true;
            }

            // This converts curves to the new scale when a parameters range has been expanded ... but only if it was already real values
            if (_active && _realValues && ((oldmin != MINVOIDF && oldmin != _min) || (oldmax != MAXVOIDF && oldmax != _max)))
            {
                ConvertChangedScale(oldmin, oldmax);
            }

            if (oldmin != MINVOIDF) _min = oldmin;
            if (oldmax != MAXVOIDF) _max = oldmax;
        }

        RenderType();
    }
}

std::string ValueCurve::Serialise()
{
    std::string res = "";

    if (IsActive())
    {
        wxASSERT(_min != MINVOIDF);
        wxASSERT(_max != MAXVOIDF);

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
            res += "P1=" + std::string(wxString::Format("%.2f", _parameter1).c_str()) + "|";
        }
        if (_parameter2 != 0)
        {
            res += "P2=" + std::string(wxString::Format("%.2f", _parameter2).c_str()) + "|";
        }
        if (_parameter3 != 0)
        {
            res += "P3=" + std::string(wxString::Format("%.2f", _parameter3).c_str()) + "|";
        }
        if (_parameter4 != 0)
        {
            res += "P4=" + std::string(wxString::Format("%.2f", _parameter4).c_str()) + "|";
        }
        if (_timeOffset != 0)
        {
            res += "TO=" + std::string(wxString::Format("%d", _timeOffset).c_str()) + "|";
        }
        if (_wrap)
        {
            res += "WRAP=TRUE|";
        }
        if (_realValues)
        {
            res += "RV=TRUE|";
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
        _parameter1 = wxAtof(wxString(s.c_str()));
    }
    else if (kk == "TO")
    {
        _timeOffset = wxAtoi(wxString(s.c_str()));
    }
    else if (kk == "WRAP")
    {
        _wrap = true;
    }
    else if (kk == "RV")
    {
        _realValues = true;
    }
    else if (kk == "P2")
    {
        _parameter2 = wxAtof(wxString(s.c_str()));
    }
    else if (kk == "P3")
    {
        _parameter3 = wxAtof(wxString(s.c_str()));
    }
    else if (kk == "P4")
    {
        _parameter4 = wxAtof(wxString(s.c_str()));
    }
    else if (kk == "Values")
    {
        wxArrayString points = wxSplit(s, ';');

        for (auto p = points.begin(); p != points.end(); ++p)
        {
            wxArrayString xy = wxSplit(*p, ':');
            _values.push_back(vcSortablePoint(wxAtof(wxString(xy.front().c_str())), wxAtof(wxString(xy.back().c_str())), false));
        }
    }

    _values.sort();
    //_active = true;

    // Start (param1) for Sine, Absolute Sine and Decaying Sine has been deprecated and replaced by TimeOffset
    if (_parameter1 != 0 && (_type == "Sine" || _type == "Abs Sine"| _type == "Decaying Sine"))
    {
        _timeOffset = 100 - _parameter1;
        _parameter1 = 0;
    }
}

void ValueCurve::SetType(std::string type)
{
    _type = type;
    RenderType();
}

float ValueCurve::GetOutputValue(float offset)
{
    wxASSERT(_min != MINVOIDF);
    wxASSERT(_max != MAXVOIDF);
    return (_min + (_max - _min) * offset) / _divisor;
}

float ValueCurve::GetOutputValueAt(float offset)
{
    wxASSERT(_min != MINVOIDF);
    wxASSERT(_max != MAXVOIDF);
    return _min + (_max - _min) * GetValueAt(offset);
}

float ValueCurve::GetOutputValueAtDivided(float offset)
{
    wxASSERT(_min != MINVOIDF);
    wxASSERT(_max != MAXVOIDF);
    return (_min + (_max - _min) * GetValueAt(offset)) / _divisor;
}

float ValueCurve::GetValueAt(float offset)
{
    float res = 0.0f;

    if (_values.size() < 2) return 1.0f;
    if (!_active) return 1.0f;

    if (offset < 0.0f) offset = 0.0;
    if (offset > 1.0f) offset = 1.0;

    offset += (float)_timeOffset / 100;
    if (offset > 1.0) offset -= 1.0;

    vcSortablePoint last = _values.front();
    auto it = _values.begin();
    ++it;

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
        ++it;
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
            ++it;
        }
    }
}

void ValueCurve::RemoveExcessCustomPoints()
{
    // go through list and remove middle points where 3 in a row have the same value
    auto it1 = _values.begin();
    auto it2 = it1;
    ++it2;
    auto it3 = it2;
    ++it3;

    while (it1 != _values.end() && it2 != _values.end() && it3 != _values.end())
    {
        if (it1->y == it2->y && it2->y == it3->y)
        {
            _values.remove(*it2);
            it2 = it1;
            ++it2;
            it3 = it2;
            ++it3;
        }
        else
        {
            ++it1;
            ++it2;
            ++it3;
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
        ++it;
    }
    _values.push_back(vcSortablePoint(offset, value, false));
    _values.sort();
}

void ValueCurve::SetWrap(bool wrap)
{
    _wrap = wrap;

    if (!_wrap)
    {
        for (auto it = _values.begin(); it != _values.end(); ++it)
        {
            it->ClearWrap();
        }
    }

    RenderType();
}

float ValueCurve::GetParameter1_100() const
{
    float range = _max - _min;
    return (GetParameter1() - _min) * 100 / range;
}

float ValueCurve::FindMinPointLessThan(float point)
{
    float res = 0.0;

    for (auto it = _values.begin(); it != _values.end(); ++it)
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

    for (auto it = _values.begin(); it != _values.end(); ++it)
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
    for (auto it = _values.begin(); it != _values.end(); ++it)
    {
        if (it->IsNear(x, y))
        {
            return true;
        }
    }

    return false;
}

wxBitmap ValueCurve::GetImage(int w, int h, double scaleFactor)
{
    if (scaleFactor < 1.0) {
        scaleFactor = 1.0;
    }
    float width = w * scaleFactor;
    float height = h * scaleFactor;

    wxBitmap bmp(width, height);

    wxMemoryDC dc(bmp);
    dc.SetBrush(*wxLIGHT_GREY_BRUSH);
    dc.DrawRectangle(0, 0, width, height);
    dc.SetPen(*wxBLACK_PEN);
    float lastY = height - 1 - (GetValueAt(0)) * height;

    for (int x = 1; x < width; x++) {
        float x1 = x;
        x1 /= (float)width;

        float y = (GetValueAt(x1)) * (float)width;
        y = (float)height - 1.0f - y;
        dc.DrawLine(x - 1, lastY, x, std::round(y));
        lastY = y;
    }
    if (scaleFactor > 1.0f) {
        wxImage img = bmp.ConvertToImage();
        return wxBitmap(img, 8, scaleFactor);
    }
    return bmp;
}
