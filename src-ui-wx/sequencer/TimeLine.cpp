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
#include <wx/brush.h>
#include <wx/textdlg.h>
#include <wx/colordlg.h>
#include <wx/dcbuffer.h>

#include "TimeLine.h"
#include "Waveform.h"
#include "MainSequencer.h"
#include "EffectsGrid.h"
#include "render/SequenceElements.h"
#include "render/SongStructureManager.h"
#include "render/RenderUtils.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/UndoManager.h"
#include "xLightsMain.h"
#include "shared/utils/wxUtilities.h"
#include <log.h>

#include <functional>

// ---------------------------------------------------------------
// SongRegionEditDialog - Combined name + color swatch dialog
// ---------------------------------------------------------------

class ColorSwatchPanel : public wxPanel
{
public:
    ColorSwatchPanel(wxWindow* parent, uint32_t colorARGB, int index, std::function<void(int)> onSelect)
        : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(28, 28), wxFULL_REPAINT_ON_RESIZE)
        , _colorARGB(colorARGB)
        , _index(index)
        , _onSelect(std::move(onSelect))
    {
        SetMinSize(wxSize(28, 28));
        SetMaxSize(wxSize(28, 28));
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        SetCursor(wxCursor(wxCURSOR_HAND));

        Bind(wxEVT_PAINT, &ColorSwatchPanel::OnPaint, this);
        Bind(wxEVT_LEFT_UP, &ColorSwatchPanel::OnClick, this);
    }

    void SetSelected(bool sel) { _selected = sel; Refresh(); }
    bool IsSelected() const { return _selected; }
    uint32_t GetColorARGB() const { return _colorARGB; }

private:
    void OnPaint(wxPaintEvent& event)
    {
        wxAutoBufferedPaintDC dc(this);
        wxSize sz = GetClientSize();

        uint8_t r = (_colorARGB >> 16) & 0xFF;
        uint8_t g = (_colorARGB >> 8) & 0xFF;
        uint8_t b = _colorARGB & 0xFF;

        dc.SetBrush(wxBrush(wxColour(r, g, b)));
        if (_selected) {
            dc.SetPen(wxPen(wxColour(255, 255, 255), 2));
        } else {
            dc.SetPen(wxPen(wxColour(80, 80, 80), 1));
        }
        dc.DrawRoundedRectangle(1, 1, sz.x - 2, sz.y - 2, 4);
    }

    void OnClick(wxMouseEvent& event)
    {
        if (_onSelect) _onSelect(_index);
    }

    uint32_t _colorARGB;
    int _index;
    bool _selected = false;
    std::function<void(int)> _onSelect;
};

class SongRegionEditDialog : public wxDialog
{
public:
    SongRegionEditDialog(wxWindow* parent, const wxString& name, uint32_t currentColorARGB)
        : wxDialog(parent, wxID_ANY, "Edit Region", wxDefaultPosition, wxDefaultSize,
                   wxDEFAULT_DIALOG_STYLE)
        , _selectedColorARGB(currentColorARGB)
    {
        wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

        // Name input
        wxStaticText* nameLabel = new wxStaticText(this, wxID_ANY, "Region Name:");
        mainSizer->Add(nameLabel, 0, wxLEFT | wxRIGHT | wxTOP, 12);

        _nameCtrl = new wxTextCtrl(this, wxID_ANY, name, wxDefaultPosition, wxSize(280, -1));
        mainSizer->Add(_nameCtrl, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 12);

        // Color swatches
        wxStaticText* colorLabel = new wxStaticText(this, wxID_ANY, "Color:");
        mainSizer->Add(colorLabel, 0, wxLEFT | wxRIGHT | wxTOP, 12);

        wxBoxSizer* swatchSizer = new wxBoxSizer(wxHORIZONTAL);

        for (int i = 0; i < SongStructureManager::PALETTE_SIZE; i++) {
            uint32_t paletteColor = SongStructureManager::GetPaletteColor(i);
            ColorSwatchPanel* swatch = new ColorSwatchPanel(this, paletteColor, i,
                [this](int idx) { SelectSwatch(idx); });
            _swatches.push_back(swatch);
            swatchSizer->Add(swatch, 0, wxRIGHT, 4);

            // Pre-select if matching current color (compare RGB only, ignore alpha)
            if ((paletteColor & 0x00FFFFFF) == (currentColorARGB & 0x00FFFFFF)) {
                swatch->SetSelected(true);
                _selectedSwatchIndex = i;
            }
        }

        mainSizer->Add(swatchSizer, 0, wxLEFT | wxRIGHT | wxTOP, 12);

        // OK / Cancel buttons
        wxSizer* btnSizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL);
        mainSizer->Add(btnSizer, 0, wxALIGN_RIGHT | wxALL, 12);

        SetSizerAndFit(mainSizer);
        CentreOnParent();

        _nameCtrl->SetFocus();
        _nameCtrl->SelectAll();
    }

    wxString GetRegionName() const { return _nameCtrl->GetValue(); }
    uint32_t GetSelectedColorARGB() const { return _selectedColorARGB; }

private:
    void SelectSwatch(int index)
    {
        for (size_t i = 0; i < _swatches.size(); i++) {
            _swatches[i]->SetSelected((int)i == index);
        }
        _selectedSwatchIndex = index;
        _selectedColorARGB = _swatches[index]->GetColorARGB();
    }

    wxTextCtrl* _nameCtrl;
    std::vector<ColorSwatchPanel*> _swatches;
    int _selectedSwatchIndex = -1;
    uint32_t _selectedColorARGB;
};

const long TimeLine::ID_ZOOMSEL = wxNewId();
const long TimeLine::ID_RESETZOOM = wxNewId();
const long TimeLine::ID_SONG_ADD_BOUNDARY = wxNewId();
const long TimeLine::ID_SONG_DELETE_BOUNDARY = wxNewId();
const long TimeLine::ID_SONG_EDIT_REGION = wxNewId();
const long TimeLine::ID_SONG_CLEAR_STRUCTURE = wxNewId();
const long TimeLine::ID_SONG_COPY_EFFECTS_BASE = wxNewId();
namespace { struct _ReserveCopyIDs { _ReserveCopyIDs() { for (int i = 0; i < 49; i++) wxNewId(); } } _reserveCopyIDs; }
const long TimeLine::ID_SONG_APPLY_PALETTE = wxNewId();
const long TimeLine::ID_SONG_EXPORT_REGION = wxNewId();
const long TimeLine::ID_SONG_EXPORT_ALL_REGIONS = wxNewId();
const long TimeLine::ID_SONG_VIEW_BASE = wxNewId();
namespace { struct _ReserveViewIDs { _ReserveViewIDs() { for (int i = 0; i < 49; i++) wxNewId(); } } _reserveViewIDs; }
const long TimeLine::ID_SONG_VIEW_NEW = wxNewId();
const long TimeLine::ID_SONG_VIEW_DUPLICATE = wxNewId();
const long TimeLine::ID_SONG_VIEW_RENAME = wxNewId();
const long TimeLine::ID_SONG_VIEW_DELETE = wxNewId();

wxDEFINE_EVENT(EVT_TIME_LINE_CHANGED, wxCommandEvent);
wxDEFINE_EVENT(EVT_SEQUENCE_CHANGED, wxCommandEvent);

BEGIN_EVENT_TABLE(TimeLine, wxWindow)
EVT_MOTION(TimeLine::mouseMoved)
EVT_LEFT_DOWN(TimeLine::mouseLeftDown)
EVT_LEFT_UP(TimeLine::mouseLeftUp)
EVT_MOUSE_CAPTURE_LOST(TimeLine::OnLostMouseCapture)
EVT_PAINT(TimeLine::Paint)
END_EVENT_TABLE()

// These values are used to calculate zoom
// 1    =   1*(1/frequency) per major tick on the scale
// 200  = 200*(1/frequency) per major tick on the scale
// etc....
const int TimeLine::ZoomLevelValues[] = {1,2,4,10,20,40,100,200,400,600,1200,2400,4800,8000,12000,16000,20000,30000,40000};
#define MAX_ZOOM_OUT_INDEX      18

static const int marker_size = 8;

void TimeLine::OnLostMouseCapture(wxMouseCaptureLostEvent& event)
{
    m_dragging = false;
    mDraggingBoundary = false;
    mDragBoundaryTimeMS = -1;
}

