#include "TimeLine.h"
#include "Waveform.h"
#include "wx/wx.h"
#include "wx/brush.h"
#include "../xLightsMain.h"

wxDEFINE_EVENT(EVT_TIME_LINE_CHANGED, wxCommandEvent);

BEGIN_EVENT_TABLE(TimeLine, wxWindow)
//EVT_MOTION(TimeLine::mouseMoved)
EVT_LEFT_DOWN(TimeLine::mouseLeftDown)
EVT_LEFT_UP(TimeLine::mouseLeftUp)
//EVT_LEAVE_WINDOW(TimeLine::mouseLeftWindow)
//EVT_RIGHT_DOWN(TimeLine::rightClick)
//EVT_SIZE(TimeLine::resized)
//EVT_KEY_DOWN(TimeLine::keyPressed)
//EVT_KEY_UP(TimeLine::keyReleased)
//EVT_MOUSEWHEEL(TimeLine::mouseWheelMoved)
EVT_PAINT(TimeLine::render)
END_EVENT_TABLE()

// These values are used to calculate zoom
// 1    =   1*(1/frequency) per major tick on the scale
// 200  = 200*(1/frequency) per major tick on the scale
// etc....
const int TimeLine::ZoomLevelValues[] = {1,2,4,10,20,40,100,200,400,600,1200,2400,4800};


void TimeLine::mouseLeftDown( wxMouseEvent& event)
{
    TimeSelected(event.GetX());
}

void TimeLine::mouseLeftUp( wxMouseEvent& event)
{
}

