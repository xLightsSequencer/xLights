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
#include <wx/panel.h>

#include "../xlLockButton.h"

wxDECLARE_EVENT(EVT_VALIDATEWINDOW, wxCommandEvent);

class wxPanel;
class wxControl;
class wxWindow;
class wxString;
class wxButton;
class wxTimer;

class wxCommandEvent;
class wxScrollEvent;
class wxSpinEvent;
class wxFileDirPickerEvent;
class wxBookCtrlEvent;
class wxFontPickerEvent;
class wxColourPickerEvent;

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


class xlEffectPanel: public wxPanel {
public:

    xlEffectPanel(wxWindow* parent);
    virtual ~xlEffectPanel();
    
    virtual void ValidateWindow() = 0;
    
    void AddChangeListeners(wxTimer *timer);
    
    void OnLockButtonClick(wxCommandEvent& event) {
        EffectPanelUtils::OnLockButtonClick(event);
    }
    void OnVCButtonClick(wxCommandEvent& event) {
        EffectPanelUtils::OnVCButtonClick(event);
    }
    void OnVCChanged(wxCommandEvent& event) {
        EffectPanelUtils::OnVCChanged(event);
        FireChangeEvent();
    }
    void OnValidateWindow(wxCommandEvent& event) {
        ValidateWindow();
    }

    
    void HandleFileDirChange(wxFileDirPickerEvent& event);
    void HandleFontChange(wxFontPickerEvent& event);
    void HandleSpinChange(wxSpinEvent& event);
    void HandleNotebookChange(wxBookCtrlEvent& event);
    void HandleScrollChange(wxScrollEvent& event);
    void HandleCommandChange(wxCommandEvent& event);
    void HandleColorChange(wxColourPickerEvent& event);
    
    static const wxBitmapBundle& GetValueCurveNotSelectedBitmap();

protected:
    void FireChangeEvent();
    void AddListeners(wxWindow *ParentWin);

    wxTimer *changeTimer;
};

