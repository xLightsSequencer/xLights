#pragma once

#include <tuple>
#include <wx/colourdata.h>

class wxConfigBase;
class wxColour;
class wxWindow;

class xlColourData
{
public:
    static xlColourData INSTANCE;

    void Load(wxConfigBase* config);
    void Save(wxConfigBase* config);

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

    [[nodiscard]] std::tuple<int, wxColour> ShowColorDialog(wxWindow* parent, const wxColour& colour);

private:
    xlColourData();
    ~xlColourData();

    wxColourData m_colorData;
};