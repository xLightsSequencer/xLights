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
        void SetCanvasSize(int width,int height);

        void SetStartTimeMS(int time);
        float GetStartTimeMS();

        void SetStartPixelOffset(int startPixel);
        int GetStartPixelOffset();

        void SetTimeFrequency(int startPixel);
        int GetTimeFrequency();

        void SetTimeLength(int ms);
        int GetTimeLength();

        void SetZoomLevel(int level);
        int GetZoomLevel();

        void TimeSelected(int x);

        int GetZoomLevelValue();
        int GetMaxZoomLevel();
        void ZoomIn();
        void ZoomOut();

        int TimePerMajorTickInMS();
        void GetViewableTimeRange(double &StartTime, double &EndTime);

        void GetPositionFromTime(double startTime,double endTime,EFFECT_SCREEN_MODE &screenMode,int &x1, int &x2);

        int GetTimeMSfromPosition(int position);
        double GetAbsoluteTimefromPosition(int position);

        static double RoundToMultipleOfPeriod(double number,double period);

        struct TIME_LINE_CHANGED_ARGS
        {
            int zoomLevel;
            int startPixel;
        };



        void MoveToLeft(int numberOfPixels);
        void MoveToRight(int numberOfPixels);
        void RaiseChangeTimeline();
        const static int ZoomLevelValues[];


    protected:
    private:
        DECLARE_EVENT_TABLE()
        void mouseLeftDown( wxMouseEvent& event);
        void mouseLeftUp( wxMouseEvent& event);
        bool mIsInitialized;
        int mStartTimeMS;
        int mEndTimeMS;
        double mStartTime;
        double mEndTime;

        int mSelectedTimeMS;
        int mSelectedPosition;
        int mStartPixelOffset;
        int mFrequency;
        int mZoomLevel;
        int mMaxZoomLevel;
        int mTimeLength;
        int mViewableTimeMS;
        void render( wxPaintEvent& event );
        float GetFirstTimeLabelFromPixelOffset(int offset);
        int GetStartTimeMSfromSelectedTimeAndPosition();
        int GetPixelOffsetFromStartTime();
        int GetMaxViewableTimeMS();
        int GetTotalViewableTimeMS();
        wxPanel* mParent;
};

class TimelineChangeArguments
{
    public:
        TimelineChangeArguments(int zoomLevel, int startPixelOffset,int SelectedPosition);
        virtual ~TimelineChangeArguments();
        int ZoomLevel;
        int StartPixelOffset;
        int SelectedPosition;

};

#endif // TIMELINE_H
