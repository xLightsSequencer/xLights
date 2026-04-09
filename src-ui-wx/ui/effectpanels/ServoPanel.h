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

#include "JsonEffectPanel.h"

class Model;
class wxButton;
class wxCheckBox;
class wxStaticText;
class BulkEditSliderF1;
class BulkEditTextCtrlF1;
class BulkEditValueCurveButton;
class LinkedToggleButton;

class ServoPanel : public JsonEffectPanel {
public:
    ServoPanel(wxWindow* parent, const nlohmann::json& metadata);
    ~ServoPanel() override = default;

    void ValidateWindow() override;
    void SetDefaultParameters() override;

protected:
    wxWindow* CreateCustomControl(wxWindow* parentWin, wxSizer* sizer,
                                   const nlohmann::json& prop, int cols) override;

private:
    wxWindow* BuildStartEndRow(wxWindow* parentWin, wxSizer* sizer);
    wxWindow* BuildButtonRow(wxWindow* parentWin, wxSizer* sizer);

    void OnStartValueUpdated(wxCommandEvent& event);
    void OnEndValueUpdated(wxCommandEvent& event);
    void OnSyncClicked(wxCommandEvent& event);
    void OnEqualClicked(wxCommandEvent& event);
    void OnSwapClicked(wxCommandEvent& event);

    // Custom-built controls (not in properties_ map).
    LinkedToggleButton* _startLinked = nullptr;
    LinkedToggleButton* _endLinked = nullptr;
    wxStaticText* _startLabel = nullptr;
    wxStaticText* _endLabel = nullptr;
    BulkEditSliderF1* _startSlider = nullptr;
    BulkEditSliderF1* _endSlider = nullptr;
    BulkEditTextCtrlF1* _startText = nullptr;
    BulkEditTextCtrlF1* _endText = nullptr;
    BulkEditValueCurveButton* _startVC = nullptr;
    wxCheckBox* _syncCheck = nullptr;
    wxButton* _equalButton = nullptr;
    wxButton* _swapButton = nullptr;
};