void TimeLine::mouseRightDown(wxMouseEvent& event)
{
    _rightClickPosition = GetAbsoluteTimeMSfromPosition(GetPositionFromSelection(event.GetX()));

    if (_rightClickPosition > GetTimeLength()) return;

    wxMenu mnuLayer;
    if (mSelectedPlayMarkerEndMS != mSelectedPlayMarkerStartMS) {
        mnuLayer.Append(ID_ZOOMSEL, "Zoom to Selection");
    }
    mnuLayer.Append(ID_RESETZOOM, "Reset Zoom");
    for (int i = 0; i < 10; ++i)
    {
        auto mnu = mnuLayer.AppendCheckItem(i+1, wxString::Format("%i", i));
        if (_tagPositions[i] != -1)
        {
            mnu->Check();
        }
    }

    mnuLayer.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&TimeLine::OnPopup, nullptr, this);

    if (GetTagCount() > 0)
    {
        wxMenu *mnuDelete = new wxMenu();

        if (GetTagCount() > 1)
        {
            mnuDelete->Append(200, "All");
        }

        for (int i = 0; i < 10; ++i)
        {
            if (_tagPositions[i] != -1)
            {
                mnuDelete->Append(100 + i + 1, wxString::Format("%i", i));
            }
        }

        mnuLayer.AppendSubMenu(mnuDelete, "Delete");
        mnuDelete->Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&TimeLine::OnPopup, nullptr, this);
    }

    // Song structure menu items
    if (_sequenceElements != nullptr) {
        mnuLayer.AppendSeparator();
        SongStructureManager& ssm = _sequenceElements->GetSongStructureManager();

        mnuLayer.Append(ID_SONG_ADD_BOUNDARY, "Add Song Structure Boundary Here");
        mnuLayer.Connect(ID_SONG_ADD_BOUNDARY, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&TimeLine::OnSongStructurePopup, nullptr, this);

        // Compute boundary hit tolerance in MS
        int toleranceMS = 0;
        if (mEndTimeMS > mStartTimeMS && GetSize().x > 0) {
            float msPerPixel = (float)(mEndTimeMS - mStartTimeMS) / (float)GetSize().x;
            toleranceMS = (int)(6.0f * msPerPixel);
        }
        int nearBoundary = ssm.FindNearestBoundary(_rightClickPosition, toleranceMS);

        if (nearBoundary >= 0) {
            mnuLayer.Append(ID_SONG_DELETE_BOUNDARY, "Delete Song Structure Boundary");
            mnuLayer.Connect(ID_SONG_DELETE_BOUNDARY, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&TimeLine::OnSongStructurePopup, nullptr, this);
        }

        int regionIdx = ssm.GetRegionIndexAtTime(_rightClickPosition);
        if (regionIdx >= 0) {
            wxString editLabel = wxString::Format("Edit Region \"%s\"...", wxString::FromUTF8(ssm.GetRegion(regionIdx).name));
            mnuLayer.Append(ID_SONG_EDIT_REGION, editLabel);
            mnuLayer.Connect(ID_SONG_EDIT_REGION, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&TimeLine::OnSongStructurePopup, nullptr, this);

            // "Copy Effects to Region >" submenu
            if (ssm.GetRegionCount() > 1) {
                wxMenu* copySubmenu = new wxMenu();
                for (size_t i = 0; i < ssm.GetRegionCount(); i++) {
                    if ((int)i == regionIdx) continue;
                    const SongStructureRegion& target = ssm.GetRegion(i);
                    int mins = target.startTimeMS / 60000;
                    int secs = (target.startTimeMS % 60000) / 1000;
                    wxString timeStr = mins > 0 ? wxString::Format("%d:%02d", mins, secs) : wxString::Format("0:%02d", secs);
                    wxString label = wxString::Format("%s (%s)", wxString::FromUTF8(target.name), timeStr);
                    copySubmenu->Append(ID_SONG_COPY_EFFECTS_BASE + (int)i, label);
                }
                mnuLayer.AppendSubMenu(copySubmenu, "Copy Effects to Region");
                copySubmenu->Connect(wxEVT_COMMAND_MENU_SELECTED,
                    (wxObjectEventFunction)&TimeLine::OnSongStructurePopup, nullptr, this);
            }

            // "Apply Selected Effect Palette to Region"
            MainSequencer* msCheck = dynamic_cast<MainSequencer*>(GetParent());
            bool hasSelection = (msCheck && msCheck->PanelEffectGrid &&
                                 msCheck->PanelEffectGrid->GetSelectedEffect() != nullptr);
            wxMenuItem* paletteItem = mnuLayer.Append(ID_SONG_APPLY_PALETTE,
                "Apply Selected Effect Palette to Region");
            paletteItem->Enable(hasSelection);
            mnuLayer.Connect(ID_SONG_APPLY_PALETTE, wxEVT_COMMAND_MENU_SELECTED,
                (wxObjectEventFunction)&TimeLine::OnSongStructurePopup, nullptr, this);

            // "Export Song Region as New Sequence"
            mnuLayer.Append(ID_SONG_EXPORT_REGION, "Export Song Region as New Sequence");
            mnuLayer.Connect(ID_SONG_EXPORT_REGION, wxEVT_COMMAND_MENU_SELECTED,
                (wxObjectEventFunction)&TimeLine::OnSongStructurePopup, nullptr, this);
        }

        if (ssm.HasRegions()) {
            mnuLayer.Append(ID_SONG_EXPORT_ALL_REGIONS, "Export All Song Regions as Sequences");
            mnuLayer.Connect(ID_SONG_EXPORT_ALL_REGIONS, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&TimeLine::OnSongStructurePopup, nullptr, this);
            mnuLayer.AppendSeparator();
            mnuLayer.Append(ID_SONG_CLEAR_STRUCTURE, "Clear Song Structure");
            mnuLayer.Connect(ID_SONG_CLEAR_STRUCTURE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&TimeLine::OnSongStructurePopup, nullptr, this);
        }

        // Song Structure Views submenu
        if (ssm.GetViewCount() > 0 || ssm.HasRegions()) {
            wxMenu* viewSubmenu = new wxMenu();

            for (size_t i = 0; i < ssm.GetViewCount(); i++) {
                wxMenuItem* item = viewSubmenu->AppendRadioItem(
                    ID_SONG_VIEW_BASE + (int)i, ssm.GetViewName(i));
                if ((int)i == ssm.GetActiveViewIndex()) {
                    item->Check(true);
                }
            }

            if (ssm.GetViewCount() > 0) {
                viewSubmenu->AppendSeparator();
            }

            viewSubmenu->Append(ID_SONG_VIEW_NEW, "New View...");
            viewSubmenu->Append(ID_SONG_VIEW_DUPLICATE, "Duplicate Current View...");

            if (ssm.GetViewCount() > 0) {
                viewSubmenu->Append(ID_SONG_VIEW_RENAME, "Rename Current View...");
                wxMenuItem* delItem = viewSubmenu->Append(ID_SONG_VIEW_DELETE, "Delete Current View");
                if (ssm.GetViewCount() <= 1) {
                    delItem->Enable(false);
                }
            }

            viewSubmenu->Connect(wxEVT_COMMAND_MENU_SELECTED,
                (wxObjectEventFunction)&TimeLine::OnSongStructurePopup, nullptr, this);

            mnuLayer.AppendSubMenu(viewSubmenu, "Song Structure Views");
        }
    }

    PopupMenu(&mnuLayer);
}

void TimeLine::OnPopup(wxCommandEvent& event)
{
    int id = event.GetId() - 1;

    if (id + 1 == ID_ZOOMSEL) {
        ZoomSelection();
    }
    else if (id + 1 == ID_RESETZOOM) {
        int maxZoom = GetMaxZoomLevel();
        SetZoomLevel(maxZoom);
    }
    else if (id == 199)
    {
        ClearTags();
        RaiseSequenceChange();
    }
    else if (id >= 100)
    {
        id -= 100;
        SetTagPosition(id, -1);
    }
    else
    {
        SetTagPosition(id, _rightClickPosition);
    }
}

void TimeLine::SetTagPosition(int tag, int position , bool flag)
{
    if (GetTimeLength() != -1 && position > GetTimeLength()) {
        position = GetTimeLength();
    }

    if (_tagPositions[tag] != position) {
        _tagPositions[tag] = position;
        if (_sequenceElements) {
            _sequenceElements->SetTagPosition(tag, position);
        }
        if (flag) {
            Refresh(false);
            RaiseSequenceChange();
        }
    }
}

void TimeLine::ClearTags()
{
    for (int i = 0; i < 10; ++i)
    {
        _tagPositions[i] = -1;
    }
    Refresh(false);
}

void TimeLine::SyncTagsFrom(const SequenceElements& elements)
{
    for (int i = 0; i < 10; ++i) {
        _tagPositions[i] = elements.GetTagPosition(i);
    }
    Refresh(false);
}

void TimeLine::SyncTagsTo(SequenceElements& elements) const
{
    for (int i = 0; i < 10; ++i) {
        elements.SetTagPosition(i, _tagPositions[i]);
    }
}

int TimeLine::GetTagCount()
{
    int count = 0;
    for (int i = 0; i < 10; ++i)
    {
        if (_tagPositions[i] != -1)
        {
            count++;
        }
    }
    return count;
}

void TimeLine::GoToNextTag()
{
    int pos = GetStartTimeMS();
    int end = GetTimeLength();

    int next = end;

    for (const auto& it : _tagPositions) {
        if (it != -1) {
            if (it > pos && it < next) {
                next = it;
            }
        }
    }

    SetStartTimeMS(next);
    RaiseChangeTimeline();
}

void TimeLine::GoToPriorTag()
{
    int pos = GetStartTimeMS();
    int prior = 0;

    for (const auto& it : _tagPositions) {
        if (it != -1) {
            if (it < pos && it > prior) {
                prior = it;
            }
        }
    }

    SetStartTimeMS(prior);
    RaiseChangeTimeline();
}

void TimeLine::GoToTag(int tag)
{
    int pos = GetTagPosition(tag);
    if (pos != -1)
    {
        SetStartTimeMS(pos);
        RaiseChangeTimeline();
    }
}

int TimeLine::GetNextTag(int pos) {
    int end = GetTimeLength();
    int next = end;

    for (const auto& it : _tagPositions) {
        if (it != -1) {
            if (it > pos && it < next) {
                next = it;
            }
        }
    }

    return next;
}

int TimeLine::GetPriorTag(int pos) {
    int prior = 0;

    for (const auto& it : _tagPositions) {
        if (it != -1) {
            if (it < pos && it > prior) {
                prior = it;
            }
        }
    }

    return prior;
}

int TimeLine::GetTagPosition(int tag)
{
    // update it if it is outside the sequence
    SetTagPosition(tag, _tagPositions[tag]);

    return _tagPositions[tag];
}

