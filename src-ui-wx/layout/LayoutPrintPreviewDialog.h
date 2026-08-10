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

#include <wx/dialog.h>
#include <wx/gdicmn.h>
#include <wx/image.h>

#include <string>
#include <vector>

class wxChoice;
class wxSpinCtrl;
class wxCheckBox;
class wxScrolledWindow;
class wxStaticBitmap;
class wxCommandEvent;
class wxColourPickerEvent;
class wxColourPickerCtrl;
class ModelPreview;
class xLightsFrame;

// Lets the user pick which Preview (or the whole layout) to print, the
// rendered pixel size, which model labels are shown, and whether colors are
// inverted for a print-friendly (white background) result -- with a live
// bitmap preview before sending the job to the printer. Replaces the old
// instant-print "Print Layout Image" behavior.
class LayoutPrintPreviewDialog : public wxDialog
{
public:
    LayoutPrintPreviewDialog(wxWindow* parent, xLightsFrame* xlights, ModelPreview* liveCanvas,
                              const std::string& initialLayoutGroup,
                              int currentPreviewWidth, int currentPreviewHeight);
    virtual ~LayoutPrintPreviewDialog();

private:
    struct ResolutionPreset {
        wxString label;
        int width;
        int height;
    };

    void PopulateGroupChoice(const std::string& initialLayoutGroup);
    std::string GetSelectedGroupName() const;
    int GetPreviewWidth() const;
    int GetPreviewHeight() const;
    void RefreshPreview();
    // Rescales the already-captured full-resolution image (cheap) to fit the
    // scrolled preview area's *current* size and displays it -- called both
    // right after a capture and whenever that area is resized, so dragging
    // the dialog bigger grows the visible preview instead of leaving it a
    // small fixed thumbnail surrounded by empty space.
    void UpdateDisplayBitmap();

    void OnResolutionChoice(wxCommandEvent& event);
    void OnPrint(wxCommandEvent& event);

    xLightsFrame* _xlights = nullptr;
    std::vector<ResolutionPreset> _presets;

    wxChoice* _groupChoice = nullptr;
    wxChoice* _resolutionChoice = nullptr;
    wxSpinCtrl* _widthSpin = nullptr;
    wxSpinCtrl* _heightSpin = nullptr;
    wxCheckBox* _showNamesCheck = nullptr;
    wxCheckBox* _showInfoCheck = nullptr;
    wxSpinCtrl* _labelSizeSpin = nullptr;
    wxColourPickerCtrl* _labelColorPicker = nullptr;
    wxCheckBox* _invertCheck = nullptr;
    wxScrolledWindow* _canvas = nullptr;
    wxStaticBitmap* _bitmapCtrl = nullptr;
    // Full-resolution image from the last capture, kept around so resizing
    // the dialog can rescale the display without re-rendering.
    wxImage _lastCapturedImage;

    // Borrowed from the Layout tab -- NOT created/owned by this dialog. Using
    // the live canvas (rather than a fresh standalone ModelPreview) means the
    // camera framing is whatever the user already has set up on the Layout
    // tab, which by construction already shows everything they care about --
    // a freshly-constructed camera has no such guarantee and requires
    // reverse-engineering a "fit to content" calculation to match. Temporarily
    // repointed at the chosen group/labels/size while this dialog is open;
    // fully restored in the destructor.
    ModelPreview* _previewCanvas = nullptr;

    std::string _originalGroup;
    // Only used to seed this dialog's own checkboxes from the last-used value
    // (persisted in the destructor) -- there's no live Layout tab state for
    // these to restore on close, since its own Show Names/Show Start Channel
    // checkboxes were removed in favor of this dialog's controls.
    bool _originalShowNames = false;
    bool _originalShowInfo = false;
    wxSize _originalSize;
    wxSize _originalMinSize;
    wxSize _originalMaxSize;
};
