/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "LayoutPrintPreviewDialog.h"

#include <algorithm>

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/clrpicker.h>
#include <wx/event.h>
#include <wx/print.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/statbmp.h>
#include <wx/statline.h>
#include <wx/stattext.h>

#include "layout/ModelPreview.h"
#include "settings/XLightsConfigAdapter.h"
#include "shared/utils/wxUtilities.h"
#include "xLightsMain.h"

#include <log.h>

namespace
{
    constexpr int CUSTOM_RESOLUTION_INDEX_SENTINEL = -1;
    const char* const LAST_WIDTH_KEY = "LayoutPrintPreviewLastWidth";
    const char* const LAST_HEIGHT_KEY = "LayoutPrintPreviewLastHeight";
    const char* const LABEL_SIZE_KEY = "LayoutPrintPreviewLabelSize";
    const char* const LABEL_COLOR_KEY = "LayoutPrintPreviewLabelColor";
    // Reuses the keys the Layout tab's own Show Names/Show Start Channel
    // checkboxes used to write before they were removed in favor of this
    // dialog's controls -- this dialog is now their sole owner.
    const char* const SHOW_NAMES_KEY = "LayoutShowNames";
    const char* const SHOW_INFO_KEY = "LayoutShowStartChannel";
    constexpr int DEFAULT_LABEL_FONT_SIZE = 14; // matches ModelPreview's own default

    // The invert-for-printing step operates on the whole captured bitmap
    // (models, background, and label text together) after rendering, so a
    // label color chosen as-is would come out flipped in the final image
    // along with everything else. Pre-invert the color we hand to the
    // renderer when invert is on, so it lands back on the color the user
    // actually picked once the whole-image invert runs.
    xlColor InvertedXlColor(const xlColor& c)
    {
        return xlColor(255 - c.red, 255 - c.green, 255 - c.blue, c.alpha);
    }

    wxRect scaledRect(int srcWidth, int srcHeight, int dstWidth, int dstHeight)
    {
        wxRect r;
        float srcAspectRatio = float(srcWidth) / srcHeight;
        float dstAspectRatio = float(dstWidth) / dstHeight;

        if (srcAspectRatio > dstAspectRatio) {
            r.SetWidth(dstWidth);
            r.SetHeight(int(dstWidth / srcAspectRatio));
            r.SetTopLeft(wxPoint(0, (dstHeight - r.GetHeight()) / 2));
        } else {
            r.SetHeight(dstHeight);
            r.SetWidth(int(dstHeight * srcAspectRatio));
            r.SetTopLeft(wxPoint((dstWidth - r.GetWidth()) / 2, 0));
        }
        return r;
    }

    // Crops away uniform-background margin around the actual content, in
    // place. Reusing the live canvas means the capture includes whatever
    // pan/zoom margin the user happens to have set up for comfortable
    // editing -- this trims that back down to just the layout. Background
    // color is sampled from the top-left corner pixel (assumed empty); a
    // small tolerance absorbs anti-aliasing noise along edges. No-ops if the
    // whole image is uniform (nothing to trim) or the sampled corner isn't
    // actually background (unusual layouts where content reaches a corner).
    void TrimImageToContent(wxImage& img)
    {
        if (!img.IsOk()) {
            return;
        }
        const int w = img.GetWidth();
        const int h = img.GetHeight();
        if (w < 8 || h < 8) {
            return;
        }

        unsigned char* data = img.GetData();
        const unsigned char bgR = data[0], bgG = data[1], bgB = data[2];
        constexpr int THRESHOLD = 12;
        auto differs = [&](int x, int y) {
            const unsigned char* p = data + (size_t(y) * w + x) * 3;
            return std::abs(p[0] - bgR) > THRESHOLD || std::abs(p[1] - bgG) > THRESHOLD || std::abs(p[2] - bgB) > THRESHOLD;
        };

        int top = -1, bottom = -1, left = -1, right = -1;
        for (int y = 0; y < h && top < 0; ++y) {
            for (int x = 0; x < w; ++x) {
                if (differs(x, y)) { top = y; break; }
            }
        }
        if (top < 0) {
            return; // uniform image, nothing to trim
        }
        for (int y = h - 1; y >= 0 && bottom < 0; --y) {
            for (int x = 0; x < w; ++x) {
                if (differs(x, y)) { bottom = y; break; }
            }
        }
        for (int x = 0; x < w && left < 0; ++x) {
            for (int y = 0; y < h; ++y) {
                if (differs(x, y)) { left = x; break; }
            }
        }
        for (int x = w - 1; x >= 0 && right < 0; --x) {
            for (int y = 0; y < h; ++y) {
                if (differs(x, y)) { right = x; break; }
            }
        }

        // Padding so trimmed content isn't flush against the new edge.
        const int padX = std::max(4, (right - left) / 40);
        const int padY = std::max(4, (bottom - top) / 40);
        left = std::max(0, left - padX);
        top = std::max(0, top - padY);
        right = std::min(w - 1, right + padX);
        bottom = std::min(h - 1, bottom + padY);

        const int newW = right - left + 1;
        const int newH = bottom - top + 1;
        if (newW < w || newH < h) {
            img = img.GetSubImage(wxRect(left, top, newW, newH));
        }
    }

