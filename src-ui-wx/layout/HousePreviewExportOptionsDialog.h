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

#include <wx/dialog.h>

#include <vector>

class wxChoice;
class wxSpinCtrl;
class wxCommandEvent;

// Lets the user pick the pixel resolution to render the House Preview video
// export at, instead of always being tied to the House Preview pane's
// current on-screen size (see GitHub issue #3763).
class HousePreviewExportOptionsDialog : public wxDialog
{
public:
    HousePreviewExportOptionsDialog(wxWindow* parent, int currentWidth, int currentHeight);
    virtual ~HousePreviewExportOptionsDialog() = default;

    int GetExportWidth() const;
    int GetExportHeight() const;

private:
    struct ResolutionPreset {
        wxString label;
        int width;
        int height;
    };

    void OnResolutionChoice(wxCommandEvent& event);
    void OnOk(wxCommandEvent& event);

    std::vector<ResolutionPreset> _presets;
    int _currentWidth;
    int _currentHeight;

    wxChoice* _resolutionChoice = nullptr;
    wxSpinCtrl* _widthSpin = nullptr;
    wxSpinCtrl* _heightSpin = nullptr;
};
