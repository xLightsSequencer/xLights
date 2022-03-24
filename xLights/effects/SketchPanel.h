#pragma once

#include "EffectPanelUtils.h"

class BulkEditCheckBox;
class BulkEditSlider;
class BulkEditTextCtrl;

class SketchPanel : public xlEffectPanel
{
public:
    SketchPanel(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~SketchPanel() = default;

    void ValidateWindow() override;

    // controls are public to allow SketchEffect access
    BulkEditTextCtrl* TextCtrl_SketchDef = nullptr;
    BulkEditCheckBox* CheckBox_MotionEnabled = nullptr;
    BulkEditSlider* Slider_MotionPercentage = nullptr;

protected:
    static const long ID_TEXTCTRL_SketchDef;
    static const long ID_CHECKBOX_MotionEnabled;
    static const long ID_SLIDER_MotionPercentage;

 private:
    DECLARE_EVENT_TABLE()

    void OnButton_DefineSketch(wxCommandEvent& event);
    //void OnCheckBox_ToggleMotion(wxCommandEvent& event);
};