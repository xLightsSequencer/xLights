/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <pugixml.hpp>

#include <cassert>
#include <cmath>
#include <filesystem>
#include <string_view>

#include "ColorCurve.h"
#include "UtilFunctions.h"

#include <log.h>

ColorCurve::ColorCurve(const std::string& id, const std::string type, xlColor c)
{
    _type = type;
    _id = id;
    _timecurve = TC_TIME;
    _values.push_back(ccSortableColorPoint(0.5, c));
    _active = false;
}

ColorCurve::ColorCurve()
{
    _type = "Gradient";
    _values.clear();
    _active = false;
    _timecurve = TC_TIME;
    _values.push_back(ccSortableColorPoint(0.5, xlBLACK));
    _id = "";
}

ColorCurve::ColorCurve(const std::string& s)
{
    _id = "";
    _type = "Gradient";
    _values.clear();
    _active = false;
    _timecurve = TC_TIME;

    if (!IsColorCurve(s))
    {
        // dont try to deserialise
    }
    else
    {
        Deserialise(s);
    }

    if (_values.size() == 0)
    {
        _values.push_back(ccSortableColorPoint(0.5, xlBLACK));
    }
}

bool ColorCurve::IsColorCurve(const std::string& s)
{
    return Contains(s, "Active=");
}

void ColorCurve::Deserialise(const std::string& s)
{
    if (s == "")
    {
        _type = "Gradient";
        _active = false;
        _timecurve = TC_TIME;
        _values.clear();
    }
    else if (s.find('|') == std::string::npos)
    {
        _active = false;
        _timecurve = TC_TIME;
        _id = s;
        _values.clear();
    }
    else
    {
        _active = true;
        _timecurve = TC_TIME;
        _values.clear();
        _type = "Gradient";
        size_t start = 0;
        while (start <= s.size()) {
            size_t end = s.find('|', start);
            std::string token = s.substr(start, end == std::string::npos ? std::string::npos : end - start);
            size_t eq = token.find('=');
            if (eq != std::string::npos && eq > 0 && eq + 1 < token.size()) {
                SetSerialisedValue(token.substr(0, eq), token.substr(eq + 1));
            }
            if (end == std::string::npos) break;
            start = end + 1;
        }
    }

    if (_values.size() == 0)
    {
        _values.push_back(ccSortableColorPoint(0.5, xlBLACK));
    }
}

