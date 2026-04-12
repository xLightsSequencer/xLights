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
class MediaPickerCtrl;
class wxCheckBox;
class wxChoice;
class wxFilePickerCtrl;
class wxSlider;
class wxTextCtrl;

class VUMeterPanel : public JsonEffectPanel {
public:
    VUMeterPanel(wxWindow* parent, const nlohmann::json& metadata);
    ~VUMeterPanel() override = default;

    void ValidateWindow() override;
    void SetPanelStatus(Model* cls) override;

private:
    void RefreshNoteTooltips();
    void EnforceNoteOrder(bool startChanged);

    // Cached pointers to framework-built controls (looked up once after build).
    wxChoice* _typeChoice = nullptr;
    wxChoice* _shapeChoice = nullptr;
    wxChoice* _timingTrackChoice = nullptr;
    wxCheckBox* _logXAxisCheck = nullptr;
    wxCheckBox* _slowDownFalls = nullptr;
    wxCheckBox* _regexCheck = nullptr;
    wxTextCtrl* _filterText = nullptr;
    wxSlider* _barsSlider = nullptr;
    wxTextCtrl* _barsText = nullptr;
    wxSlider* _gainSlider = nullptr;
    wxTextCtrl* _gainText = nullptr;
    wxWindow* _gainVC = nullptr;
    wxSlider* _sensSlider = nullptr;
    wxTextCtrl* _sensText = nullptr;
    wxSlider* _startNoteSlider = nullptr;
    wxTextCtrl* _startNoteText = nullptr;
    wxSlider* _endNoteSlider = nullptr;
    wxTextCtrl* _endNoteText = nullptr;
    wxSlider* _xOffsetSlider = nullptr;
    wxTextCtrl* _xOffsetText = nullptr;
    wxSlider* _yOffsetSlider = nullptr;
    wxTextCtrl* _yOffsetText = nullptr;
    wxWindow* _yOffsetVC = nullptr;

    wxFilePickerCtrl* _hiddenSvgPicker = nullptr;
    MediaPickerCtrl* _svgMediaPicker = nullptr;
    wxChoice* _audioTrackChoice = nullptr;
};
