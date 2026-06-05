#pragma once

#include <tuple>
#include <wx/colourdata.h>

class XLightsConfigAdapter;
class wxColour;
class wxWindow;

class xlColourData
{
public:
    static xlColourData INSTANCE;

    void Load(XLightsConfigAdapter* config);
    void Save(XLightsConfigAdapter* config);

    [[nodiscard]] const wxColour& GetColor() const
    {
        return m_colorData.GetColour();
    }
    [[nodiscard]] wxColour& GetColor()
    {
        return m_colorData.GetColour();
    }
    void SetColour(const wxColour& colour)
    {
        m_colorData.SetColour(colour);
    }

    [[nodiscard]] wxColour GetCustomColour(int i) const
    {
        return m_colorData.GetCustomColour(i);
    }
    void SetCustomColour(int i, const wxColour& c)
    {
        m_colorData.SetCustomColour(i, c);
    }
    int GetNumCustomColours() const
    {
        return m_colorData.NUM_CUSTOM;
    }

    bool UseCustomPicker() const { return m_useCustomPicker; }
    void SetUseCustomPicker(bool v);

    [[nodiscard]] std::tuple<int, wxColour> ShowColorDialog(wxWindow* parent, const wxColour& colour);

private:
    xlColourData();
    ~xlColourData();

    wxColourData m_colorData;
    bool m_useCustomPicker = false;
};