TimeLine::TimeLine(wxPanel* parent, wxWindowID id, const wxPoint &pos, const wxSize &size,long style, const wxString &name):
                   wxWindow((wxWindow*)parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
{
    mParent = (wxPanel*)parent;
    DOUBLE_BUFFER(this);
    SetCanvasSize(1000,25);
    mIsInitialized=false;

    mStartPixelOffset = 100;
    mFrequency = 40;
    mZoomLevel = 10;
    mSequenceMaxSeconds = 300;

    mStartTimeMS = 0;
    mEndTimeMS = GetMaxViewableTimeMS();
    mStartTime = 0;
    mEndTime = (double)mEndTimeMS/(double)1000;
}

TimeLine::~TimeLine()
{
}

void TimeLine::RaiseChangeTimeline()
{
    TimelineChangeArguments *tla = new TimelineChangeArguments(mZoomLevel,mStartPixelOffset,mSelectedPosition);
    wxCommandEvent eventTimeLineChanged(EVT_TIME_LINE_CHANGED);
    eventTimeLineChanged.SetClientData((void*)tla);
    eventTimeLineChanged.SetInt(0);
    wxPostEvent(mParent, eventTimeLineChanged);
}


void TimeLine::SetStartTime(float time)
{
    mStartTimeMS = time;
    mEndTimeMS = GetMaxViewableTimeMS();
    mStartTime = (double)mStartTimeMS/(double)1000;
    mEndTime = (double)mEndTimeMS/(double)1000;
}

float TimeLine::GetStartTime()
{
    return mStartTimeMS;
}

void TimeLine::SetStartPixelOffset(int offset)
{
    mStartPixelOffset = offset;
    mStartTimeMS = GetFirstTimeLabelFromPixelOffset(mStartPixelOffset);
    mEndTimeMS = GetMaxViewableTimeMS();
    mStartTime = (double)mStartTimeMS/(double)1000;
    mEndTime = (double)mEndTimeMS/(double)1000;


}


int TimeLine::GetStartPixelOffset()
{
    return  mStartPixelOffset;
}

float TimeLine::GetFirstTimeLabelFromPixelOffset(int offset)
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

int TimeLine::GetTimeFrequency()
{
    return  mFrequency;
}

void TimeLine::SetZoomLevel(int level)
{
    mZoomLevel = level;
}

int TimeLine::GetZoomLevel()
{
    return  mZoomLevel;
}

void TimeLine::ZoomIn()
{
    if(mZoomLevel>0)
    {
        mZoomLevel--;
        mViewableTimeMS = GetMaxViewableTimeMS();
        mStartTimeMS = GetStartTimeMSfromSelectedTimeAndPosition();
        mEndTimeMS = GetMaxViewableTimeMS();

        mStartTime = (double)mStartTimeMS/(double)1000;
        mEndTime = (double)mEndTimeMS/(double)1000;

        mStartPixelOffset = GetPixelOffsetFromStartTime();
        RaiseChangeTimeline();
        Refresh();
    }
}

int TimeLine::GetStartTimeMSfromSelectedTimeAndPosition()
{
    float nMajorHashs = (float)mSelectedPosition/(float)PIXELS_PER_MAJOR_HASH;
    int startTime = (int)(mSelectedTimeMS - (float)(nMajorHashs*TimePerMajorTickInMS()));
    if(startTime < 0)
    {
        startTime = 0;
    }
    return startTime;
}

int TimeLine::GetPixelOffsetFromStartTime()
{
    float nMajorHashs = (float)mStartTimeMS/(float)TimePerMajorTickInMS();
    int offset = nMajorHashs * PIXELS_PER_MAJOR_HASH;
    return offset;
}

void TimeLine::GetPositionFromTime(double startTime,double endTime,EFFECT_SCREEN_MODE &screenMode,int &x1, int &x2)
{
    if(startTime < mStartTime && endTime > mEndTime)
    {
        screenMode = SCREEN_L_R_ACROSS;
        x1 = 0;
        x2 = GetSize().x;
    }
    else if(startTime < mStartTime && endTime <= mEndTime)
    {
        screenMode = SCREEN_R_ON;
        double majorHashs = (double)((endTime*1000) - mStartTimeMS)/(double)TimePerMajorTickInMS();
        x1=0;
        x2=(int)(majorHashs * (double)PIXELS_PER_MAJOR_HASH);
    }
    else if(startTime >= mStartTime && endTime > mEndTime)
    {
        screenMode = SCREEN_L_ON;
        double majorHashs = (double)((startTime*1000) - mStartTimeMS)/(double)TimePerMajorTickInMS();
        x1=(int)(majorHashs * (double)PIXELS_PER_MAJOR_HASH);
        x2=GetSize().x;
    }
    else if(startTime >= mStartTime && endTime <= mEndTime)
    {
        screenMode = SCREEN_L_R_ON;
        double majorHashs = (double)((startTime*1000) - mStartTimeMS)/(double)TimePerMajorTickInMS();
        x1=(int)(majorHashs * (double)PIXELS_PER_MAJOR_HASH);
        majorHashs = (double)((endTime*1000) - mStartTimeMS)/(double)TimePerMajorTickInMS();
        x2=(int)(majorHashs * (double)PIXELS_PER_MAJOR_HASH);
    }
    else if((startTime < mStartTime && endTime < mStartTime) ||
            (startTime > mStartTime && endTime > mStartTime))
    {
        screenMode = SCREEN_L_R_OFF;
        x1=0;
        x2=0;
    }

}



void TimeLine::SetTimeLength(int ms)
{
    mTimeLength = ms;
}

void TimeLine::TimeSelected(int x)
{
    mSelectedPosition = x;
    mSelectedTimeMS = GetTimeMSfromPosition(mSelectedPosition+mStartPixelOffset);
    RaiseChangeTimeline();
    Refresh();
}

int TimeLine::GetTimeMSfromPosition(int position)
{
    float nMajorHashs = (float)position/(float)PIXELS_PER_MAJOR_HASH;
    int time = (int)(nMajorHashs*TimePerMajorTickInMS());
    return time;
}

double TimeLine::GetAbsoluteTimefromPosition(int position)
{
    double nMajorHashs = (double)position/(double)PIXELS_PER_MAJOR_HASH;
    double time = mStartTime + (double)(nMajorHashs*TimePerMajorTickInMS()/(double)1000);
    return time;
}



void TimeLine::ZoomOut()
{
    if(mZoomLevel<MAX_ZOOM_OUT_INDEX)
    {
        if(mViewableTimeMS < mTimeLength)
        {
            mZoomLevel++;
            mViewableTimeMS = GetMaxViewableTimeMS();
            mStartTimeMS = GetStartTimeMSfromSelectedTimeAndPosition();
            mEndTimeMS = GetMaxViewableTimeMS();
            mStartTime = (double)mStartTimeMS/(double)1000;
            mEndTime = (double)mEndTimeMS/(double)1000;
            mStartPixelOffset = GetPixelOffsetFromStartTime();
            //if (mStartTimeMS==0)
            //{
            //    float nMajorHashs = (float)mSelectedTimeMS/(float)TimePerMajorTickInMS();
            //    mSelectedPosition = (int)(nMajorHashs * PIXELS_PER_MAJOR_HASH);
            //}
        }
        else
        {
            int selectedTime = mStartTimeMS+GetTimeMSfromPosition(mSelectedPosition);
            mStartTimeMS = 0;
            mStartPixelOffset = 0;
            mEndTimeMS = GetMaxViewableTimeMS();
            mStartTime = 0;
            mEndTime = (double)mEndTimeMS/(double)1000;

            float nMajorHashs = (float)mSelectedTimeMS/(float)TimePerMajorTickInMS();
            mSelectedPosition = (int)(nMajorHashs * PIXELS_PER_MAJOR_HASH);
        }
        Refresh();
        RaiseChangeTimeline();
    }
}

int TimeLine::GetMaxViewableTimeMS()
{
    wxSize s = GetSize();
    int majorTicks = s.GetWidth()/PIXELS_PER_MAJOR_HASH;
    return (majorTicks * TimePerMajorTickInMS()) + mStartTimeMS;
}

int TimeLine::GetZoomLevelValue()
{
    return  ZoomLevelValues[mZoomLevel];
}


void TimeLine::MoveToLeft(int numberOfPixels)
{

}

void TimeLine::MoveToRight(int numberOfPixels)
{

}


void TimeLine::SetCanvasSize(int width,int height)
{
    SetSize(width,height);
    wxSize s;
    s.SetWidth(width);
    s.SetHeight(height);
    SetMaxSize(s);
    SetMinSize(s);
}

void TimeLine::Initialize()
{
    mIsInitialized = true;
}

void TimeLine::render( wxPaintEvent& event )
{
        if (!mIsInitialized)
            return;
        wxCoord w,h;
        float t;
        int labelCount=0;
        wxPaintDC dc(this);
        wxPen pen(wxColor(128,128,128), .5); // red pen of width 1
        dc.SetPen(pen);
        dc.GetSize(&w,&h);
        wxBrush brush(wxColor(212,208,200),wxBRUSHSTYLE_SOLID);
        dc.SetBrush(brush);
        dc.DrawRectangle(0,0,w,h+1);

        wxFont f = dc.GetFont();
        f.SetPointSize(7.0);
        dc.SetFont(f);

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
                dc.DrawLine(x,20,x,h-1);
            }
            // Draw time label
            if((x+mStartPixelOffset)%PIXELS_PER_MAJOR_HASH==0)
            {
                t = GetTimeMSfromPosition(x+mStartPixelOffset);
                wxRect r(x-25,h-20,50,12);
                minutes = t/60000;
                seconds = (t-(minutes*60000))/1000;
                subsecs = t - (minutes*60000 + seconds*1000);
                if (subsecs%5!=0)
                {
                    int h=0;
                }
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
        dc.DrawLine(mSelectedPosition,0,mSelectedPosition,h-1);

}

int TimeLine::TimePerMajorTickInMS()
{
    return (int)((double)ZoomLevelValues[mZoomLevel] * ((double)1/(double)mFrequency) * 1000);
}

void TimeLine::GetViewableTimeRange(double &StartTime, double &EndTime)
{
    StartTime = mStartTime;
    EndTime = mEndTime;
}

TimelineChangeArguments::TimelineChangeArguments(int zoomLevel, int startPixelOffset,int selectedPosition)
{
    ZoomLevel = zoomLevel;
    StartPixelOffset = startPixelOffset;
    SelectedPosition = selectedPosition;
}

TimelineChangeArguments::~TimelineChangeArguments()
{
}
