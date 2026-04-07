/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "MediaPickerCtrl.h"
#include "xLightsMain.h"
#include "xLightsApp.h"
#include "ui/media/ManageMediaPanel.h"
#include "render/SequenceElements.h"
#include "ui/shared/utils/wxUtilities.h"

#include <wx/sizer.h>
#include <wx/filepicker.h>
#include <wx/filename.h>
#include <wx/artprov.h>
#include <wx/bmpbuttn.h>
#include <filesystem>

MediaPickerCtrl::MediaPickerCtrl(wxWindow* parent, wxWindowID id,
                                 MediaType mediaType,
                                 const wxString& name)
    : wxPanel(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, name)
    , _mediaType(mediaType)
{
    auto* sizer = new wxBoxSizer(wxHORIZONTAL);

    // Thumbnail preview (small, left side)
    _preview = new wxStaticBitmap(this, wxID_ANY, wxNullBitmap);
    _preview->SetMinSize(wxSize(32, 32));
    _preview->SetMaxSize(wxSize(48, 48));
    sizer->Add(_preview, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);

    // Filename display (read-only)
    _pathCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
                               wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    sizer->Add(_pathCtrl, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);

    // Select button
    _selectButton = new wxButton(this, wxID_ANY, "Select...",
                                 wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    sizer->Add(_selectButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 2);

    // Clear button — bitmap X matching the PicturesPanel style
    wxBitmap clearBmp = wxArtProvider::GetBitmap(wxART_DELETE, wxART_BUTTON);
    _clearButton = new wxBitmapButton(this, wxID_ANY, clearBmp,
                                      wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    _clearButton->SetToolTip("Clear selection");
    sizer->Add(_clearButton, 0, wxALIGN_CENTER_VERTICAL);

    SetSizer(sizer);

    _selectButton->Bind(wxEVT_BUTTON, &MediaPickerCtrl::OnSelectClick, this);
    _clearButton->Bind(wxEVT_BUTTON, &MediaPickerCtrl::OnClearClick, this);
}

void MediaPickerCtrl::SetLinkedPicker(wxFilePickerCtrl* picker) {
    // Unbind from old picker if any
    if (_linkedPicker) {
        _linkedPicker->Unbind(wxEVT_FILEPICKER_CHANGED, &MediaPickerCtrl::OnLinkedPickerChanged, this);
    }
    _linkedPicker = picker;
    if (_linkedPicker) {
        // Listen for changes on the linked picker (triggered by the settings framework)
        _linkedPicker->Bind(wxEVT_FILEPICKER_CHANGED, &MediaPickerCtrl::OnLinkedPickerChanged, this);
        // Sync initial value
        wxString path = _linkedPicker->GetPath();
        if (!path.IsEmpty()) {
            _pathCtrl->SetValue(path);
            UpdatePreview();
        }
    }
}

void MediaPickerCtrl::OnLinkedPickerChanged(wxFileDirPickerEvent& event) {
    // Sync the display when the hidden picker's value changes (e.g., effect selection)
    wxString path = _linkedPicker ? _linkedPicker->GetPath() : wxString();
    if (_pathCtrl->GetValue() != path) {
        _pathCtrl->SetValue(path);
        UpdatePreview();
    }
    event.Skip();
}

wxString MediaPickerCtrl::GetPath() const {
    return _pathCtrl->GetValue();
}

void MediaPickerCtrl::SetPath(const wxString& path) {
    _pathCtrl->SetValue(path);
    UpdatePreview();
}

void MediaPickerCtrl::OnSelectClick(wxCommandEvent& event) {
    xLightsFrame* xl = dynamic_cast<xLightsFrame*>(xLightsApp::GetFrame());
    if (!xl) return;

    SequenceMedia& media = xl->GetSequenceElements().GetSequenceMedia();
    SequenceElements& elements = xl->GetSequenceElements();

    std::string currentPath = _pathCtrl->GetValue().ToStdString();
    SelectMediaDialog dlg(this, &media, &elements,
                          xl->GetShowDirectory(), xl, _mediaType, currentPath);
    if (dlg.ShowModal() != wxID_OK) return;

    std::string selected = dlg.GetSelectedPath();
    if (selected.empty()) return;

    _pathCtrl->SetValue(selected);
    UpdatePreview();

    // Sync to linked file picker (for bulk edit framework compatibility)
    if (_linkedPicker) {
        _linkedPicker->SetPath(selected);
        wxFileDirPickerEvent evt(wxEVT_FILEPICKER_CHANGED, _linkedPicker,
                                 _linkedPicker->GetId(), selected);
        ProcessWindowEvent(evt);
    }
}

void MediaPickerCtrl::OnClearClick(wxCommandEvent& event) {
    _pathCtrl->SetValue(wxEmptyString);
    _preview->SetBitmap(wxNullBitmap);
    _preview->Refresh();

    if (_linkedPicker) {
        _linkedPicker->SetPath(wxEmptyString);
        wxFileDirPickerEvent evt(wxEVT_FILEPICKER_CHANGED, _linkedPicker,
                                 _linkedPicker->GetId(), wxString());
        ProcessWindowEvent(evt);
    }
}

void MediaPickerCtrl::UpdatePreview() {
    wxString path = _pathCtrl->GetValue();
    if (path.IsEmpty()) {
        _preview->SetBitmap(wxNullBitmap);
        _preview->Refresh();
        return;
    }

    xLightsFrame* xl = dynamic_cast<xLightsFrame*>(xLightsApp::GetFrame());
    if (!xl) return;

    SequenceMedia& media = xl->GetSequenceElements().GetSequenceMedia();
    std::string filepath = path.ToStdString();
    double scaleFactor = GetContentScaleFactor();
    int maxPx = (int)(48.0 * scaleFactor);

    std::shared_ptr<xlImage> thumb;

    switch (_mediaType) {
        case MediaType::Image: {
            if (media.HasImage(filepath)) {
                auto entry = media.GetImage(filepath);
                if (entry && entry->IsOk()) {
                    thumb = entry->GetScaledImage(0, maxPx, maxPx, false);
                }
            }
            break;
        }
        case MediaType::SVG: {
            auto entry = media.GetSVG(filepath);
            if (entry && !entry->GetSVGContent().empty()) {
                thumb = entry->GetThumbnail(maxPx, maxPx);
            }
            break;
        }
        case MediaType::Video: {
            auto entry = media.GetVideo(filepath);
            if (entry && !entry->GetResolvedPath().empty()) {
                thumb = entry->GetThumbnail(maxPx, maxPx);
            }
            break;
        }
        // TODO: Shader icon placeholder
        default:
            break;
    }

    if (thumb && thumb->IsOk()) {
        wxBitmap bmp(xlImageToWxImage(*thumb));
        bmp.SetScaleFactor(scaleFactor);
        _preview->SetBitmap(bmp);
    } else {
        _preview->SetBitmap(wxNullBitmap);
    }
    _preview->Refresh();
}
