#ifdef __WXMAC__
 #include "OpenGL/glu.h"
 #include "OpenGL/gl.h"
#else
// #define GLEW_STATIC
 #include <GL/glu.h>
 #include <GL/gl.h>
// #include <GL/glext.h>
#endif

#include "wx/wx.h"
#include "wx/sizer.h"
#include "wx/glcanvas.h"
#define INBUFF  16384
#define OUTBUFF 32768
#include "mpg123.h"

#include <wx/event.h>
#include <wx/timer.h>
#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/settings.h>
#include <wx/font.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>

#include "Waveform.h"
#include "TimeLine.h"
#include <wx/file.h>
//#include "glut.h"

enum
{
    SCROLL_TIMER_LEFT= 0,
    SCROLL_TIMER_RIGHT= 1
};


wxDEFINE_EVENT(EVT_WAVE_FORM_MOVED, wxCommandEvent);
BEGIN_EVENT_TABLE(Waveform, wxGLCanvas)
//EVT_TIMER(SCROLL_TIMER_LEFT, Waveform::OnWaveScrollLeft)
//EVT_TIMER(SCROLL_TIMER_RIGHT, Waveform::OnWaveScrollRight)
EVT_MOTION(Waveform::mouseMoved)
EVT_LEFT_DOWN(Waveform::mouseLeftDown)
EVT_LEFT_UP(Waveform::mouseLeftUp)
EVT_LEAVE_WINDOW(Waveform::mouseLeftWindow)
EVT_LEFT_DCLICK(Waveform::OnLeftDClick)
EVT_IDLE(Waveform::OnIdle)

//EVT_RIGHT_DOWN(ModelPreview::rightClick)
//EVT_SIZE(ModelPreview::resized)
//EVT_KEY_DOWN(ModelPreview::keyPressed)
//EVT_KEY_UP(ModelPreview::keyReleased)
//EVT_MOUSEWHEEL(ModelPreview::mouseWheelMoved)
EVT_PAINT(Waveform::render)
END_EVENT_TABLE()
// Custom Events


