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

#include <map>
#include <string>

#include "../xlLockButton.h"

class wxCommandEvent;
class wxScrollEvent;
class wxPanel;
class wxControl;
class wxWindow;
class wxString;
class wxButton;

class EffectPanelUtils
{
public:
    static void enableControlsByName(wxWindow *window, const wxString &name, bool enable);

    static void OnLockButtonClick(wxCommandEvent& event);
    static void OnVCButtonClick(wxCommandEvent& event);
    static void OnVCChanged(wxCommandEvent& event);

    static void SetLock(wxButton* button);
    static bool IsLocked(std::string name);
    static bool IsLockable(wxControl* ctl);
private:
    static std::map<std::string, bool> buttonStates;
};

#define PANEL_EVENT_HANDLERS(cls) \
void cls::OnLockButtonClick(wxCommandEvent& event) { \
    EffectPanelUtils::OnLockButtonClick(event); \
} \
void cls::OnVCButtonClick(wxCommandEvent& event) { \
    EffectPanelUtils::OnVCButtonClick(event); \
} \
void cls::OnVCChanged(wxCommandEvent& event) { \
        EffectPanelUtils::OnVCChanged(event); \
}
