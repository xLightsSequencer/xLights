#ifndef BASEEFFECTPANEL_H
#define BASEEFFECTPANEL_H

#include <map>
#include <string>

class wxCommandEvent;
class wxScrollEvent;
class wxPanel;

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
#endif