    // GrabImage()'s own size-scaling only reliably renders at a size that
    // differs from the canvas's actual current widget size once that size has
    // taken effect -- the render viewport is driven off the widget's own
    // mWindowWidth/mWindowHeight, not the requested FBO size. Grow the canvas
    // to the exact target size, grab, then shrink it back to restoreSize. Same
    // workaround xLightsFrame::ExportVideoPreview uses for the House Preview
    // pane (issue #3763); also pins min/max size during the resize, matching
    // that function's defensive comment: a stray Layout() pass during the
    // wxYield() below (e.g. from an unrelated event) would otherwise snap the
    // canvas straight back to its normal sizer-driven size mid-capture.
    wxImage* GrabPreviewImage(ModelPreview* canvas, const wxSize& captureSize, const wxSize& restoreSize)
    {
        if (canvas == nullptr || captureSize.GetWidth() <= 0 || captureSize.GetHeight() <= 0) {
            return nullptr;
        }
        wxSize originalMin = canvas->GetMinSize();
        wxSize originalMax = canvas->GetMaxSize();
        canvas->SetMinSize(captureSize);
        canvas->SetMaxSize(captureSize);
        canvas->SetSize(captureSize);
        wxYield();
        wxImage* img = canvas->GrabImage(wxSize(canvas->getWidth(), canvas->getHeight()));
        canvas->SetMinSize(originalMin);
        canvas->SetMaxSize(originalMax);
        canvas->SetSize(restoreSize);
        if (img != nullptr) {
            TrimImageToContent(*img);
        }
        return img;
    }

    // Reused by both the live preview refresh and the real print job.
    class Printout : public wxPrintout
    {
    public:
        Printout(ModelPreview* canvas, const wxSize& restoreSize, bool invert) :
            m_canvas(canvas), _restoreSize(restoreSize), _invert(invert) {}
        virtual ~Printout() { clearImage(); }

        void clearImage()
        {
            if (m_image != nullptr) {
                delete m_image;
                m_image = nullptr;
            }
        }

        virtual bool GrabImage()
        {
            clearImage();

            wxRect rect = GetLogicalPageRect();
            rect.Deflate(rect.GetWidth() / 20, rect.GetHeight() / 20);
            wxRect adjustedRect = scaledRect(m_canvas->getWidth(), m_canvas->getHeight(), rect.GetWidth(), rect.GetHeight());

            m_image = GrabPreviewImage(m_canvas, wxSize(adjustedRect.GetWidth(), adjustedRect.GetHeight()), _restoreSize);

            if (_invert && m_image != nullptr) {
                InvertImageRGB(*m_image);
            }

            m_grabbedImage = (m_image != nullptr);
            return m_grabbedImage;
        }

        virtual bool OnPrintPage(int page) override
        {
            if (GrabImage() == false)
                return false;

            wxDC* dc = GetDC();
            wxRect rect = GetLogicalPageRect();

            wxRect r = scaledRect(m_image->GetWidth(), m_image->GetHeight(), rect.GetWidth(), rect.GetHeight());
            wxAffineMatrix2D mtx;
            double xScale = r.GetWidth() / double(m_image->GetWidth());
            double yScale = r.GetHeight() / double(m_image->GetHeight());
            mtx.Scale(xScale, yScale);
            dc->SetTransformMatrix(mtx);
            dc->DrawBitmap(*m_image, rect.GetTopLeft());
            return true;
        }

        bool grabbedImage() const { return m_grabbedImage; }

    protected:
        ModelPreview* m_canvas = nullptr;
        wxSize _restoreSize;
        wxImage* m_image = nullptr;
        bool m_grabbedImage = false;
        bool _invert = false;
    };
}

