/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/wx.h>
#include <wx/brush.h>

#include "TimeLine.h"
#include "Waveform.h"
#include "../xLightsMain.h"
#include <log4cpp/Category.hh>

const long TimeLine::ID_ZOOMSEL = wxNewId();

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
}

void TimeLine::mouseRightDown(wxMouseEvent& event)
{
    _rightClickPosition = GetAbsoluteTimeMSfromPosition(GetPositionFromSelection(event.GetX()));

    if (_rightClickPosition > GetTimeLength()) return;

    wxMenu mnuLayer;
    if (mSelectedPlayMarkerEndMS != mSelectedPlayMarkerStartMS) {
        mnuLayer.Append(ID_ZOOMSEL, "Zoom to Selection");
    }
    for (int i = 0; i < 10; ++i)
    {
        auto mnu = mnuLayer.Append(i+1, wxString::Format("%i", i));
        mnu->SetCheckable(true);
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

    PopupMenu(&mnuLayer);
}

void TimeLine::OnPopup(wxCommandEvent& event)
{
    int id = event.GetId() - 1;

    if (id + 1 == ID_ZOOMSEL) {
        ZoomSelection();
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

int TimeLine::GetTagPosition(int tag)
{
    // update it if it is outside the sequence
    SetTagPosition(tag, _tagPositions[tag]);

    return _tagPositions[tag];
}

void TimeLine::mouseLeftDown( wxMouseEvent& event)
{
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
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("                Creating Timeline");
    logger_base.info("If xLights crashes after this log message then the root cause is almost always a problem between OpenGL and their video drivers.\nWe recommend they download the latest drivers from their card provider ... not from their operating system provider.");

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
    return  mZoomLevel;
}

void TimeLine::ZoomOut()
{
    if (mZoomLevel < mMaxZoomLevel)
    {
        SetZoomLevel(mZoomLevel + 1);
        if (GetTotalViewableTimeMS() > mTimeLength)
        {
            mStartTimeMS = 0;
            mStartPixelOffset = 0;
            mEndTimeMS = GetMaxViewableTimeMS();
            mEndPos = GetPositionFromTimeMS(mEndTimeMS);
            mSequenceEndMarker = GetPositionFromTimeMS(mSequenceEndMarkerMS);
            RaiseChangeTimeline();
        }
    }
}

void TimeLine::ZoomIn()
{
    wxString s;
    if (mZoomLevel > 0)
    {
        SetZoomLevel(mZoomLevel - 1);
    }
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
    int labelCount=0;
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
    wxString format;
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

            if(minutes > 0)
            {
                if(mFrequency>=40)
                    sTime =  wxString::Format("%d:%02d.%.3d",minutes,seconds,subsecs);
                else
                    sTime =  wxString::Format("%d:%.2d.%.2d",minutes,seconds,subsecs/10);
            }
            else
            {
                if(mFrequency>=40)
                    sTime =  wxString::Format("%2d.%.3d",seconds,subsecs);
                else
                    sTime =  wxString::Format("%2d.%.2d",seconds,subsecs/10);
            }
            labelCount++;
            dc.DrawLabel(sTime,r,wxALIGN_CENTER);
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

    // grey out where sequence ends
    dc.SetBrush(brush_past_end);
    dc.DrawRectangle(mSequenceEndMarker, 0, mEndPos, h);

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
    const wxPen* pen_black = wxBLUE_PEN;

    dc.SetPen(*pen_black);
    dc.DrawLine(position-5, y_bottom - 1, position+5, y_bottom -1);
    dc.DrawLine(position-5, y_bottom - 1, position-5, y_bottom - 12);
    dc.DrawLine(position+5, y_bottom - 1, position+5, y_bottom - 12);
    dc.DrawLine(position-5, y_bottom - 12, position, y_bottom - 15);
    dc.DrawLine(position+5, y_bottom - 12, position, y_bottom - 15);
    dc.FloodFill(position, y_bottom - 6, *wxLIGHT_GREY);
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
    int ms = 1000 / frequency;
    int periods = (number + ms / 2) / ms;
    return periods * ms;
}

TimelineChangeArguments::~TimelineChangeArguments()
{
}

void TimeLine::RecalcEndTime()
{
    mEndTimeMS = GetMaxViewableTimeMS();
}

