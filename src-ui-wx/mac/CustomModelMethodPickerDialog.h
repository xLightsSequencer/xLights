#pragma once

// macOS-only picker shown from OnMenu_GenerateCustomModel when a
// Continuity Camera is available. Lets the user choose between the
// classic GenerateCustomModelDialog flow and the new KLightMapper
// camera-scan flow (with a per-camera selection when more than one
// Continuity device is paired).
//
// The dialog itself is plain wx — the platform gating is done at
// the call site (only the macOS branch invokes it). The .cpp lives
// under src-ui-wx/mac/ so it's not picked up by the Linux/Windows
// build file lists.

#include <wx/dialog.h>
#include <wx/choice.h>
#include <wx/radiobut.h>

#include <string>
#include <vector>

#include "mac/KLightMapperBridge.h"

class CustomModelMethodPickerDialog : public wxDialog {
public:
    enum class Choice { Classic, CameraScan };

    CustomModelMethodPickerDialog(
        wxWindow* parent,
        const std::vector<klbridge::CameraInfo>& cameras);

    Choice      GetChoice() const           { return choice_; }
    /// Empty when GetChoice() != CameraScan, or when the camera
    /// list was empty.
    std::string GetSelectedCameraID() const { return selectedCameraID_; }

private:
    void OnRadioChanged(wxCommandEvent& event);
    void OnOK(wxCommandEvent& event);

    std::vector<klbridge::CameraInfo> cameras_;
    wxRadioButton* classicRadio_ = nullptr;
    wxRadioButton* cameraRadio_  = nullptr;
    wxChoice*      cameraChoice_ = nullptr;

    Choice      choice_           = Choice::Classic;
    std::string selectedCameraID_;
};
