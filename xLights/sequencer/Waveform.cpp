

#include "wx/wx.h"

#ifdef __WXMAC__
 #include "OpenGL/gl.h"
#else
#ifdef _MSC_VER
#include "../GL/glut.h"
#else
#include <GL/gl.h>
#endif
#endif

#include "wx/sizer.h"
#include "wx/glcanvas.h"
#define INBUFF  16384
#define OUTBUFF 32768

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

#include "../DrawGLUtils.h"

wxDEFINE_EVENT(EVT_WAVE_FORM_MOVED, wxCommandEvent);

BEGIN_EVENT_TABLE(Waveform, xlGLCanvas)
EVT_MOTION(Waveform::mouseMoved)
EVT_LEFT_DOWN(Waveform::mouseLeftDown)
EVT_LEFT_UP(Waveform::mouseLeftUp)
EVT_LEFT_DCLICK(Waveform::OnLeftDClick)
EVT_MOUSE_CAPTURE_LOST(Waveform::OnLostMouseCapture)
EVT_LEAVE_WINDOW(Waveform::mouseLeftWindow)
EVT_SIZE(Waveform::Resized)
EVT_MOUSEWHEEL(Waveform::mouseWheelMoved)
EVT_PAINT(Waveform::renderGL)
END_EVENT_TABLE()


Waveform::Waveform(wxPanel* parent, wxWindowID id, const wxPoint &pos, const wxSize &size,
                   long style, const wxString &name):
                   xlGLCanvas(parent,wxID_ANY,wxDefaultPosition, wxDefaultSize, 0, "", true)
{
    m_dragging = false;
    m_drag_mode = DRAG_NORMAL;
    mParent = parent;
    mCurrentWaveView = NO_WAVE_VIEW_SELECTED;
    mZoomLevel=0;
    mStartPixelOffset = 0;
    mFrequency = 40;
}

Waveform::~Waveform()
{
    CloseMedia();
}

void Waveform::CloseMedia()
{
    views.clear();
    mCurrentWaveView = NO_WAVE_VIEW_SELECTED;
	_media = NULL;
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
    renderGL();
}

void Waveform::CheckNeedToScroll()
{
    int StartTime;
    int EndTime;
    mTimeline->GetViewableTimeRange(StartTime, EndTime);
    int scroll_point = mTimeline->GetPositionFromTimeMS(EndTime) * 0.99;
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
    if( m_drag_mode == DRAG_NORMAL )
    {
        mTimeline->SetSelectedPositionStart(event.GetX());
    }
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
        if( m_drag_mode == DRAG_LEFT_EDGE )
        {
            mTimeline->SetSelectedPositionStart(event.GetX(), false);
        }
        else
        {
            mTimeline->SetSelectedPositionEnd(event.GetX());
        }
        Refresh(false);
    }
    else
    {
        int selected_x1 = mTimeline->GetSelectedPositionStart();
        int selected_x2 = mTimeline->GetSelectedPositionEnd();
        if( event.GetX() >= selected_x1 && event.GetX() < selected_x1+6 )
        {
            SetCursor(wxCURSOR_POINT_LEFT);
            m_drag_mode = DRAG_LEFT_EDGE;
        }
        else if( event.GetX() > selected_x2-6 && event.GetX() <= selected_x2 )
        {
            SetCursor(wxCURSOR_POINT_RIGHT);
            m_drag_mode = DRAG_RIGHT_EDGE;
        }
        else
        {
            SetCursor(wxCURSOR_ARROW);
            m_drag_mode = DRAG_NORMAL;
        }
    }
    int mouseTimeMS = mTimeline->GetAbsoluteTimeMSfromPosition(event.GetX());
    UpdateMousePosition(mouseTimeMS);
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

// Open Media file and return elapsed time in millseconds
int Waveform::OpenfileMedia(AudioManager* media, wxString& error)
{
	_media = media;
    views.clear();
	if (_media != NULL)
	{
		float samplesPerLine = GetSamplesPerLineFromZoomLevel(mZoomLevel);
		WaveView wv(mZoomLevel, samplesPerLine, media);
		views.push_back(wv);
		mCurrentWaveView = 0;
		return media->LengthMS();
	}
	else
	{
		return 0;
	}
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
    mIsInitialized = true;
    SetZoomLevel(mZoomLevel);
}

void Waveform::renderGL( wxPaintEvent& event )
{
    renderGL();
}
void Waveform::renderGL( )
{
    if(!mIsInitialized) { InitializeGLCanvas(); }

    if(!IsShownOnScreen()) return;

    SetCurrentGLContext();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if( mWindowResized )
    {
        prepare2DViewport(0,0,mWindowWidth, mWindowHeight);
    }

	if (mCurrentWaveView >= 0)
	{
		DrawWaveView(views[mCurrentWaveView]);
	}
    SwapBuffers();
}

