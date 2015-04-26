#ifndef TIMELINE_H
#define TIMELINE_H

#include "wx/wx.h"
#include "wx/window.h"
#define HORIZONTAL_PADDING      10
#define PIXELS_PER_MAJOR_HASH   100

#define ZOOM_OUT        0
#define ZOOM_IN         1

#define MAX_ZOOM_OUT_INDEX      12

enum EFFECT_SCREEN_MODE{
    SCREEN_L_R_OFF,
    SCREEN_L_ON,
    SCREEN_R_ON,
    SCREEN_L_R_ON,
    SCREEN_L_R_ACROSS,
};


wxDECLARE_EVENT(EVT_TIME_LINE_CHANGED, wxCommandEvent);

class TimeLine : public wxWindow
{
    public:
        TimeLine(wxPanel* parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize,
                       long style=0, const wxString &name=wxPanelNameStr);
        virtual ~TimeLine();
        void Initialize();

        void SetStartTimeMS(int time);
        float GetStartTimeMS();

        void SetPlayMarkerMS(int ms);
        int GetPlayMarker();

        void SetSelectedPositionStart(int pos);
        void SetSelectedPositionStartMS(int time);
        void SetSelectedPositionEnd(int pos);
        int GetSelectedPositionStart() { return mSelectedPlayMarkerStart; }
        int GetSelectedPositionEnd() { return mSelectedPlayMarkerEnd; }
        void LatchSelectedPositions();

        void ResetMarkers(int ms);

        void SetStartPixelOffset(int startPixel);
        int GetStartPixelOffset();

        void SetTimeFrequency(int frequency);
        int GetTimeFrequency();

        void SetTimeLength(int ms);
        int GetTimeLength();

        void SetZoomLevel(int level);
        int GetZoomLevel();

        int GetZoomLevelValue();
        int GetMaxZoomLevel();
        void ZoomIn();
        void ZoomOut();

        int TimePerMajorTickInMS();
        void GetViewableTimeRange(double &StartTime, double &EndTime);

        void GetPositionsFromTimeRange(double startTime,double endTime,EFFECT_SCREEN_MODE &screenMode,int &x1, int &x2, int& x3, int& x4);
        int GetPositionFromTime(double time);
        double GetPositionFromTimeMS(int time);

        void SetSequenceEnd(int ms);

        int GetTimeMSfromPosition(int position);
        int GetAbsoluteTimeMSfromPosition(int position);
        double GetAbsoluteTimefromPosition(int position);

        static double RoundToMultipleOfPeriod(double number,double period);

        struct TIME_LINE_CHANGED_ARGS
        {
            int zoomLevel;
            int startPixel;
        };

        void PlayStarted();       // signal the start of play so timeline can adjust marks
        void PlayStopped();       // signal play stop so timeline can adjust marks
        int GetNewStartTimeMS();  // return the time where to begin playing
        int GetNewEndTimeMS();    // return the time where to end playing

        void MoveToLeft(int numberOfPixels);
        void MoveToRight(int numberOfPixels);
        void RaiseChangeTimeline();
        const static int ZoomLevelValues[];


    protected:
    private:
        DECLARE_EVENT_TABLE()
        void mouseLeftDown( wxMouseEvent& event);
        void mouseLeftUp( wxMouseEvent& event);
        void mouseMoved( wxMouseEvent& event);
        void OnLostMouseCapture(wxMouseCaptureLostEvent& event);
        void triggerPlay();
        bool mIsInitialized;
        int mStartTimeMS;
        int mEndTimeMS;
        int mEndPos;
        double mStartTime;
        double mEndTime;

        int mStartPixelOffset;
        int mFrequency;
        int mZoomLevel;
        int mMaxZoomLevel;
        int mTimeLength;
        int mCurrentPlayMarkerStart;
        int mCurrentPlayMarkerEnd;
        int mSelectedPlayMarkerStart;
        int mSelectedPlayMarkerEnd;
        int mCurrentPlayMarker;
        int mCurrentPlayMarkerStartMS;
        int mCurrentPlayMarkerEndMS;
        int mSelectedPlayMarkerStartMS;
        int mSelectedPlayMarkerEndMS;
        int mCurrentPlayMarkerMS;
        int mSequenceEndMarker;
        int mSequenceEndMarkerMS;
        bool m_dragging;
        bool timeline_initiated_play;

        void render( wxPaintEvent& event );
        float GetFirstTimeLabelFromPixelOffset(int offset);
        int GetPixelOffsetFromStartTime();
        int GetMaxViewableTimeMS();
        int GetTotalViewableTimeMS();
        int GetPositionFromSelection(int position);
        void DrawTriangleMarkerFacingLeft(wxPaintDC& dc, int& play_start_mark, const int& tri_size, int& height);
        void DrawTriangleMarkerFacingRight(wxPaintDC& dc, int& play_start_mark, const int& tri_size, int& height);
        void DrawTriangleMarkerFacingDown(wxPaintDC& dc, int& play_start_mark, const int& tri_size);
        void DrawRectangle(wxPaintDC& dc, int x1, int y1, int x2, int y2);
        void RecalcMarkerPositions();
        wxPanel* mParent;
};

class TimelineChangeArguments
{
    public:
        TimelineChangeArguments(int zoomLevel, int startPixelOffset,int currentTime);
        virtual ~TimelineChangeArguments();
        int ZoomLevel;
        int StartPixelOffset;
        int CurrentTimeMS;

};

#endif // TIMELINE_H
