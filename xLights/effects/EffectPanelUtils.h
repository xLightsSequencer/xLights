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

    //static void UpdateLinkedSliderFloat(wxCommandEvent& event);
    //static void UpdateLinkedSliderFloat2(wxCommandEvent& event);
    //static void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
    //static void UpdateLinkedTextCtrlFloat2(wxScrollEvent& event);
    //static void UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event);
    //static void UpdateLinkedTextCtrl360(wxScrollEvent& event);
    //static void UpdateLinkedSlider360(wxCommandEvent& event);
    //static void UpdateLinkedTextCtrl(wxScrollEvent& event);
    //static void UpdateLinkedTextCtrlVC(wxScrollEvent& event);
    //static void UpdateLinkedSlider(wxCommandEvent& event);
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
//#define PANEL_EVENT_HANDLERS(cls) \
//void cls::UpdateLinkedSliderFloat(wxCommandEvent& event) { \
//   EffectPanelUtils::UpdateLinkedSliderFloat(event); \
//} \
//void cls::UpdateLinkedSliderFloat2(wxCommandEvent& event) { \
//   EffectPanelUtils::UpdateLinkedSliderFloat2(event); \
//} \
//void cls::UpdateLinkedTextCtrlFloat(wxScrollEvent& event) { \
//    EffectPanelUtils::UpdateLinkedTextCtrlFloat(event); \
//} \
//void cls::UpdateLinkedTextCtrlFloat2(wxScrollEvent& event) { \
//    EffectPanelUtils::UpdateLinkedTextCtrlFloat2(event); \
//} \
//void cls::UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event) { \
//    EffectPanelUtils::UpdateLinkedTextCtrlFloatVC(event); \
//} \
//void cls::UpdateLinkedTextCtrl360(wxScrollEvent& event) { \
//    EffectPanelUtils::UpdateLinkedTextCtrl360(event); \
//} \
//void cls::UpdateLinkedSlider360(wxCommandEvent& event) { \
//    EffectPanelUtils::UpdateLinkedSlider360(event); \
//} \
//void cls::UpdateLinkedTextCtrl(wxScrollEvent& event) { \
//    EffectPanelUtils::UpdateLinkedTextCtrl(event); \
//} \
//void cls::UpdateLinkedTextCtrlVC(wxScrollEvent& event) { \
//    EffectPanelUtils::UpdateLinkedTextCtrlVC(event); \
//} \
//void cls::UpdateLinkedSlider(wxCommandEvent& event) { \
//    EffectPanelUtils::UpdateLinkedSlider(event); \
//} \
//void cls::OnLockButtonClick(wxCommandEvent& event) { \
//    EffectPanelUtils::OnLockButtonClick(event); \
//} \
//void cls::OnVCButtonClick(wxCommandEvent& event) { \
//    EffectPanelUtils::OnVCButtonClick(event); \
//} \
//void cls::OnVCChanged(wxCommandEvent& event) { \
//        EffectPanelUtils::OnVCChanged(event); \
//} 
#endif
