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

wxDEFINE_EVENT(EVT_WAVE_FORM_MOVED, wxCommandEvent);

BEGIN_EVENT_TABLE(Waveform, xlGLCanvas)
EVT_MOTION(Waveform::mouseMoved)
EVT_LEFT_DOWN(Waveform::mouseLeftDown)
EVT_LEFT_UP(Waveform::mouseLeftUp)
EVT_LEFT_DCLICK(Waveform::OnLeftDClick)
EVT_MOUSE_CAPTURE_LOST(Waveform::OnLostMouseCapture)
EVT_SIZE(Waveform::Resized)
EVT_MOUSEWHEEL(Waveform::mouseWheelMoved)
EVT_PAINT(Waveform::renderGL)
END_EVENT_TABLE()


Waveform::Waveform(wxPanel* parent, wxWindowID id, const wxPoint &pos, const wxSize &size,
                   long style, const wxString &name):
                   xlGLCanvas(parent,wxID_ANY,wxDefaultPosition, wxDefaultSize)
{
    m_left_data = NULL;
    m_right_data = NULL;
    mAudioIsLoaded = false;
    m_dragging = false;
    mParent = parent;
    mCurrentWaveView = NO_WAVE_VIEW_SELECTED;
    mZoomLevel=0;
    mStartPixelOffset = 0;
    mFrequency = 40;
}

Waveform::~Waveform()
{
    CloseMediaFile();
}

void Waveform::CloseMediaFile()
{
    views.clear();
    if (m_left_data)
    {
        delete m_left_data;
        m_left_data = nullptr;
    }
    if (m_right_data)
    {
        delete m_right_data;
        m_right_data = nullptr;
    }
    mCurrentWaveView = NO_WAVE_VIEW_SELECTED;
    mAudioIsLoaded = false;
}

