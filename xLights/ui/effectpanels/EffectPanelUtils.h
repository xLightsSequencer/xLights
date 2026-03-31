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

#include <map>
#include <string>
#include <wx/panel.h>
#include <wx/tglbtn.h>
#include <wx/bmpbuttn.h>

#include "../shared/utils/xlLockButton.h"

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
class wxSlider;
class wxSpinCtrl;
class wxChoice;
class wxTextCtrl;
class wxCheckBox;
class wxRadioButton;
class Model;
class SequenceElements;
class xLightsFrame;

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

    xlEffectPanel();
    virtual ~xlEffectPanel();
    
    virtual void ValidateWindow() = 0;
    virtual void SetDefaultParameters() {}
    virtual void SetPanelStatus(Model* cls) {}
    virtual void SetEffectTimeRange(int startTimeMs, int endTimeMs) {}
    void SetSequenceElements(SequenceElements* els) { mSequenceElements = els; }
    virtual wxString GetEffectString();

    virtual bool HasAssistPanel() { return false; }
    virtual class AssistPanel* GetAssistPanel(wxWindow* parent, xLightsFrame* xl_frame);

    void AddChangeListeners(wxTimer *timer);

    static void SetSliderValue(wxSlider* slider, int value);
    static void SetSpinValue(wxSpinCtrl* spin, int value);
    static void SetChoiceValue(wxChoice* choice, std::string value);
    static void SetRadioValue(wxRadioButton* radio);
    static void SetTextValue(wxTextCtrl* text, std::string value);
    static void SetCheckBoxValue(wxCheckBox* w, bool b);

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
    SequenceElements* mSequenceElements = nullptr;
};


class LinkedToggleButton : public wxBitmapToggleButton {
public:
    LinkedToggleButton(wxWindow* parent, wxWindowID id, const wxString& label, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name);
};