Waveform::Waveform(wxPanel* parent, wxWindowID id, const wxPoint &pos, const wxSize &size,
                   long style, const wxString &name):
                   wxGLCanvas(parent,wxID_ANY,nullptr, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
{
    m_left_data = NULL;
    m_right_data = NULL;
    tmrScrollLeft = NULL;
    tmrScrollRight = NULL;

    mIsInitialized = false;
    mParent = parent;
    m_context = new wxGLContext(this);
    mCurrentWaveView = NO_WAVE_VIEW_SELECTED;
    mZoomLevel=0;
    mStartPixelOffset = 0;
    SetTimeFrequency(40);
    tmrScrollLeft = new wxTimer(this,SCROLL_TIMER_LEFT);
    tmrScrollRight = new wxTimer(this,SCROLL_TIMER_RIGHT);
    mPaintOnIdleCounter = 0;
}

Waveform::~Waveform()
{
    if (m_left_data) delete m_left_data;
    if (m_right_data) delete m_right_data;
    if (tmrScrollLeft) delete tmrScrollLeft;
    if (tmrScrollRight) delete tmrScrollRight;
}

void Waveform::CloseMediaFile()
{
    mIsInitialized = false;
    views.clear();
}



void Waveform::OnIdle(wxIdleEvent &event)
{
    // It will only repaint on idle for 5 times
    // mPaintOnIdleCounter is reset to "0".
    if(!mIsInitialized){return;}
    if(mPaintOnIdleCounter < 5)
    {
        Refresh(false);
        mPaintOnIdleCounter++;
    }
}

void Waveform::SetTimeline(TimeLine* timeLine)
{
    mTimeline = timeLine;
}

void Waveform::OnLeftDClick(wxMouseEvent& event)
{
    if(!mIsInitialized){return;}
    // Zoom on double click
    wxCommandEvent eventZoom(EVT_ZOOM);

    if(event.ShiftDown() || event.ControlDown()){eventZoom.SetInt(ZOOM_OUT);}
    else {eventZoom.SetInt(ZOOM_IN);}

    wxPostEvent(GetParent(), eventZoom);
}

void Waveform::OnWaveScrollLeft(wxTimerEvent& event)
{
    if(!mIsInitialized){return;}
    wxCommandEvent eventWaveMoved(EVT_WAVE_FORM_MOVED);
    int offset = SetStartPixelOffset(mStartPixelOffset+25);
    eventWaveMoved.SetInt(offset);
    wxPostEvent(mParent, eventWaveMoved);
    Refresh();
}

void Waveform::OnWaveScrollRight(wxTimerEvent& event)
{
    if(!mIsInitialized){return;}
    wxCommandEvent eventWaveMoved(EVT_WAVE_FORM_MOVED);
    int offset = SetStartPixelOffset(mStartPixelOffset-25);
    eventWaveMoved.SetInt(offset);
    wxPostEvent(mParent, eventWaveMoved);
    Refresh();
}

void Waveform::mouseLeftWindow( wxMouseEvent& event)
{
    if (mIsInitialized)
    {
        m_dragging = false;
        if(m_scrolling)
            StopScrolling();
    }
}

void Waveform::mouseLeftDown( wxMouseEvent& event)
{
    if (mIsInitialized)
    {
        m_dragging = true;
        m_shaded_region_x1 = event.GetPosition().x;
        m_shaded_region_x2 = event.GetPosition().x;
        wxCommandEvent eventTimeSelected(EVT_TIME_SELECTED);
        eventTimeSelected.SetInt(event.GetX());
        wxPostEvent(mParent, eventTimeSelected);
        Refresh();
    }
}

void Waveform::mouseLeftUp( wxMouseEvent& event)
{
    m_dragging = false;
}

void Waveform::mouseMoved( wxMouseEvent& event)
{
    if(!mIsInitialized){return;}
    if (m_dragging)
    {
        m_shaded_region_x2 = event.GetPosition().x;

         double time = mTimeline->GetAbsoluteTimefromPosition(m_shaded_region_x2);
        // Round to nearest period
        time = mTimeline->RoundToMultipleOfPeriod(time,mFrequency);
        // Recalulate Position with corrected time
        m_shaded_region_x2 = mTimeline->GetPositionFromTime(time);
        // Force refresh on idle
        mPaintOnIdleCounter=0;
        Refresh(false);
    }

    if(event.GetPosition().x > getWidth()-WAVEFORM_SIDE_MARGIN)
    {
        ScrollWaveLeft(event.GetPosition().x-(getWidth()-WAVEFORM_SIDE_MARGIN));
    }
    else if (event.GetPosition().x < WAVEFORM_SIDE_MARGIN)
    {
        ScrollWaveRight(WAVEFORM_SIDE_MARGIN - event.GetPosition().x);
    }
    else
    {
       StopScrolling();
    }
}

void Waveform::StopScrolling()
{
    m_scrolling=false;
    tmrScrollLeft->Stop();
    tmrScrollRight->Stop();
}

void Waveform::ScrollWaveLeft(int xBasedSpeed)
{
    if(!mIsInitialized){return;}

    m_scrolling = true;
    if(tmrScrollLeft->IsRunning())
        tmrScrollLeft->Stop();
    tmrScrollLeft->Start(WAVEFORM_SIDE_MARGIN-xBasedSpeed);
}

void Waveform::ScrollWaveRight(int xBasedSpeed)
{
    if(!mIsInitialized){return;}

    m_scrolling = true;
    if(tmrScrollRight->IsRunning())
        tmrScrollRight->Stop();
    tmrScrollRight->Start(WAVEFORM_SIDE_MARGIN-xBasedSpeed);
}

// Open Media file and return elapsed time in millseconds
int Waveform::OpenfileMediaFile(const char* filename)
{
    mpg123_handle *mh;
    int err;

    int channels, encoding;
    long rate;

    mpg123_init();
    mh = mpg123_new(NULL, &err);
    /* open the file and get the decoding format */
    mpg123_open(mh, filename);
    mpg123_getformat(mh, &rate, &channels, &encoding);
    /* set the output format and open the output device */
    m_bits = mpg123_encsize(encoding);
    m_rate = rate;
    m_channels = channels;
    /* Get Track Size */
    mMediaTrackSize = GetTrackSize(mh,m_bits,m_channels);
    char * trackData = (char*)malloc(mMediaTrackSize*m_bits*m_channels);
    LoadTrackData(mh,trackData);
    // Split data into left and right and normalize -1 to 1
    m_left_data = (float*)malloc(sizeof(float)*mMediaTrackSize);
    m_right_data = (float*)malloc(sizeof(float)*mMediaTrackSize);
    SplitTrackDataAndNormalize((signed short*)trackData,mMediaTrackSize,m_left_data,m_right_data);
    views.clear();
    int samplesPerLine = GetSamplesPerLineFromZoomLevel(mZoomLevel);
    WaveView wv(mZoomLevel,samplesPerLine,m_left_data,mMediaTrackSize);
    views.push_back(wv);
    mCurrentWaveView = 0;
    mIsInitialized = true;

    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    free(trackData);
    float seconds = (float)mMediaTrackSize * ((float)1/(float)rate);
    return (int)(seconds * (float)1000);
}

int Waveform::GetLengthOfMusicFileInMS(const char* filename)
{
    mpg123_handle *mh;
    int err;
    int channels, encoding;
    long rate;
    mpg123_init();
    mh = mpg123_new(NULL, &err);
    /* open the file and get the decoding format */
    mpg123_open(mh, filename);
    mpg123_getformat(mh, &rate, &channels, &encoding);
    int bits = mpg123_encsize(encoding);

    /* Get Track Size */
    int trackSize = GetTrackSize(mh,bits,channels);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    float seconds = (float)trackSize * ((float)1/(float)rate);
    return (int)(seconds * (float)1000);
}


void Waveform::SplitTrackDataAndNormalize(signed short* trackData,int trackSize, float* leftData, float* rightData)
{
    signed short lSample,rSample;
    for(int i=0;i<trackSize;i++)
    {
        lSample = trackData[i*2];
        leftData[i] = (float)lSample/(float)32768;
        rSample = trackData[(i*2)+1];
        rightData[i] = (float)rSample/(float)32768;
    }
}

void Waveform::LoadTrackData(mpg123_handle *mh,char  * data)
{
    size_t buffer_size;
    unsigned char *buffer;
    size_t done;
    int bytesRead=0;
    buffer_size = mpg123_outblock(mh);
    buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));
    mpg123_seek(mh,0,SEEK_SET);
    for (bytesRead = 0 ; mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK ; )
    {
        memcpy(data+bytesRead,buffer,done);
        bytesRead+=done;
    }
    if (done> 0)
    {
        memcpy(data+bytesRead,buffer,done);
        bytesRead+=done;
    }

    free(buffer);

}

