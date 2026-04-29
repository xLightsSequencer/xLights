/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/dcclient.h>
#include <wx/dcbuffer.h>
#include <wx/colordlg.h>
#include <wx/textdlg.h>
#include <wx/numdlg.h>
#include <wx/filename.h>
#include <wx/dir.h>

#include "StemsPanel.h"
#include "StemWaveform.h"
#include "TimeLine.h"
#include "Waveform.h"
#include "RowHeading.h"
#include "MainSequencer.h"
#include "UtilFunctions.h"
#include "StemOnsetDialog.h"
#include "xLightsApp.h"
#include "xLightsMain.h"
#include "shared/utils/wxUtilities.h"
#include "../../src-core/render/SequenceFile.h"


wxDEFINE_EVENT(EVT_STEMS_CHANGED, wxCommandEvent);

const long StemsPanel::ID_MNU_IMPORT_FILES = wxNewId();
const long StemsPanel::ID_MNU_IMPORT_FOLDER = wxNewId();
const long StemsPanel::ID_MNU_REMOVE_STEM = wxNewId();
const long StemsPanel::ID_MNU_REMOVE_ALL = wxNewId();
const long StemsPanel::ID_MNU_MOVE_UP = wxNewId();
const long StemsPanel::ID_MNU_MOVE_DOWN = wxNewId();
const long StemsPanel::ID_MNU_RENAME = wxNewId();
const long StemsPanel::ID_MNU_RECOLOR = wxNewId();
const long StemsPanel::ID_MNU_SET_HEIGHT = wxNewId();
const long StemsPanel::ID_MNU_ONSET_DETECT = wxNewId();

BEGIN_EVENT_TABLE(StemsPanel, wxPanel)
END_EVENT_TABLE()

static const xlColor DEFAULT_STEM_COLORS[] = {
    xlColor(0xFF, 0x44, 0x44), // Red - Vocals
    xlColor(0x44, 0xFF, 0x44), // Green - Drums
    xlColor(0x44, 0x88, 0xFF), // Blue - Bass
    xlColor(0xFF, 0xAA, 0x22), // Orange - Other
    xlColor(0xCC, 0x44, 0xFF), // Purple
    xlColor(0x22, 0xFF, 0xCC), // Teal
    xlColor(0xFF, 0xFF, 0x44), // Yellow
    xlColor(0xFF, 0x44, 0xAA), // Pink
};
static const int NUM_DEFAULT_COLORS = sizeof(DEFAULT_STEM_COLORS) / sizeof(DEFAULT_STEM_COLORS[0]);

static const int RESIZE_HANDLE_HEIGHT = 5;
static const int MIN_PANEL_HEIGHT = 24;
static const int MAX_PANEL_HEIGHT = 400;
static const int COLLAPSED_HEIGHT = 6;

// Font sizing to match RowHeading exactly
static float ComputeStemFontSize() {
    float fontSize = 15.0f * DEFAULT_ROW_HEADING_HEIGHT / 22.0f;
    if (fontSize < 9) fontSize = 8;
    return fontSize;
}

#ifndef __WXMSW__
static void SetStemFontPixelSize(wxFont &font, float f) {
    float i = font.GetPixelSize().y;
    float p = font.GetFractionalPointSize();
    float points = f * p / i;
    font.SetFractionalPointSize(points);
}
#else
static void SetStemFontPixelSize(wxFont &font, float f) {
    wxSize sz(0, (int)(std::round(f * 0.8f)));
    font.SetPixelSize(sz);
}
#endif

// =========================================================================
// StemsPanel
// =========================================================================

StemsPanel::StemsPanel(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
    : wxPanel(parent, id, pos, size, wxTAB_TRAVERSAL | wxBORDER_NONE)
{
    Hide();

    // Outer headers panel: clips content, no scrollbar
    _stemHeadersOuter = new wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                     wxTAB_TRAVERSAL | wxBORDER_NONE | wxCLIP_CHILDREN);
    _stemHeadersOuter->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
    _headerWindow = new StemHeaderWindow(_stemHeadersOuter, this);
    wxBoxSizer* hdrSizer = new wxBoxSizer(wxVERTICAL);
    hdrSizer->Add(_headerWindow, 1, wxEXPAND, 0);
    _stemHeadersOuter->SetSizer(hdrSizer);
    _stemHeadersOuter->Hide();

    // Outer waveforms panel: clips content, no sizer — inner panel sized/positioned manually
    _stemWaveformsOuter = new wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                       wxTAB_TRAVERSAL | wxBORDER_NONE | wxCLIP_CHILDREN);
    _stemWaveformsOuter->SetBackgroundColour(wxColour(30, 30, 35));
    _waveformsInner = new wxPanel(_stemWaveformsOuter, wxID_ANY);
    _waveformsInner->SetBackgroundColour(wxColour(30, 30, 35));
    _waveformsSizer = new wxBoxSizer(wxVERTICAL);
    _waveformsInner->SetSizer(_waveformsSizer);
    // No outer sizer — we manage _waveformsInner position/size manually for scrolling
    _stemWaveformsOuter->Hide();

    // Resize handle between stems and effects grid
    _resizeHandle = new StemResizeHandle(parent, this);
    _resizeHandle->Hide();

    // When outer waveforms panel resizes, update inner panel width
    _stemWaveformsOuter->Bind(wxEVT_SIZE, [this](wxSizeEvent& evt) {
        evt.Skip();
        wxSize sz = evt.GetSize();
        if (sz.GetWidth() > 0 && !_stems.empty()) {
            int contentHeight = GetTotalContentHeight();
            _waveformsInner->SetSize(sz.GetWidth(), contentHeight);
            _waveformsInner->Layout();
        }
    });

    // Mouse wheel on both outer panels for scrolling
    _stemHeadersOuter->Bind(wxEVT_MOUSEWHEEL, &StemsPanel::OnMouseWheel, this);
    _stemWaveformsOuter->Bind(wxEVT_MOUSEWHEEL, &StemsPanel::OnMouseWheel, this);

    // Right-click on waveforms panel background
    _stemWaveformsOuter->Bind(wxEVT_RIGHT_DOWN, &StemsPanel::OnRightDown, this);
}

StemsPanel::~StemsPanel()
{
    // Tear down rows without touching SequenceFile::alt_tracks — the panel
    // is a view; the model owns the data and will free it on its own
    // teardown. Calling RemoveAllStems() here would mutate the sequence on
    // shutdown and dirty it.
    ClearRowsUiOnly();
}

xlColor StemsPanel::GetDefaultStemColor(int index) const
{
    return DEFAULT_STEM_COLORS[index % NUM_DEFAULT_COLORS];
}

