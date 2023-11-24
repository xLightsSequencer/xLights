#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/position.h>
#include <string>
#include <list>

#define MINVOID -91234
#define MAXVOID 91234
#define MINVOIDF -9.1234f
#define MAXVOIDF 9.1234f

#define VC_X_POINTS 200.0

class wxFileName;
class AudioManager;
class SequenceElements;

class vcSortablePoint
{
public:

    static float Normalise(float v)
    {
        return std::round(v * VC_X_POINTS) / VC_X_POINTS;
    }
    static float perPoint()
    {
        return (float)(1.0f / VC_X_POINTS);
    }
    float x;
    float y;
    bool wrapped;
    vcSortablePoint(float xx, float yy, bool wrap)
    {
        x = Normalise(xx);
        y = yy;
        wrapped = wrap;
    }
    bool IsNear(float xx, float yy) const
    {
        return (x == Normalise(xx) && yy >= y - 0.05 && yy <= y + 0.05);
    }
    void ClearWrap() { wrapped = false; }
    bool IsWrapped() const { return wrapped; }
    bool operator==(const vcSortablePoint& r) const
    {
        return x == r.x;
    }
    bool operator==(const float r) const
    {
        return x == Normalise(r);
    }
    bool operator<(const vcSortablePoint& r) const
    {
        return x < r.x;
    }
    bool operator<(const float r) const
    {
        return x < Normalise(r);
    }
    bool operator<=(const vcSortablePoint& r) const
    {
        return x <= r.x;
    }
    bool operator<=(const float r) const
    {
        return x <= Normalise(r);
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
    std::string _timingTrack;
    float _max;
    float _min;
    float _divisor;
    int _timeOffset;
    float _parameter1;
    float _parameter2;
    float _parameter3;
    float _parameter4;
    bool _active;
    bool _wrap;
    bool _realValues;
    static AudioManager* __audioManager;
    static SequenceElements* __sequenceElements;

    void RenderType();
    void SetSerialisedValue(const std::string &k, const std::string &s);
    float SafeParameter(size_t p, float v);
    float Safe01(float v);
    void ConvertToRealValues(float oldmin, float oldmax);
    float Normalise(int parm, float value);
    float Denormalise(int parm, float value) const;
    float ApplyGain(float value, int gain) const;
    int GetSubsequentTimingMark(const std::string& timingTrack, int time, bool startsOnly);
    int GetPriorTimingMark(const std::string& timingTrack, int time, bool startsOnly);

public:

    static void SetAudio(AudioManager* am) { __audioManager = am; }
    static void SetSequenceElements(SequenceElements* se) { __sequenceElements = se; }
    static SequenceElements* GetSequenceElements() { return __sequenceElements; }
    static std::string GetValueCurveFolder(const std::string& showFolder);

    ValueCurve() { _divisor = 1; _min = MINVOIDF; _max = MAXVOIDF; SetDefault(); }
    ValueCurve(const std::string& serialised);
    ValueCurve(const std::string& id, float min, float max = 100.0f, const std::string type = "Flat", float parameter1 = 0.0f, float parameter2 = 0.0f, float parameter3 = 0.0f, float parameter4 = 0.0f, bool wrap = false, float divisor = 1.0, const std::string& timingTrack = "");
    void SetDefault(float min = MINVOIDF, float max = MAXVOIDF, int divisor = MAXVOID);
    wxBitmap GetImage(int x, int y, double scaleFactor = 1.0);
    std::string Serialise();
    void LoadXVC(const wxFileName& fn);
    void LoadXVC(const std::string& fn);
    void SaveXVC(const wxFileName& fn);
    void SaveXVC(const std::string& fn);
    static void GetRangeParm(int parm, const std::string& type, float& low, float& high);
    bool IsOk() const { return _id != ""; }
    void Deserialise(const std::string& s, bool holdminmax = false);
    void SetType(std::string type);
    void FixChangedScale(float oldmin, float oldmax, int divisor);
    void UnFixChangedScale(float newmin, float newmax);
    void ConvertChangedScale(float newmin, float newmax);
    float GetMax() const { wxASSERT(_max != MAXVOIDF); return _max; }
    float GetMin() const { wxASSERT(_min != MINVOIDF); return _min; }
    int GetDivisor() const { wxASSERT(_divisor != MAXVOID); return (int)_divisor; }
    void SetRealValue() { _realValues = true; }
    void SetLimits(float min, float max) { _min = min; _max = max; }
    void FixScale(int scale);
    float GetValueAt(float offset, long startMS, long endMS);
    float GetOutputValueAt(float offset, long startMS, long endMS);
    float GetOutputValueAtDivided(float offset, long startMS, long endMS);
    float GetMaxValueDivided();
    float GetScaledValue(float offset) const;
    void SetActive(bool a) { _active = a; RenderType(); }
    bool IsActive() const { return _active && IsOk(); }
    void ToggleActive() { _active = !_active; if (_active) RenderType(); }
    void SetValueAt(float offset, float value);
    void DeletePoint(float offset);
    bool IsSetPoint(float offset);
    void SetDivisor(float divisor) { _divisor = divisor; }
    bool IsRealValue() const { return _realValues; }
    int GetPointCount() const { return _values.size(); }
    void SetTimingTrack(const std::string& timingTrack) { _timingTrack = timingTrack; }
    std::string GetTimingTrack() const { return _timingTrack; }
    void SetParameter1(float parameter1) { _parameter1 = SafeParameter(1, parameter1); RenderType(); }
    void SetParameter2(float parameter2) { _parameter2 = SafeParameter(2, parameter2); RenderType(); }
    void SetParameter3(float parameter3) { _parameter3 = SafeParameter(3, parameter3); RenderType(); }
    void SetParameter4(float parameter4) { _parameter4 = SafeParameter(4, parameter4); RenderType(); }
    void SetTimeOffset(int timeOffset) { _timeOffset = timeOffset; RenderType(); }
    void SetWrap(bool wrap);
    int GetTimeOffset() const { return _timeOffset; }
    float GetParameter1() const { return _parameter1; }
    float GetParameter1_100() const;
    float GetParameter2() const { return _parameter2; }
    float GetParameter3() const { return _parameter3; }
    float GetParameter4() const { return _parameter4; }
    bool GetWrap() const { return _wrap; }
    std::string GetType() const { return _type; }
    std::list<vcSortablePoint> GetPoints() const { return _values; }
    void RemoveExcessCustomPoints();
    float FindMinPointLessThan(float point) const;
    float FindMaxPointGreaterThan(float point) const;
    bool NearCustomPoint(float x, float y) const;
    float GetPointAt(float x) const;
    void SetPointAt(float x, float y);
    std::string GetId() const { return _id; }
    void SetId(const std::string& id) { _id = id; }
    static void GetRangeParm1(const std::string& type, float& low, float &high);
    static void GetRangeParm2(const std::string& type, float& low, float &high);
    static void GetRangeParm3(const std::string& type, float& low, float &high);
    static void GetRangeParm4(const std::string& type, float& low, float &high);
    void Reverse();
    void Flip();
    void ConvertDivider(int oldDivider, int newDivider);
    void ScaleAndOffsetValues(float scale, int offset);
};
