#ifndef VALUECURVE_H
#define VALUECURVE_H

#include <wx/position.h>
#include <string>
#include <list>

class vcSortablePoint
{
public:
    float x;
    float y;
    vcSortablePoint(float xx, float yy)
    {
        x = xx;
        y = yy;
    }
    bool operator==(const vcSortablePoint& r)
    {
        return x == r.x;
    }
    bool operator==(const float r)
    {
        return x == r;
    }
    bool operator<(const vcSortablePoint& r)
    {
        return x < r.x;
    }
    bool operator<(const float r)
    {
        return x < r;
    }
    bool operator<=(const vcSortablePoint& r)
    {
        return x <= r.x;
    }
    bool operator<=(const float r)
    {
        return x <= r;
    }
    bool operator>(const vcSortablePoint& r)
    {
        return x > r.x;
    }
};

class ValueCurve
{
    std::list<vcSortablePoint> _values;
    std::string _type;
    std::string _id;
    float _max;
    float _min;
    float _parameter1;
    float _parameter2;
    float _parameter3;
    bool _active;

    void RenderType();

public:
    ValueCurve() { ValueCurve(""); };
    ValueCurve(const std::string& id, float min = 0.0f, float max = 100.0f, float parameter1 = 0.0f, float parameter2 = 0.0f, float parameter3 = 0.0f);
    std::string Serialise();
    void SetSerialisedValue(std::string k, std::string s);
    void SetType(std::string type);
    float GetMax() { return _max; }
    float GetMin() { return _min; }
    float GetValueAt(float offset);
    void SetActive(bool a) { _active = a; }
    bool IsActive() { return _active; }
    void ToggleActive() { _active = !_active; }
    void SetValueAt(float offset, float value);
    void SetParameter1(float parameter1) { _parameter1 = parameter1; RenderType(); }
    void SetParameter2(float parameter2) { _parameter2 = parameter2; RenderType(); }
    void SetParameter3(float parameter3) { _parameter3 = parameter3; RenderType(); }
    float GetParameter1() { return _parameter1; }
    float GetParameter2() { return _parameter2; }
    float GetParameter3() { return _parameter3; }
    std::string GetType() { return _type; }
    std::list<vcSortablePoint> GetPoints() { return _values; }
};

#endif