void TimeLine::mouseLeftDown( wxMouseEvent& event)
{
    // Check for song structure boundary interaction
    if (_sequenceElements != nullptr) {
        SongStructureManager& ssm = _sequenceElements->GetSongStructureManager();

        // Option+click adds a boundary
        if (event.AltDown()) {
            int clickTimeMS = GetAbsoluteTimeMSfromPosition(event.GetX());
            ssm.AddBoundary(clickTimeMS, mSequenceEndMarkerMS);
            RaiseSequenceChange();
            Refresh(false);
            return;
        }

        // Check for boundary drag
        int boundaryTime = HitTestBoundary(event.GetX());
        if (boundaryTime >= 0) {
            mDraggingBoundary = true;
            mDragBoundaryTimeMS = boundaryTime;
            if (!m_dragging) {
                CaptureMouse();
                m_dragging = true;
            }
            SetFocus();
            return;
        }
    }

    mCurrentPlayMarkerStart = GetPositionFromSelection(event.GetX());
    mCurrentPlayMarkerStartMS = GetAbsoluteTimeMSfromPosition(mCurrentPlayMarkerStart);
    mCurrentPlayMarkerEnd = -1;
    mCurrentPlayMarkerEndMS = -1;
    if( !m_dragging )
    {
        m_dragging = true;
        CaptureMouse();
    }
    SetFocus();
    Refresh(false);
}

void TimeLine::mouseMoved( wxMouseEvent& event)
{
    if (mDraggingBoundary && _sequenceElements != nullptr) {
        int newTimeMS = GetAbsoluteTimeMSfromPosition(event.GetX());
        newTimeMS = RoundToMultipleOfPeriod(newTimeMS, mFrequency);
        SongStructureManager& ssm = _sequenceElements->GetSongStructureManager();
        ssm.MoveBoundary(mDragBoundaryTimeMS, newTimeMS);
        // Update the drag reference to the new boundary position
        int nearBoundary = ssm.FindNearestBoundary(newTimeMS, 1000);
        if (nearBoundary >= 0) {
            mDragBoundaryTimeMS = nearBoundary;
        }
        Refresh(false);
        // Also refresh the effects grid so region overlays update
        MainSequencer* ms = dynamic_cast<MainSequencer*>(GetParent());
        if (ms && ms->PanelEffectGrid) {
            ms->PanelEffectGrid->Refresh(false);
        }
        return;
    }

    // Update cursor for boundary hover
    if (!m_dragging && _sequenceElements != nullptr) {
        int boundaryTime = HitTestBoundary(event.GetX());
        if (boundaryTime >= 0) {
            SetCursor(wxCursor(wxCURSOR_SIZEWE));
        } else {
            SetCursor(wxNullCursor);
        }
    }

    if( m_dragging ) {
        mCurrentPlayMarkerEndMS = GetAbsoluteTimeMSfromPosition(event.GetX());
        if (mCurrentPlayMarkerEndMS < mStartTimeMS ) {
            mCurrentPlayMarkerEndMS = mStartTimeMS;
        }
        mCurrentPlayMarkerEnd = GetPositionFromTimeMS(mCurrentPlayMarkerEndMS);
        Refresh(false);
    }
}

void TimeLine::mouseLeftUp( wxMouseEvent& event)
{
    if (mDraggingBoundary) {
        mDraggingBoundary = false;
        mDragBoundaryTimeMS = -1;
        if (m_dragging) {
            ReleaseMouse();
            m_dragging = false;
        }
        RaiseSequenceChange();
        Refresh(false);
        return;
    }

    triggerPlay();
    if(m_dragging)
    {
        ReleaseMouse();
        m_dragging = false;
    }
    Refresh(false);
}

void TimeLine::triggerPlay()
{
    timeline_initiated_play = true;
    if( mCurrentPlayMarkerEndMS != -1 && mCurrentPlayMarkerStartMS > mCurrentPlayMarkerEndMS )
    {
        std::swap(mCurrentPlayMarkerStart, mCurrentPlayMarkerEnd);
        std::swap(mCurrentPlayMarkerStartMS, mCurrentPlayMarkerEndMS);
    }
    wxCommandEvent playEvent(EVT_PLAY_SEQUENCE);
    wxPostEvent(this, playEvent);
}

int TimeLine::GetPositionFromSelection(int position)
{
    int time = GetAbsoluteTimeMSfromPosition(position);
    time = RoundToMultipleOfPeriod(time,mFrequency);     // Round to nearest period
    return GetPositionFromTimeMS(time); // Recalulate Position with corrected time
}

