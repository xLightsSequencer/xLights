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

#include <wx/wx.h>

class MHDimmerPresetBitmapButton : public wxBitmapButton
{
public:
    MHDimmerPresetBitmapButton(wxWindow* parent, wxWindowID id, const wxBitmapBundle& bitmap, const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = wxBU_AUTODRAW, const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxButtonNameStr);
    virtual void DoSetSizeHints(int minW, int minH,
        int maxW, int maxH,
        int incW, int incH);
    virtual ~MHDimmerPresetBitmapButton();
    void SetBitmap(const wxBitmapBundle& bmp);
    void SetPreset(const std::string& _settings);
    std::string& GetPreset() { return mSettings; }

protected:
    void RenderNewBitmap();
    wxBitmap CreateImage( int w, int h, double scaleFactor );

private:
    [[nodiscard]] wxPoint2DDouble NormalizedToUI(const wxPoint2DDouble& pt, double scaleFactor) const;
    [[nodiscard]] wxPoint NormalizedToUI2(const wxPoint2DDouble& pt, double scaleFactor) const;

    wxString(*GetStringFunction)();
    std::string mSettings;
};