void Waveform::DrawWaveView(const WaveView &wv)
{
    int index;
    glLineWidth( 1 );

    xlColor color(212,208,200);
    DrawGLUtils::AddVertex(0, 0, color);
    DrawGLUtils::AddVertex(mWindowWidth, 0, color);
    DrawGLUtils::AddVertex(mWindowWidth, mWindowHeight, color);
    DrawGLUtils::AddVertex(0, mWindowHeight, color);
    DrawGLUtils::End(GL_TRIANGLE_FAN);

    int max_wave_ht = mWindowHeight - VERTICAL_PADDING;

    // Draw Outside rectangle
    color.Set(128, 128, 128);
    DrawGLUtils::AddVertex(0.25, 0, color);
    DrawGLUtils::AddVertex(mWindowWidth, 0, color);
    DrawGLUtils::AddVertex(mWindowWidth, mWindowHeight-0.5, color);
    DrawGLUtils::AddVertex(0.25, mWindowHeight-0.5, color);
    DrawGLUtils::End(GL_LINE_LOOP);

    // Get selection positions from timeline
    int selected_x1 = mTimeline->GetSelectedPositionStart();
    int selected_x2 = mTimeline->GetSelectedPositionEnd();

    // draw shaded region if needed
    if( selected_x1 != -1 && selected_x2 != -1)
    {
        color.Set(0, 0, 200, 45);
        DrawGLUtils::AddVertex(selected_x1, 1, color);
        DrawGLUtils::AddVertex(selected_x2, 1, color);
        DrawGLUtils::AddVertex(selected_x2, mWindowHeight-1, color);
        DrawGLUtils::AddVertex(selected_x1, mWindowHeight-1, color);
        DrawGLUtils::End(GL_TRIANGLE_FAN, GL_BLEND);
    }

    if(_media != NULL)
    {
        xlColor c(130,178,207,255);

        int max = std::min(mWindowWidth, (int)wv.MinMaxs.size());
        if (mStartPixelOffset != wv.lastRenderStart || max != wv.lastRenderSize) {
            wv.background.Reset();
            wv.outline.Reset();
            wv.background.PreAlloc((mWindowWidth + 2) * 2);
            wv.outline.PreAlloc((mWindowWidth + 2) + 4);
            
            std::vector<double> vertexes;
            vertexes.resize((mWindowWidth + 2));
            
            for (int x=0;x<mWindowWidth && (x)<wv.MinMaxs.size();x++)
            {
                index = x+mStartPixelOffset;
                if (index >= 0 && index < wv.MinMaxs.size())
                {
                    double y1 = ((wv.MinMaxs[index].min * (float)(max_wave_ht/2))+ (mWindowHeight/2));
                    double y2 = ((wv.MinMaxs[index].max * (float)(max_wave_ht/2))+ (mWindowHeight/2));
                    
                    
                    wv.background.AddVertex(x, y1);
                    wv.background.AddVertex(x, y2);
                    
                    wv.outline.AddVertex(x, y1);
                    vertexes[x] = y2;
                }
            }
            for(int x=mWindowWidth;x >= 0 ; x--) {
                if (x<wv.MinMaxs.size()) {
                    wv.outline.AddVertex(x, vertexes[x]);
                }
            }
            wv.lastRenderSize = max;
            wv.lastRenderStart = mStartPixelOffset;
        }

        DrawGLUtils::Draw(wv.background, c, GL_TRIANGLE_STRIP);
        DrawGLUtils::Draw(wv.outline, xlWHITE, GL_LINE_STRIP);
    }

    // draw selection line if not a range
    if( selected_x1 != -1 && selected_x2 == -1 )
    {
        color.Set(0, 0, 0, 128);
        DrawGLUtils::AddVertex(selected_x1, 1, color);
        DrawGLUtils::AddVertex(selected_x1, mWindowHeight-1, color);
    }

    // draw mouse position line
    int mouse_marker = mTimeline->GetMousePosition();
    if( mouse_marker != -1 )
    {
        color.Set(0, 0, 255, 255);
        DrawGLUtils::AddVertex(mouse_marker, 1, color);
        DrawGLUtils::AddVertex(mouse_marker, mWindowHeight-1, color);
    }

    // draw play marker line
    int play_marker = mTimeline->GetPlayMarker();
    if( play_marker != -1 )
    {
        color.Set(0, 0, 0, 255);
        DrawGLUtils::AddVertex(play_marker, 1, color);
        DrawGLUtils::AddVertex(play_marker, mWindowHeight-1, color);
    }
    if (DrawGLUtils::VertexCount() > 0) {
        DrawGLUtils::End(GL_LINES);
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
        WaveView wv(mZoomLevel, samplesPerLine, _media);
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
	if (_media != NULL)
	{
		return (float)timePerPixel * (float)_media->GetRate();
	}
	else
	{
		return 0.0f;
	}
}

void Waveform::WaveView::SetMinMaxSampleSet(float SamplesPerPixel, AudioManager* media)
{
	MinMaxs.clear();

	if (media != NULL)
	{
		float minimum=1;
		float maximum=-1;
		int trackSize = media->GetTrackSize();
		int totalMinMaxs = (int)((float)trackSize/SamplesPerPixel)+1;

		for (int i = 0; i < totalMinMaxs; i++) {
			// Use float calculation to minimize compounded rounding of position
			int start = (int)((float)i*SamplesPerPixel);
			if (start >= trackSize) {
				return;
			}
			int end = start + SamplesPerPixel;
			if (end >= trackSize) {
				end = trackSize;
			}
			minimum = 1;
			maximum = -1;
			for (int j = start; j < end; j++) {
				float data = media->GetLeftData(j);
				if (data < minimum) {
					minimum = data;
				}
				if (data > maximum) {
					maximum = data;
				}
			}
			MINMAX mm;
			mm.min = minimum;
			mm.max = maximum;
			MinMaxs.push_back(mm);
		}
    }
}

void Waveform::mouseLeftWindow(wxMouseEvent& event)
{
    UpdateMousePosition(-1);
}

void Waveform::UpdateMousePosition(int time)
{
    // Update time selection
    wxCommandEvent eventMousePos(EVT_MOUSE_POSITION);
    eventMousePos.SetInt(time);
    wxPostEvent(mParent, eventMousePos);
}