int Waveform::GetTrackSize(mpg123_handle *mh,int bits, int channels)
{
    size_t buffer_size;
    unsigned char *buffer;
    size_t done;
    int trackSize=0;
    int fileSize=0;

    if(mpg123_length(mh) > 0)
    {
        return mpg123_length(mh);
    }

    buffer_size = mpg123_outblock(mh);
    buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));

    mpg123_seek(mh,0,SEEK_SET);
    for (fileSize = 0 ; mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK ; )
    {
        fileSize += done;
    }
    // Get size of last read and add it to size
    if (done> 0)
    {
        fileSize += done;
    }
    free(buffer);
    // Debug
    trackSize = fileSize/(bits*channels);
    return trackSize;
}

void Waveform::render( wxPaintEvent& event )
{
    if(!mIsInitialized){return;}

    wxGLCanvas::SetCurrent(*m_context);
    wxPaintDC(this); // only to be used in paint events. use wxClientDC to paint outside the paint event
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    prepare2DViewport(0,0,getWidth(), getHeight());
    DrawWaveView(views[mCurrentWaveView]); // continue the event
    glFlush();
    SwapBuffers();
}

void Waveform::DrawWaveView(const WaveView &wv)
{
    int x,y1,y2,y1_2,y2_2;
    int index;
    glColor3ub(212,208,200);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(getWidth(), 0);
    glVertex2f(getWidth(), getHeight());
    glVertex2f(0,getHeight());
    glEnd();

    int max_wave_ht = getHeight() - VERTICAL_PADDING;


    // Draw Outside rectangle
    glPointSize( .3 );
    glColor3ub(128,128,128);
    glBegin(GL_LINES);
    glVertex2f(1, 0);
    glVertex2f(getWidth(), 1);
    glVertex2f(getWidth(), 1);
    glVertex2f(getWidth(), getHeight());
    glVertex2f(getWidth(), getHeight());
    glVertex2f(1,getHeight());
    glVertex2f(1,getHeight());
    glVertex2f(1, 0);
    glEnd();

    if (m_shaded_region_x1!=m_shaded_region_x2)
    {
        glColor4ub(0,0,200,45);
        glEnable(GL_BLEND);
        glBegin(GL_QUADS);
        glVertex2f(m_shaded_region_x1, 1);
        glVertex2f(m_shaded_region_x2, 1);
        glVertex2f(m_shaded_region_x2, getHeight()-1);
        glVertex2f(m_shaded_region_x1,getHeight()-1);
        glEnd();
        glDisable(GL_BLEND);
    }

    int minMax = wv.MinMaxs.size();
    for (int x=0;x<getWidth() && (x)<wv.MinMaxs.size();x++)
    {
        index = x+mStartPixelOffset;
        if (index >= 0 && index < wv.MinMaxs.size())
        {
            y1 = (int)((wv.MinMaxs[index].min * (float)(max_wave_ht/2))+ (getHeight()/2));
            y2 = (int)((wv.MinMaxs[index].max * (float)(max_wave_ht/2))+ (getHeight()/2));

            if(y1 == y2)
            {
                glColor3ub(130,178,207);
                glBegin(GL_POINTS);
                glVertex2f(x,y1);
                glEnd();
            }
            else
            {
                glPointSize( 1 );
                glColor3ub(130,178,207);
                glBegin(GL_LINES);
                glVertex2f(x, y1);
                glVertex2f(x, y2);
                glEnd();
            }

        }
    }

    glBegin(GL_LINES);
    glPointSize( 2 );
    glColor3ub(255,255,255);
    for(int x=0;x<getWidth()-1 && (x)<wv.MinMaxs.size();x++)
    {
        index = x + mStartPixelOffset ;
        if (index >= 0 && index < wv.MinMaxs.size())
        {
            y1 = (int)((wv.MinMaxs[mStartPixelOffset+x].min * (float)(max_wave_ht/2))+ (getHeight()/2));
            y2 = (int)((wv.MinMaxs[mStartPixelOffset+x].max * (float)(max_wave_ht/2))+ (getHeight()/2));
            if(y1 != y2)
            {
                if(x<wv.MinMaxs.size()-1)
                {
                    y1_2 = (int)((wv.MinMaxs[mStartPixelOffset+x+1].min * (float)(max_wave_ht/2))+ (getHeight()/2));
                    y2_2 = (int)((wv.MinMaxs[mStartPixelOffset+x+1].max * (float)(max_wave_ht/2))+ (getHeight()/2));
                    glVertex2f(x, y1);
                    glVertex2f(x+1, y1_2);

                    glVertex2f(x, y2);
                    glVertex2f(x+1, y2_2);
                }
            }
        }
    }
    glEnd();

    glColor4ub(0,0,0,128);
    glBegin(GL_LINES);
    glVertex2f(mSelectedPosition+1, 1);
    glVertex2f(mSelectedPosition+1,getHeight()-1);
    if(m_shaded_region_x1!=m_shaded_region_x2)
    {
        glVertex2f(m_shaded_region_x2+1, 1);
        glVertex2f(m_shaded_region_x2+1,getHeight()-1);
    }
    glEnd();
}

