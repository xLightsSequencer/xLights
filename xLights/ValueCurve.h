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
    bool operator==(const vcSortablePoint& r) const
    {
        return x == r.x;
    }
    bool operator==(const float r) const
    {
        return x == r;
    }
    bool operator<(const vcSortablePoint& r) const
    {
        return x < r.x;
    }
    bool operator<(const float r) const
    {
        return x < r;
    }
    bool operator<=(const vcSortablePoint& r) const
    {
        return x <= r.x;
    }
    bool operator<=(const float r) const
    {
        return x <= r;
    }
    bool operator>(const vcSortablePoint& r) const
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
    float _parameter4;
    bool _active;

    void RenderType();
    void SetSerialisedValue(std::string k, std::string s);
    float SafeParameter(size_t p, float v);
    float Safe01(float v);

public:
    ValueCurve() { ValueCurve(""); };
    ValueCurve(const std::string& id, float min = 0.0f, float max = 100.0f, const std::string type = "Flat", float parameter1 = 0.0f, float parameter2 = 0.0f, float parameter3 = 0.0f, float parameter4 = 0.0f);
    void SetDefault(float min = 0.0f, float max = 100.0f);
    std::string Serialise();
    bool IsOk() { return _id != ""; }
    void Deserialise(std::string s);
    void SetType(std::string type);
    float GetMax() { return _max; }
    float GetMin() { return _min; }
    void SetLimits(float min, float max) { _min = min; _max = max; }
    float GetValueAt(float offset);
    float GetOutputValueAt(float offset);
    void SetActive(bool a) { _active = a; RenderType(); }
    bool IsActive() { return IsOk() && _active; }
    void ToggleActive() { _active = !_active; if (_active) RenderType(); }
    void SetValueAt(float offset, float value);
    void DeletePoint(float offset);
    bool IsSetPoint(float offset);
    int GetPointCount() { return _values.size(); }
    void SetParameter1(float parameter1) { _parameter1 = SafeParameter(1, parameter1); RenderType(); }
    void SetParameter2(float parameter2) { _parameter2 = SafeParameter(2, parameter2); RenderType(); }
    void SetParameter3(float parameter3) { _parameter3 = SafeParameter(3, parameter3); RenderType(); }
    void SetParameter4(float parameter4) { _parameter4 = SafeParameter(4, parameter4); RenderType(); }
    float GetParameter1() { return _parameter1; }
    float GetParameter2() { return _parameter2; }
    float GetParameter3() { return _parameter3; }
    float GetParameter4() { return _parameter4; }
    std::string GetType() { return _type; }
    std::list<vcSortablePoint> GetPoints() { return _values; }
};

#endif