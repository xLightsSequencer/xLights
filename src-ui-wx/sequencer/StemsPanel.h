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

#include <wx/wx.h>
#include <wx/xml/xml.h>
#include <vector>
#include <string>
#include <memory>

#include "Color.h"

class StemWaveform;
class TimeLine;
class StemHeaderWindow;
class StemResizeHandle;

wxDECLARE_EVENT(EVT_STEMS_CHANGED, wxCommandEvent);

struct StemInfo {
    std::string name;
    std::string path;
    xlColor color;
    StemWaveform* waveform = nullptr;
    int altTrackIdx = -1;  // index into SequenceFile::alt_tracks; kept aligned with _stems index
};

class StemsPanel : public wxPanel
{
public:
    StemsPanel(wxWindow* parent, wxWindowID id = wxID_ANY,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize);
    virtual ~StemsPanel();

    void SetTimeline(TimeLine* timeline) { _timeline = timeline; }

    // Sub-panels for split layout (children of parent, not of this panel)
    wxWindow* GetStemHeadersPanel() { return _stemHeadersOuter; }
    wxWindow* GetStemWaveformsPanel() { return _stemWaveformsOuter; }
    StemResizeHandle* GetResizeHandle() { return _resizeHandle; }

    // Stem management
    bool AddStem(const std::string& name, const std::string& filepath, const xlColor& color);
    void RemoveStem(int index);
    void RemoveAllStems();
    // Tear down UI rows without mutating SequenceFile::alt_tracks. Used on
    // sequence-open to discard stale rows from the previous sequence.
    void ClearRowsUiOnly();
    void MoveStemUp(int index);
    void MoveStemDown(int index);
    int GetStemCount() const { return (int)_stems.size(); }
    StemInfo* GetStem(int index);

    // Rename/recolor
    void RenameStem(int index, const std::string& name);
    void RecolorStem(int index, const xlColor& color);

    // Import
    void ImportStemFiles();
    void ImportStemFolder();

    // Row height
    void SetStemRowHeight(int height);
    int GetStemRowHeight() const { return _stemRowHeight; }

    // Panel height (the visible container area)
    void SetPanelHeight(int height);
    int GetPanelHeight() const { return _panelHeight; }

    // Collapse / expand
    void SetCollapsed(bool collapsed);
    bool IsCollapsed() const { return _collapsed; }
    void ToggleCollapsed();

    // Sync header width with row headings
    void SetHeaderWidth(int width);

    // Visibility management
    void SetUserVisible(bool visible);
    bool IsUserVisible() const { return _userVisible; }
    void UpdateVisibility();
    bool IsStemsVisible() const { return _userVisible && !_stems.empty(); }

    // Scroll offset sync between headers and waveforms
    void SetScrollOffset(int offset);
    int GetScrollOffset() const { return _scrollOffset; }
    int GetTotalContentHeight() const;

    // Sync with main timeline
    void SetZoomLevel(int level);
    void SetStartPixelOffset(int offset);
    void SetTimeFrequency(int frequency);
    void UpdatePlayMarker();
    void ForceRedraw();

    // XML persistence (UI overlay: colors, row height, panel state). Audio paths
    // live in SequenceFile::alt_tracks. LoadFromXml also migrates legacy <Stems>
    // nodes (which embedded audio paths) into alt_tracks.
    bool LoadFromXml(wxXmlNode* stemsNode, const wxString& showDir);
    wxXmlNode* SaveToXml(const wxString& showDir) const;

    // Rebuild rows from SequenceFile::alt_tracks, creating/destroying StemWaveforms
    // as needed and preserving UI state (color, waveform pointer) where possible.
    void RefreshFromAltTracks();

    // Solo: switch the main waveform's active track to this stem (alt-track index + 1).
    // Click again on the active stem to switch back to Main.
    void SoloStem(int stemIndex);
    int GetActiveStemIndex() const;  // -1 if Main is active

    // Show directory for relative path resolution
    void SetShowDirectory(const std::string& dir) { _showDir = dir; }

