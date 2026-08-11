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
class RenderableEffect;
class SequenceElements;
class xLightsFrame;
class ValueCurveButton;
class ValueCurve;

class EffectPanelUtils
{
public:
    static void enableControlsByName(wxWindow *window, const wxString &name, bool enable);

    static void OnLockButtonClick(wxCommandEvent& event);
    static void OnVCButtonClick(wxCommandEvent& event);
    static void OnVCChanged(wxCommandEvent& event);
    // Callers holding a live button must use this rather than posting an
    // EVT_VC_CHANGED naming it: the queued event keeps a raw pointer, and
    // panels that rebuild their controls free the button before it drains.
    static void SyncValueCurveControls(ValueCurveButton* vcb);

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
    virtual void SetRenderableEffect(RenderableEffect* eff) {}
    virtual void SetEffectTimeRange(int startTimeMs, int endTimeMs) {}
    // Called the first time this panel is actually displayed (the effect
    // choicebook page becomes selected). Panels are all pre-built hidden at
    // startup (EffectsPanel builds every effect's panel up front), so any
    // internal wxNotebook can be stuck at a stale, too-small page size from
    // that hidden construction pass -- wxNotebook only resizes a page as part
    // of SetSelection(), which already ran once back then, before the outer
    // AUI "Effect" pane had its real geometry. Override to force such
    // children to re-layout at their now-correct size.
    virtual void RefreshLayoutOnShow() {}
    void SetSequenceElements(SequenceElements* els) { mSequenceElements = els; }
    virtual wxString GetEffectString();

    virtual bool HasAssistPanel() { return false; }
    virtual class AssistPanel* GetAssistPanel(wxWindow* parent, xLightsFrame* xl_frame);

    // Panel-specific gate on top of the generic "2+ effects of this type selected"
    // check in IsBulkEditAvailable() -- lets a panel hide "Bulk Edit" for a control
    // when its own UI state means the edit wouldn't apply anywhere (e.g. the Moving
    // Head panel with no fixture checked in the Fixtures row).
    virtual bool IsBulkEditAllowed() { return true; }

    // Lets a panel take over what a bulk edit actually does to the other selected
    // effects, instead of the generic "write id/value onto every selected effect's
    // settings map" behaviour. rawId is the window name of the control the bulk
    // edit menu was raised from (e.g. "ID_SLIDER_MHPan"), before FixIdForPanel().
    // Return true if this call was fully handled (including applying to the other
    // selected effects); return false to fall back to the generic behaviour.
    virtual bool BulkEditApplySetting(const std::string& rawId, const std::string& value, ValueCurve* vc, const std::string& vcid) { return false; }

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