static SequenceFile* CurrentSeqFile()
{
    auto* frame = xLightsApp::GetFrame();
    return frame != nullptr ? frame->CurrentSeqXmlFile : nullptr;
}

static Waveform* MainWaveform()
{
    auto* frame = xLightsApp::GetFrame();
    if (frame == nullptr) return nullptr;
    auto* ms = frame->GetMainSequencer();
    return ms != nullptr ? ms->PanelWaveForm : nullptr;
}

bool StemsPanel::AddStem(const std::string& name, const std::string& filepath, const xlColor& color)
{
    spdlog::debug("[DEBUG: StemsPanel::AddStem '{}' from '{}']", name, filepath);

    SequenceFile* sf = CurrentSeqFile();
    if (sf == nullptr) {
        spdlog::warn("StemsPanel::AddStem called with no current sequence");
        return false;
    }

    sf->AddAltTrack(_showDir, filepath, name);
    int altIdx = sf->GetAltTrackCount() - 1;
    if (altIdx < 0) return false;

    // The on-disk path may have been canonicalised by AddAltTrack (FixFile).
    std::string resolvedPath = sf->GetAltTrack(altIdx).path;
    std::string finalName = sf->GetAltTrackDisplayName(altIdx);

    StemInfo stem;
    stem.name = finalName;
    stem.path = resolvedPath;
    stem.color = color;
    stem.altTrackIdx = altIdx;

    StemWaveform* wf = new StemWaveform(_waveformsInner, wxID_ANY);
    wf->SetTimeline(_timeline);
    wf->SetEventTarget(GetParent());
    wf->SetStemName(finalName);
    wf->SetStemColor(color);
    wf->SetTimeFrequency(_frequency);
    wf->SetZoomLevel(_zoomLevel);
    wf->SetStartPixelOffset(_startPixelOffset);

    wxString error;
    if (!wf->LoadMedia(resolvedPath, error)) {
        spdlog::error("Failed to load stem '{}': {}", finalName, (const char*)error.c_str());
        delete wf;
        // Roll back the alt-track since we can't display it.
        sf->RemoveAltTrack(altIdx);
        return false;
    }

    wf->SetRowHeight(_stemRowHeight);
    stem.waveform = wf;
    _stems.push_back(stem);

    wf->Bind(wxEVT_MOUSEWHEEL, &StemsPanel::OnMouseWheel, this);

    RebuildLayout();
    UpdateVisibility();

    wxCommandEvent evt(EVT_STEMS_CHANGED);
    wxPostEvent(GetParent(), evt);
    return true;
}

void StemsPanel::RemoveStem(int index)
{
    if (index < 0 || index >= (int)_stems.size()) return;

    int altIdx = _stems[index].altTrackIdx;

    if (_stems[index].waveform) {
        _stems[index].waveform->CloseMedia();
        _stems[index].waveform->Destroy();
    }
    _stems.erase(_stems.begin() + index);

    if (SequenceFile* sf = CurrentSeqFile()) {
        if (altIdx >= 0 && altIdx < sf->GetAltTrackCount()) {
            // If main waveform is currently following this alt track, reset to Main first.
            if (Waveform* wf = MainWaveform()) {
                if (wf->GetActiveAudioTrackIndex() == altIdx + 1) {
                    wf->SetActiveAudioTrack(0);
                }
            }
            sf->RemoveAltTrack(altIdx);
        }
    }

    // Re-sequence altTrackIdx for stems that came after the removed one.
    for (auto& s : _stems) {
        if (s.altTrackIdx > altIdx) s.altTrackIdx--;
    }

    if (_stems.empty()) _scrollOffset = 0;

    RebuildLayout();
    UpdateVisibility();

    wxCommandEvent evt(EVT_STEMS_CHANGED);
    wxPostEvent(GetParent(), evt);
}

void StemsPanel::RemoveAllStems()
{
    for (auto& stem : _stems) {
        if (stem.waveform) {
            stem.waveform->CloseMedia();
            stem.waveform->Destroy();
        }
    }
    if (SequenceFile* sf = CurrentSeqFile()) {
        // Reset playback to Main if any alt track was active.
        if (Waveform* wf = MainWaveform()) {
            if (wf->GetActiveAudioTrackIndex() != 0) wf->SetActiveAudioTrack(0);
        }
        // Remove only the alt-tracks that the panel owned (matching altTrackIdx).
        // Iterate in descending order so indexes stay valid.
        std::vector<int> indices;
        indices.reserve(_stems.size());
        for (auto& s : _stems) {
            if (s.altTrackIdx >= 0) indices.push_back(s.altTrackIdx);
        }
        std::sort(indices.begin(), indices.end(), std::greater<int>());
        for (int idx : indices) sf->RemoveAltTrack(idx);
    }
    _stems.clear();
    _scrollOffset = 0;

    RebuildLayout();
    UpdateVisibility();

    wxCommandEvent evt(EVT_STEMS_CHANGED);
    wxPostEvent(GetParent(), evt);
}

void StemsPanel::ClearRowsUiOnly()
{
    for (auto& stem : _stems) {
        if (stem.waveform) {
            stem.waveform->CloseMedia();
            stem.waveform->Destroy();
        }
    }
    _stems.clear();
    _scrollOffset = 0;
    RebuildLayout();
    UpdateVisibility();
}

void StemsPanel::MoveStemUp(int index)
{
    if (index <= 0 || index >= (int)_stems.size()) return;
    if (SequenceFile* sf = CurrentSeqFile()) {
        sf->MoveAltTrack(_stems[index].altTrackIdx, _stems[index - 1].altTrackIdx);
    }
    std::swap(_stems[index], _stems[index - 1]);
    std::swap(_stems[index].altTrackIdx, _stems[index - 1].altTrackIdx);
    RebuildLayout();

    wxCommandEvent evt(EVT_STEMS_CHANGED);
    wxPostEvent(GetParent(), evt);
}

void StemsPanel::MoveStemDown(int index)
{
    if (index < 0 || index >= (int)_stems.size() - 1) return;
    if (SequenceFile* sf = CurrentSeqFile()) {
        sf->MoveAltTrack(_stems[index].altTrackIdx, _stems[index + 1].altTrackIdx);
    }
    std::swap(_stems[index], _stems[index + 1]);
    std::swap(_stems[index].altTrackIdx, _stems[index + 1].altTrackIdx);
    RebuildLayout();

    wxCommandEvent evt(EVT_STEMS_CHANGED);
    wxPostEvent(GetParent(), evt);
}