TimeLine::TimeLine(wxPanel* parent, wxWindowID id, const wxPoint &pos, const wxSize &size,long style, const wxString &name):
                   wxWindow((wxWindow*)parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
{
    
    spdlog::debug("                Creating Timeline");
    spdlog::info("If xLights crashes after this log message then the root cause is almost always a problem between OpenGL and their video drivers.\nWe recommend they download the latest drivers from their card provider ... not from their operating system provider.");

    _savedPosition = -1;
    mParent = (wxPanel*)parent;
    DOUBLE_BUFFER(this);
    mIsInitialized = false;
    mFrequency = 40;
    mZoomMarkerMS = -1;
    mStartPixelOffset = 0;
    mZoomLevel = 0;
    mStartTimeMS = 0;
    mEndTimeMS = 0;
    mMousePosition = -1;
    mCurrentPlayMarkerStart = -1;
    mCurrentPlayMarkerEnd = -1;
    mSelectedPlayMarkerStart = -1;
    mSelectedPlayMarkerEnd = -1;
    mCurrentPlayMarker = -1;
    mCurrentPlayMarkerStartMS = -1;
    mCurrentPlayMarkerEndMS = -1;
    mSelectedPlayMarkerStartMS = -1;
    mSelectedPlayMarkerEndMS = -1;
    mCurrentPlayMarkerMS = -1;
    timeline_initiated_play = false;
    m_dragging = false;
    mTimeLength = -1;
    ClearTags();

    Connect(wxEVT_RIGHT_DOWN, (wxObjectEventFunction)&TimeLine::mouseRightDown, 0, this);
}

TimeLine::~TimeLine()
{
}

void TimeLine::CheckNeedToScrollToPlayStart(bool paused)
{
    int marker = paused ? mCurrentPlayMarker : mCurrentPlayMarkerStart;
    int StartTime;
    int EndTime;
    GetViewableTimeRange(StartTime, EndTime);
    int scroll_start = GetPositionFromTimeMS(StartTime);
    int scroll_end = GetPositionFromTimeMS(EndTime);
    if(marker < scroll_start || marker > scroll_end)
    {
        int markerMS = paused ? mCurrentPlayMarkerMS : mCurrentPlayMarkerStartMS;
        int new_start_time = markerMS - 1000;
        if( new_start_time < 0 )
        {
            new_start_time = 0;
        }
        SetStartTimeMS(new_start_time);
        RaiseChangeTimeline();
    }
}

void TimeLine::RaiseChangeTimeline()
{
    Refresh();
    Update();
    TimelineChangeArguments* tla = new TimelineChangeArguments(mZoomLevel, mStartPixelOffset, mCurrentPlayMarkerMS);
    wxCommandEvent eventTimeLineChanged(EVT_TIME_LINE_CHANGED);
    eventTimeLineChanged.SetClientData((void*)tla);
    eventTimeLineChanged.SetInt(0);
    wxPostEvent(mParent, eventTimeLineChanged);
}

void TimeLine::RaiseSequenceChange() const
{
    wxCommandEvent eventSequenceChanged(EVT_SEQUENCE_CHANGED);
    wxPostEvent(mParent, eventSequenceChanged);
}

void TimeLine::SetSequenceEnd(int ms)
{
    mSequenceEndMarkerMS = ms;
    mSequenceEndMarker = GetPositionFromTimeMS(ms);
    mEndPos = GetPositionFromTimeMS(mEndTimeMS);
    Refresh();
    Update();
}

bool TimeLine::SetPlayMarkerMS(int ms)
{
    mCurrentPlayMarkerMS = ms;
    int oldmCurrentPlayMarker = mCurrentPlayMarker;
    bool changed = false;
    if (ms < mStartTimeMS) {
        if (mCurrentPlayMarker != -1) {
            changed = true;
        }
        mCurrentPlayMarker = -1;
    } else {
        int i = GetPositionFromTimeMS(ms);
        if (mCurrentPlayMarker != i) {
            changed = true;
        }
        mCurrentPlayMarker = i;
    }
    if (changed) {
        wxRect rct(std::min(oldmCurrentPlayMarker - marker_size - 1, mCurrentPlayMarker - marker_size - 1), 0,
                   std::max(oldmCurrentPlayMarker + marker_size + 1, mCurrentPlayMarker + marker_size + 1), GetSize().GetHeight());
        RefreshRect(rct);
        Update();
    }
    return changed;
}

void TimeLine::SetZoomMarkerMS(int ms)
{
    mZoomMarkerMS = ms;
}

int TimeLine::GetPlayMarker() const
{
    return mCurrentPlayMarker;
}

void TimeLine::SetSelectedPositionStart(int pos, bool reset_end)
{
    mSelectedPlayMarkerStart = GetPositionFromSelection(pos);
    mSelectedPlayMarkerStartMS = GetAbsoluteTimeMSfromPosition(mSelectedPlayMarkerStart);
    if( reset_end )
    {
        mSelectedPlayMarkerEnd = -1;
        mSelectedPlayMarkerEndMS = -1;
    }
    mZoomMarkerMS = mSelectedPlayMarkerStartMS;
    Refresh(false);

    mCurrentPlayMarker = mSelectedPlayMarkerStart;
    mCurrentPlayMarkerMS = mSelectedPlayMarkerStartMS;

    // This draws the new start time
    RaiseChangeTimeline();
}

void TimeLine::SetSelectedPositionStartMS(int time)
{
    mSelectedPlayMarkerStartMS = time;
    mSelectedPlayMarkerStart = GetPositionFromTimeMS(mSelectedPlayMarkerStartMS);
    mSelectedPlayMarkerEnd = -1;
    mSelectedPlayMarkerEndMS = -1;
    mZoomMarkerMS = mSelectedPlayMarkerStartMS;
    Refresh(false);
}

void TimeLine::SetSelectedPositionEndMS(int time)
{
    mSelectedPlayMarkerEndMS = time;
    mSelectedPlayMarkerEnd = GetPositionFromTimeMS(mSelectedPlayMarkerEndMS);
    mZoomMarkerMS = mSelectedPlayMarkerStartMS;
    Refresh(false);
}

void TimeLine::SetSelectedPositionEnd(int pos)
{
    mSelectedPlayMarkerEndMS = GetAbsoluteTimeMSfromPosition(pos);
    if( mSelectedPlayMarkerEndMS < mStartTimeMS ) {
        mSelectedPlayMarkerEndMS = mStartTimeMS;
    }
    mSelectedPlayMarkerEnd = GetPositionFromTimeMS(mSelectedPlayMarkerEndMS);
    Refresh(false);
}

void TimeLine::SetMousePositionMS(int ms)
{
    mMousePositionMS = ms;
    if( ms < mStartTimeMS ) {
        mMousePosition = -1;
    } else {
        mMousePosition = GetPositionFromTimeMS(ms);
    }
    Refresh(false);
}

void TimeLine::SavePosition()
{
    _savedPosition = mStartTimeMS;
}

void TimeLine::RestorePosition()
{
    if (_savedPosition >= 0 && _savedPosition <= mTimeLength)
    {
        SetStartTimeMS(_savedPosition);
        RaiseChangeTimeline();
    }
}

void TimeLine::SetTimelinePosition(int pos)
{
    if (pos >= 0 && pos <= mTimeLength) {
        SetStartTimeMS(pos);
        RaiseChangeTimeline();
    }
}

void TimeLine::LatchSelectedPositions()
{
    if (mSelectedPlayMarkerEndMS != -1)
    {
        // if we have selected very few horizontal pixels then assume we were trying to click rather than select
        if (std::abs(mSelectedPlayMarkerStart - mSelectedPlayMarkerEnd) < 5)
        {
            mSelectedPlayMarkerEndMS = -1;
            mSelectedPlayMarkerEnd = -1;
        }
    }

    if( mSelectedPlayMarkerEndMS != -1 && mSelectedPlayMarkerStartMS > mSelectedPlayMarkerEndMS )
    {
        std::swap(mSelectedPlayMarkerStart, mSelectedPlayMarkerEnd);
        std::swap(mSelectedPlayMarkerStartMS, mSelectedPlayMarkerEndMS);
    }
}

// signal the start of play so timeline can adjust marks
void TimeLine::PlayStarted()
{
    if( !timeline_initiated_play )
    {
        mCurrentPlayMarkerStart = mSelectedPlayMarkerStart;
        mCurrentPlayMarkerEnd = mSelectedPlayMarkerEnd;
        mCurrentPlayMarkerStartMS = mSelectedPlayMarkerStartMS;
        mCurrentPlayMarkerEndMS = mSelectedPlayMarkerEndMS;
    }
    CheckNeedToScrollToPlayStart();
    timeline_initiated_play = false;
    Refresh(false);
}

// signal play stop so timeline can adjust marks
void TimeLine::PlayStopped()
{
    mCurrentPlayMarkerStart = -1;
    mCurrentPlayMarkerEnd = -1;
    mCurrentPlayMarker = -1;
    mCurrentPlayMarkerStartMS = -1;
    mCurrentPlayMarkerEndMS = -1;
    mCurrentPlayMarkerMS = -1;
    Refresh(false);
}

// return the time where to begin playing
int TimeLine::GetNewStartTimeMS() const
{
    int time = 0;
    if( timeline_initiated_play ) {
        time = mCurrentPlayMarkerStartMS;
    } else {
        if( mSelectedPlayMarkerStartMS > 0 ) {
            time = mSelectedPlayMarkerStartMS;
        }
    }
    return time;
}

// return the time where to end playing
int TimeLine::GetNewEndTimeMS() const
{
    int time = -1;
    if( timeline_initiated_play )
    {
        if( mCurrentPlayMarkerEndMS >= 0 )
        {
            time = mCurrentPlayMarkerEndMS;
        }
    }
    else
    {
        if( mSelectedPlayMarkerEndMS >= 0 )
        {
            time = mSelectedPlayMarkerEndMS;
        }
    }
    return time;
}

void TimeLine::SetStartTimeMS(int time)
{
    mStartTimeMS = time;
    mEndTimeMS = GetMaxViewableTimeMS();
    mStartPixelOffset = GetPixelOffsetFromStartTime();
    RecalcMarkerPositions();
}

float TimeLine::GetStartTimeMS() const
{
    return mStartTimeMS;
}

void TimeLine::SetStartPixelOffset(int offset)
{
    mStartPixelOffset = offset;
    mStartTimeMS = GetFirstTimeLabelFromPixelOffset(mStartPixelOffset);
    mEndTimeMS = GetMaxViewableTimeMS();
}

void TimeLine::ResetMarkers(int ms)
{
    mCurrentPlayMarkerEnd = -1;
    mCurrentPlayMarkerEndMS = -1;
    mSelectedPlayMarkerStartMS = ms;
    mSelectedPlayMarkerStart = GetPositionFromTimeMS(ms);
    mSelectedPlayMarkerEnd = -1;
    mSelectedPlayMarkerEndMS = -1;
    mCurrentPlayMarker = -1;
    mCurrentPlayMarkerMS = -1;
    mCurrentPlayMarkerStart = mSelectedPlayMarkerStart;
    mCurrentPlayMarkerStartMS = mSelectedPlayMarkerStartMS;
    mZoomMarkerMS = mStartTimeMS + (mEndTimeMS - mStartTimeMS)/2;
    mMousePositionMS = -1;
    mMousePosition = -1;
}

int TimeLine::GetStartPixelOffset()
{
    return  mStartPixelOffset;
}

float TimeLine::GetFirstTimeLabelFromPixelOffset(int offset) const
{
    if (offset == 0)
    {
        return 0;
    }
    else
    {
        return (float)((offset/PIXELS_PER_MAJOR_HASH)+1) * (float)ZoomLevelValues[mZoomLevel]/(float)(mFrequency);
    }
}

void TimeLine::SetTimeFrequency(int frequency)
{
    mFrequency = frequency;
}

int TimeLine::GetTimeFrequency() const
{
    return  mFrequency;
}

void TimeLine::SetZoomLevel(int level)
{
    mInFitZoom = false;
    mZoomLevel = level;
    if( (mZoomMarkerMS != -1) && ((mEndTimeMS - mStartTimeMS) > 0) )
    {
        float marker_ratio = std::abs((double)(mZoomMarkerMS - mStartTimeMS) / (double)(mEndTimeMS - mStartTimeMS));
        int total_ms = GetTotalViewableTimeMS();
        mStartTimeMS = mZoomMarkerMS - marker_ratio * total_ms;
        if( mStartTimeMS < 0 )
        {
            mStartTimeMS = 0;
        }
    }
    else
    {
        mStartTimeMS = 0;
    }
    mEndTimeMS = GetMaxViewableTimeMS();
    mStartPixelOffset = GetPixelOffsetFromStartTime();
    RecalcMarkerPositions();
    RaiseChangeTimeline();
}

void TimeLine::RecalcMarkerPositions()
{
    if (mCurrentPlayMarkerMS == -1) {
        mCurrentPlayMarker = -1;
    }
    else {
        mCurrentPlayMarker = GetPositionFromTimeMS(mCurrentPlayMarkerMS);
    }
    if (mCurrentPlayMarkerStartMS == -1) {
        mCurrentPlayMarkerStart = -1;
    }
    else {
        mCurrentPlayMarkerStart = GetPositionFromTimeMS(mCurrentPlayMarkerStartMS);
    }
    if (mCurrentPlayMarkerEndMS == -1) {
        mCurrentPlayMarkerEnd = -1;
    }
    else {
        mCurrentPlayMarkerEnd = GetPositionFromTimeMS(mCurrentPlayMarkerEndMS);
    }
    if (mSelectedPlayMarkerStartMS == -1) {
        mSelectedPlayMarkerStart = -1;
    }
    else {
        mSelectedPlayMarkerStart = GetPositionFromTimeMS(mSelectedPlayMarkerStartMS);
    }
    if (mSelectedPlayMarkerEndMS == -1) {
        mSelectedPlayMarkerEnd = -1;
    }
    else {
        mSelectedPlayMarkerEnd = GetPositionFromTimeMS(mSelectedPlayMarkerEndMS);
    }
    mSequenceEndMarker = GetPositionFromTimeMS(mSequenceEndMarkerMS);
    mEndPos = GetPositionFromTimeMS(mEndTimeMS);
}

int TimeLine::GetZoomLevel() const
{
    return mInFitZoom ? mMaxZoomLevel : mZoomLevel;
}

void TimeLine::ZoomOut()
{
    if (mInFitZoom) {
        mInFitZoom = false;
        int maxZoom = GetMaxZoomLevel();
        SetZoomLevel(maxZoom);
        if (GetTotalViewableTimeMS() > mTimeLength) {
            mStartTimeMS = 0;
            mStartPixelOffset = 0;
            mEndTimeMS = GetMaxViewableTimeMS();
            mEndPos = GetPositionFromTimeMS(mEndTimeMS);
            mSequenceEndMarker = GetPositionFromTimeMS(mSequenceEndMarkerMS);
            RaiseChangeTimeline();
        }
        return;
    }
    int maxZoom = GetMaxZoomLevel();
    if (mZoomLevel < maxZoom)
    {
        if (mZoomLevel >= maxZoom - 1) {
            SetFitZoom();
        } else {
            SetZoomLevel(mZoomLevel + 1);
            if (GetTotalViewableTimeMS() > mTimeLength) {
                mStartTimeMS = 0;
                mStartPixelOffset = 0;
                mEndTimeMS = GetMaxViewableTimeMS();
                mEndPos = GetPositionFromTimeMS(mEndTimeMS);
                mSequenceEndMarker = GetPositionFromTimeMS(mSequenceEndMarkerMS);
                RaiseChangeTimeline();
            }
        }
    }
}

void TimeLine::ZoomIn()
{
    if (mInFitZoom) {
        mInFitZoom = false;
        SetZoomLevel(GetMaxZoomLevel() - 1);
        return;
    }
    if (mZoomLevel >= GetMaxZoomLevel()) {
        SetFitZoom();
        return;
    }
    if (mZoomLevel > 0) {
        SetZoomLevel(mZoomLevel - 1);
    }
}

static double niceFitTickMS(double raw)
{
    // Round up to next "nice" value so tick labels land on round numbers
    // (e.g. 12/24/36, 15/30/45, 18/36/54, 20/40/60, 2.5/5/7.5).
    if (raw <= 0.0) return 1000.0;
    double mag = 1.0;
    if (raw >= 1.0) {
        while (mag * 10.0 <= raw) mag *= 10.0;
    } else {
        while (mag > raw) mag /= 10.0;
    }
    double norm = raw / mag;
    double niceNorm;
    if (norm <= 1.0)       niceNorm = 1.0;
    else if (norm <= 1.5)  niceNorm = 1.5;
    else if (norm <= 2.0)  niceNorm = 2.0;
    else if (norm <= 2.5)  niceNorm = 2.5;
    else if (norm <= 3.0)  niceNorm = 3.0;
    else if (norm <= 3.5)  niceNorm = 3.5;
    else if (norm <= 4.0)  niceNorm = 4.0;
    else if (norm <= 4.5)  niceNorm = 4.5;
    else if (norm <= 5.0)  niceNorm = 5.0;
    else if (norm <= 5.5)  niceNorm = 5.5;
    else if (norm <= 6.0)  niceNorm = 6.0;
    else if (norm <= 6.5)  niceNorm = 6.5;
    else if (norm <= 7.0)  niceNorm = 7.0;
    else if (norm <= 7.5)  niceNorm = 7.5;
    else if (norm <= 8.0)  niceNorm = 8.0;
    else if (norm <= 8.5)  niceNorm = 8.5;
    else if (norm <= 9.0)  niceNorm = 9.0;
    else if (norm <= 9.5)  niceNorm = 9.5;
    else                   niceNorm = 10.0;
    return niceNorm * mag;
}

void TimeLine::SetFitZoom()
{
    int w = GetSize().x;
    if (w <= 0 || mTimeLength <= 0) return;

    double idealTickMS = (double)mTimeLength * PIXELS_PER_MAJOR_HASH / (double)w;
    mFitTimePerMajorTickMS = niceFitTickMS(idealTickMS);
    mZoomLevel = GetMaxZoomLevel();  // keep Waveform using overview-level audio data
    mInFitZoom = true;
    mStartTimeMS = 0;
    mStartPixelOffset = 0;
    mEndTimeMS = GetMaxViewableTimeMS();  // may be slightly > mTimeLength due to rounding up
    RecalcMarkerPositions();
    RaiseChangeTimeline();
}

void TimeLine::ZoomSelection()
{
    // how much time is selected
    int sel = mSelectedPlayMarkerEndMS - mSelectedPlayMarkerStartMS;

    // set the zoom level so it all shows
    int zoom = 1;
    for (int z = 0; z < mMaxZoomLevel; ++z) {
        if (GetTotalViewableTimeMS(z) > sel) {
            zoom = z;
            break;
        }
    }

    SetZoomLevel(zoom);

    // set start time to the start of the selection but centre it
    int offset = (GetTotalViewableTimeMS(zoom) - sel) / 2;
    SetStartTimeMS(std::max(0, mSelectedPlayMarkerStartMS - offset));


    RaiseChangeTimeline();
}

int TimeLine::GetPixelOffsetFromStartTime()
{
    float nMajorHashs = (float)mStartTimeMS / (float)TimePerMajorTickInMS();
    int offset = nMajorHashs * PIXELS_PER_MAJOR_HASH;
    return offset;
}

int TimeLine::GetPositionFromTimeMS(int timeMS)
{
    double majorHashs = (double)timeMS/(double)TimePerMajorTickInMS();
    double xAbsolutePosition = majorHashs * (double)PIXELS_PER_MAJOR_HASH;
    return (int)(xAbsolutePosition - mStartPixelOffset);
}

void TimeLine::GetPositionsFromTimeRange(int startTimeMS, int endTimeMS, EFFECT_SCREEN_MODE &screenMode, int &x1, int &x2, int& x3, int& x4)
{
    if (startTimeMS < mStartTimeMS && endTimeMS > mEndTimeMS)
    {
        screenMode = EFFECT_SCREEN_MODE::SCREEN_L_R_ACROSS;
        x1 = 0;
        x2 = GetSize().x;
        double majorHashs = (double)(startTimeMS - mStartTimeMS) / (double)TimePerMajorTickInMS();
        x3 = (int)(majorHashs * (double)PIXELS_PER_MAJOR_HASH);
        majorHashs = (double)(endTimeMS - mStartTimeMS) / (double)TimePerMajorTickInMS();
        x4 = (int)(majorHashs * (double)PIXELS_PER_MAJOR_HASH);
    }
    else if (startTimeMS < mStartTimeMS && endTimeMS > mStartTimeMS && endTimeMS <= mEndTimeMS)
    {
        screenMode = EFFECT_SCREEN_MODE::SCREEN_R_ON;
        double majorHashs = (double)(endTimeMS - mStartTimeMS) / (double)TimePerMajorTickInMS();
        x1 = 0;
        x2 = (int)(majorHashs * (double)PIXELS_PER_MAJOR_HASH);
        majorHashs = (double)(startTimeMS - mStartTimeMS) / (double)TimePerMajorTickInMS();
        x3 = (int)(majorHashs * (double)PIXELS_PER_MAJOR_HASH);
        x4 = x2;
    }
    else if (startTimeMS >= mStartTimeMS && startTimeMS < mEndTimeMS && endTimeMS > mEndTimeMS)
    {
        screenMode = EFFECT_SCREEN_MODE::SCREEN_L_ON;
        double majorHashs = (double)(startTimeMS - mStartTimeMS) / (double)TimePerMajorTickInMS();
        x1 = (int)(majorHashs * (double)PIXELS_PER_MAJOR_HASH);
        x2 = GetSize().x;
        majorHashs = (double)(endTimeMS - mStartTimeMS) / (double)TimePerMajorTickInMS();
        x4 = (int)(majorHashs * (double)PIXELS_PER_MAJOR_HASH);
        x3 = x1;
    }
    else if (startTimeMS >= mStartTimeMS && endTimeMS <= mEndTimeMS)
    {
        screenMode = EFFECT_SCREEN_MODE::SCREEN_L_R_ON;
        double majorHashs = (double)(startTimeMS - mStartTimeMS) / (double)TimePerMajorTickInMS();
        x1 = (int)(majorHashs * (double)PIXELS_PER_MAJOR_HASH);
        majorHashs = (double)(endTimeMS - mStartTimeMS) / (double)TimePerMajorTickInMS();
        x2 = (int)(majorHashs * (double)PIXELS_PER_MAJOR_HASH);
        x3 = x1;
        x4 = x2;
    }
    else if ((startTimeMS < mStartTimeMS && endTimeMS < mStartTimeMS) ||
        (startTimeMS > mStartTimeMS && endTimeMS > mStartTimeMS))
    {
        screenMode = EFFECT_SCREEN_MODE::SCREEN_L_R_OFF;
        x1 = 0;
        x2 = 0;
        x3 = x1;
        x4 = x2;
    }
}

void TimeLine::SetTimeLength(int ms)
{
    mTimeLength = ms;
}

int TimeLine::GetTimeLength() const
{
    return mTimeLength;
}

int TimeLine::GetTimeMSfromPosition(int position)
{
    float nMajorHashs = (float)position / (float)PIXELS_PER_MAJOR_HASH;
    int time = (int)(nMajorHashs*TimePerMajorTickInMS());
    return time;
}

int TimeLine::GetAbsoluteTimeMSfromPosition(int position)
{
    float nMajorHashs = (float)position/(float)PIXELS_PER_MAJOR_HASH;
    int time = mStartTimeMS + (int)(nMajorHashs*TimePerMajorTickInMS());
    time = RoundToMultipleOfPeriod(time,mFrequency);
    return time;
}

int TimeLine::GetRawTimeMSfromPosition(int position)
{
    float nMajorHashs = (float)position/(float)PIXELS_PER_MAJOR_HASH;
    int time = mStartTimeMS + (int)(nMajorHashs*TimePerMajorTickInMS());
    return time;
}

int TimeLine::GetMaxViewableTimeMS()
{
    float width = (float)GetSize().x;
    float majorTicks = width / (float)PIXELS_PER_MAJOR_HASH;
    return (int)((majorTicks * (float)TimePerMajorTickInMS()) + mStartTimeMS);
}

int TimeLine::GetTotalViewableTimeMS()
{
    float width = (float)GetSize().x;
    float majorTicks = width / (float)PIXELS_PER_MAJOR_HASH;
    return (int)((majorTicks * (float)TimePerMajorTickInMS()));
}

int TimeLine::GetTotalViewableTimeMS(int zoom)
{
    float width = (float)GetSize().x;
    float majorTicks = width / (float)PIXELS_PER_MAJOR_HASH;
    return (int)((majorTicks * (float)TimePerMajorTickInMS(zoom)));
}

int TimeLine::GetZoomLevelValue() const
{
    return  ZoomLevelValues[mZoomLevel];
}

int TimeLine::GetMaxZoomLevel()
{
    float width = (float)GetSize().x;
    mMaxZoomLevel = MAX_ZOOM_OUT_INDEX;
    for (int i = 0; i <= MAX_ZOOM_OUT_INDEX; i++) {
        float majorTicks = width / (float)PIXELS_PER_MAJOR_HASH;
        int timeMS = (int)((float)ZoomLevelValues[i] * ((float)1000 / (float)mFrequency) * majorTicks);
        if (timeMS > mTimeLength) {
            mMaxZoomLevel = i;
            break;
        }
    }
    return mMaxZoomLevel;
}

void TimeLine::Initialize()
{
    mIsInitialized = true;
    mStartPixelOffset = 0;
    mZoomLevel = 0;
    mStartTimeMS = 0;
    mEndTimeMS = GetMaxViewableTimeMS();
    mCurrentPlayMarkerStart = -1;
    mCurrentPlayMarkerEnd = -1;
    mSelectedPlayMarkerStart = -1;
    mSelectedPlayMarkerEnd = -1;
    mCurrentPlayMarker = -1;
    mCurrentPlayMarkerStartMS = -1;
    mCurrentPlayMarkerEndMS = -1;
    mSelectedPlayMarkerStartMS = -1;
    mSelectedPlayMarkerEndMS = -1;
    mCurrentPlayMarkerMS = -1;
    timeline_initiated_play = false;
    m_dragging = false;
}

void TimeLine::Paint( wxPaintEvent& event )
{
    wxPaintDC dc(this);
    render(dc);
}

void TimeLine::render( wxDC& dc ) {
    wxCoord w,h;
    wxPen pen(wxColor(128,128,128));
    const wxPen* pen_black = wxBLACK_PEN;
    const wxPen* pen_green = wxGREEN_PEN;
    const wxPen* pen_transparent = wxTRANSPARENT_PEN;
    dc.SetPen(pen);
    dc.GetSize(&w,&h);
    wxBrush brush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE),wxBRUSHSTYLE_SOLID);
    wxBrush brush_range(wxColor(187, 173,193),wxBRUSHSTYLE_SOLID);
    wxBrush brush_past_end(wxColor(153, 204, 255),wxBRUSHSTYLE_CROSSDIAG_HATCH);
    dc.SetBrush(brush);
    dc.DrawRectangle(0,0,w,h+1);

    if (!mIsInitialized)
        return;

    // Region color bands are drawn first, as a background tint, so the ruler
    // hash marks and time labels painted below remain visible on top of them.
    DrawSongStructureBands(dc, w, h);

    wxFont f = dc.GetFont();
    f.SetPointSize(7.0);
    dc.SetFont(f);

    // Draw the selection fill if its a range
    if( mSelectedPlayMarkerStart != -1 && mSelectedPlayMarkerEnd != -1 ) {
        int left_pos = std::min(mSelectedPlayMarkerStart, mSelectedPlayMarkerEnd);
        int right_pos = std::max(mSelectedPlayMarkerStart, mSelectedPlayMarkerEnd) - 1;
        dc.SetPen(*pen_transparent);
        dc.SetBrush(brush_range);
        dc.DrawRectangle(left_pos, 0, right_pos - left_pos + 1, h);
    }

    dc.SetBrush(brush);
    dc.SetPen(pen);
    int minutes=0;
    int seconds=0;
    int subsecs=0;
    wxString sTime;
    for(int x=0;x<w;x++)
    {

        // Draw hash marks
        if ((x+mStartPixelOffset)%(PIXELS_PER_MAJOR_HASH/2)==0)
        {
            dc.DrawLine(x,h - 10,x,h-1);
        }
        // Draw time label
        if((x+mStartPixelOffset)%PIXELS_PER_MAJOR_HASH==0)
        {
            float t = GetTimeMSfromPosition(x+mStartPixelOffset);
            wxRect r(x-25,h/2-10,50,12);
            minutes = t/60000;
            seconds = (t-(minutes*60000))/1000;
            subsecs = t - (minutes*60000 + seconds*1000);

            if (mShowAlternateTimingFormat) {
                int totalSeconds = (minutes * 60) + seconds;
                if (mFrequency >= 40)
                    sTime = wxString::Format("%d.%.3d", totalSeconds, subsecs);
                else
                    sTime = wxString::Format("%d.%.2d", totalSeconds, subsecs / 10);
            } else {
                if (minutes > 0) {
                    if (mFrequency >= 40)
                        sTime = wxString::Format("%d:%02d.%.3d", minutes, seconds, subsecs);
                    else
                        sTime = wxString::Format("%d:%.2d.%.2d", minutes, seconds, subsecs / 10);
                } else {
                    if (mFrequency >= 40)
                        sTime = wxString::Format("%2d.%.3d", seconds, subsecs);
                    else
                        sTime = wxString::Format("%2d.%.2d", seconds, subsecs / 10);
                }
            }
            dc.DrawLabel(sTime, r, wxALIGN_CENTER);
        }
    }

    // draw timeline selection range or point
    if( mCurrentPlayMarkerStart >= 0 ) {
        int left_pos = mCurrentPlayMarkerStart + 1;
        if( mCurrentPlayMarkerEnd >= 0 && mCurrentPlayMarkerStart != mCurrentPlayMarkerEnd)
        {
            left_pos = std::min(mCurrentPlayMarkerStart, mCurrentPlayMarkerEnd) + 1;
            int right_pos = std::max(mCurrentPlayMarkerStart, mCurrentPlayMarkerEnd) - 1;
            DrawTriangleMarkerFacingRight(dc, right_pos, marker_size, h);
            DrawRectangle(dc, left_pos+marker_size+1, h-marker_size-1, right_pos-marker_size, h-marker_size+1);
        }
        DrawTriangleMarkerFacingLeft(dc, left_pos, marker_size, h);
    }
    // draw green current play arrow
    if (mCurrentPlayMarker >= 0) {
        wxPoint points[4];
        int play_start_mark = mCurrentPlayMarker - marker_size;
        int play_end_mark = mCurrentPlayMarker + marker_size;
        points[0].x = play_start_mark;
        points[0].y = 0;
        points[1].x = play_end_mark + 1;
        points[1].y = 0;
        points[2].x = mCurrentPlayMarker;
        points[2].y = (play_end_mark - play_start_mark) / 2 + 1;
        points[3].x = play_start_mark;
        points[3].y = 0;
        
        dc.SetPen(*pen_green);
        dc.SetBrush(*wxGREEN_BRUSH);
        dc.DrawPolygon(4, points);
        dc.SetPen(*pen_black);
        dc.SetBrush(wxNullBrush);
        dc.DrawLines(4, points);
    }

    // Draw the selection line if not a range
    if (mSelectedPlayMarkerStart != -1 && mSelectedPlayMarkerEnd == -1) {
        dc.SetPen(*pen_black);
        dc.DrawLine(mSelectedPlayMarkerStart, 0, mSelectedPlayMarkerStart, h-1);
    }

    // grey out where sequence ends — clamp to viewport width
    int hashStart = std::max(0, mSequenceEndMarker);
    if (hashStart < w) {
        dc.SetBrush(brush_past_end);
        dc.DrawRectangle(hashStart, 0, w - hashStart, h);
    }

    // Draw song structure regions
    DrawSongStructureRegions(dc, w, h);
    DrawSongStructureBoundaries(dc, w, h);

    for (int i = 0; i < 10; ++i)
    {
        if (_tagPositions[i] < mStartTimeMS || _tagPositions[i] > mEndTimeMS)
        {
            // dont draw marks outside visibile
        }
        else
        {
            float pos = (float)(_tagPositions[i] - mStartTimeMS) / (float)(mEndTimeMS - mStartTimeMS);
            DrawTag(dc, i, pos * mEndPos, h);
        }
    }
}