LayoutPrintPreviewDialog::LayoutPrintPreviewDialog(wxWindow* parent, xLightsFrame* xlights, ModelPreview* liveCanvas,
                                                     const std::string& initialLayoutGroup,
                                                     int currentPreviewWidth, int currentPreviewHeight) :
    wxDialog(parent, wxID_ANY, _("Print Layout"), wxDefaultPosition, wxSize(900, 760), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX),
    _xlights(xlights),
    _previewCanvas(liveCanvas)
{
    // Cheap insurance against any in-flight model-list reload not yet
    // reflected in xlights->PreviewModels/AllModels -- matches the pattern in
    // xLightsFrame::ExportVideoPreview ("Ensure all pending work is done
    // before we do anything"). Only needed once, not on every refresh.
    xlights->DoAllWork();

    _presets.push_back({ wxString::Format(_("Current Layout Tab Size (%d x %d)"), currentPreviewWidth, currentPreviewHeight), currentPreviewWidth, currentPreviewHeight });
    _presets.push_back({ "1280 x 720 (HD)", 1280, 720 });
    _presets.push_back({ "1920 x 1080 (Full HD)", 1920, 1080 });
    _presets.push_back({ "2560 x 1440 (QHD)", 2560, 1440 });
    _presets.push_back({ "3840 x 2160 (4K UHD)", 3840, 2160 });
    _presets.push_back({ _("Custom"), CUSTOM_RESOLUTION_INDEX_SENTINEL, CUSTOM_RESOLUTION_INDEX_SENTINEL });

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    wxFlexGridSizer* grid = new wxFlexGridSizer(0, 2, 5, 10);
    grid->AddGrowableCol(1);

    grid->Add(new wxStaticText(this, wxID_ANY, _("Preview:")), 0, wxALIGN_CENTER_VERTICAL);
    _groupChoice = new wxChoice(this, wxID_ANY);
    PopulateGroupChoice(initialLayoutGroup);
    _groupChoice->SetToolTip(_("Choose a specific Preview to print, or \"All Models\" for the whole layout."));
    grid->Add(_groupChoice, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, _("Resolution:")), 0, wxALIGN_CENTER_VERTICAL);
    _resolutionChoice = new wxChoice(this, wxID_ANY);
    for (auto const& preset : _presets) {
        _resolutionChoice->Append(preset.label);
    }
    grid->Add(_resolutionChoice, 1, wxEXPAND);

    wxSize spinSize = wxSize(FromDIP(80), -1);

    grid->Add(new wxStaticText(this, wxID_ANY, _("Width:")), 0, wxALIGN_CENTER_VERTICAL);
    _widthSpin = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, spinSize, wxSP_ARROW_KEYS, 16, 7680, currentPreviewWidth);
    grid->Add(_widthSpin, 0, wxALIGN_LEFT);

    grid->Add(new wxStaticText(this, wxID_ANY, _("Height:")), 0, wxALIGN_CENTER_VERTICAL);
    _heightSpin = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, spinSize, wxSP_ARROW_KEYS, 16, 4320, currentPreviewHeight);
    grid->Add(_heightSpin, 0, wxALIGN_LEFT);

    mainSizer->Add(grid, 0, wxALL | wxEXPAND, 10);

    auto* config = GetXLightsConfig();
    _originalShowNames = config->ReadBool(SHOW_NAMES_KEY, false);
    _originalShowInfo = config->ReadBool(SHOW_INFO_KEY, false);
    _originalGroup = initialLayoutGroup;
    _originalSize = _previewCanvas->GetSize();
    _originalMinSize = _previewCanvas->GetMinSize();
    _originalMaxSize = _previewCanvas->GetMaxSize();

    int lastWidth = (int)config->Read(LAST_WIDTH_KEY, -1L);
    int lastHeight = (int)config->Read(LAST_HEIGHT_KEY, -1L);

    int initialSelection = 0;
    int initialWidth = currentPreviewWidth;
    int initialHeight = currentPreviewHeight;
    if (lastWidth > 0 && lastHeight > 0) {
        initialWidth = lastWidth;
        initialHeight = lastHeight;
        bool matchedPreset = false;
        for (size_t i = 0; i < _presets.size(); ++i) {
            if (_presets[i].width == lastWidth && _presets[i].height == lastHeight) {
                initialSelection = (int)i;
                matchedPreset = true;
                break;
            }
        }
        if (!matchedPreset) {
            initialSelection = (int)_presets.size() - 1;
        }
    }
    _resolutionChoice->SetSelection(initialSelection);
    _widthSpin->SetValue(initialWidth);
    _heightSpin->SetValue(initialHeight);
    bool isCustom = _presets[initialSelection].width == CUSTOM_RESOLUTION_INDEX_SENTINEL;
    _widthSpin->Enable(isCustom);
    _heightSpin->Enable(isCustom);

    wxBoxSizer* checkSizer = new wxBoxSizer(wxHORIZONTAL);
    _showNamesCheck = new wxCheckBox(this, wxID_ANY, _("Show Names"));
    _showNamesCheck->SetValue(_originalShowNames);
    checkSizer->Add(_showNamesCheck, 0, wxALIGN_CENTER_VERTICAL | wxALL, 6);

    _showInfoCheck = new wxCheckBox(this, wxID_ANY, _("Show Start Channel"));
    _showInfoCheck->SetValue(_originalShowInfo);
    checkSizer->Add(_showInfoCheck, 0, wxALIGN_CENTER_VERTICAL | wxALL, 6);

    checkSizer->Add(new wxStaticText(this, wxID_ANY, _("Label Size:")), 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxTOP | wxBOTTOM, 6);
    _labelSizeSpin = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(FromDIP(60), -1), wxSP_ARROW_KEYS, 6, 200,
                                     (int)config->Read(LABEL_SIZE_KEY, (long)DEFAULT_LABEL_FONT_SIZE));
    checkSizer->Add(_labelSizeSpin, 0, wxALIGN_CENTER_VERTICAL | wxALL, 6);

    checkSizer->Add(new wxStaticText(this, wxID_ANY, _("Label Color:")), 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxTOP | wxBOTTOM, 6);
    wxString colorStr = wxWHITE->GetAsString(wxC2S_HTML_SYNTAX);
    config->Read(LABEL_COLOR_KEY, &colorStr, colorStr);
    wxColour initialLabelColor;
    initialLabelColor.Set(colorStr);
    _labelColorPicker = new wxColourPickerCtrl(this, wxID_ANY, initialLabelColor);
    checkSizer->Add(_labelColorPicker, 0, wxALIGN_CENTER_VERTICAL | wxALL, 6);

    _invertCheck = new wxCheckBox(this, wxID_ANY, _("Invert Colors For Printing"));
    _invertCheck->SetValue(true);
    checkSizer->Add(_invertCheck, 0, wxALIGN_CENTER_VERTICAL | wxALL, 6);
    mainSizer->Add(checkSizer, 0, wxLEFT | wxRIGHT, 4);

    mainSizer->Add(new wxStaticText(this, wxID_ANY,
                                     _("Resolution controls the preview render only -- the printed page still fits\n"
                                       "to whatever paper size/orientation you choose in the print dialog.")),
                    0, wxALL, 10);

    _canvas = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
    _canvas->SetBackgroundColour(*wxLIGHT_GREY);
    _bitmapCtrl = new wxStaticBitmap(_canvas, wxID_ANY, wxBitmap());
    wxBoxSizer* canvasSizer = new wxBoxSizer(wxVERTICAL);
    canvasSizer->Add(_bitmapCtrl, 0, wxALL, 10);
    _canvas->SetSizer(canvasSizer);
    _canvas->SetScrollRate(20, 20);
    mainSizer->Add(_canvas, 1, wxEXPAND | wxALL, 2);

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonSizer->AddStretchSpacer();
    wxButton* printButton = new wxButton(this, wxID_ANY, _("Print..."));
    buttonSizer->Add(printButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 6);
    wxButton* closeButton = new wxButton(this, wxID_CANCEL, _("Close"));
    buttonSizer->Add(closeButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 6);
    mainSizer->Add(buttonSizer, 0, wxEXPAND);

    SetSizer(mainSizer);
    CentreOnParent();

    _groupChoice->Bind(wxEVT_CHOICE, [this](wxCommandEvent&) { RefreshPreview(); });
    _resolutionChoice->Bind(wxEVT_CHOICE, &LayoutPrintPreviewDialog::OnResolutionChoice, this);
    _widthSpin->Bind(wxEVT_SPINCTRL, [this](wxSpinEvent&) { RefreshPreview(); });
    _widthSpin->Bind(wxEVT_TEXT, [this](wxCommandEvent&) { RefreshPreview(); });
    _heightSpin->Bind(wxEVT_SPINCTRL, [this](wxSpinEvent&) { RefreshPreview(); });
    _heightSpin->Bind(wxEVT_TEXT, [this](wxCommandEvent&) { RefreshPreview(); });
    _showNamesCheck->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent&) { RefreshPreview(); });
    _showInfoCheck->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent&) { RefreshPreview(); });
    _labelSizeSpin->Bind(wxEVT_SPINCTRL, [this](wxSpinEvent&) { RefreshPreview(); });
    _labelSizeSpin->Bind(wxEVT_TEXT, [this](wxCommandEvent&) { RefreshPreview(); });
    _labelColorPicker->Bind(wxEVT_COLOURPICKER_CHANGED, [this](wxColourPickerEvent&) { RefreshPreview(); });
    _invertCheck->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent&) { RefreshPreview(); });
    printButton->Bind(wxEVT_BUTTON, &LayoutPrintPreviewDialog::OnPrint, this);
    _canvas->Bind(wxEVT_SIZE, [this](wxSizeEvent& event) { event.Skip(); UpdateDisplayBitmap(); });
    _xlights->Bind(wxEVT_ACTIVATE, &LayoutPrintPreviewDialog::OnOwnerActivate, this);

    RefreshPreview();
}