void Waveform::prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    // Rotate Axis and tranlate
    glRotatef(180,0,0,1);
    glRotatef(180,0,1,0);
    glTranslatef(0,-getHeight(),0);
    // Set view port
    glViewport(topleft_x, topleft_y, bottomrigth_x-topleft_x, bottomrigth_y-topleft_y);


    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(topleft_x, bottomrigth_x, bottomrigth_y, topleft_y, -1, 1);
    glMatrixMode(GL_MODELVIEW);
}

int Waveform::getWidth()
{
    return GetSize().x;
}

int Waveform::getHeight()
{
    return GetSize().y;
}

void Waveform::SetZoomLevel(int level)
{
    if(!mIsInitialized){return;}

    mCurrentWaveView = NO_WAVE_VIEW_SELECTED;
    mZoomLevel = level;
    for(int i=0;i<views.size();i++)
    {
        if(views[i].GetZoomLevel() == mZoomLevel)
        {
            mCurrentWaveView = i;
        }
    }
    if (mCurrentWaveView == NO_WAVE_VIEW_SELECTED)
    {
        float samplesPerLine = GetSamplesPerLineFromZoomLevel(mZoomLevel);
        WaveView wv(mZoomLevel,samplesPerLine,m_left_data,mMediaTrackSize);
        views.push_back(wv);
        mCurrentWaveView = views.size()-1;
    }
}

