// Method picker shown from OnMenu_GenerateCustomModel. Plain wx; the klbridge::
// backend is per-platform (KLightMapperBridge.mm on macOS,
// KLightMapperBridge_win.cpp on Windows + Linux). KLightMapper is a required,
// auto-fetched dependency on every desktop, so this compiles unconditionally —
// it's a desktop-only translation unit (not built for iPad).

#include "CustomModelMethodPickerDialog.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/msgdlg.h>
#include <wx/utils.h>   // wxBusyCursor

CustomModelMethodPickerDialog::CustomModelMethodPickerDialog(
    wxWindow* parent,
    const std::vector<klbridge::CameraInfo>& cameras)
    : wxDialog(parent, wxID_ANY, _("Generate Custom Model")),
      cameras_(cameras) {

    auto* top = new wxBoxSizer(wxVERTICAL);

    top->Add(new wxStaticText(this, wxID_ANY,
        _("How do you want to build this model?")),
        0, wxALL, 10);

    classicRadio_ = new wxRadioButton(this, wxID_ANY,
        _("Classic flow (record video, decode manually)"),
        wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    cameraRadio_  = new wxRadioButton(this, wxID_ANY,
        _("Camera scan (KLightMapper, automatic)"));
    classicRadio_->SetValue(true);

    top->Add(classicRadio_, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);
    top->Add(cameraRadio_,  0, wxLEFT | wxRIGHT | wxBOTTOM, 5);

    // Camera dropdown, indented under the camera-scan radio so the
    // visual association is obvious.
    auto* cameraRow = new wxBoxSizer(wxHORIZONTAL);
    cameraRow->AddSpacer(20);
    cameraRow->Add(new wxStaticText(this, wxID_ANY, _("Camera:")),
                   0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
    wxArrayString cameraLabels;
    cameraLabels.reserve(cameras_.size());
    for (const auto& c : cameras_) {
        cameraLabels.Add(wxString::FromUTF8(c.localizedName));
    }
    cameraChoice_ = new wxChoice(this, wxID_ANY,
                                  wxDefaultPosition, wxDefaultSize,
                                  cameraLabels);
    if (!cameras_.empty()) cameraChoice_->SetSelection(0);
    cameraChoice_->Enable(false);  // matches the initial Classic selection
    cameraRow->Add(cameraChoice_, 1, wxEXPAND);
    top->Add(cameraRow, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // No local camera paired → grey out the camera-scan option, but
    // leave Remote RTSP available (that's the whole point of it: a Mac
    // with no usable built-in / Continuity camera).
    if (cameras_.empty()) {
        cameraRadio_->Enable(false);
        cameraChoice_->Enable(false);
    }

    // Remote RTSP / IP camera option. Lets a Mac scan from a networked
    // camera (FFmpeg-free decode in KLightMapper). The operator must
    // have fixed the camera's exposure beforehand — Apple can't control
    // a remote camera's AE (see docs/rtsp-remote-camera-plan.md).
    rtspRadio_ = new wxRadioButton(this, wxID_ANY,
        _("Remote camera (RTSP / IP camera)"));
    top->Add(rtspRadio_, 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);

    auto* rtspGrid = new wxFlexGridSizer(0, 2, 5, 8);
    rtspGrid->AddGrowableCol(1, 1);

    // Discovered ONVIF cameras (populated by the Discover button). The
    // dropdown's first entry means "use the manual URL below"; picking a
    // discovered camera auto-selects its highest-resolution stream on OK.
    rtspGrid->Add(new wxStaticText(this, wxID_ANY, _("Discovered:")),
                  0, wxALIGN_CENTER_VERTICAL);
    auto* discRow = new wxBoxSizer(wxHORIZONTAL);
    netCameraChoice_ = new wxChoice(this, wxID_ANY);
    netCameraChoice_->Append(_("(enter URL manually below)"));
    netCameraChoice_->SetSelection(0);
    discRow->Add(netCameraChoice_, 1, wxEXPAND | wxRIGHT, 6);
    discoverButton_ = new wxButton(this, wxID_ANY, _("Discover"));
    discRow->Add(discoverButton_, 0);
    rtspGrid->Add(discRow, 1, wxEXPAND);

    rtspGrid->Add(new wxStaticText(this, wxID_ANY, _("or Stream URL:")),
                  0, wxALIGN_CENTER_VERTICAL);
    rtspURLCtrl_ = new wxTextCtrl(this, wxID_ANY, "rtsp://");
    rtspURLCtrl_->SetMinSize(wxSize(340, -1));
    rtspGrid->Add(rtspURLCtrl_, 1, wxEXPAND);
    rtspGrid->Add(new wxStaticText(this, wxID_ANY, _("Username:")),
                  0, wxALIGN_CENTER_VERTICAL);
    rtspUserCtrl_ = new wxTextCtrl(this, wxID_ANY, "");
    rtspGrid->Add(rtspUserCtrl_, 1, wxEXPAND);
    rtspGrid->Add(new wxStaticText(this, wxID_ANY, _("Password:")),
                  0, wxALIGN_CENTER_VERTICAL);
    rtspPassCtrl_ = new wxTextCtrl(this, wxID_ANY, "",
                                   wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    rtspGrid->Add(rtspPassCtrl_, 1, wxEXPAND);

    auto* rtspRow = new wxBoxSizer(wxHORIZONTAL);
    rtspRow->AddSpacer(20);
    rtspRow->Add(rtspGrid, 1, wxEXPAND);
    top->Add(rtspRow, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    netCameraChoice_->Enable(false);   // matches the initial Classic selection
    discoverButton_->Enable(false);
    rtspURLCtrl_->Enable(false);
    rtspUserCtrl_->Enable(false);
    rtspPassCtrl_->Enable(false);

    auto* buttons = CreateButtonSizer(wxOK | wxCANCEL);
    if (buttons) top->Add(buttons, 0, wxEXPAND | wxALL, 10);

    SetSizerAndFit(top);

    Bind(wxEVT_RADIOBUTTON, &CustomModelMethodPickerDialog::OnRadioChanged, this, classicRadio_->GetId());
    Bind(wxEVT_RADIOBUTTON, &CustomModelMethodPickerDialog::OnRadioChanged, this, cameraRadio_->GetId());
    Bind(wxEVT_RADIOBUTTON, &CustomModelMethodPickerDialog::OnRadioChanged, this, rtspRadio_->GetId());
    Bind(wxEVT_BUTTON,      &CustomModelMethodPickerDialog::OnDiscover,     this, discoverButton_->GetId());
    Bind(wxEVT_BUTTON,      &CustomModelMethodPickerDialog::OnOK,           this, wxID_OK);
}

void CustomModelMethodPickerDialog::OnRadioChanged(wxCommandEvent& /*event*/) {
    cameraChoice_->Enable(cameraRadio_->GetValue());
    const bool rtsp = rtspRadio_->GetValue();
    netCameraChoice_->Enable(rtsp);
    discoverButton_->Enable(rtsp);
    rtspURLCtrl_->Enable(rtsp);
    rtspUserCtrl_->Enable(rtsp);
    rtspPassCtrl_->Enable(rtsp);
}

// Discover ONVIF cameras on the LAN and repopulate the dropdown. Blocks
// briefly (WS-Discovery wait) — wxBusyCursor covers it.
void CustomModelMethodPickerDialog::OnDiscover(wxCommandEvent& /*event*/) {
    wxBusyCursor busy;
    networkCameras_ = klbridge::DiscoverNetworkCameras(2500);
    netCameraChoice_->Clear();
    netCameraChoice_->Append(_("(enter URL manually below)"));
    for (const auto& c : networkCameras_) {
        netCameraChoice_->Append(wxString::FromUTF8(c.name + " (" + c.host + ")"));
    }
    if (!networkCameras_.empty()) {
        netCameraChoice_->SetSelection(1);   // first discovered camera
    } else {
        netCameraChoice_->SetSelection(0);
        wxMessageBox(_("No ONVIF cameras were found on the network.\n"
                       "You can still enter a stream URL manually."),
                     _("Discover Cameras"), wxOK | wxICON_INFORMATION, this);
    }
    Layout();
}

void CustomModelMethodPickerDialog::OnOK(wxCommandEvent& event) {
    if (cameraRadio_->GetValue()) {
        choice_ = Choice::CameraScan;
        int sel = cameraChoice_->GetSelection();
        if (sel != wxNOT_FOUND && sel < static_cast<int>(cameras_.size())) {
            selectedCameraID_ = cameras_[sel].identifier;
        }
    } else if (rtspRadio_->GetValue()) {
        choice_ = Choice::RTSPScan;
        rtspUsername_ = rtspUserCtrl_->GetValue().ToStdString(wxConvUTF8);
        rtspPassword_ = rtspPassCtrl_->GetValue().ToStdString(wxConvUTF8);
        const int sel = netCameraChoice_->GetSelection();
        if (sel >= 1 && sel - 1 < static_cast<int>(networkCameras_.size())) {
            // Discovered camera → enumerate its streams and auto-pick the
            // highest resolution (profiles come back largest-first).
            wxBusyCursor busy;
            const auto& cam = networkCameras_[sel - 1];
            const auto profiles = klbridge::EnumerateCameraProfiles(
                cam.deviceService, rtspUsername_, rtspPassword_);
            if (profiles.empty()) {
                wxMessageBox(_("Could not read the camera's streams.\n"
                               "Check the username / password."),
                             _("Remote Camera"), wxOK | wxICON_ERROR, this);
                return;   // keep the dialog open (do not Skip)
            }
            rtspURL_ = profiles.front().rtspUrl;
        } else {
            rtspURL_ = rtspURLCtrl_->GetValue().ToStdString(wxConvUTF8);
        }
    } else {
        choice_ = Choice::Classic;
        selectedCameraID_.clear();
    }
    event.Skip();   // let the default OK handler close the dialog
}
