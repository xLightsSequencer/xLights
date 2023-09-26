/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/wx.h>
#include <wx/string.h>
#include <wx/xml/xml.h>
#include <wx/bitmap.h>
#include <wx/colour.h>
#include <wx/colordlg.h>
#include <wx/graphics.h>

#include "ColorCurve.h"
#include "ColorCurveDialog.h"
#include "UtilFunctions.h"
#include "ColorPanel.h"

#include <log4cpp/Category.hh>

#if wxUSE_GRAPHICS_CONTEXT == 0
#error Please refer to README.windows to make necessary changes to wxWidgets setup.h file.
#error You will also need to rebuild wxWidgets once the change is made.
#endif

wxColourData ColorCurveButton::_colorData;

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
    _values.push_back(ccSortableColorPoint(0.5, *wxBLACK));
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
        _values.push_back(ccSortableColorPoint(0.5, *wxBLACK));
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
        wxArrayString v = wxSplit(wxString(s.c_str()), '|');
        for (auto vs = v.begin(); vs != v.end(); vs++)
        {
            if (vs->Find('=') != std::string::npos)
            {
                wxArrayString v1;
                v1.Add(vs->SubString(0, vs->Find('=')-1));
                v1.Add(vs->SubString(vs->Find('=')+1, vs->Length()));
                if (v1.size() == 2)
                {
                    SetSerialisedValue(v1[0].ToStdString(), v1[1].ToStdString());
                }
            }
        }
    }

    if (_values.size() == 0)
    {
        _values.push_back(ccSortableColorPoint(0.5, *wxBLACK));
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
            res += "Timecurve=" + wxString::Format("%d", _timecurve).ToStdString() + "|";
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
    else if (kk == "Timecurve")
    {
        _timecurve = wxAtoi(s);
    }
    else if (kk == "Values")
        {
            wxArrayString points = wxSplit(s, ';');

            for (auto p = points.begin(); p != points.end(); p++)
            {
                std::string ss = p->ToStdString();
                _values.push_back(ccSortableColorPoint(ss));
            }
        }

    _values.sort();
}

void ColorCurve::SetType(std::string type)
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
        wxBell();
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

void ColorCurve::SetDefault(const wxColor& color)
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

    wxXmlDocument doc(filename);

    if (doc.IsOk())
    {
        wxXmlNode* root = doc.GetRoot();

        if (root->GetName() == "colorcurve")
        {
            wxString data = root->GetAttribute("data");
            wxString v = root->GetAttribute("SourceVersion");

            // Add any colorcurve version conversion logic here
            // Source version will be the program version that created the custom model

            Deserialise(data.ToStdString());
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

wxBitmap ColorCurve::GetImage(int x, int y, bool bars)
{
    wxImage bmp(x, y);
    wxBitmap b(bmp);
    wxMemoryDC dc(b);

    dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK)));
    dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK)));
    dc.DrawRectangle(0, 0, x, y);
    dc.SetBrush(*wxTRANSPARENT_BRUSH);

    for (int i = 0; i < x; i++)
    {
        wxColor c = GetValueAt(static_cast<float>(i) / static_cast<float>(x)).asWxColor();
        dc.SetPen(wxPen(c, 1, wxPENSTYLE_SOLID));
        if (bars)
        {
            //dc.DrawLine(wxPoint(i, static_cast<float>(y)*0.05), wxPoint(i, static_cast<float>(y)*0.95));
            dc.DrawLine(wxPoint(i, 0), wxPoint(i, static_cast<float>(y) * 0.75));
        }
        else
        {
            dc.DrawLine(wxPoint(i, 0), wxPoint(i, y));
        }
    }

    if (!bars)
    {
        dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK)));
        dc.DrawRectangle(0, 0, x, y);
    }
    return b;
}

wxBitmap ColorCurve::GetSolidColourImage(int x, int y, const wxColour& c)
{
    wxImage bmp(x, y);
    wxBitmap b(bmp);
    wxMemoryDC dc(b);
    dc.SetPen(wxPen(c));
    dc.SetBrush(wxBrush(c));
    dc.DrawRectangle(0, 0, x, y);
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK)));
    dc.DrawRectangle(0, 0, x, y);
    return b;
}

