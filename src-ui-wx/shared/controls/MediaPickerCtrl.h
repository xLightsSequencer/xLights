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

#include <wx/panel.h>
#include <wx/statbmp.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/bmpbuttn.h>
#include <wx/filepicker.h>
#include <optional>
#include <string>

#include "render/SequenceMedia.h"

class wxFilePickerCtrl;

/**
 * MediaPickerCtrl — A media-aware file picker that replaces BulkEditFilePickerCtrl.
 *
 * Shows a read-only text field with the current filename and a "Select..." button
 * that opens a SelectMediaDialog filtered to the appropriate MediaType.
 * Optionally shows a small thumbnail preview for images, SVGs, and videos.
 *
 * Usage in effect panels:
 *   1. Hide the existing BulkEditFilePickerCtrl
 *   2. Create a MediaPickerCtrl in the same sizer position
 *   3. Call SetLinkedPicker() to connect to the hidden file picker
 *   4. When user selects media, the linked picker's value is updated automatically,
 *      which triggers the existing bulk edit framework and event handlers.
 */
class MediaPickerCtrl : public wxPanel
{
public:
    MediaPickerCtrl(wxWindow* parent, wxWindowID id,
                    MediaType mediaType,
                    const wxString& name = wxEmptyString);

    // Get/set the current media path
    wxString GetPath() const;
    void SetPath(const wxString& path);

    // Link to a hidden BulkEditFilePickerCtrl so value changes propagate
    // through the existing bulk edit framework. Also listens for changes
    // on the picker (e.g., when effect settings are loaded by the framework).
    void SetLinkedPicker(wxFilePickerCtrl* picker);

    // Update the thumbnail preview (call when path changes externally)
    void UpdatePreview();

private:
    void OnSelectClick(wxCommandEvent& event);
    void OnClearClick(wxCommandEvent& event);
    void OnLinkedPickerChanged(wxFileDirPickerEvent& event);

    MediaType _mediaType;
    wxTextCtrl* _pathCtrl = nullptr;
    wxButton* _selectButton = nullptr;
    wxBitmapButton* _clearButton = nullptr;
    wxStaticBitmap* _preview = nullptr;
    wxFilePickerCtrl* _linkedPicker = nullptr;
};