LayoutPrintPreviewDialog::~LayoutPrintPreviewDialog()
{
    _xlights->Unbind(wxEVT_ACTIVATE, &LayoutPrintPreviewDialog::OnOwnerActivate, this);

    auto* config = GetXLightsConfig();
    config->Write(LAST_WIDTH_KEY, GetPreviewWidth());
    config->Write(LAST_HEIGHT_KEY, GetPreviewHeight());
    config->Write(LABEL_SIZE_KEY, (long)_labelSizeSpin->GetValue());
    config->Write(LABEL_COLOR_KEY, _labelColorPicker->GetColour().GetAsString(wxC2S_HTML_SYNTAX));
    config->Write(SHOW_NAMES_KEY, (bool)_showNamesCheck->GetValue());
    config->Write(SHOW_INFO_KEY, (bool)_showInfoCheck->GetValue());

    if (_previewCanvas != nullptr) {
        _previewCanvas->SetActiveLayoutGroup(_originalGroup);
        // No live-tab state to restore any of these four to anymore -- the
        // Layout tab's own Show Names/Show Start Channel checkboxes were
        // removed in favor of this dialog's controls, so nothing else sets
        // them on the shared canvas. "Restore" is just putting it back to the
        // defaults it always had before any of this existed.
        _previewCanvas->SetShowModelNames(false);
        _previewCanvas->SetShowModelInfo(false);
        _previewCanvas->SetLabelFontSize(DEFAULT_LABEL_FONT_SIZE);
        _previewCanvas->SetLabelColor(xlWHITE);
        _previewCanvas->SetMinSize(_originalMinSize);
        _previewCanvas->SetMaxSize(_originalMaxSize);
        _previewCanvas->SetSize(_originalSize);
        _previewCanvas->Refresh();
    }
}