void TimeLine::DrawTag(wxDC& dc, int tag, int position, int y_bottom)
{
    dc.SetPen(*wxBLUE_PEN);

    if( IsDarkMode() ) {
        dc.SetBrush(*wxBLUE_BRUSH);
    } else {
        dc.SetBrush(*wxLIGHT_GREY_BRUSH);
    }

    wxPoint points[5];
    points[0] = wxPoint(position+5, y_bottom -1);
    points[1] = wxPoint(position-5, y_bottom - 1);
    points[2] = wxPoint(position-5, y_bottom - 12);
    points[3] = wxPoint(position,   y_bottom - 15);
    points[4] = wxPoint(position+5, y_bottom - 12);

    dc.DrawPolygon(5, points, 0,0, wxWINDING_RULE);
    dc.SetBrush(*wxBLACK_BRUSH);
    dc.DrawLabel(wxString::Format("%i", tag), wxRect(position - 4, y_bottom - 13, 10, 13), wxALIGN_CENTRE_HORIZONTAL | wxALIGN_CENTRE_VERTICAL);
}

void TimeLine::DrawTriangleMarkerFacingLeft(wxDC& dc, int& play_start_mark, const int& tri_size, int& height)
{
    const wxPen* pen_black = wxBLACK_PEN;
    const wxPen* pen_grey = wxLIGHT_GREY_PEN;
    int y_top = height-tri_size;
    int y_bottom = y_top;
    int arrow_end = play_start_mark + 1;
    dc.SetPen(*pen_grey);
    for( ; y_bottom < height-1; y_bottom++, y_top--, arrow_end++ )
    {
        dc.DrawLine(arrow_end,y_top,arrow_end,y_bottom);
    }
    dc.SetPen(*pen_black);
    dc.DrawLine(play_start_mark,y_top,play_start_mark,height-1);
    dc.DrawLine(play_start_mark+1,height-tri_size,arrow_end,y_top);
    dc.DrawLine(play_start_mark+1,height-tri_size,arrow_end,y_bottom);
    dc.DrawLine(arrow_end,y_top,arrow_end,y_bottom);
}