int Waveform::GetZoomLevel()
{
    return  mZoomLevel;
}

int Waveform::SetStartPixelOffset(int offset)
{
    if (mCurrentWaveView != NO_WAVE_VIEW_SELECTED)
    {
       mStartPixelOffset = offset;
    }
    else
    {
        mStartPixelOffset = 0;
    }
    return mStartPixelOffset;
}

void Waveform::PositionSelected(int x)
{
    mSelectedPosition = x;
    m_shaded_region_x1 = x;
    m_shaded_region_x2 = x;
    Refresh();
    Update();
}



int Waveform::GetStartPixelOffset()
{
    return  mStartPixelOffset;
}

void Waveform::SetTimeFrequency(int frequency)
{
    mFrequency = frequency;
}

int Waveform::GetTimeFrequency()
{
    return  mFrequency;
}

void Waveform::SetShadedRegion(int x1,int x2)
{
    m_shaded_region_x1 = x1;
    m_shaded_region_x1 = x2;
}

void Waveform::GetShadedRegion(int* x1, int* x2)
{

    *x1 = m_shaded_region_x1;
    *x2 = m_shaded_region_x2;
}


float Waveform::GetSamplesPerLineFromZoomLevel(int ZoomLevel)
{
    // period           = 1/frequency
    // Frequency        = number of frames per second.
    // mMediaTrackSize  = number of samples in media track

    // The number of periods for each Zoomlevel is held in ZoomLevelValues array
    int periodsPerMajorHash = TimeLine::ZoomLevelValues[mZoomLevel];
    float timePerPixel = ((float)periodsPerMajorHash/(float)mFrequency)/(float)PIXELS_PER_MAJOR_HASH;
    float samplesPerPixel = (float)timePerPixel * (float)m_rate;
//    if (mZoomLevel==2)
//       samplesPerPixel+= .7;
    return samplesPerPixel;
}