std::string ColorCurve::GetColorCurveFolder(const std::string& showFolder)
{
    if (showFolder == "") return "";

    std::string ccf = showFolder + "/colorcurves";
    if (!wxDir::Exists(ccf))
    {
        wxMkdir(ccf);
        if (!wxDir::Exists(ccf))
        {
            return "";
        }
    }
    return ccf;
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

#pragma region ColorCurveButton
#include <wx/dcmemory.h>

wxDEFINE_EVENT(EVT_CC_CHANGED, wxCommandEvent);

ColorCurveButton::ColorCurveButton(wxWindow *parent,
    wxWindowID id,
    const wxBitmap& bitmap,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxValidator& validator,
    const wxString& name) : wxBitmapButton(parent, id, bitmap, pos, size, style, validator, name)
{
    _color = "#FFFFFF";
    _cc = new ColorCurve(name.ToStdString(), "Gradient", _color);
    Connect(id, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ColorCurveButton::LeftClick);
    Connect(id, wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&ColorCurveButton::RightClick);
}

void ColorCurveButton::LeftClick(wxCommandEvent& event)
{
    ColorCurveButton* w = static_cast<ColorCurveButton*>(event.GetEventObject());
    wxColour color = w->GetBackgroundColour();
    _colorData.SetColour(color);
    wxColourDialog dialog(this, &_colorData);
    if (dialog.ShowModal() == wxID_OK)
    {
        _cc->SetActive(false);
        _colorData = dialog.GetColourData();
        color = _colorData.GetColour();
        _color = color.GetAsString();
        _cc->SetDefault(color);
        UpdateBitmap();
        NotifyChange(true);
    }
}

void ColorCurveButton::RightClick(wxContextMenuEvent& event)
{
    ColorCurveButton* w = static_cast<ColorCurveButton*>(event.GetEventObject());

    ColorCurveDialog ccd(this, w->GetValue(), _colorData);
    if (ccd.ShowModal() == wxID_OK)
    {
        w->SetActive(true);
        UpdateBitmap();
        NotifyChange(true);
    }
    else
    {
        if (ccd.DidExport())
        {
            NotifyChange(true);
        }
    }
}

ColorCurveButton::~ColorCurveButton()
{
    if (_cc != nullptr)
    {
        delete _cc;
    }
}

void ColorCurveButton::SetActive(bool active, bool notify)
{
    _cc->SetActive(active);
    UpdateState(notify);
}

void ColorCurveButton::ToggleActive()
{
    _cc->ToggleActive();
    UpdateState();
}

void ColorCurveButton::SetDefaultCC(const std::string& color)
{
    _cc->SetDefault(wxColor(color));
}

void ColorCurveButton::SetColor(std::string color, bool notify)
{
    _cc->SetActive(false);
    _cc->SetDefault(wxColor(color));
    _color = color;
    UpdateBitmap();
    if (notify) {
        NotifyChange();
    }
}

void ColorCurveButton::UpdateBitmap() {
    wxSize sz = GetSize();
    if (GetValue()->IsActive())
    {
        SetBitmap(_cc->GetImage(sz.GetWidth(), sz.GetHeight(), false));
        UnsetToolTip();
    }
    else
    {
        wxColor color(_color);
        SetBackgroundColour(color);
        SetForegroundColour(color);
        wxImage image(sz.GetWidth(), sz.GetHeight());
        if (color.IsOk()) {
            image.SetRGB(wxRect(0, 0, sz.GetWidth(), sz.GetHeight()),
                color.Red(), color.Green(), color.Blue());
        }
        wxBitmap bmp(image);
        SetBitmap(bmp);
        SetToolTip(wxString::Format("%s\n%d,%d,%d\n%s", _color, color.Red(), color.Green(), color.Blue(), GetColourName(color)));
    }
    Refresh();
}

void ColorCurveButton::UpdateState(bool notify)
{
    UpdateBitmap();
    if (notify) {
        NotifyChange();
    }
}

void ColorCurveButton::SetValue(const wxString& value)
{
    _cc->Deserialise(value.ToStdString());
    _cc->SetId(GetName().ToStdString());
    UpdateState();
}

void ColorCurveButton::NotifyChange(bool coloursPanelReload)
{
    wxCommandEvent eventCCChange(EVT_CC_CHANGED);
    eventCCChange.SetInt(coloursPanelReload);
    eventCCChange.SetEventObject(this);
    wxPostEvent(GetParent(), eventCCChange);
}

ColorCurve* ColorCurveButton::GetValue() const
{
    return _cc;
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

#pragma endregion
