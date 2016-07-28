#ifndef COLORCURVE_H
#define COLORCURVE_H

#include <wx/position.h>
#include <wx/string.h>
#include <wx/wx.h>
#include <string>
#include <list>
#include "Color.h"

class ccSortableColorPoint
{
public:

    static float Normalise(float v)
    {
        return std::round(v * 40.0) / 40.0;
    }
    
	float x; // 0-1 ... the start point of this point
	xlColor color; // the colour of the mid point of this 
	float fadein; // % of the duration of this point that should fade in
	float fadeout; // % of the duration of this point that should fade out

    std::string Serialise()
    {
        std::string res = "";
        res += "x=" + wxString::Format("%.2f", x).ToStdString();
        res += "|c=" + ((wxString)color).ToStdString();

        if (fadein != 0)
        {
            res += "|fi=" + wxString::Format("%.2f", fadein).ToStdString();
        }

        if (fadeout != 0)
        {
            res += "|fo=" + wxString::Format("%.2f", fadeout).ToStdString();
        }

        return res;
    }

    void Deserialise(std::string s)
    {
        if (s == "")
        {
            throw;
        }
        else if (s.find('|') == std::string::npos)
        {
            throw;
        }
        else
        {
            fadein = 0;
            fadeout = 0;
            wxArrayString v = wxSplit(wxString(s.c_str()), '|');
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
            x = wxAtof(wxString(v));
        }
        else if (k == "c")
        {
            color.SetFromString(v);
        }
        else if (k == "fi")
        {
            fadein = wxAtof(wxString(v));
        }
        else if (k == "fo")
        {
            fadeout = wxAtof(wxString(v));
        }
    }
    ccSortableColorPoint(std::string& s)
    {
        Deserialise(s);
    }
    ccSortableColorPoint(float xx, xlColor c, float fi, float fo)
    {
        x = Normalise(xx);
		color = c;
		fadein = fi;
		fadeout = fo;
    }
    bool IsNear(float xx)
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
        return x < Normalise(r);
    }
    bool operator<=(const ccSortableColorPoint& r) const
    {
        return x <= r.x;
    }
    bool operator<=(const float r) const
    {
        return x <= Normalise(r);
    }
    bool operator>(const ccSortableColorPoint& r) const
    {
        return x > r.x;
    }
};

class ColorCurve
{
    std::list<ccSortableColorPoint> _values;
    std::string _type;
    std::string _id;
    bool _active;

    float Safe01(float v);
    void SetSerialisedValue(std::string k, std::string v);
    ccSortableColorPoint* GetActivePoint(float x, float& duration);
    ccSortableColorPoint* GetPriorActivePoint(float x, float& duration);
    ccSortableColorPoint* GetNextActivePoint(float x, float& duration);

public:
    ColorCurve() { ColorCurve(""); };
    ColorCurve(const std::string& serialised);
    ColorCurve(const std::string& id, const std::string type, xlColor c = xlBLACK);
    std::string Serialise();
    bool IsOk() { return _id != ""; }
    void Deserialise(const std::string& s);
    void SetType(std::string type);
    xlColor GetValueAt(float offset);
	wxImage GetImage(int x, int y);
    void SetActive(bool a) { _active = a; }
    bool IsActive() { return IsOk() && _active; }
    void ToggleActive() { _active = !_active; }
    void SetValueAt(float offset, xlColor x, float fi, float fo);
    void DeletePoint(float offset);
    bool IsSetPoint(float offset);
    int GetPointCount() { return _values.size(); }
    std::string GetType() { return _type; }
    std::list<ccSortableColorPoint> GetPoints() { return _values; }
    bool NearPoint(float x);
};

wxDECLARE_EVENT(EVT_CC_CHANGED, wxCommandEvent);

class ColorCurveButton :
    public wxBitmapButton
{
    ColorCurve* _cc;
    void NotifyChange();


    void RenderNewBitmap();
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
    ColorCurve* GetValue();
    void ToggleActive();
    void SetActive(bool active);
    void UpdateState();
    void UpdateBitmap();
};

#endif