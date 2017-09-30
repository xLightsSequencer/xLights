#ifndef COLORCURVE_H
#define COLORCURVE_H

#include <wx/position.h>
#include <wx/string.h>
#include <wx/wx.h>
#include <wx/colour.h>
#include <wx/colourdata.h>
#include <list>

#include "Color.h"

class ccSortableColorPoint
{
public:

    static float Normalise(float v)
    {
        if (v < 0) v = 0;
        if (v > 1) v = 1;

        return std::round(v * 40.0) / 40.0;
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
        res += "x=" + wxString::Format("%.3f", x).ToStdString();
        wxString c;
        c = color;
        c.Replace(",", "@", true);
        res += "^c=" + c.ToStdString();

        return res;
    }

    void Deserialise(std::string s)
    {
        if (s == "")
        {
            throw;
        }
        else if (s.find('^') == std::string::npos)
        {
            throw;
        }
        else
        {
            wxArrayString v = wxSplit(wxString(s.c_str()), '^');
            for (auto vs = v.begin(); vs != v.end(); vs++)
            {
                wxArrayString v1 = wxSplit(*vs, '=');
                if (v1.size() == 2)
                {
                    SetSerialisedValue(v1[0].ToStdString(), v1[1].ToStdString());
                }
            }
        }
    }
    void SetSerialisedValue(std::string k, std::string v)
    {
        if (k == "x")
        {
            x = ccSortableColorPoint::Normalise(wxAtof(wxString(v)));
        }
        else if (k == "c")
        {
            wxString c(v);
            c.Replace("@", ",", true);
            color = xlColor(c);
        }
    }
    ccSortableColorPoint(std::string& s)
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
        //return x < Normalise(r);
    }
    bool operator<=(const ccSortableColorPoint& r) const
    {
        return x <= r.x;
    }
    bool operator<=(const float r) const
    {
        return x <= r;
        //return x <= Normalise(r);
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
    void NextTimeCurve(bool supportslinear, bool supportsradial);
    void SetValidTimeCurve(bool supportslinear, bool supportsradial);
    int GetTimeCurve() const { return _timecurve; }
    std::string GetId() const { return _id; }
    void SetId(std::string& id) { _id = id; }
    ColorCurve() { ColorCurve(""); _active = false; _timecurve = TC_TIME; };
    ColorCurve(const std::string& serialised);
    ColorCurve(const std::string& id, const std::string type, xlColor c = xlBLACK);
    std::string Serialise();
    bool IsOk() const
    { return _id != ""; }
    void Deserialise(const std::string& s);
    void SetType(std::string type);
    xlColor GetValueAt(float offset) const;
    ccSortableColorPoint* GetPointAt(float offset);
	wxBitmap GetImage(int x, int y, bool bars);
    void SetActive(bool a) { _active = a; }
    bool IsActive() const
    { return IsOk() && _active; }
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
    void SetDefault(const wxColor& color);
};

wxDECLARE_EVENT(EVT_CC_CHANGED, wxCommandEvent);

class ColorCurveButton :
    public wxBitmapButton
{
    ColorCurve* _cc;
    std::string _color;
    static wxColourData _colorData;
    void NotifyChange();
    void LeftClick(wxCommandEvent& event);
    void RightClick(wxContextMenuEvent& event);

public:
    ColorCurveButton(wxWindow *parent, 
        wxWindowID id,
        const wxBitmap& bitmap,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxBU_AUTODRAW,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxButtonNameStr);
    ~ColorCurveButton();
    virtual void SetValue(const wxString& value);
    ColorCurve* GetValue() const;
    void ToggleActive();
    void SetActive(bool active, bool notify = true);
    void UpdateState(bool notify = true);
    void UpdateBitmap();
    std::string GetColor() const { return _color; }
    void SetColor(std::string color, bool notify = true);
    void SetDefaultCC(const std::string& color);
};

#endif
