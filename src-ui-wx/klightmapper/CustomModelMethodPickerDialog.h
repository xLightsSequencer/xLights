#pragma once

// Desktop picker shown from OnMenu_GenerateCustomModel. Lets the user choose
// between the classic GenerateCustomModelDialog flow, a local camera scan
// (Continuity Camera / webcam), and a remote RTSP/IP camera. Plain wx; the
// per-platform klbridge backend supplies the camera data. Used on all desktop
// platforms (macOS / Windows / Linux) — KLightMapper is a required dependency.

#include <wx/dialog.h>
#include <wx/choice.h>
#include <wx/radiobut.h>
#include <wx/textctrl.h>
#include <wx/button.h>

#include <string>
#include <vector>

#include "KLightMapperBridge.h"
#ifdef __WXMSW__
#include "media/LiveCameraCapture.h"
#endif

class CustomModelMethodPickerDialog : public wxDialog {
public:
    // WebcamTouchUp (Windows only): re-detect/drag-correct an existing
    // model's node positions live via a local webcam (#3791), rather than
    // building a brand-new model like the other three choices.
    enum class Choice { Classic, CameraScan, RTSPScan, WebcamTouchUp };

    CustomModelMethodPickerDialog(
        wxWindow* parent,
        const std::vector<klbridge::CameraInfo>& cameras);

    Choice      GetChoice() const           { return choice_; }
    /// Empty when GetChoice() != CameraScan, or when the camera
    /// list was empty.
    std::string GetSelectedCameraID() const { return selectedCameraID_; }
    /// RTSP stream URL / credentials — populated only when
    /// GetChoice() == RTSPScan. Username/password are empty when the
    /// user left those fields blank (or embedded them in the URL).
    std::string GetRTSPURL() const           { return rtspURL_; }
    std::string GetRTSPUsername() const       { return rtspUsername_; }
    std::string GetRTSPPassword() const       { return rtspPassword_; }
    /// Media Foundation device symbolic link, populated only when
    /// GetChoice() == WebcamTouchUp. Empty when no camera was found.
    std::string GetSelectedWebcamSymbolicLink() const { return webcamSymbolicLink_; }

private:
    void OnRadioChanged(wxCommandEvent& event);
    void OnDiscover(wxCommandEvent& event);
    void OnOK(wxCommandEvent& event);

    std::vector<klbridge::CameraInfo> cameras_;
    wxRadioButton* classicRadio_ = nullptr;
    wxRadioButton* cameraRadio_  = nullptr;
    wxChoice*      cameraChoice_ = nullptr;
    wxRadioButton* rtspRadio_    = nullptr;
#ifdef __WXMSW__
    wxRadioButton* webcamTouchUpRadio_ = nullptr;
    wxChoice*      webcamCameraChoice_ = nullptr;
    std::vector<LiveCameraDevice> webcamCameras_;
#endif
    wxChoice*      netCameraChoice_ = nullptr;   // discovered ONVIF cameras
    wxButton*      discoverButton_  = nullptr;
    wxTextCtrl*    rtspURLCtrl_  = nullptr;
    wxTextCtrl*    rtspUserCtrl_ = nullptr;
    wxTextCtrl*    rtspPassCtrl_ = nullptr;
    std::vector<klbridge::NetworkCamera> networkCameras_;

    Choice      choice_           = Choice::Classic;
    std::string selectedCameraID_;
    std::string rtspURL_;
    std::string rtspUsername_;
    std::string rtspPassword_;
    std::string webcamSymbolicLink_;
};