StemInfo* StemsPanel::GetStem(int index)
{
    if (index < 0 || index >= (int)_stems.size()) return nullptr;
    return &_stems[index];
}

void StemsPanel::RenameStem(int index, const std::string& name)
{
    if (index < 0 || index >= (int)_stems.size()) return;
    std::string finalName = name;
    if (SequenceFile* sf = CurrentSeqFile()) {
        sf->SetAltTrackShortname(_stems[index].altTrackIdx, name);
        finalName = sf->GetAltTrackDisplayName(_stems[index].altTrackIdx);
    }
    _stems[index].name = finalName;
    if (_stems[index].waveform) {
        _stems[index].waveform->SetStemName(finalName);
    }
    _headerWindow->Refresh();

    wxCommandEvent evt(EVT_STEMS_CHANGED);
    wxPostEvent(GetParent(), evt);
}

void StemsPanel::RecolorStem(int index, const xlColor& color)
{
    if (index < 0 || index >= (int)_stems.size()) return;
    _stems[index].color = color;
    if (_stems[index].waveform) {
        _stems[index].waveform->SetStemColor(color);
    }
    _headerWindow->Refresh();

    wxCommandEvent evt(EVT_STEMS_CHANGED);
    wxPostEvent(GetParent(), evt);
}

void StemsPanel::ImportStemFiles()
{
    wxFileDialog dlg(GetParent(), "Select Stem Audio Files", _showDir, "",
                     "Audio files (*.wav;*.mp3;*.ogg;*.flac;*.m4a)|*.wav;*.mp3;*.ogg;*.flac;*.m4a|All files (*.*)|*.*",
                     wxFD_OPEN | wxFD_MULTIPLE | wxFD_FILE_MUST_EXIST);

    if (dlg.ShowModal() == wxID_OK) {
        wxArrayString paths;
        dlg.GetPaths(paths);
        for (size_t i = 0; i < paths.size(); i++) {
            wxFileName fn(paths[i]);
            std::string name = fn.GetName().ToStdString();
            xlColor color = GetDefaultStemColor((int)_stems.size() + (int)i);
            AddStem(name, paths[i].ToStdString(), color);
        }
    }
}

