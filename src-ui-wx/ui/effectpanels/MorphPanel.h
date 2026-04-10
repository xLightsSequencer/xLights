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

class wxButton;
class wxChoice;
class wxSizer;
class wxTextCtrl;
class BulkEditValueCurveButton;

class MorphPanel : public JsonEffectPanel {
public:
    MorphPanel(wxWindow* parent, const nlohmann::json& metadata);
    ~MorphPanel() override = default;

    bool HasAssistPanel() override { return true; }
    AssistPanel* GetAssistPanel(wxWindow* parent, xLightsFrame* xl_frame) override;

protected:
    wxWindow* CreateCustomControl(wxWindow* parentWin, wxSizer* sizer,
                                   const nlohmann::json& prop, int cols) override;

private:
    wxWindow* BuildQuickSet(wxWindow* parentWin, wxSizer* sizer, int cols);
    wxWindow* BuildSwapButton(wxWindow* parentWin, wxSizer* sizer, int cols);

    void OnQuickSetSelect(wxCommandEvent& event);
    void OnSwapClick(wxCommandEvent& event);

    void CacheCoordControls();
    void SwapTextCtrls(wxTextCtrl* a, wxTextCtrl* b);
    void SwapValueCurves(BulkEditValueCurveButton* a, BulkEditValueCurveButton* b);
    void ApplyStartLimits();

    wxChoice* _quickSetChoice = nullptr;
    wxButton* _swapButton = nullptr;

    // Cached pointers to the framework-built X/Y text controls for the Swap handler.
    wxTextCtrl* _textStartX1 = nullptr;
    wxTextCtrl* _textStartY1 = nullptr;
    wxTextCtrl* _textStartX2 = nullptr;
    wxTextCtrl* _textStartY2 = nullptr;
    wxTextCtrl* _textEndX1 = nullptr;
    wxTextCtrl* _textEndY1 = nullptr;
    wxTextCtrl* _textEndX2 = nullptr;
    wxTextCtrl* _textEndY2 = nullptr;

    // Cached pointers to the framework-built value curve buttons.
    BulkEditValueCurveButton* _vcStartX1 = nullptr;
    BulkEditValueCurveButton* _vcStartY1 = nullptr;
    BulkEditValueCurveButton* _vcStartX2 = nullptr;
    BulkEditValueCurveButton* _vcStartY2 = nullptr;
    BulkEditValueCurveButton* _vcEndX1 = nullptr;
    BulkEditValueCurveButton* _vcEndY1 = nullptr;
    BulkEditValueCurveButton* _vcEndX2 = nullptr;
    BulkEditValueCurveButton* _vcEndY2 = nullptr;
};
