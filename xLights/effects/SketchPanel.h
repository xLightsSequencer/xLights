#pragma once

#include "EffectPanelUtils.h"

class BulkEditCheckBox;
class BulkEditSlider;
class BulkEditTextCtrl;
class BulkEditValueCurveButton;
class SketchAssistPanel;

class wxFilePickerCtrl;
class wxSlider;

class SketchPanel : public xlEffectPanel
{
public:
    SketchPanel(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~SketchPanel() = default;

    void ValidateWindow() override;

    // controls are public to allow SketchEffect access
    BulkEditTextCtrl* TextCtrl_SketchDef = nullptr;
    wxFilePickerCtrl* FilePicker_SketchBackground = nullptr;
    wxSlider* Slider_SketchBackgroundOpacity = nullptr;
    BulkEditSlider* Slider_DrawPercentage = nullptr;
    BulkEditValueCurveButton* BitmapButton_DrawPercentage = nullptr;
    BulkEditTextCtrl* TextCtrl_DrawPercentage = nullptr;
    BulkEditSlider* Slider_Thickness = nullptr;
    BulkEditValueCurveButton* BitmapButton_Thickness = nullptr;
    BulkEditTextCtrl* TextCtrl_Thickness = nullptr;
    BulkEditCheckBox* CheckBox_MotionEnabled = nullptr;
    BulkEditSlider* Slider_MotionPercentage = nullptr;
    BulkEditValueCurveButton* BitmapButton_MotionPercentage = nullptr;
    BulkEditTextCtrl* TextCtrl_MotionPercentage = nullptr;

    static const int DrawPercentageMin = 0;
    static const int DrawPercentageDef = 40;
    static const int DrawPercentageMax = 100;

    static const int ThicknessMin = 1;
    static const int ThicknessDef = 1;
    static const int ThicknessMax = 25;

    static const int MotionPercentageMin = 1;
    static const int MotionPercentageDef = 100;
    static const int MotionPercentageMax = 100;

protected:
    static const long ID_TEXTCTRL_SketchDef;
    static const long ID_FILEPICKER_SketchBackground;
    static const long ID_SLIDER_SketchBackgroundOpacity;
    static const long ID_SLIDER_DrawPercentage;
    static const long ID_TEXTCTRL_DrawPercentage;
    static const long ID_VALUECURVE_DrawPercentage;
    static const long ID_SLIDER_Thickness;
    static const long ID_VALUECURVE_Thickness;
    static const long ID_TEXTCTRL_Thickness;
    static const long ID_CHECKBOX_MotionEnabled;
    static const long ID_SLIDER_MotionPercentage;
    static const long ID_VALUECURVE_MotionPercentage;
    static const long ID_TEXTCTRL_MotionPercentage;

 private:
    DECLARE_EVENT_TABLE()

    void OnFilePickerCtrl_FileChanged(wxCommandEvent& event);
    void OnSlider_BgAlphaChanged(wxCommandEvent& event);
    void OnCheckBox_MotionClick(wxCommandEvent& event);

    void updateSketchAssist(SketchAssistPanel* panel);
};
