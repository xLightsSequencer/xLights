#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <list>
#include <string>

#include "Color.h"

#define CC_X_POINTS 100.0

class ccSortableColorPoint
{
public:

    static float Normalise(float v)
    {
        if (v < 0) v = 0;
        if (v > 1) v = 1;

        return std::round(v * CC_X_POINTS) / CC_X_POINTS;
    }

	float x; // 0-1 ... the start point of this point
	xlColor color; // the colour of the mid point of this
    bool donext;

    bool DoNext() const
    {
        return donext;
    }

    std::string Serialise() const
    {
        std::string res = "";
        char buffer[32];
        std::snprintf(buffer, sizeof(buffer), "%.3f", x);
        res += "x=";
        res += buffer;
        std::string c = color;
        std::replace(c.begin(), c.end(), ',', '@');
        res += "^c=" + c;

        return res;
    }

    void Deserialise(const std::string &s)
    {
        if (s.empty())
        {
            throw;
        }
        else if (s.find('^') == std::string::npos)
        {
            throw;
        }
        else
        {
            size_t start = 0;
            while (start <= s.size()) {
                size_t end = s.find('^', start);
                std::string token = s.substr(start, end == std::string::npos ? std::string::npos : end - start);
                size_t eq = token.find('=');
                if (eq != std::string::npos && eq > 0 && eq + 1 < token.size()) {
                    SetSerialisedValue(token.substr(0, eq), token.substr(eq + 1));
                }
                if (end == std::string::npos) break;
                start = end + 1;
            }
        }
    }

    void SetSerialisedValue(const std::string &k, const std::string &v)
    {
        if (k == "x")
        {
            x = ccSortableColorPoint::Normalise(static_cast<float>(std::strtod(v.c_str(), nullptr)));
        }
        else if (k == "c")
        {
            std::string c(v);
            std::replace(c.begin(), c.end(), '@', ',');
            color = xlColor(c);
        }
    }

    ccSortableColorPoint(const std::string& s)
    {
        Deserialise(s);
    }

    ccSortableColorPoint(float xx, xlColor c, bool dn = false)
    {
        x = Normalise(xx);
		color = c;
        donext = dn;
    }

    bool IsNear(float xx) const
    {
        return (x == Normalise(xx));
    }

    bool operator==(const ccSortableColorPoint& r) const
    {
        return x == r.x;
    }

    bool operator==(const float r) const
    {
        return x == Normalise(r);
    }

    bool operator<(const ccSortableColorPoint& r) const
    {
        return x < r.x;
    }

    bool operator<(const float r) const
    {
        return x < r;
    }

    bool operator<=(const ccSortableColorPoint& r) const
    {
        return x <= r.x;
    }

    bool operator<=(const float r) const
    {
        return x <= r;
    }

    bool operator>(const ccSortableColorPoint& r) const
    {
        return x > r.x;
    }
};

#define TC_TIME 0
#define TC_RIGHT 1
#define TC_DOWN 2
#define TC_LEFT 3
#define TC_UP 4
#define TC_RADIALIN 5
#define TC_RADIALOUT 6
#define TC_CW 7
#define TC_CCW 8

class ColorCurve
{
    std::list<ccSortableColorPoint> _values;
    std::string _type;
    std::string _id;
    bool _active;
    int _timecurve;

    void SetSerialisedValue(std::string k, std::string v);
    const ccSortableColorPoint* GetActivePoint(float x, float& duration) const;
    const ccSortableColorPoint* GetPriorActivePoint(float x, float& duration) const;
    const ccSortableColorPoint* GetNextActivePoint(float x, float& duration) const;

public:
    static std::string GetColorCurveFolder(const std::string& showFolder);
    static bool IsColorCurve(const std::string& s);
    bool IsOk() const { return !_id.empty(); }
    void NextTimeCurve(bool supportslinear, bool supportsradial);
    void SetValidTimeCurve(bool supportslinear, bool supportsradial);
    int GetTimeCurve() const { return _timecurve; }
    std::string GetId() const { return _id; }
    void SetId(const std::string& id) { _id = id; }
    ColorCurve();
    ColorCurve(const std::string& serialised);
    ColorCurve(const std::string& id, const std::string type, xlColor c = xlBLACK);
    std::string Serialise();
    void Deserialise(const std::string& s);
    void SetType(const std::string &type);
    xlColor GetValueAt(float offset) const;
    ccSortableColorPoint* GetPointAt(float offset);
    void SetActive(bool a) { _active = a; }
    bool IsActive() const { return _active && IsOk(); }
    void ToggleActive() { _active = !_active; }
    void SetValueAt(float offset, xlColor x);
    void DeletePoint(float offset);
    void Flip();
    bool IsSetPoint(float offset);
    int GetPointCount() const
    { return _values.size(); }
    std::string GetType() const
    { return _type; }
    std::list<ccSortableColorPoint> GetPoints() const
    { return _values; }
    bool NearPoint(float x);
    float FindMinPointLessThan(float point);
    float FindMaxPointGreaterThan(float point);
    void SetDefault(const xlColor& color);
    void LoadXCC(const std::string& filename);
};
