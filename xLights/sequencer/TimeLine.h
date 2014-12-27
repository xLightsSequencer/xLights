#ifndef TIMELINE_H
#define TIMELINE_H

#include "wx/wx.h"
#include "wx/window.h"
#define HORIZONTAL_PADDING      10
#define PIXELS_PER_MAJOR_HASH   100

#define MAX_ZOOM_OUT_INDEX      12

wxDECLARE_EVENT(EVT_TIME_LINE_CHANGED, wxCommandEvent);

class TimeLine : public wxWindow
{
    public:
        TimeLine(wxWindow* parent);
        virtual ~TimeLine();

        void SetCanvasSize(int width,int height);

        void SetStartTime(float time);
        float GetStartTime();

        void SetStartPixelOffset(int startPixel);
        int GetStartPixelOffset();

        void SetTimeFrequency(int startPixel);
        int GetTimeFrequency();

        void SetTimeLength(int ms);

        void SetZoomLevel(int level);
        int GetZoomLevel();

        void TimeSelected(int x);

        int GetZoomLevelValue();
        void ZoomIn();
        void ZoomOut();
        void PositionPercentageMoved(int percentage);

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
        int mSelectedTimeMS;
        int mSelectedPosition;
        int mStartPixelOffset;
        int mFrequency;
        int mZoomLevel;
        int mTimeLength;
        int mViewableTimeMS;
        int mSequenceMaxSeconds;
        void render( wxPaintEvent& event );
        float GetFirstTimeLabelFromPixelOffset(int offset);
        int GetStartTimeMSfromSelectedTimeAndPosition();
        int GetPixelOffsetFromStartTime();
        int GetTimeMSfromPosition(int x);
        int GetViewableTimeMS();
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
