#pragma once

#include "EffectPanelUtils.h"

class BulkEditCheckBox;
class BulkEditSlider;
class BulkEditTextCtrl;
class BulkEditValueCurveButton;

class SketchPanel : public xlEffectPanel
{
public:
    SketchPanel(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~SketchPanel() = default;

    void ValidateWindow() override;

    // controls are public to allow SketchEffect access
    BulkEditTextCtrl* TextCtrl_SketchDef = nullptr;
    BulkEditSlider* Slider_DrawPercentage = nullptr;
    BulkEditTextCtrl* TextCtrl_DrawPercentage = nullptr;
    BulkEditSlider* Slider_Thickness = nullptr;
    BulkEditValueCurveButton* BitmapButton_Thickness = nullptr;
    BulkEditTextCtrl* TextCtrl_Thickness = nullptr;
    BulkEditCheckBox* CheckBox_MotionEnabled = nullptr;
    BulkEditSlider* Slider_MotionPercentage = nullptr;
    BulkEditTextCtrl* TextCtrl_MotionPercentage = nullptr;

protected:
    static const long ID_TEXTCTRL_SketchDef;
    static const long ID_SLIDER_DrawPercentage;
    static const long ID_TEXTCTRL_DrawPercentage;
    static const long ID_SLIDER_Thickness;
    static const long ID_VALUECURVE_Thickness;
    static const long ID_TEXTCTRL_Thickness;
    static const long ID_CHECKBOX_MotionEnabled;
    static const long ID_SLIDER_MotionPercentage;
    static const long ID_TEXTCTRL_MotionPercentage;

 private:
    DECLARE_EVENT_TABLE()

    void OnButton_DefineSketch(wxCommandEvent& event);
};