void StemsPanel::ImportStemFolder()
{
    wxDirDialog dlg(GetParent(), "Select Folder Containing Stem Files", _showDir,
                    wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

    if (dlg.ShowModal() == wxID_OK) {
        wxString folder = dlg.GetPath();
        wxDir dir(folder);
        if (!dir.IsOpened()) return;

        wxString filename;
        wxArrayString files;
        const wxString patterns[] = { "*.wav", "*.mp3", "*.flac", "*.ogg", "*.m4a" };
        for (const auto& pat : patterns) {
            bool cont = dir.GetFirst(&filename, pat, wxDIR_FILES);
            while (cont) {
                files.Add(folder + wxFileName::GetPathSeparator() + filename);
                cont = dir.GetNext(&filename);
            }
        }

        files.Sort();
        for (size_t i = 0; i < files.size(); i++) {
            wxFileName fn(files[i]);
            std::string name = fn.GetName().ToStdString();
            xlColor color = GetDefaultStemColor((int)_stems.size() + (int)i);
            AddStem(name, files[i].ToStdString(), color);
        }
    }
}

void StemsPanel::SetStemRowHeight(int height)
{
    if (height < 16) height = 16;
    if (height > 80) height = 80;
    _stemRowHeight = height;

    for (auto& stem : _stems) {
        if (stem.waveform) {
            stem.waveform->SetRowHeight(_stemRowHeight);
        }
    }

    RebuildLayout();
    UpdateVisibility();
}

void StemsPanel::SetPanelHeight(int height)
{
    if (height < MIN_PANEL_HEIGHT) height = MIN_PANEL_HEIGHT;
    if (height > MAX_PANEL_HEIGHT) height = MAX_PANEL_HEIGHT;
    _panelHeight = height;
    _collapsed = false;
    UpdateVisibility();
}

void StemsPanel::SetCollapsed(bool collapsed)
{
    _collapsed = collapsed;
    UpdateVisibility();
}

void StemsPanel::ToggleCollapsed()
{
    _collapsed = !_collapsed;
    UpdateVisibility();
}

void StemsPanel::SetHeaderWidth(int width)
{
    _headerWidth = width;
    if (_headerWidth < 158) _headerWidth = 158;

    _stemHeadersOuter->SetMinSize(wxSize(_headerWidth, -1));
    _stemHeadersOuter->SetMaxSize(wxSize(_headerWidth, -1));

    _stemHeadersOuter->Layout();
    _headerWindow->Refresh();
    if (_stemHeadersOuter->GetParent()) {
        _stemHeadersOuter->GetParent()->Layout();
    }
}

void StemsPanel::SetUserVisible(bool visible)
{
    _userVisible = visible;
    UpdateVisibility();
}

int StemsPanel::GetTotalContentHeight() const
{
    if (_stems.empty()) return MIN_PANEL_HEIGHT;
    return (int)_stems.size() * _stemRowHeight;
}

void StemsPanel::UpdateVisibility()
{
    bool show = _userVisible;

    if (!show) {
        _stemHeadersOuter->SetMinSize(wxSize(0, 0));
        _stemHeadersOuter->SetMaxSize(wxSize(0, 0));
        _stemWaveformsOuter->SetMinSize(wxSize(0, 0));
        _stemWaveformsOuter->SetMaxSize(wxSize(0, 0));
        _resizeHandle->SetMinSize(wxSize(0, 0));
        _resizeHandle->SetMaxSize(wxSize(0, 0));
    } else if (_collapsed) {
        _stemHeadersOuter->SetMinSize(wxSize(0, 0));
        _stemHeadersOuter->SetMaxSize(wxSize(0, 0));
        _stemWaveformsOuter->SetMinSize(wxSize(0, 0));
        _stemWaveformsOuter->SetMaxSize(wxSize(0, 0));
        _resizeHandle->SetMinSize(wxSize(-1, RESIZE_HANDLE_HEIGHT));
        _resizeHandle->SetMaxSize(wxSize(-1, RESIZE_HANDLE_HEIGHT));
    } else {
        int displayHeight;
        if (_stems.empty()) {
            displayHeight = MIN_PANEL_HEIGHT;
        } else {
            int contentHeight = GetTotalContentHeight();
            displayHeight = std::min(_panelHeight, contentHeight);
        }

        _stemHeadersOuter->SetMinSize(wxSize(_headerWidth, displayHeight));
        _stemHeadersOuter->SetMaxSize(wxSize(_headerWidth, displayHeight));
        _stemWaveformsOuter->SetMinSize(wxSize(-1, displayHeight));
        _stemWaveformsOuter->SetMaxSize(wxSize(-1, displayHeight));
        _resizeHandle->SetMinSize(wxSize(-1, RESIZE_HANDLE_HEIGHT));
        _resizeHandle->SetMaxSize(wxSize(-1, RESIZE_HANDLE_HEIGHT));

        // Clamp scroll offset
        int maxScroll = std::max(0, GetTotalContentHeight() - displayHeight);
        if (_scrollOffset > maxScroll) _scrollOffset = maxScroll;
        if (_scrollOffset < 0) _scrollOffset = 0;

        SyncScrollPositions();
    }

    _stemHeadersOuter->Show(show && !_collapsed);
    _stemWaveformsOuter->Show(show && !_collapsed);
    _resizeHandle->Show(show);

    wxWindow* parent = _stemHeadersOuter->GetParent();
    if (parent) parent->Layout();
}

void StemsPanel::SetScrollOffset(int offset)
{
    int contentHeight = GetTotalContentHeight();
    int displayHeight = _stemWaveformsOuter->GetSize().GetHeight();
    int maxScroll = std::max(0, contentHeight - displayHeight);
    _scrollOffset = std::clamp(offset, 0, maxScroll);
    SyncScrollPositions();
}

void StemsPanel::SyncScrollPositions()
{
    // Set inner panel to full content height and outer panel's width
    int outerWidth = _stemWaveformsOuter->GetSize().GetWidth();
    if (outerWidth <= 0) outerWidth = _stemWaveformsOuter->GetMinSize().GetWidth();
    int contentHeight = GetTotalContentHeight();
    _waveformsInner->SetSize(outerWidth, contentHeight);
    _waveformsInner->Layout();

    // Position for scrolling
    _waveformsInner->SetPosition(wxPoint(0, -_scrollOffset));
    _headerWindow->Refresh();
    _stemWaveformsOuter->Refresh();
}

void StemsPanel::OnMouseWheel(wxMouseEvent& event)
{
    if (_collapsed) return;

#ifdef __WXOSX__
    // macOS trackpad horizontal swipe → forward as horizontal scroll
    if (event.GetWheelAxis() == wxMOUSE_WHEEL_HORIZONTAL) {
        int i = event.GetWheelRotation();
        wxCommandEvent eventScroll(EVT_GSCROLL);
        eventScroll.SetInt(i > 0 ? SCROLL_RIGHT : SCROLL_LEFT);
        wxPostEvent(GetParent(), eventScroll);
        return;
    }
#endif

    // Cmd+wheel → zoom
    if (event.CmdDown()) {
        int i = event.GetWheelRotation();
        wxCommandEvent eventZoom(EVT_ZOOM);
        eventZoom.SetInt(i < 0 ? ZOOM_OUT : ZOOM_IN);
        wxPostEvent(GetParent(), eventZoom);
        return;
    }

    // Shift+wheel → horizontal scroll
    if (event.ShiftDown()) {
        int i = event.GetWheelRotation();
        wxCommandEvent eventScroll(EVT_GSCROLL);
        eventScroll.SetInt(i < 0 ? SCROLL_RIGHT : SCROLL_LEFT);
        wxPostEvent(GetParent(), eventScroll);
        return;
    }

    // Plain vertical scroll → scroll stems area
    if (_stems.empty()) return;
    int contentHeight = GetTotalContentHeight();
    int displayHeight = _stemWaveformsOuter->GetSize().GetHeight();
    if (contentHeight <= displayHeight) return;

    int delta = event.GetWheelRotation();
    int scrollStep = _stemRowHeight / 2;
    if (scrollStep < 8) scrollStep = 8;

    if (delta > 0) {
        SetScrollOffset(_scrollOffset - scrollStep);
    } else if (delta < 0) {
        SetScrollOffset(_scrollOffset + scrollStep);
    }
}

void StemsPanel::SetZoomLevel(int level)
{
    _zoomLevel = level;
    for (auto& stem : _stems) {
        if (stem.waveform) {
            stem.waveform->SetZoomLevel(level);
        }
    }
}

void StemsPanel::SetStartPixelOffset(int offset)
{
    _startPixelOffset = offset;
    for (auto& stem : _stems) {
        if (stem.waveform) {
            stem.waveform->SetStartPixelOffset(offset);
        }
    }
}

void StemsPanel::SetTimeFrequency(int frequency)
{
    _frequency = frequency;
    for (auto& stem : _stems) {
        if (stem.waveform) {
            stem.waveform->SetTimeFrequency(frequency);
        }
    }
}

void StemsPanel::UpdatePlayMarker()
{
    for (auto& stem : _stems) {
        if (stem.waveform) {
            stem.waveform->UpdatePlayMarker();
        }
    }
}

void StemsPanel::ForceRedraw()
{
    for (auto& stem : _stems) {
        if (stem.waveform) {
            stem.waveform->ForceRedraw();
            stem.waveform->render();
        }
    }
    _headerWindow->Refresh();
}

bool StemsPanel::LoadFromXml(wxXmlNode* stemsNode, const wxString& showDir)
{
    spdlog::debug("[DEBUG: StemsPanel::LoadFromXml starting]");
    if (stemsNode == nullptr || stemsNode->GetName() != "Stems") return false;

    // Don't call RemoveAllStems() — it would also delete the new sequence's
    // alt_tracks. We only want to clear leftover UI rows from a prior sequence.
    ClearRowsUiOnly();

    _stemRowHeight = wxAtoi(stemsNode->GetAttribute("rowHeight", "32"));
    _panelHeight = wxAtoi(stemsNode->GetAttribute("panelHeight", "96"));
    _collapsed = stemsNode->GetAttribute("collapsed", "0") == "1";
    _userVisible = stemsNode->GetAttribute("visible", "1") != "0";

    SequenceFile* sf = CurrentSeqFile();

    // Legacy migration: <Stem path="..." /> entries owned the audio. New canonical
    // location is SequenceFile::alt_tracks. AddStem will route through AddAltTrack.
    // For new-format files the <Stems> node will only carry colors keyed by index.
    int colorIdx = 0;
    for (wxXmlNode* child = stemsNode->GetChildren(); child != nullptr; child = child->GetNext()) {
        if (child->GetName() != "Stem") continue;

        std::string name = child->GetAttribute("name", "Stem").ToStdString();
        std::string path = child->GetAttribute("path", "").ToStdString();
        std::string colorStr = child->GetAttribute("color", "").ToStdString();
        xlColor color = colorStr.empty() ? GetDefaultStemColor(colorIdx) : xlColor(colorStr);

        if (!path.empty()) {
            // Legacy form — bring the path into alt_tracks.
            std::string resolvedPath = ResolveRelativePath(path, showDir.ToStdString());
            AddStem(name, resolvedPath, color);
        }
        // New form: just remember the color slot; the actual stems come from alt_tracks
        // via RefreshFromAltTracks below. We'll re-apply colors after refresh.
        colorIdx++;
    }

    // If the panel currently has no stems (i.e. either empty XML or new-format
    // overlay without legacy paths), pull stems from SequenceFile::alt_tracks.
    if (_stems.empty() && sf != nullptr && sf->GetAltTrackCount() > 0) {
        RefreshFromAltTracks();

        // Apply colors from the overlay back onto the refreshed rows.
        int idx = 0;
        for (wxXmlNode* child = stemsNode->GetChildren(); child != nullptr; child = child->GetNext()) {
            if (child->GetName() != "Stem") continue;
            if (idx >= (int)_stems.size()) break;
            std::string colorStr = child->GetAttribute("color", "").ToStdString();
            if (!colorStr.empty()) {
                xlColor c(colorStr);
                _stems[idx].color = c;
                if (_stems[idx].waveform) _stems[idx].waveform->SetStemColor(c);
            }
            idx++;
        }
    }

    SetStemRowHeight(_stemRowHeight);
    UpdateVisibility();

    spdlog::debug("[DEBUG: StemsPanel::LoadFromXml done, {} stems loaded]", _stems.size());
    return true;
}

wxXmlNode* StemsPanel::SaveToXml(const wxString& /*showDir*/) const
{
    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "Stems");
    node->AddAttribute("rowHeight", wxString::Format("%d", _stemRowHeight));
    node->AddAttribute("panelHeight", wxString::Format("%d", _panelHeight));
    node->AddAttribute("collapsed", _collapsed ? "1" : "0");
    node->AddAttribute("visible", _userVisible ? "1" : "0");

    // UI overlay: just colors, indexed by alt-track position. Audio paths and
    // shortnames live in SequenceFile::alt_tracks (<altAudioTracks> XML).
    for (const auto& stem : _stems) {
        wxXmlNode* stemNode = new wxXmlNode(wxXML_ELEMENT_NODE, "Stem");
        stemNode->AddAttribute("color", wxString::Format("#%02X%02X%02X",
            (int)stem.color.red, (int)stem.color.green, (int)stem.color.blue));
        node->AddChild(stemNode);
    }

    return node;
}