void TimeLine::DrawTriangleMarkerFacingRight(wxDC& dc, int& play_start_mark, const int& tri_size, int& height)
{
    const wxPen* pen_black = wxBLACK_PEN;
    const wxPen* pen_grey = wxLIGHT_GREY_PEN;
    int y_top = height-tri_size;
    int y_bottom = y_top;
    int arrow_end = play_start_mark - 1;
    dc.SetPen(*pen_grey);
    for( ; y_bottom < height-1; y_bottom++, y_top--, arrow_end-- )
    {
        dc.DrawLine(arrow_end,y_top,arrow_end,y_bottom);
    }
    dc.SetPen(*pen_black);
    dc.DrawLine(play_start_mark,y_top,play_start_mark,height-1);
    dc.DrawLine(play_start_mark-1,height-tri_size,arrow_end,y_top);
    dc.DrawLine(play_start_mark-1,height-tri_size,arrow_end,y_bottom);
    dc.DrawLine(arrow_end,y_top,arrow_end,y_bottom);
}

void TimeLine::DrawRectangle(wxDC& dc, int x1, int y1, int x2, int y2)
{
    const wxPen* pen_outline = wxMEDIUM_GREY_PEN;
    const wxPen* pen_grey = wxLIGHT_GREY_PEN;
    dc.SetPen(*pen_grey);
    for( int y = y1; y <= y2; y++ )
    {
        dc.DrawLine(x1, y, x2, y);
    }
    dc.SetPen(*pen_outline);
    dc.DrawLine(x1, y1, x2, y1);
    dc.DrawLine(x1, y2, x2, y2);
}