void LayoutPrintPreviewDialog::PopulateGroupChoice(const std::string& initialLayoutGroup)
{
    _groupChoice->Append("Default");
    _groupChoice->Append("All Models");
    _groupChoice->Append("Unassigned");
    for (const auto& [name, grp] : _xlights->LayoutGroups) {
        _groupChoice->Append(name);
    }
    int sel = _groupChoice->FindString(initialLayoutGroup);
    _groupChoice->SetSelection(sel >= 0 ? sel : 1 /* All Models */);
}

std::string LayoutPrintPreviewDialog::GetSelectedGroupName() const
{
    return _groupChoice->GetStringSelection().ToStdString();
}

int LayoutPrintPreviewDialog::GetPreviewWidth() const
{
    return _widthSpin->GetValue();
}

int LayoutPrintPreviewDialog::GetPreviewHeight() const
{
    return _heightSpin->GetValue();
}

void LayoutPrintPreviewDialog::OnResolutionChoice(wxCommandEvent& event)
{
    int idx = _resolutionChoice->GetSelection();
    if (idx < 0 || (size_t)idx >= _presets.size()) {
        return;
    }
    auto const& preset = _presets[idx];
    bool isCustom = preset.width == CUSTOM_RESOLUTION_INDEX_SENTINEL;
    _widthSpin->Enable(isCustom);
    _heightSpin->Enable(isCustom);
    if (!isCustom) {
        _widthSpin->SetValue(preset.width);
        _heightSpin->SetValue(preset.height);
    }
    RefreshPreview();
}