void StemsPanel::RefreshFromAltTracks()
{
    SequenceFile* sf = CurrentSeqFile();
    if (sf == nullptr) return;

    int n = sf->GetAltTrackCount();

    // Drop excess rows.
    while ((int)_stems.size() > n) {
        if (_stems.back().waveform) {
            _stems.back().waveform->CloseMedia();
            _stems.back().waveform->Destroy();
        }
        _stems.pop_back();
    }

    // Update existing rows, append new ones.
    for (int i = 0; i < n; i++) {
        const AlternateAudioTrack& at = sf->GetAltTrack(i);
        std::string name = sf->GetAltTrackDisplayName(i);

        if (i < (int)_stems.size()) {
            StemInfo& s = _stems[i];
            s.altTrackIdx = i;
            if (s.path != at.path) {
                if (s.waveform) {
                    wxString error;
                    s.waveform->LoadMedia(at.path, error);
                }
                s.path = at.path;
            }
            if (s.name != name) {
                s.name = name;
                if (s.waveform) s.waveform->SetStemName(name);
            }
            continue;
        }

        StemInfo stem;
        stem.name = name;
        stem.path = at.path;
        stem.color = GetDefaultStemColor(i);
        stem.altTrackIdx = i;

        StemWaveform* wf = new StemWaveform(_waveformsInner, wxID_ANY);
        wf->SetTimeline(_timeline);
        wf->SetEventTarget(GetParent());
        wf->SetStemName(name);
        wf->SetStemColor(stem.color);
        wf->SetTimeFrequency(_frequency);
        wf->SetZoomLevel(_zoomLevel);
        wf->SetStartPixelOffset(_startPixelOffset);
        wxString error;
        if (!wf->LoadMedia(at.path, error)) {
            spdlog::error("RefreshFromAltTracks: failed to load '{}': {}",
                          at.path, (const char*)error.c_str());
            delete wf;
            continue;
        }
        wf->SetRowHeight(_stemRowHeight);
        stem.waveform = wf;
        wf->Bind(wxEVT_MOUSEWHEEL, &StemsPanel::OnMouseWheel, this);
        _stems.push_back(stem);
    }

    RebuildLayout();
    UpdateVisibility();
}

void StemsPanel::SoloStem(int stemIndex)
{
    if (stemIndex < 0 || stemIndex >= (int)_stems.size()) return;
    Waveform* wf = MainWaveform();
    if (wf == nullptr) return;

    int altIdx = _stems[stemIndex].altTrackIdx;
    int targetTrack = altIdx + 1;  // 0 = Main

    // Toggle: clicking the active solo returns to Main.
    if (wf->GetActiveAudioTrackIndex() == targetTrack) {
        wf->SetActiveAudioTrack(0);
    } else {
        wf->SetActiveAudioTrack(targetTrack);
    }
    if (_headerWindow) _headerWindow->Refresh();
}

int StemsPanel::GetActiveStemIndex() const
{
    Waveform* wf = MainWaveform();
    if (wf == nullptr) return -1;
    int track = wf->GetActiveAudioTrackIndex();
    if (track <= 0) return -1;
    int altIdx = track - 1;
    for (int i = 0; i < (int)_stems.size(); i++) {
        if (_stems[i].altTrackIdx == altIdx) return i;
    }
    return -1;
}

void StemsPanel::RebuildLayout()
{
    _waveformsSizer->Clear(false);

    // Detach all waveforms from sizer (don't destroy)
    for (auto& stem : _stems) {
        if (stem.waveform) {
            _waveformsSizer->Add(stem.waveform, 0, wxEXPAND, 0);
        }
    }

    int contentHeight = GetTotalContentHeight();
    _waveformsInner->SetMinSize(wxSize(-1, contentHeight));
    _waveformsInner->Layout();

    _headerWindow->Refresh();
}

int StemsPanel::HitTestStemIndex(int y)
{
    if (_stems.empty() || _stemRowHeight <= 0) return -1;
    int adjustedY = y + _scrollOffset;
    int index = adjustedY / _stemRowHeight;
    if (index < 0 || index >= (int)_stems.size()) return -1;
    return index;
}