void Waveform::OnLostMouseCapture(wxMouseCaptureLostEvent& event)
{
    m_dragging = false;
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

void Waveform::UpdatePlayMarker()
{
    Refresh(false);
    Update();
}

void Waveform::CheckNeedToScroll()
{
    double StartTime;
    double EndTime;
    mTimeline->GetViewableTimeRange(StartTime, EndTime);
    int scroll_point = mTimeline->GetPositionFromTime(EndTime) * 0.99;
    if(mTimeline->GetPlayMarker() > scroll_point)
    {
        wxCommandEvent eventScroll(EVT_SCROLL_RIGHT);
        wxPostEvent(mParent, eventScroll);
    }
}

void Waveform::mouseLeftDown( wxMouseEvent& event)
{
    if(!mIsInitialized){return;}
    if( !m_dragging )
    {
        m_dragging = true;
        CaptureMouse();
    }
    mTimeline->SetSelectedPositionStart(event.GetX());
    SetFocus();
    Refresh(false);
}

void Waveform::mouseLeftUp( wxMouseEvent& event)
{
    if(m_dragging)
    {
        ReleaseMouse();
        m_dragging = false;
    }

    mTimeline->LatchSelectedPositions();

    Refresh(false);
}

void Waveform::mouseMoved( wxMouseEvent& event)
{
    if(!mIsInitialized){return;}
    if (m_dragging)
    {
        mTimeline->SetSelectedPositionEnd(event.GetX());
        Refresh(false);
    }
}

void Waveform::mouseWheelMoved(wxMouseEvent& event)
{
    if(event.CmdDown())
    {
        int i = event.GetWheelRotation();
        if(i<0)
        {
            wxCommandEvent eventZoom(EVT_ZOOM);
            eventZoom.SetInt(ZOOM_OUT);
            wxPostEvent(mParent, eventZoom);
        }
        else
        {
            wxCommandEvent eventZoom(EVT_ZOOM);
            eventZoom.SetInt(ZOOM_IN);
            wxPostEvent(mParent, eventZoom);
        }
    }
    else
    {
        wxPostEvent(GetParent()->GetEventHandler(), event);
        event.Skip();
    }
}

void Waveform::cleanup(mpg123_handle *mh)
{
    /* It's really to late for error checks here;-) */
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
}

// Open Media file and return elapsed time in millseconds
int Waveform::OpenfileMediaFile(const char* filename, wxString& error)
{
    mpg123_handle *mh = NULL;
    int err;
    size_t buffer_size;
    int channels, encoding;
    long rate;

    err = mpg123_init();
    if(err != MPG123_OK || (mh = mpg123_new(NULL, &err)) == NULL)
    {
        error = wxString::Format("Basic setup goes wrong: %s", mpg123_plain_strerror(err));
        if (mh != NULL) {
            cleanup(mh);
        }
        return -1;
    }

    /* open the file and get the decoding format */
    if( mpg123_open(mh, filename) != MPG123_OK ||
        mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK )
    {
        error = wxString::Format("Trouble with mpg123: %s", mpg123_strerror(mh));
        cleanup(mh);
        return -1;
    }

    if( encoding != MPG123_ENC_SIGNED_16 )
    {
        error = "Encoding unsupported.  Must be signed 16 bit.";
        cleanup(mh);
        return -2;
    }

    /* set the output format and open the output device */
    m_bits = mpg123_encsize(encoding);
    m_rate = rate;
    m_channels = channels;
    /* Get Track Size */
    mMediaTrackSize = GetTrackSize(mh,m_bits,m_channels);
    buffer_size = mpg123_outblock(mh);
    int size = (mMediaTrackSize+buffer_size)*m_bits*m_channels;
    char * trackData = (char*)malloc(size);
    LoadTrackData(mh,trackData, size);
    // Split data into left and right and normalize -1 to 1
    m_left_data = (float*)malloc(sizeof(float)*mMediaTrackSize);
    if( m_channels == 2 )
    {
        m_right_data = (float*)malloc(sizeof(float)*mMediaTrackSize);
        SplitTrackDataAndNormalize((signed short*)trackData,mMediaTrackSize,m_left_data,m_right_data);
    }
    else if( m_channels == 1 )
    {
        NormalizeMonoTrackData((signed short*)trackData,mMediaTrackSize,m_left_data);
    }
    else
    {
        error = "More than 2 audio channels is not supported yet.";
    }
    views.clear();
    int samplesPerLine = GetSamplesPerLineFromZoomLevel(mZoomLevel);
    WaveView wv(mZoomLevel,samplesPerLine,m_left_data,mMediaTrackSize);
    views.push_back(wv);
    mCurrentWaveView = 0;

    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    free(trackData);
    float seconds = (float)mMediaTrackSize * ((float)1/(float)rate);
    mAudioIsLoaded = true;
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

void Waveform::NormalizeMonoTrackData(signed short* trackData,int trackSize,float* leftData)
{
    signed short lSample;
    for(int i=0;i<trackSize;i++)
    {
        lSample = trackData[i];
        leftData[i] = (float)lSample/(float)32768;
    }
}

void Waveform::LoadTrackData(mpg123_handle *mh,char  * data, int maxSize)
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
        if ((bytesRead + done) >= maxSize) {
            wxMessageBox("Error reading data from mp3, too much data read.");
            free(buffer);
            return;
        }
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

    free(buffer);
    trackSize = fileSize/(bits*channels);
    return trackSize;
}

void Waveform::InitializeGLCanvas()
{
    if(!IsShownOnScreen()) return;
    SetCurrentGLContext();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Black Background
    glDisable(GL_TEXTURE_2D);   // textures
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    prepare2DViewport(0,0,mWindowWidth, mWindowHeight);
    glLoadIdentity();
    mIsInitialized = true;
    SetZoomLevel(mZoomLevel);
}

void Waveform::renderGL( wxPaintEvent& event )
{
    if(!mIsInitialized) { InitializeGLCanvas(); }

    if(!IsShownOnScreen()) return;

    SetCurrentGLContext();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if( mWindowResized )
    {
        prepare2DViewport(0,0,mWindowWidth, mWindowHeight);
    }
    DrawWaveView(views[mCurrentWaveView]);

    glFlush();
    SwapBuffers();
}

void Waveform::DrawWaveView(const WaveView &wv)
{
    int y1,y2,y1_2,y2_2;
    int index;
    glColor3ub(212,208,200);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(mWindowWidth, 0);
    glVertex2f(mWindowWidth, mWindowHeight);
    glVertex2f(0,mWindowHeight);
    glEnd();

    int max_wave_ht = mWindowHeight - VERTICAL_PADDING;


    // Draw Outside rectangle
    glPointSize( translateToBacking(.3) );
    glColor3ub(128,128,128);
    glBegin(GL_LINES);
    glVertex2f(1, 0);
    glVertex2f(mWindowWidth, 1);
    glVertex2f(mWindowWidth, 1);
    glVertex2f(mWindowWidth, mWindowHeight);
    glVertex2f(mWindowWidth, mWindowHeight);
    glVertex2f(1,mWindowHeight);
    glVertex2f(1,mWindowHeight);
    glVertex2f(1, 0);
    glEnd();

    // Get selection positions from timeline
    int selected_x1 = mTimeline->GetSelectedPositionStart();
    int selected_x2 = mTimeline->GetSelectedPositionEnd();

    // draw shaded region if needed
    if( selected_x1 != -1 && selected_x2 != -1)
    {
        glColor4ub(0,0,200,45);
        glEnable(GL_BLEND);
        glBegin(GL_QUADS);
        glVertex2f(selected_x1, 1);
        glVertex2f(selected_x2, 1);
        glVertex2f(selected_x2, mWindowHeight-1);
        glVertex2f(selected_x1,mWindowHeight-1);
        glEnd();
        glDisable(GL_BLEND);
    }

    if( mAudioIsLoaded )
    {
        
        glPointSize( translateToBacking(1.1) );
        glLineWidth( 1.1 );
        glColor3ub(130,178,207);
        for (int x=0;x<mWindowWidth && (x)<wv.MinMaxs.size();x++)
        {
            index = x+mStartPixelOffset;
            if (index >= 0 && index < wv.MinMaxs.size())
            {
                y1 = (int)((wv.MinMaxs[index].min * (float)(max_wave_ht/2))+ (mWindowHeight/2));
                y2 = (int)((wv.MinMaxs[index].max * (float)(max_wave_ht/2))+ (mWindowHeight/2));

                if(y1 == y2)
                {
                    glBegin(GL_POINTS);
                    glVertex2f(x,y1);
                    glEnd();
                }
                else
                {
                    glBegin(GL_LINES);
                    glVertex2f(x, y1);
                    glVertex2f(x, y2);
                    glEnd();
                }

            }
        }

        glBegin(GL_LINES);
        glColor3ub(255,255,255);
        for(int x=0;x<mWindowWidth-1 && (x)<wv.MinMaxs.size();x++)
        {
            index = x + mStartPixelOffset ;
            if (index >= 0 && index < wv.MinMaxs.size())
            {
                y1 = (int)((wv.MinMaxs[mStartPixelOffset+x].min * (float)(max_wave_ht/2))+ (mWindowHeight/2));
                y2 = (int)((wv.MinMaxs[mStartPixelOffset+x].max * (float)(max_wave_ht/2))+ (mWindowHeight/2));
                if(y1 != y2)
                {
                    if(x<wv.MinMaxs.size()-1)
                    {
                        y1_2 = (int)((wv.MinMaxs[mStartPixelOffset+x+1].min * (float)(max_wave_ht/2))+ (mWindowHeight/2));
                        y2_2 = (int)((wv.MinMaxs[mStartPixelOffset+x+1].max * (float)(max_wave_ht/2))+ (mWindowHeight/2));
                        glVertex2f(x, y1);
                        glVertex2f(x+1, y1_2);

                        glVertex2f(x, y2);
                        glVertex2f(x+1, y2_2);
                    }
                }
            }
        }
        glEnd();
    }

    // draw selection line if not a range
    if( selected_x1 != -1 && selected_x2 == -1 )
    {
        glColor4ub(0,0,0,128);
        glBegin(GL_LINES);
        glVertex2f(selected_x1, 1);
        glVertex2f(selected_x1,mWindowHeight-1);
        glEnd();
    }

    // draw play marker line
    int play_marker = mTimeline->GetPlayMarker();
    if( play_marker != -1 )
    {
        glColor4ub(0,0,0,255);
        glBegin(GL_LINES);
        glVertex2f(play_marker, 1);
        glVertex2f(play_marker,mWindowHeight-1);
        glEnd();
    }
}

void Waveform::SetZoomLevel(int level)
{
    mZoomLevel = level;

    if(!mIsInitialized){return;}

    mCurrentWaveView = NO_WAVE_VIEW_SELECTED;
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

float Waveform::GetSamplesPerLineFromZoomLevel(int ZoomLevel)
{
    // The number of periods for each Zoomlevel is held in ZoomLevelValues array
    int periodsPerMajorHash = TimeLine::ZoomLevelValues[mZoomLevel];
    float timePerPixel = ((float)periodsPerMajorHash/(float)mFrequency)/(float)PIXELS_PER_MAJOR_HASH;
    float samplesPerPixel = (float)timePerPixel * (float)m_rate;
    return samplesPerPixel;
}