int TimeLine::TimePerMajorTickInMS()
{
    if (mInFitZoom && mTimeLength > 0) {
        int w = GetSize().x;
        if (w > 0) {
            double idealTickMS = (double)mTimeLength * PIXELS_PER_MAJOR_HASH / (double)w;
            return (int)niceFitTickMS(idealTickMS);
        }
        if (mFitTimePerMajorTickMS > 0.0)
            return (int)mFitTimePerMajorTickMS;
    }
    return (int)((double)ZoomLevelValues[mZoomLevel] * ((double)1000.0/(double)mFrequency));
}

int TimeLine::TimePerMajorTickInMS(int zoom)
{
    return (int)((double)ZoomLevelValues[std::min(GetMaxZoomLevel(), zoom)] * ((double)1000.0 / (double)mFrequency));
}

void TimeLine::GetViewableTimeRange(int &StartTime, int &EndTime)
{
    StartTime = mStartTimeMS;
    EndTime = mEndTimeMS;
}

TimelineChangeArguments::TimelineChangeArguments(int zoomLevel, int startPixelOffset,int currentTime)
{
    ZoomLevel = zoomLevel;
    StartPixelOffset = startPixelOffset;
    CurrentTimeMS = currentTime;
}

int TimeLine::RoundToMultipleOfPeriod(int number, double frequency)
{
    return ::RoundToMultipleOfPeriod(number, frequency);
}

TimelineChangeArguments::~TimelineChangeArguments()
{
}

void TimeLine::RecalcEndTime()
{
    mEndTimeMS = GetMaxViewableTimeMS();
}

wxColour TimeLine::GetSongStructureBandColor(const SongStructureRegion& region) const
{
    // wxDC ignores the alpha channel on some platforms (notably MSW/GDI), so a
    // wxColour with alpha renders fully opaque. Manually blend the region color
    // over the timeline background instead - this looks translucent on every
    // platform and naturally adapts to light/dark mode (the background differs).
    const wxColour bg = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
    constexpr double bandAlpha = 50.0 / 255.0;
    uint8_t r = (region.colorARGB >> 16) & 0xFF;
    uint8_t g = (region.colorARGB >> 8) & 0xFF;
    uint8_t b = region.colorARGB & 0xFF;
    return wxColour((uint8_t)(r * bandAlpha + bg.Red() * (1.0 - bandAlpha)),
                    (uint8_t)(g * bandAlpha + bg.Green() * (1.0 - bandAlpha)),
                    (uint8_t)(b * bandAlpha + bg.Blue() * (1.0 - bandAlpha)));
}

void TimeLine::DrawSongStructureBands(wxDC& dc, int w, int h)
{
    // Drawn BEHIND the ruler hash marks/time labels (called before they are
    // painted) so the timing stays visible - the band is just a background tint.
    if (_sequenceElements == nullptr) return;
    const SongStructureManager& ssm = _sequenceElements->GetSongStructureManager();
    if (!ssm.HasRegions()) return;

    for (size_t i = 0; i < ssm.GetRegionCount(); i++) {
        const SongStructureRegion& region = ssm.GetRegion(i);

        int x1 = GetPositionFromTimeMS(region.startTimeMS);
        int x2 = GetPositionFromTimeMS(region.endTimeMS);

        if (x2 < 0 || x1 > w) continue;
        x1 = std::max(0, x1);
        x2 = std::min(w, x2);

        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.SetBrush(wxBrush(GetSongStructureBandColor(region)));
        dc.DrawRectangle(x1, 0, x2 - x1, h);
    }
}

void TimeLine::DrawSongStructureRegions(wxDC& dc, int w, int h)
{
    // Region name labels only - drawn on top of the ruler so both the name and
    // the timing marks stay visible. The band tint is painted separately by
    // DrawSongStructureBands() before the ruler.
    if (_sequenceElements == nullptr) return;
    const SongStructureManager& ssm = _sequenceElements->GetSongStructureManager();
    if (!ssm.HasRegions()) return;

    wxFont f = dc.GetFont();
    f.SetPointSize(7.0);
    dc.SetFont(f);

    for (size_t i = 0; i < ssm.GetRegionCount(); i++) {
        const SongStructureRegion& region = ssm.GetRegion(i);

        int x1 = GetPositionFromTimeMS(region.startTimeMS);
        int x2 = GetPositionFromTimeMS(region.endTimeMS);

        if (x2 < 0 || x1 > w) continue;
        x1 = std::max(0, x1);
        x2 = std::min(w, x2);

        // Draw region name in the lower portion of the timeline so it sits
        // below the timing values which are drawn in the upper half.
        if (x2 - x1 > 30) {
            wxColour bandColor = GetSongStructureBandColor(region);
            wxString name = region.name;
            wxSize textSize = dc.GetTextExtent(name);
            while (name.length() > 1 && textSize.GetWidth() > (x2 - x1 - 8)) {
                name = name.Left(name.length() - 1);
                textSize = dc.GetTextExtent(name);
            }
            if (textSize.GetWidth() <= (x2 - x1 - 8)) {
                int textX = x1 + ((x2 - x1) - textSize.GetWidth()) / 2;
                int textY = h - textSize.GetHeight() - 1;
                double luminance = 0.299 * bandColor.Red() + 0.587 * bandColor.Green() + 0.114 * bandColor.Blue();
                dc.SetTextForeground(luminance > 140.0 ? wxColour(40, 40, 40) : wxColour(220, 220, 220));
                dc.DrawText(name, textX, textY);
            }
        }
    }
}

void TimeLine::DrawSongStructureBoundaries(wxDC& dc, int w, int h)
{
    if (_sequenceElements == nullptr) return;
    const SongStructureManager& ssm = _sequenceElements->GetSongStructureManager();
    if (ssm.GetRegionCount() < 2) return;

    // White is invisible against the light-mode timeline background; pick a
    // contrasting boundary color per appearance.
    const wxColour boundaryColor = IsDarkMode() ? wxColour(235, 235, 235) : wxColour(60, 60, 60);
    dc.SetPen(wxPen(boundaryColor, 1));
    for (size_t i = 1; i < ssm.GetRegionCount(); i++) {
        int boundaryTimeMS = ssm.GetRegion(i).startTimeMS;
        int xPos = GetPositionFromTimeMS(boundaryTimeMS);
        if (xPos >= 0 && xPos <= w) {
            dc.DrawLine(xPos, 0, xPos, h);
            // Draw small handle diamond at center
            int cy = h / 2;
            wxPoint diamond[4];
            diamond[0] = wxPoint(xPos, cy - 4);
            diamond[1] = wxPoint(xPos + 3, cy);
            diamond[2] = wxPoint(xPos, cy + 4);
            diamond[3] = wxPoint(xPos - 3, cy);
            dc.SetBrush(wxBrush(boundaryColor));
            dc.DrawPolygon(4, diamond);
        }
    }
}

int TimeLine::HitTestBoundary(int x)
{
    if (_sequenceElements == nullptr) return -1;
    const SongStructureManager& ssm = _sequenceElements->GetSongStructureManager();
    if (ssm.GetRegionCount() < 2) return -1;

    int clickTimeMS = GetAbsoluteTimeMSfromPosition(x);

    // 6 pixel hit-test tolerance
    float msPerPixel = 1.0f;
    if (mEndTimeMS > mStartTimeMS && GetSize().x > 0) {
        msPerPixel = (float)(mEndTimeMS - mStartTimeMS) / (float)GetSize().x;
    }
    int toleranceMS = (int)(6.0f * msPerPixel);

    return ssm.FindNearestBoundary(clickTimeMS, toleranceMS);
}