void StemsPanel::OnRightDown(wxMouseEvent& event)
{
    _contextMenuStemIndex = -1;

    // Try to determine which stem was clicked
    wxWindow* src = dynamic_cast<wxWindow*>(event.GetEventObject());
    if (src == _headerWindow) {
        _contextMenuStemIndex = HitTestStemIndex(event.GetY());
    } else if (src == _stemWaveformsOuter) {
        _contextMenuStemIndex = HitTestStemIndex(event.GetY());
    }

    wxMenu menu;
    menu.Append(ID_MNU_IMPORT_FILES, "Import Stem Files...");
    menu.Append(ID_MNU_IMPORT_FOLDER, "Import Stem Folder...");
    menu.AppendSeparator();

    if (_contextMenuStemIndex >= 0 && _contextMenuStemIndex < (int)_stems.size()) {
        menu.Append(ID_MNU_RENAME, wxString::Format("Rename '%s'...", _stems[_contextMenuStemIndex].name));
        menu.Append(ID_MNU_RECOLOR, wxString::Format("Change Color of '%s'...", _stems[_contextMenuStemIndex].name));
        menu.AppendSeparator();
        menu.Append(ID_MNU_ONSET_DETECT, "Create Timing Track from Transients...");
        menu.AppendSeparator();
        menu.Append(ID_MNU_MOVE_UP, "Move Up");
        menu.Append(ID_MNU_MOVE_DOWN, "Move Down");
        menu.Enable(ID_MNU_MOVE_UP, _contextMenuStemIndex > 0);
        menu.Enable(ID_MNU_MOVE_DOWN, _contextMenuStemIndex < (int)_stems.size() - 1);
        menu.AppendSeparator();
        menu.Append(ID_MNU_REMOVE_STEM, wxString::Format("Remove '%s'", _stems[_contextMenuStemIndex].name));
    }

    if (!_stems.empty()) {
        menu.Append(ID_MNU_REMOVE_ALL, "Remove All Stems");
        menu.AppendSeparator();
    }

    menu.Append(ID_MNU_SET_HEIGHT, "Set Row Height...");

    menu.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&StemsPanel::OnPopupMenu, nullptr, this);

    if (src) {
        src->PopupMenu(&menu);
    } else {
        _stemHeadersOuter->PopupMenu(&menu);
    }

    _contextMenuStemIndex = -1;
}

void StemsPanel::OnPopupMenu(wxCommandEvent& event)
{
    int id = event.GetId();
    if (id == ID_MNU_IMPORT_FILES) {
        ImportStemFiles();
    } else if (id == ID_MNU_IMPORT_FOLDER) {
        ImportStemFolder();
    } else if (id == ID_MNU_REMOVE_STEM) {
        if (_contextMenuStemIndex >= 0) {
            RemoveStem(_contextMenuStemIndex);
        }
    } else if (id == ID_MNU_REMOVE_ALL) {
        RemoveAllStems();
    } else if (id == ID_MNU_MOVE_UP) {
        if (_contextMenuStemIndex >= 0) {
            MoveStemUp(_contextMenuStemIndex);
        }
    } else if (id == ID_MNU_MOVE_DOWN) {
        if (_contextMenuStemIndex >= 0) {
            MoveStemDown(_contextMenuStemIndex);
        }
    } else if (id == ID_MNU_RENAME) {
        if (_contextMenuStemIndex >= 0 && _contextMenuStemIndex < (int)_stems.size()) {
            wxTextEntryDialog dlg(GetParent(), "Enter new name:", "Rename Stem",
                                  _stems[_contextMenuStemIndex].name);
            if (dlg.ShowModal() == wxID_OK) {
                RenameStem(_contextMenuStemIndex, dlg.GetValue().ToStdString());
            }
        }
    } else if (id == ID_MNU_RECOLOR) {
        if (_contextMenuStemIndex >= 0 && _contextMenuStemIndex < (int)_stems.size()) {
            xlColor c = _stems[_contextMenuStemIndex].color;
            wxColourData data;
            data.SetColour(wxColour(c.red, c.green, c.blue));
            wxColourDialog dlg(GetParent(), &data);
            if (dlg.ShowModal() == wxID_OK) {
                wxColour nc = dlg.GetColourData().GetColour();
                RecolorStem(_contextMenuStemIndex, xlColor(nc.Red(), nc.Green(), nc.Blue()));
            }
        }
    } else if (id == ID_MNU_SET_HEIGHT) {
        long height = wxGetNumberFromUser("Set stem row height (16-80):", "Height",
                                           "Stem Row Height", _stemRowHeight, 16, 80, GetParent());
        if (height != -1) {
            SetStemRowHeight((int)height);
        }
    } else if (id == ID_MNU_ONSET_DETECT) {
        if (_contextMenuStemIndex >= 0 && _contextMenuStemIndex < (int)_stems.size()) {
            StemInfo& stem = _stems[_contextMenuStemIndex];
            xLightsFrame* frame = xLightsApp::GetFrame();
            if (frame != nullptr && stem.waveform != nullptr) {
                StemOnsetDialog* dlg = new StemOnsetDialog(this, stem.waveform, frame, stem.name);
                dlg->Show();
            }
        }
    }
}

void StemsPanel::OnStemHeaderDClick(int stemIndex)
{
    if (stemIndex < 0 || stemIndex >= (int)_stems.size()) return;

    wxTextEntryDialog dlg(GetParent(), "Enter new name:", "Rename Stem",
                          _stems[stemIndex].name);
    if (dlg.ShowModal() == wxID_OK) {
        RenameStem(stemIndex, dlg.GetValue().ToStdString());
    }
}

void StemsPanel::BeginDragReorder(int stemIndex, int mouseY)
{
    if (stemIndex < 0 || stemIndex >= (int)_stems.size()) return;
    _dragging = true;
    _dragSourceIndex = stemIndex;
    _dragInsertIndex = stemIndex;
    _dragStartY = mouseY;
    _headerWindow->Refresh();
}

void StemsPanel::UpdateDragReorder(int mouseY)
{
    if (!_dragging) return;
    int adjustedY = mouseY + _scrollOffset;
    int newIndex = adjustedY / _stemRowHeight;
    if (newIndex < 0) newIndex = 0;
    if (newIndex >= (int)_stems.size()) newIndex = (int)_stems.size() - 1;
    if (newIndex != _dragInsertIndex) {
        _dragInsertIndex = newIndex;
        _headerWindow->Refresh();
    }
}