    // Drag reorder support
    void BeginDragReorder(int stemIndex, int mouseY);
    void UpdateDragReorder(int mouseY);
    void EndDragReorder();
    int GetDragSourceIndex() const { return _dragSourceIndex; }
    int GetDragInsertIndex() const { return _dragInsertIndex; }
    bool IsDragging() const { return _dragging; }

private:
    void RebuildLayout();
    void OnRightDown(wxMouseEvent& event);
    void OnPopupMenu(wxCommandEvent& event);
    void OnStemHeaderDClick(int stemIndex);
    void OnMouseWheel(wxMouseEvent& event);
    int HitTestStemIndex(int y);
    std::string MakeRelativePath(const std::string& absPath, const std::string& showDir) const;
    std::string ResolveRelativePath(const std::string& relPath, const std::string& showDir) const;
    xlColor GetDefaultStemColor(int index) const;
    void SyncScrollPositions();

    // Outer containers (children of MainSequencer, placed in grid sizer)
    wxPanel* _stemHeadersOuter = nullptr;
    wxPanel* _stemWaveformsOuter = nullptr;
    StemResizeHandle* _resizeHandle = nullptr;

    // Inner scrolling content
    StemHeaderWindow* _headerWindow = nullptr;
    wxPanel* _waveformsInner = nullptr;
    wxBoxSizer* _waveformsSizer = nullptr;

    TimeLine* _timeline = nullptr;
    std::vector<StemInfo> _stems;

    std::string _showDir;
    int _stemRowHeight = 32;
    int _panelHeight = 96;
    int _headerWidth = 158;
    bool _userVisible = false;
    bool _collapsed = false;
    int _scrollOffset = 0;
    int _zoomLevel = 0;
    int _startPixelOffset = 0;
    int _frequency = 40;

    // Drag reorder state
    bool _dragging = false;
    int _dragSourceIndex = -1;
    int _dragInsertIndex = -1;
    int _dragStartY = 0;

    static const long ID_MNU_IMPORT_FILES;
    static const long ID_MNU_IMPORT_FOLDER;
    static const long ID_MNU_REMOVE_STEM;
    static const long ID_MNU_REMOVE_ALL;
    static const long ID_MNU_MOVE_UP;
    static const long ID_MNU_MOVE_DOWN;
    static const long ID_MNU_RENAME;
    static const long ID_MNU_RECOLOR;
    static const long ID_MNU_SET_HEIGHT;
    static const long ID_MNU_ONSET_DETECT;

    int _contextMenuStemIndex = -1;

    friend class StemHeaderWindow;

    DECLARE_EVENT_TABLE()
};

// Custom-drawn stem header area (matches RowHeading style)
class StemHeaderWindow : public wxWindow
{
public:
    StemHeaderWindow(wxWindow* parent, StemsPanel* stemsPanel);

    // Geometry of the per-row Solo (S) button. Caller passes the row's startY
    // (already adjusted for scroll), rowHeight, and the header window width.
    static wxRect SoloButtonRect(int headerW, int startY, int rowHeight);

private:
    void OnPaint(wxPaintEvent& event);
    void OnLeftDown(wxMouseEvent& event);
    void OnLeftUp(wxMouseEvent& event);
    void OnMotion(wxMouseEvent& event);
    void OnLeftDClick(wxMouseEvent& event);
    void OnRightDown(wxMouseEvent& event);

    StemsPanel* _stemsPanel;
    bool _mouseDown = false;
    int _mouseDownY = 0;
    int _mouseDownIndex = -1;
};

// Drag handle between stems row and effects grid row
class StemResizeHandle : public wxWindow
{
public:
    StemResizeHandle(wxWindow* parent, StemsPanel* stemsPanel);

private:
    void OnPaint(wxPaintEvent& event);
    void OnLeftDown(wxMouseEvent& event);
    void OnLeftUp(wxMouseEvent& event);
    void OnMotion(wxMouseEvent& event);
    void OnLeftDClick(wxMouseEvent& event);
    void OnEnter(wxMouseEvent& event);
    void OnLeave(wxMouseEvent& event);

    StemsPanel* _stemsPanel;
    bool _resizing = false;
    int _resizeStartY = 0;
    int _resizeStartHeight = 0;
};
