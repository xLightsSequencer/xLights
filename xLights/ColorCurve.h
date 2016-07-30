#ifndef COLORCURVE_H
#define COLORCURVE_H

#include <wx/position.h>
#include <wx/string.h>
#include <wx/wx.h>
#include <wx/colour.h>
#include <list>

class ccSortableColorPoint
{
public:

    static float Normalise(float v)
    {
        return std::round(v * 40.0) / 40.0;
    }

	float x; // 0-1 ... the start point of this point
	wxColor color; // the colour of the mid point of this

    std::string Serialise() const
    {
        std::string res = "";
        res += "x=" + wxString::Format("%.2f", x).ToStdString();
        res += "|c=" + color.GetAsString().ToStdString();

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
            color = wxColor(v);
        }
    }
    ccSortableColorPoint(std::string& s)
    {
        Deserialise(s);
    }
    ccSortableColorPoint(float xx, wxColor c)
    {
        x = Normalise(xx);
		color = c;
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
    ColorCurve() { ColorCurve(""); _active = false; };
    ColorCurve(const std::string& serialised);
    ColorCurve(const std::string& id, const std::string type, wxColor c = *wxBLACK);
    std::string Serialise();
    bool IsOk() const
    { return _id != ""; }
    void Deserialise(const std::string& s);
    void SetType(std::string type);
    wxColor GetValueAt(float offset);
	wxBitmap GetImage(int x, int y);
    void SetActive(bool a) { _active = a; }
    bool IsActive() const
    { return IsOk() && _active; }
    void ToggleActive() { _active = !_active; }
    void SetValueAt(float offset, wxColor x);
    void DeletePoint(float offset);
    bool IsSetPoint(float offset);
    int GetPointCount() const
    { return _values.size(); }
    std::string GetType() const
    { return _type; }
    std::list<ccSortableColorPoint> GetPoints() const
    { return _values; }
    bool NearPoint(float x);
};

wxDECLARE_EVENT(EVT_CC_CHANGED, wxCommandEvent);

class ColorCurveButton :
    public wxBitmapButton
{
    ColorCurve* _cc;
    std::string _color;
    void NotifyChange();
    void LeftClick(wxCommandEvent& event);
    void RightClick(wxContextMenuEvent& event);
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
    std::string GetColor() const { return _color; }
    void SetValue(std::string color);
};

#endif