void StemsPanel::EndDragReorder()
{
    if (!_dragging) return;
    _dragging = false;

    if (_dragSourceIndex != _dragInsertIndex &&
        _dragSourceIndex >= 0 && _dragSourceIndex < (int)_stems.size() &&
        _dragInsertIndex >= 0 && _dragInsertIndex < (int)_stems.size()) {

        if (SequenceFile* sf = CurrentSeqFile()) {
            sf->MoveAltTrack(_stems[_dragSourceIndex].altTrackIdx,
                             _stems[_dragInsertIndex].altTrackIdx);
        }

        StemInfo moving = _stems[_dragSourceIndex];
        _stems.erase(_stems.begin() + _dragSourceIndex);
        _stems.insert(_stems.begin() + _dragInsertIndex, moving);

        // Re-anchor altTrackIdx so each stem matches its new position.
        for (int i = 0; i < (int)_stems.size(); i++) {
            _stems[i].altTrackIdx = i;
        }

        RebuildLayout();

        wxCommandEvent evt(EVT_STEMS_CHANGED);
        wxPostEvent(GetParent(), evt);
    }

    _dragSourceIndex = -1;
    _dragInsertIndex = -1;
    _headerWindow->Refresh();
}

std::string StemsPanel::MakeRelativePath(const std::string& absPath, const std::string& showDir) const
{
    if (showDir.empty() || absPath.empty()) return absPath;

    wxFileName fn(absPath);
    if (fn.MakeRelativeTo(showDir)) {
        return fn.GetFullPath().ToStdString();
    }
    return absPath;
}

std::string StemsPanel::ResolveRelativePath(const std::string& relPath, const std::string& showDir) const
{
    if (relPath.empty()) return relPath;

    wxFileName fn(relPath);
    if (fn.IsRelative() && !showDir.empty()) {
        fn.MakeAbsolute(showDir);
    }
    return fn.GetFullPath().ToStdString();
}

// =========================================================================
// StemHeaderWindow — custom drawn header (matches RowHeading style)
// =========================================================================

wxRect StemHeaderWindow::SoloButtonRect(int headerW, int startY, int rowHeight)
{
    int btnSize = std::min(rowHeight - 6, 22);
    if (btnSize < 12) btnSize = 12;
    int x = headerW - btnSize - 6;
    int y = startY + (rowHeight - btnSize) / 2;
    return wxRect(x, y, btnSize, btnSize);
}

StemHeaderWindow::StemHeaderWindow(wxWindow* parent, StemsPanel* stemsPanel)
    : wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
    , _stemsPanel(stemsPanel)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);

    Bind(wxEVT_PAINT, &StemHeaderWindow::OnPaint, this);
    Bind(wxEVT_LEFT_DOWN, &StemHeaderWindow::OnLeftDown, this);
    Bind(wxEVT_LEFT_UP, &StemHeaderWindow::OnLeftUp, this);
    Bind(wxEVT_MOTION, &StemHeaderWindow::OnMotion, this);
    Bind(wxEVT_LEFT_DCLICK, &StemHeaderWindow::OnLeftDClick, this);
    Bind(wxEVT_RIGHT_DOWN, &StemHeaderWindow::OnRightDown, this);
}

void StemHeaderWindow::OnPaint(wxPaintEvent& event)
{
    wxAutoBufferedPaintDC dc(this);
    wxCoord w, h;
    dc.GetSize(&w, &h);

    bool isDark = IsDarkMode();
    xlColor outlineCol(32, 32, 32);
    if (isDark) outlineCol.Set(55, 55, 55);
    wxPen penOutline(xlColorToWxColour(outlineCol));

    // Font matching RowHeading
    auto font = dc.GetFont();
    auto fontSize = ComputeStemFontSize();
    SetStemFontPixelSize(font, fontSize);
    dc.SetFont(font);

    int scrollOffset = _stemsPanel->GetScrollOffset();
    int rowHeight = _stemsPanel->GetStemRowHeight();
    int stemCount = _stemsPanel->GetStemCount();
    int dragSource = _stemsPanel->GetDragSourceIndex();
    int dragInsert = _stemsPanel->GetDragInsertIndex();
    bool dragging = _stemsPanel->IsDragging();

    // Background
    xlColor bgColor = xlColor(60, 60, 65);
    dc.SetBrush(wxBrush(xlColorToWxColour(bgColor)));
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(0, 0, w, h);

    if (stemCount == 0) {
        // Empty state hint
        dc.SetTextForeground(wxColour(120, 120, 120));
        wxFont hintFont = dc.GetFont();
        hintFont.SetPointSize(9);
        dc.SetFont(hintFont);
        wxRect r(DEFAULT_ROW_HEADING_MARGIN, 0, w - DEFAULT_ROW_HEADING_MARGIN, h);
        dc.DrawLabel("Right-click to import", r, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT);
        return;
    }

    int activeStem = _stemsPanel->GetActiveStemIndex();

    for (int i = 0; i < stemCount; i++) {
        StemInfo* stem = _stemsPanel->GetStem(i);
        if (!stem) continue;

        int startY = i * rowHeight - scrollOffset;
        if (startY + rowHeight < 0) continue;
        if (startY > h) break;

        // Stem color stripe on left edge (3px)
        wxColour stemColor(stem->color.red, stem->color.green, stem->color.blue);
        dc.SetBrush(wxBrush(stemColor));
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.DrawRectangle(0, startY, 4, rowHeight);

        // Row background
        xlColor rowBg(48, 48, 52);
        if (dragging && i == dragSource) {
            rowBg.Set(35, 35, 40); // dimmed source row
        }
        dc.SetBrush(wxBrush(xlColorToWxColour(rowBg)));
        dc.SetPen(penOutline);
        dc.DrawRectangle(4, startY, w - 4, rowHeight);

        // Grab handle dots (6px from left edge, centered vertically)
        int handleX = 8;
        int cy = startY + rowHeight / 2;
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.SetBrush(wxBrush(wxColour(100, 100, 105)));
        for (int dot = -1; dot <= 1; dot++) {
            dc.DrawCircle(handleX, cy + dot * 4, 1);
            dc.DrawCircle(handleX + 4, cy + dot * 4, 1);
        }

        // Solo (S) button on the right edge — DAW-style. Active = filled yellow.
        wxRect soloRect = StemHeaderWindow::SoloButtonRect(w, startY, rowHeight);
        bool soloActive = (i == activeStem);
        wxColour soloFill = soloActive ? wxColour(240, 200, 40) : wxColour(70, 70, 75);
        wxColour soloText = soloActive ? wxColour(20, 20, 20) : wxColour(200, 200, 200);
        dc.SetBrush(wxBrush(soloFill));
        dc.SetPen(wxPen(wxColour(30, 30, 30)));
        dc.DrawRoundedRectangle(soloRect, 2);
        dc.SetTextForeground(soloText);
        wxFont btnFont = dc.GetFont();
        btnFont.SetWeight(wxFONTWEIGHT_BOLD);
        dc.SetFont(btnFont);
        dc.DrawLabel("S", soloRect, wxALIGN_CENTER);

        // Text — same margin as RowHeading
        dc.SetTextForeground(*wxWHITE);
        // Reset to stem font (non-bold)
        auto f2 = dc.GetFont();
        f2.SetWeight(wxFONTWEIGHT_NORMAL);
        SetStemFontPixelSize(f2, fontSize);
        dc.SetFont(f2);

        int textRight = soloRect.GetX() - 4;
        wxRect textRect(DEFAULT_ROW_HEADING_MARGIN, startY,
                        std::max(0, textRight - DEFAULT_ROW_HEADING_MARGIN), rowHeight);
        dc.DrawLabel(stem->name, textRect, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT);
    }

    // Drag insert indicator line
    if (dragging && dragInsert >= 0) {
        int lineY = dragInsert * rowHeight - scrollOffset;
        if (dragInsert > dragSource) lineY += rowHeight;
        dc.SetPen(wxPen(wxColour(255, 180, 0), 2));
        dc.DrawLine(0, lineY, w, lineY);
    }
}