void TimeLine::OnSongStructurePopup(wxCommandEvent& event)
{
    if (_sequenceElements == nullptr) return;
    SongStructureManager& ssm = _sequenceElements->GetSongStructureManager();

    int id = event.GetId();

    MainSequencer* ms = dynamic_cast<MainSequencer*>(GetParent());

    if (id == ID_SONG_ADD_BOUNDARY) {
        ssm.AddBoundary(_rightClickPosition, mSequenceEndMarkerMS);
        RaiseSequenceChange();
        Refresh(false);
        if (ms && ms->PanelEffectGrid) ms->PanelEffectGrid->Refresh(false);
    } else if (id == ID_SONG_DELETE_BOUNDARY) {
        float msPerPixel = 1.0f;
        if (mEndTimeMS > mStartTimeMS && GetSize().x > 0) {
            msPerPixel = (float)(mEndTimeMS - mStartTimeMS) / (float)GetSize().x;
        }
        int toleranceMS = (int)(6.0f * msPerPixel);
        int boundary = ssm.FindNearestBoundary(_rightClickPosition, toleranceMS);
        if (boundary >= 0) {
            ssm.DeleteBoundary(boundary);
            RaiseSequenceChange();
            Refresh(false);
            if (ms && ms->PanelEffectGrid) ms->PanelEffectGrid->Refresh(false);
        }
    } else if (id == ID_SONG_EDIT_REGION) {
        int regionIdx = ssm.GetRegionIndexAtTime(_rightClickPosition);
        if (regionIdx >= 0) {
            SongStructureRegion& region = ssm.GetRegion(regionIdx);

            SongRegionEditDialog dlg(this, region.name, region.colorARGB);
            if (dlg.ShowModal() == wxID_OK) {
                wxString newName = dlg.GetRegionName();
                if (!newName.empty()) {
                    ssm.SetRegionName(regionIdx, newName.ToStdString());
                }
                uint32_t newColor = dlg.GetSelectedColorARGB();
                // Preserve original alpha, use RGB from selection
                uint8_t a = (region.colorARGB >> 24) & 0xFF;
                newColor = ((uint32_t)a << 24) | (newColor & 0x00FFFFFF);
                ssm.SetRegionColor(regionIdx, newColor);
            }

            RaiseSequenceChange();
            Refresh(false);
            if (ms && ms->PanelEffectGrid) ms->PanelEffectGrid->Refresh(false);
        }
    } else if (id == ID_SONG_CLEAR_STRUCTURE) {
        if (wxMessageBox("Are you sure you want to clear all song structure regions?",
                          "Clear Song Structure", wxYES_NO | wxICON_QUESTION, this) == wxYES) {
            ssm.Clear();
            RaiseSequenceChange();
            Refresh(false);
            if (ms && ms->PanelEffectGrid) ms->PanelEffectGrid->Refresh(false);
        }
    } else if (id >= ID_SONG_COPY_EFFECTS_BASE &&
               id < ID_SONG_COPY_EFFECTS_BASE + (int)ssm.GetRegionCount()) {
        int sourceRegionIdx = ssm.GetRegionIndexAtTime(_rightClickPosition);
        int targetRegionIdx = id - ID_SONG_COPY_EFFECTS_BASE;
        if (sourceRegionIdx >= 0 && targetRegionIdx >= 0 &&
            targetRegionIdx < (int)ssm.GetRegionCount() &&
            sourceRegionIdx != targetRegionIdx) {
            CopyEffectsToRegion(sourceRegionIdx, targetRegionIdx);
        }
    } else if (id == ID_SONG_APPLY_PALETTE) {
        int regionIdx = ssm.GetRegionIndexAtTime(_rightClickPosition);
        if (regionIdx >= 0) {
            ApplyPaletteToRegion(regionIdx);
        }
    } else if (id == ID_SONG_EXPORT_REGION) {
        int regionIdx = ssm.GetRegionIndexAtTime(_rightClickPosition);
        if (regionIdx >= 0) {
            const SongStructureRegion& region = ssm.GetRegion(regionIdx);
            xLightsFrame::GetFrame()->ExportSongRegion(region.startTimeMS, region.endTimeMS, region.name);
        }
    } else if (id == ID_SONG_EXPORT_ALL_REGIONS) {
        xLightsFrame::GetFrame()->ExportAllSongRegions();
    } else if (id >= ID_SONG_VIEW_BASE && id < ID_SONG_VIEW_BASE + (int)ssm.GetViewCount()) {
        ssm.SetActiveView(id - ID_SONG_VIEW_BASE);
        RaiseSequenceChange();
        Refresh(false);
        if (ms && ms->PanelEffectGrid) ms->PanelEffectGrid->Refresh(false);
    } else if (id == ID_SONG_VIEW_NEW) {
        wxString name = wxGetTextFromUser("Enter name for new view:", "New Song Structure View", "", this);
        if (!name.empty()) {
            int idx = ssm.AddView(name.ToStdString());
            ssm.SetActiveView(idx);
            RaiseSequenceChange();
            Refresh(false);
            if (ms && ms->PanelEffectGrid) ms->PanelEffectGrid->Refresh(false);
        }
    } else if (id == ID_SONG_VIEW_DUPLICATE) {
        wxString defaultName = ssm.GetActiveViewName() + " (copy)";
        wxString name = wxGetTextFromUser("Enter name for duplicated view:",
            "Duplicate Song Structure View", defaultName, this);
        if (!name.empty()) {
            int idx = ssm.DuplicateView(ssm.GetActiveViewIndex(), name.ToStdString());
            ssm.SetActiveView(idx);
            RaiseSequenceChange();
            Refresh(false);
            if (ms && ms->PanelEffectGrid) ms->PanelEffectGrid->Refresh(false);
        }
    } else if (id == ID_SONG_VIEW_RENAME) {
        wxString name = wxGetTextFromUser("Enter new name for current view:",
            "Rename Song Structure View", ssm.GetActiveViewName(), this);
        if (!name.empty()) {
            ssm.RenameView(ssm.GetActiveViewIndex(), name.ToStdString());
            RaiseSequenceChange();
        }
    } else if (id == ID_SONG_VIEW_DELETE) {
        if (wxMessageBox(wxString::Format("Delete view \"%s\"?", wxString::FromUTF8(ssm.GetActiveViewName())),
                          "Delete Song Structure View", wxYES_NO | wxICON_QUESTION, this) == wxYES) {
            ssm.DeleteView(ssm.GetActiveViewIndex());
            RaiseSequenceChange();
            Refresh(false);
            if (ms && ms->PanelEffectGrid) ms->PanelEffectGrid->Refresh(false);
        }
    }
}

void TimeLine::CopyEffectsToRegion(int sourceRegionIdx, int targetRegionIdx)
{
    if (_sequenceElements == nullptr) return;

    SongStructureManager& ssm = _sequenceElements->GetSongStructureManager();
    const SongStructureRegion& sourceRegion = ssm.GetRegion(sourceRegionIdx);
    const SongStructureRegion& targetRegion = ssm.GetRegion(targetRegionIdx);

    int timeOffset = targetRegion.startTimeMS - sourceRegion.startTimeMS;

    UndoManager& undoMgr = _sequenceElements->get_undo_mgr();
    undoMgr.CreateUndoStep();

    int effectsCopied = 0;

    for (size_t i = 0; i < _sequenceElements->GetElementCount(); i++) {
        Element* elem = _sequenceElements->GetElement(i);
        if (elem == nullptr) continue;
        if (elem->GetType() == ElementType::ELEMENT_TYPE_TIMING) continue;

        for (int layer = 0; layer < (int)elem->GetEffectLayerCount(); layer++) {
            EffectLayer* el = elem->GetEffectLayer(layer);
            if (el == nullptr) continue;

            std::vector<Effect*> sourceEffects = el->GetAllEffectsByTime(
                sourceRegion.startTimeMS, sourceRegion.endTimeMS);

            for (Effect* eff : sourceEffects) {
                int newStartMS = eff->GetStartTimeMS() + timeOffset;
                int newEndMS = eff->GetEndTimeMS() + timeOffset;

                newStartMS = RoundToMultipleOfPeriod(newStartMS, mFrequency);
                newEndMS = RoundToMultipleOfPeriod(newEndMS, mFrequency);

                if (newStartMS < 0 || newStartMS >= newEndMS) continue;

                if (el->GetRangeIsClearMS(newStartMS, newEndMS)) {
                    Effect* newEff = el->AddEffect(0,
                        eff->GetEffectName(),
                        eff->GetSettingsAsString(),
                        eff->GetPaletteAsString(),
                        newStartMS,
                        newEndMS,
                        EFFECT_NOT_SELECTED,
                        false);

                    if (newEff != nullptr) {
                        undoMgr.CaptureAddedEffect(
                            elem->GetName(),
                            el->GetIndex(),
                            newEff->GetID());
                        effectsCopied++;
                    }
                }
            }
        }
    }

    if (effectsCopied == 0) {
        undoMgr.CancelLastStep();
    }

    MainSequencer* ms = dynamic_cast<MainSequencer*>(GetParent());
    if (ms && ms->PanelEffectGrid) {
        ms->PanelEffectGrid->sendRenderDirtyEvent();
        ms->PanelEffectGrid->Refresh(false);
    }
    RaiseSequenceChange();
    Refresh(false);
}

void TimeLine::ApplyPaletteToRegion(int regionIdx)
{
    if (_sequenceElements == nullptr) return;

    MainSequencer* ms = dynamic_cast<MainSequencer*>(GetParent());
    if (ms == nullptr || ms->PanelEffectGrid == nullptr) return;

    Effect* selectedEffect = ms->PanelEffectGrid->GetSelectedEffect();
    if (selectedEffect == nullptr) return;

    SongStructureManager& ssm = _sequenceElements->GetSongStructureManager();
    const SongStructureRegion& region = ssm.GetRegion(regionIdx);

    std::string paletteString = selectedEffect->GetPaletteAsString();

    UndoManager& undoMgr = _sequenceElements->get_undo_mgr();
    undoMgr.CreateUndoStep();

    int effectsModified = 0;

    for (size_t i = 0; i < _sequenceElements->GetElementCount(); i++) {
        Element* elem = _sequenceElements->GetElement(i);
        if (elem == nullptr) continue;
        if (elem->GetType() == ElementType::ELEMENT_TYPE_TIMING) continue;

        for (int layer = 0; layer < (int)elem->GetEffectLayerCount(); layer++) {
            EffectLayer* el = elem->GetEffectLayer(layer);
            if (el == nullptr) continue;

            std::vector<Effect*> regionEffects = el->GetAllEffectsByTime(
                region.startTimeMS, region.endTimeMS);

            for (Effect* eff : regionEffects) {
                undoMgr.CaptureModifiedEffect(
                    elem->GetName(),
                    el->GetIndex(),
                    eff);

                eff->SetPalette(paletteString);
                effectsModified++;
            }
        }
    }

    if (effectsModified == 0) {
        undoMgr.CancelLastStep();
    }

    if (ms->PanelEffectGrid) {
        ms->PanelEffectGrid->sendRenderDirtyEvent();
        ms->PanelEffectGrid->Refresh(false);
    }
    RaiseSequenceChange();
    Refresh(false);
}