void LayoutPrintPreviewDialog::RefreshPreview()
{
    if (_previewCanvas == nullptr) {
        return;
    }

    _previewCanvas->SetActiveLayoutGroup(GetSelectedGroupName());
    _previewCanvas->SetShowModelNames(_showNamesCheck->GetValue());
    _previewCanvas->SetShowModelInfo(_showInfoCheck->GetValue());
    _previewCanvas->SetLabelFontSize(_labelSizeSpin->GetValue());
    xlColor labelColor = wxColourToXlColor(_labelColorPicker->GetColour());
    if (_invertCheck->GetValue()) {
        labelColor = InvertedXlColor(labelColor);
    }
    _previewCanvas->SetLabelColor(labelColor);

    wxImage* img = GrabPreviewImage(_previewCanvas, wxSize(GetPreviewWidth(), GetPreviewHeight()), _originalSize);
    if (img == nullptr) {
        spdlog::warn("LayoutPrintPreviewDialog::RefreshPreview - problem grabbing preview image");
        return;
    }
    if (_invertCheck->GetValue()) {
        InvertImageRGB(*img);
    }

    // Display a scaled-down copy, not the full-resolution image directly --
    // an uncapped wxStaticBitmap here (e.g. at a 1080p/4K chosen resolution)
    // makes the containing wxScrolledWindow report that as its own minimum
    // size, ballooning the dialog and pushing the Print/Close buttons off the
    // bottom. The full-resolution image is unaffected: printing re-grabs at
    // the page-fit size via Printout::GrabImage(), not from this bitmap.
    _lastCapturedImage = *img;
    delete img;
    UpdateDisplayBitmap();
}

void LayoutPrintPreviewDialog::UpdateDisplayBitmap()
{
    if (!_lastCapturedImage.IsOk()) {
        return;
    }

    // Fit within the scrolled area's current viewport, but never upscale --
    // that would just blur a smaller capture rather than reveal more detail.
    wxSize avail = _canvas->GetClientSize();
    int maxW = std::max(100, avail.GetWidth() - 20);
    int maxH = std::max(100, avail.GetHeight() - 20);

    wxImage displayImg = _lastCapturedImage;
    double scale = std::min(1.0, std::min(double(maxW) / displayImg.GetWidth(), double(maxH) / displayImg.GetHeight()));
    if (scale < 1.0) {
        displayImg.Rescale(std::max(1, int(displayImg.GetWidth() * scale)), std::max(1, int(displayImg.GetHeight() * scale)), wxIMAGE_QUALITY_BILINEAR);
    }
    _bitmapCtrl->SetBitmap(wxBitmap(displayImg));
    _canvas->SetVirtualSize(_bitmapCtrl->GetBitmap().GetSize());
    _canvas->FitInside();
    _canvas->Layout();
    _canvas->Refresh();
}

void LayoutPrintPreviewDialog::OnOwnerActivate(wxActivateEvent& event)
{
    event.Skip();
    if (event.GetActive() && IsShown()) {
        Raise();
    }
}

void LayoutPrintPreviewDialog::OnPrint(wxCommandEvent& event)
{
    _previewCanvas->SetActiveLayoutGroup(GetSelectedGroupName());
    _previewCanvas->SetShowModelNames(_showNamesCheck->GetValue());
    _previewCanvas->SetShowModelInfo(_showInfoCheck->GetValue());
    _previewCanvas->SetLabelFontSize(_labelSizeSpin->GetValue());
    xlColor labelColor = wxColourToXlColor(_labelColorPicker->GetColour());
    if (_invertCheck->GetValue()) {
        labelColor = InvertedXlColor(labelColor);
    }
    _previewCanvas->SetLabelColor(labelColor);

    Printout printout(_previewCanvas, _originalSize, _invertCheck->GetValue());

    wxPrintData printdata;
    static wxPrintDialogData printDialogData(printdata);
    wxPrinter printer(&printDialogData);

    if (!printer.Print(this, &printout, true)) {
        if (wxPrinter::GetLastError() == wxPRINTER_ERROR) {
            DisplayError(wxString::Format("Problem printing. %d", wxPrinter::GetLastError()).ToStdString(), this);
        }
    } else {
        printDialogData = printer.GetPrintDialogData();
        if (!printout.grabbedImage()) {
            DisplayError("Problem grabbing ModelPreview image for printing", this);
        }
    }

    RefreshPreview();
}