void StemHeaderWindow::OnLeftDown(wxMouseEvent& event)
{
    int idx = _stemsPanel->HitTestStemIndex(event.GetY());
    if (idx >= 0) {
        int rowHeight = _stemsPanel->GetStemRowHeight();
        int startY = idx * rowHeight - _stemsPanel->GetScrollOffset();
        wxCoord w, h;
        GetClientSize(&w, &h);
        wxRect soloRect = SoloButtonRect(w, startY, rowHeight);
        if (soloRect.Contains(event.GetX(), event.GetY())) {
            _stemsPanel->SoloStem(idx);
            return;  // do NOT start drag-reorder
        }
    }
    _mouseDown = true;
    _mouseDownY = event.GetY();
    _mouseDownIndex = idx;
    CaptureMouse();
}

void StemHeaderWindow::OnLeftUp(wxMouseEvent& event)
{
    if (HasCapture()) ReleaseMouse();

    if (_stemsPanel->IsDragging()) {
        _stemsPanel->EndDragReorder();
    }

    _mouseDown = false;
    _mouseDownIndex = -1;
}

void StemHeaderWindow::OnMotion(wxMouseEvent& event)
{
    if (!_mouseDown) return;

    int dy = std::abs(event.GetY() - _mouseDownY);

    if (!_stemsPanel->IsDragging() && dy > 4 && _mouseDownIndex >= 0) {
        _stemsPanel->BeginDragReorder(_mouseDownIndex, event.GetY());
    }

    if (_stemsPanel->IsDragging()) {
        _stemsPanel->UpdateDragReorder(event.GetY());
    }
}

void StemHeaderWindow::OnLeftDClick(wxMouseEvent& event)
{
    int index = _stemsPanel->HitTestStemIndex(event.GetY());
    if (index >= 0) {
        _stemsPanel->OnStemHeaderDClick(index);
    }
}

void StemHeaderWindow::OnRightDown(wxMouseEvent& event)
{
    _stemsPanel->OnRightDown(event);
}

// =========================================================================
// StemResizeHandle — drag to resize stems area
// =========================================================================

StemResizeHandle::StemResizeHandle(wxWindow* parent, StemsPanel* stemsPanel)
    : wxWindow(parent, wxID_ANY, wxDefaultPosition, wxSize(-1, RESIZE_HANDLE_HEIGHT))
    , _stemsPanel(stemsPanel)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetCursor(wxCURSOR_SIZENS);

    Bind(wxEVT_PAINT, &StemResizeHandle::OnPaint, this);
    Bind(wxEVT_LEFT_DOWN, &StemResizeHandle::OnLeftDown, this);
    Bind(wxEVT_LEFT_UP, &StemResizeHandle::OnLeftUp, this);
    Bind(wxEVT_MOTION, &StemResizeHandle::OnMotion, this);
    Bind(wxEVT_LEFT_DCLICK, &StemResizeHandle::OnLeftDClick, this);
    Bind(wxEVT_ENTER_WINDOW, &StemResizeHandle::OnEnter, this);
    Bind(wxEVT_LEAVE_WINDOW, &StemResizeHandle::OnLeave, this);
}

void StemResizeHandle::OnPaint(wxPaintEvent& event)
{
    wxAutoBufferedPaintDC dc(this);
    wxCoord w, h;
    dc.GetSize(&w, &h);

    bool collapsed = _stemsPanel->IsCollapsed();

    // Dark bar with subtle grip indicator
    dc.SetBrush(wxBrush(wxColour(45, 45, 50)));
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(0, 0, w, h);

    int cx = w / 2;
    int cy = h / 2;

    if (collapsed) {
        // Show expand indicator: small triangle pointing down
        dc.SetBrush(wxBrush(wxColour(120, 120, 130)));
        dc.SetPen(*wxTRANSPARENT_PEN);
        wxPoint tri[3] = { {cx - 4, cy - 1}, {cx + 4, cy - 1}, {cx, cy + 2} };
        dc.DrawPolygon(3, tri);
    } else {
        // Center grip dots
        dc.SetBrush(wxBrush(wxColour(90, 90, 95)));
        for (int i = -2; i <= 2; i++) {
            dc.DrawCircle(cx + i * 6, cy, 1);
        }
    }
}

void StemResizeHandle::OnLeftDown(wxMouseEvent& event)
{
    _resizing = true;
    _resizeStartY = event.GetPosition().y + GetPosition().y;
    _resizeStartHeight = _stemsPanel->GetPanelHeight();
    CaptureMouse();
}

void StemResizeHandle::OnLeftUp(wxMouseEvent& event)
{
    if (HasCapture()) ReleaseMouse();
    _resizing = false;
}

void StemResizeHandle::OnMotion(wxMouseEvent& event)
{
    if (!_resizing) return;

    int currentY = event.GetPosition().y + GetPosition().y;
    int delta = currentY - _resizeStartY;
    int newHeight = _resizeStartHeight + delta;
    _stemsPanel->SetPanelHeight(newHeight);
}

void StemResizeHandle::OnLeftDClick(wxMouseEvent& event)
{
    _stemsPanel->ToggleCollapsed();
}

void StemResizeHandle::OnEnter(wxMouseEvent& event)
{
    SetCursor(wxCURSOR_SIZENS);
}

void StemResizeHandle::OnLeave(wxMouseEvent& event)
{
    SetCursor(wxCURSOR_DEFAULT);
}
