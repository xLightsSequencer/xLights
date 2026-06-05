// macOS-only dialog. Body is wrapped in __APPLE__ because xLights's
// CMake (cmake_vs/) uses GLOB_RECURSE over src-ui-wx for the Windows
// build — without this guard, the file would compile on Windows
// and then link-fail on the klbridge:: symbols that only exist on
// macOS.
#ifdef __APPLE__

#include "CustomModelMethodPickerDialog.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>

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

    auto* buttons = CreateButtonSizer(wxOK | wxCANCEL);
    if (buttons) top->Add(buttons, 0, wxEXPAND | wxALL, 10);

    SetSizerAndFit(top);

    Bind(wxEVT_RADIOBUTTON, &CustomModelMethodPickerDialog::OnRadioChanged, this, classicRadio_->GetId());
    Bind(wxEVT_RADIOBUTTON, &CustomModelMethodPickerDialog::OnRadioChanged, this, cameraRadio_->GetId());
    Bind(wxEVT_BUTTON,      &CustomModelMethodPickerDialog::OnOK,           this, wxID_OK);
}

void CustomModelMethodPickerDialog::OnRadioChanged(wxCommandEvent& /*event*/) {
    cameraChoice_->Enable(cameraRadio_->GetValue());
}

void CustomModelMethodPickerDialog::OnOK(wxCommandEvent& event) {
    if (cameraRadio_->GetValue()) {
        choice_ = Choice::CameraScan;
        int sel = cameraChoice_->GetSelection();
        if (sel != wxNOT_FOUND && sel < static_cast<int>(cameras_.size())) {
            selectedCameraID_ = cameras_[sel].identifier;
        }
    } else {
        choice_ = Choice::Classic;
        selectedCameraID_.clear();
    }
    event.Skip();   // let the default OK handler close the dialog
}

#endif // __APPLE__