std::string ColorCurve::Serialise()
{
    std::string res = "";

    if (IsActive())
    {
        res += "Active=TRUE|";
        res += "Id=" + _id + "|";
        if (_type != "Gradient")
        {
            res += "Type=" + _type + "|";
        }
        if (_timecurve != TC_TIME)
        {
            res += "Timecurve=" + std::to_string(_timecurve) + "|";
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
    if (k == "Id")
    {
        _id = s;
    }
    else if (k == "Active")
    {
        if (s == "FALSE")
        {
            _active = false;
        }
        else
        {
            // it should already be true
            assert(_active == true);
        }
    }
    else if (k == "Type")
    {
        _type = s;
    }
    else if (k == "Timecurve")
    {
        _timecurve = static_cast<int>(std::strtol(s.c_str(), nullptr, 10));
    }
    else if (k == "Values")
    {
        size_t start = 0;
        while (start <= s.size()) {
            size_t end = s.find(';', start);
            std::string point = s.substr(start, end == std::string::npos ? std::string::npos : end - start);
            if (!point.empty()) {
                _values.push_back(ccSortableColorPoint(point));
            }
            if (end == std::string::npos) break;
            start = end + 1;
        }
    }

    _values.sort();
}

void ColorCurve::SetType(const std::string &type)
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

ccSortableColorPoint* ColorCurve::GetPointAt(float offset)
{
    float x = ccSortableColorPoint::Normalise(offset);
    for (auto it = _values.begin(); it != _values.end(); ++it)
    {
        if (*it == x)
        {
            return &(*it);
        }
    }

    return nullptr;
}

xlColor ColorCurve::GetValueAt(float offset) const
{
    if (_type == "Gradient")
    {
        float start;
        float end;
        xlColor startc;
        xlColor endc;

        // find the value before the offset
        float d = 0;
        const ccSortableColorPoint* pt = GetActivePoint(offset, d);

        if (pt == nullptr)
        {
            const ccSortableColorPoint* ptp = GetNextActivePoint(offset, d);
            return ptp->color;
        }
        else if (pt->x == offset)
        {
            return pt->color;
        }
        else if (offset - pt->x < 0.5 * d)
        {
            end = pt->x + 0.5 *d;
            endc = pt->color;

            float dp = 0;
            const ccSortableColorPoint* ptp = GetPriorActivePoint(offset, dp);

            if (ptp == nullptr)
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
            const ccSortableColorPoint* ptn = GetNextActivePoint(offset, dn);

            if (ptn == nullptr)
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
    else if (_type == "None")
    {
        // find the value immediately before the offset ... that is the color to return
        float d = 0;
        const ccSortableColorPoint* pt = GetActivePoint(offset, d);
        if (pt == nullptr)
        {
            const ccSortableColorPoint* ptp = GetNextActivePoint(offset, d);
            return ptp->color;
        }
        return pt->color;
    }
    else if (_type == "Random")
    {
        xlColor c1;
        float d = 0;
        const ccSortableColorPoint* pt = GetActivePoint(offset, d);
        if (pt == nullptr)
        {
            const ccSortableColorPoint* ptp = GetNextActivePoint(offset, d);
            c1 = ptp->color;

            if (offset == ptp->x) return c1;
        }
        else
        {
            c1 = pt->color;
            if (offset == pt->x) return c1;
        }

        xlColor c2;
        pt = GetNextActivePoint(offset, d);
        if (pt == nullptr)
        {
            const ccSortableColorPoint* ptp = GetActivePoint(offset, d);
            c2 = ptp->color;
            if (offset == ptp->x) return c2;
        }
        else
        {
            c2 = pt->color;
            if (offset == pt->x) return c2;
        }

        // handle black & white differently
        if (c1.Red() == c1.Green() && c1.Green() == c1.Blue() && c2.Red() == c2.Green() && c2.Green() == c2.Blue())
        {
            double r = rand01();
            return xlColor(r * std::abs((float)c1.Red() - (float)c2.Red()) + std::min(c1.Red(), c2.Red()),
                           r * std::abs((float)c1.Green() - (float)c2.Green()) + std::min(c1.Green(), c2.Green()),
                           r * std::abs((float)c1.Blue() - (float)c2.Blue()) + std::min(c1.Blue(), c2.Blue()));
        }

        return xlColor(rand01() * std::abs((float)c1.Red() - (float)c2.Red()) + std::min(c1.Red(), c2.Red()),
                       rand01() * std::abs((float)c1.Green() - (float)c2.Green()) + std::min(c1.Green(), c2.Green()),
                       rand01() * std::abs((float)c1.Blue() - (float)c2.Blue()) + std::min(c1.Blue(), c2.Blue()));
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
        ++it;
    }
    
    return false;
}

void ColorCurve::DeletePoint(float offset)
{
    if (GetPointCount() > 1)
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
    else
    {
        // No-op in core logic; UI decides whether to alert.
    }
}

void ColorCurve::Flip()
{
    auto oldvalues = _values;
    _values.clear();
    for (auto it = oldvalues.begin(); it != oldvalues.end(); ++it)
    {
        ccSortableColorPoint scp(1.0f - it->x, it->color);
        _values.push_front(scp);
    }
}

void ColorCurve::SetDefault(const xlColor& color)
{
    // we should only set default if the current CC only has one point
    if (_values.size() == 1)
    {
        _values.front().color = color;
    }
}

void ColorCurve::LoadXCC(const std::string& filename)
{
    // reset everything
    auto oldid = _id;
    _id = "";
    _type = "Gradient";
    _values.clear();
    _active = false;
    _timecurve = TC_TIME;

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(filename.c_str());

    if (result)
    {
        pugi::xml_node root = doc.document_element();

        if (root && std::string_view(root.name()) == "colorcurve")
        {
            std::string data = root.attribute("data").as_string();

            Deserialise(data);
            SetActive(true);
        }
    }
    _id = oldid;
}

void ColorCurve::SetValueAt(float offset, xlColor c)
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

    _values.push_back(ccSortableColorPoint(offset, c));
    _values.sort();
}

std::string ColorCurve::GetColorCurveFolder(const std::string& showFolder)
{
    if (showFolder.empty()) return "";

    std::filesystem::path ccf = std::filesystem::path(showFolder) / "colorcurves";
    std::error_code ec;
    if (!std::filesystem::exists(ccf, ec)) {
        std::filesystem::create_directories(ccf, ec);
    }
    if (ec || !std::filesystem::exists(ccf, ec)) {
        return "";
    }
    return ccf.string();
}

bool ColorCurve::NearPoint(float x)
{
    for (auto it = _values.begin(); it != _values.end(); ++it)
    {
        if (it->IsNear(x))
        {
            return true;
        }
    }

    return false;
}

const ccSortableColorPoint* ColorCurve::GetActivePoint(float x, float& duration) const
{
    const ccSortableColorPoint* candidate = nullptr;
    for (auto it = _values.begin(); it != _values.end(); ++it)
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

const ccSortableColorPoint* ColorCurve::GetPriorActivePoint(float x, float& duration) const
{
    const ccSortableColorPoint* candidate = nullptr;
    const ccSortableColorPoint* last = nullptr;
    for (auto it = _values.begin(); it != _values.end(); ++it)
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

const ccSortableColorPoint* ColorCurve::GetNextActivePoint(float x, float& duration) const
{
    for (auto it = _values.begin(); it != _values.end(); ++it)
    {
        if (!(*it <= x))
        {
            return &(*it);
        }
    }

    return nullptr;
}

float ColorCurve::FindMinPointLessThan(float point)
{
    float res = 0.0;

    for (auto it = _values.begin(); it != _values.end(); ++it)
    {
        if (it->x < point)
        {
            res = it->x + 0.025;
        }
    }

    return ccSortableColorPoint::Normalise(res);
}
float ColorCurve::FindMaxPointGreaterThan(float point)
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

    return ccSortableColorPoint::Normalise(res);
}
void ColorCurve::NextTimeCurve(bool supportslinear, bool supportsradial)
{
    _timecurve++; 

    SetValidTimeCurve(supportslinear, supportsradial);
}

void ColorCurve::SetValidTimeCurve(bool supportslinear, bool supportsradial)
{
    if (_timecurve > TC_CCW) _timecurve = TC_TIME;

    if ((_timecurve == TC_LEFT || _timecurve == TC_RIGHT || _timecurve == TC_UP || _timecurve == TC_DOWN) && !supportslinear)
    {
        _timecurve = TC_RADIALIN;
    }

    if ((_timecurve == TC_CW || _timecurve == TC_CCW || _timecurve == TC_RADIALIN || +_timecurve == TC_RADIALOUT) && !supportsradial)
    {
        _timecurve = TC_TIME;
    }
}
