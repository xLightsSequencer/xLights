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

#include <memory>
#include <vector>
#include <wx/timer.h>

#ifndef __WXOSX__
#include "../graphics/opengl/xlGLCanvas.h"
#endif

class ShaderConfig;
class ShaderMediaCacheEntry;
class SequenceElements;
class xlImage;
class wxBitmapButton;
class wxButton;
class wxFilePickerCtrl;
class wxFlexGridSizer;
class wxSizer;
class wxStaticBitmap;
class wxStaticText;
class BulkEditSliderF2;
class BulkEditTextCtrlF2;
class BulkEditValueCurveButton;

class ShaderPanel : public JsonEffectPanel {
public:
    ShaderPanel(wxWindow* parent, const nlohmann::json& metadata);
    ~ShaderPanel() override;

    void ValidateWindow() override;
    void SetDefaultParameters() override;

#ifndef __WXOSX__
    xlGLCanvas* GetPreview() const { return _preview; }
#endif

protected:
    wxWindow* CreateCustomControl(wxWindow* parentWin, wxSizer* sizer,
                                   const nlohmann::json& prop, int cols) override;

private:
    // Custom block builders
    wxWindow* BuildFilenameBlock(wxWindow* parentWin, wxSizer* sizer, int cols);
    wxWindow* BuildSpeedRow(wxWindow* parentWin, wxSizer* sizer, int cols);
    wxWindow* BuildDynamicParams(wxWindow* parentWin, wxSizer* sizer, int cols);

    // Handlers
    void OnSelectClicked(wxCommandEvent& event);
    void OnClearClicked(wxCommandEvent& event);
    void OnDownloadClicked(wxCommandEvent& event);
    void OnFilePickerChanged(wxFileDirPickerEvent& event);
    void OnPreviewTimer(wxTimerEvent& event);
    void OnShowPanel(wxShowEvent& event);

    // Internal helpers
    void ApplyShaderConfig(bool resetParams);
    void BuildDynamicUI();
    void ClearDynamicUI();
    void ShowHideStaticControls(bool hasTime, bool hasCoord);
    void UpdatePreview();
    void ShowPreviewFrame(size_t index);

    // Custom-built controls
    wxFilePickerCtrl* _hiddenFilePicker = nullptr;
    wxButton* _selectButton = nullptr;
    wxBitmapButton* _clearButton = nullptr;
    wxButton* _downloadButton = nullptr;
    wxStaticBitmap* _previewBitmap = nullptr;
    wxStaticText* _filenameLabel = nullptr;

    // Shader_Speed custom row (SLIDER-primary F2 with buddy text + VC)
    BulkEditSliderF2* _speedSlider = nullptr;
    BulkEditTextCtrlF2* _speedText = nullptr;
    BulkEditValueCurveButton* _speedVC = nullptr;
    wxStaticText* _speedLabel = nullptr;

    // Dynamic uniform container
    wxFlexGridSizer* _dynamicSizer = nullptr;

    // Hold a ref to keep the ShaderMediaCacheEntry alive while we reference its _shaderConfig
    std::shared_ptr<ShaderMediaCacheEntry> _shaderCacheEntry;
    ShaderConfig* _shaderConfig = nullptr;

    // Animated preview
    wxTimer _previewTimer;
    std::vector<std::shared_ptr<xlImage>> _previewFrames;
    std::vector<long> _previewFrameTimes;
    size_t _currentPreviewFrame = 0;

#ifndef __WXOSX__
    xlGLCanvas* _preview = nullptr;
    static const long ID_CANVAS;
#endif
};
