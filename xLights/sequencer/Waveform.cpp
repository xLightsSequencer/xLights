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
#ifdef __WXMAC__
    #include "OpenGL/gl.h"
#else
    #include <GL/gl.h>
#endif
#include <wx/glcanvas.h>
#include <wx/event.h>
#include <wx/bitmap.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>

#include "../xLightsTimer.h"
#include "Waveform.h"
#include "TimeLine.h"
#include "../RenderCommandEvent.h"
#include <wx/file.h>
#include "../DrawGLUtils.h"
#include "ColorManager.h"
#include "../xLightsApp.h"
#include "../xLightsMain.h"
#include "MainSequencer.h"
#include "../NoteRangeDialog.h"

#include <log4cpp/Category.hh>

wxDEFINE_EVENT(EVT_WAVE_FORM_MOVED, wxCommandEvent);
wxDEFINE_EVENT(EVT_WAVE_FORM_HIGHLIGHT, wxCommandEvent);

BEGIN_EVENT_TABLE(Waveform, xlGLCanvas)
EVT_MOTION(Waveform::mouseMoved)
EVT_LEFT_DOWN(Waveform::mouseLeftDown)
EVT_LEFT_UP(Waveform::mouseLeftUp)
EVT_LEFT_DCLICK(Waveform::OnLeftDClick)
EVT_RIGHT_DOWN(Waveform::rightClick)
EVT_MOUSE_CAPTURE_LOST(Waveform::OnLostMouseCapture)
EVT_LEAVE_WINDOW(Waveform::mouseLeftWindow)
EVT_SIZE(Waveform::Resized)
EVT_MOUSEWHEEL(Waveform::mouseWheelMoved)
EVT_PAINT(Waveform::renderGL)
END_EVENT_TABLE()

const long Waveform::ID_WAVE_MNU_RENDER = wxNewId();
const long Waveform::ID_WAVE_MNU_RAW = wxNewId();
const long Waveform::ID_WAVE_MNU_BASS = wxNewId();
const long Waveform::ID_WAVE_MNU_ALTO = wxNewId();
const long Waveform::ID_WAVE_MNU_TREBLE = wxNewId();
const long Waveform::ID_WAVE_MNU_CUSTOM = wxNewId();
const long Waveform::ID_WAVE_MNU_NONVOCALS = wxNewId();

Waveform::Waveform(wxPanel* parent, wxWindowID id, const wxPoint &pos, const wxSize &size,
                   long style, const wxString &name):
                   xlGLCanvas(parent,wxID_ANY,wxDefaultPosition, wxDefaultSize, 0, "WaveForm")
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("                Creating Waveform");
    m_dragging = false;
    m_drag_mode = DRAG_NORMAL;
    mParent = parent;
    mCurrentWaveView = NO_WAVE_VIEW_SELECTED;
    mZoomLevel=0;
    mStartPixelOffset = 0;
    mFrequency = 40;
    _media = nullptr;
    mTimeline = nullptr;
}

Waveform::~Waveform()
{
    CloseMedia();
}

void Waveform::CloseMedia()
{
    views.clear();
    mCurrentWaveView = NO_WAVE_VIEW_SELECTED;
	_media = nullptr;
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

void Waveform::CheckNeedToScroll() const
{
    int StartTime;
    int EndTime;
    mTimeline->GetViewableTimeRange(StartTime, EndTime);
    int scroll_point = mTimeline->GetPositionFromTimeMS(EndTime) * 0.99;
    if (mTimeline->GetPlayMarker() > scroll_point)
    {
        wxCommandEvent eventScroll(EVT_SCROLL_RIGHT);
        wxPostEvent(mParent, eventScroll);
    }
}

void Waveform::mouseLeftDown( wxMouseEvent& event)
{
    if(!mIsInitialized) return;

    if (!m_dragging)
    {
        m_dragging = true;
        CaptureMouse();
    }
    if (m_drag_mode == DRAG_NORMAL)
    {
        mTimeline->SetSelectedPositionStart(event.GetX());
    }
    SetFocus();
    Refresh(false);

    wxCommandEvent eventSelected(EVT_WAVE_FORM_HIGHLIGHT);
    eventSelected.SetInt(0);
    wxPostEvent(mParent, eventSelected);
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

    wxCommandEvent eventSelected(EVT_WAVE_FORM_HIGHLIGHT);
    if (mTimeline->GetNewEndTimeMS() == -1)
    {
        eventSelected.SetInt(0);
    }
    else
    {
        eventSelected.SetInt(abs(mTimeline->GetNewStartTimeMS() - mTimeline->GetNewEndTimeMS()));
    }
    wxPostEvent(mParent, eventSelected);
}

void Waveform::rightClick(wxMouseEvent& event)
{
    wxMenu mnuWave;
    if( (mTimeline->GetSelectedPositionStartMS() != -1 ) &&
        (mTimeline->GetSelectedPositionEndMS() != -1 ) )
    {
        mnuWave.Append(ID_WAVE_MNU_RENDER,"Render Selected Region");
    }
    if (_media != nullptr)
    {
        if (mnuWave.GetMenuItemCount() > 0)
        {
            mnuWave.AppendSeparator();
        }

        mnuWave.AppendRadioItem(ID_WAVE_MNU_RAW, "Raw waveform")->Check(_type == AUDIOSAMPLETYPE::RAW);
        mnuWave.AppendRadioItem(ID_WAVE_MNU_BASS, "Bass waveform")->Check(_type == AUDIOSAMPLETYPE::BASS);
        mnuWave.AppendRadioItem(ID_WAVE_MNU_TREBLE, "Treble waveform")->Check(_type == AUDIOSAMPLETYPE::TREBLE);
        mnuWave.AppendRadioItem(ID_WAVE_MNU_ALTO, "Alto waveform")->Check(_type == AUDIOSAMPLETYPE::ALTO);
        mnuWave.AppendRadioItem(ID_WAVE_MNU_CUSTOM, "Custom filtered waveform")->Check(_type == AUDIOSAMPLETYPE::CUSTOM);
        mnuWave.AppendRadioItem(ID_WAVE_MNU_NONVOCALS, "Non Vocals waveform")->Check(_type == AUDIOSAMPLETYPE::NONVOCALS);
    }
    if (mnuWave.GetMenuItemCount() > 0)
    {
        mnuWave.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)& Waveform::OnGridPopup, nullptr, this);
        renderGL();
        PopupMenu(&mnuWave);
    }
}

void Waveform::OnGridPopup(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    int id = event.GetId();
    if(id == ID_WAVE_MNU_RENDER)
    {
        logger_base.debug("OnGridPopup - ID_WAVE_MNU_RENDER");
        RenderCommandEvent rcEvent("", mTimeline->GetSelectedPositionStartMS(), mTimeline->GetSelectedPositionEndMS(), true, false);
        wxPostEvent(mParent, rcEvent);
    }
    else if (id == ID_WAVE_MNU_RAW)
    {
        _type = AUDIOSAMPLETYPE::RAW;
    }
    else if (id == ID_WAVE_MNU_BASS)
    {
        _type = AUDIOSAMPLETYPE::BASS;
    }
    else if (id == ID_WAVE_MNU_TREBLE)
    {
        _type = AUDIOSAMPLETYPE::TREBLE;
    }
    else if (id == ID_WAVE_MNU_ALTO)
    {
        _type = AUDIOSAMPLETYPE::ALTO;
    }
    else if (id == ID_WAVE_MNU_NONVOCALS)
    {
        _type = AUDIOSAMPLETYPE::NONVOCALS;
    }
    else if (id == ID_WAVE_MNU_CUSTOM)
    {
        NoteRangeDialog dlg(GetParent(), _lowNote, _highNote);
        if (dlg.ShowModal() == wxID_CANCEL) return;
        _type = AUDIOSAMPLETYPE::CUSTOM;
    }

    wxSetCursor(wxCURSOR_WAIT);

    mCurrentWaveView = NO_WAVE_VIEW_SELECTED;
    for (size_t i = 0; i < views.size(); i++)
    {
        if (views[i].GetZoomLevel() == mZoomLevel && views[i].GetType() == _type)
        {
            if (_type == AUDIOSAMPLETYPE::CUSTOM)
            {
                if (views[i].GetLowNote() == _lowNote && views[i].GetHighNote() == _highNote)
                {
                    mCurrentWaveView = i;
                    break;
                }
            }
            else
            {
                mCurrentWaveView = i;
                break;
            }
        }
    }
    if (_media) {
        _media->SwitchTo(_type, _lowNote, _highNote);
    }
    if (mCurrentWaveView == NO_WAVE_VIEW_SELECTED)
    {
        float samplesPerLine = GetSamplesPerLineFromZoomLevel(mZoomLevel);
        WaveView wv(mZoomLevel, samplesPerLine, _media, _type, _lowNote, _highNote);
        views.push_back(wv);
        mCurrentWaveView = views.size() - 1;
    }

    wxSetCursor(wxCURSOR_ARROW);

    Refresh();
}

void Waveform::SetSelectedInterval(int startMS, int endMS)
{
    mTimeline->SetSelectedPositionStart(mTimeline->GetPositionFromTimeMS(startMS), false);
    mTimeline->SetSelectedPositionEnd(mTimeline->GetPositionFromTimeMS(endMS));
    mTimeline->LatchSelectedPositions();

    wxCommandEvent eventSelected(EVT_WAVE_FORM_HIGHLIGHT);
    eventSelected.SetInt(abs(mTimeline->GetNewStartTimeMS() - mTimeline->GetNewEndTimeMS()));
    wxPostEvent(mParent, eventSelected);

    Refresh(false);
}

void Waveform::mouseMoved(wxMouseEvent& event)
{
    if (!mIsInitialized) { return; }
    if (m_dragging) {
        if (m_drag_mode == DRAG_LEFT_EDGE) {
            mTimeline->SetSelectedPositionStart(event.GetX(), false);
        }
        else {
            mTimeline->SetSelectedPositionEnd(event.GetX());
        }
        Refresh(false);
        wxCommandEvent eventSelected(EVT_WAVE_FORM_HIGHLIGHT);
        eventSelected.SetInt(abs(mTimeline->GetNewStartTimeMS() - mTimeline->GetNewEndTimeMS()));
        wxPostEvent(mParent, eventSelected);
    }
    else {
        int selected_x1 = mTimeline->GetSelectedPositionStart();
        int selected_x2 = mTimeline->GetSelectedPositionEnd();
        if (event.GetX() >= selected_x1 && event.GetX() < selected_x1 + 6) {
            SetCursor(wxCURSOR_POINT_LEFT);
            m_drag_mode = DRAG_LEFT_EDGE;
        }
        else if (event.GetX() > selected_x2 - 6 && event.GetX() <= selected_x2) {
            SetCursor(wxCURSOR_POINT_RIGHT);
            m_drag_mode = DRAG_RIGHT_EDGE;
        }
        else {
            SetCursor(wxCURSOR_ARROW);
            m_drag_mode = DRAG_NORMAL;
        }
    }
    int mouseTimeMS = mTimeline->GetAbsoluteTimeMSfromPosition(event.GetX());
    UpdateMousePosition(mouseTimeMS);

    // Scrubbing
    if (event.LeftIsDown() && event.ControlDown()) {
        if (_media != nullptr) {
            int msperpixel = std::max(1000 / GetTimeFrequency(), mTimeline->TimePerMajorTickInMS() / mTimeline->PixelsPerMajorTick());
            _media->Play(mouseTimeMS, msperpixel);
        }

        wxCommandEvent eventScrub(EVT_SCRUB);
        eventScrub.SetInt(mouseTimeMS);
        wxPostEvent(mParent, eventScrub);
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
    else if(event.ShiftDown())
    {
        int i = event.GetWheelRotation();
        if(i<0)
        {
            wxCommandEvent eventScroll(EVT_GSCROLL);
            eventScroll.SetInt(SCROLL_RIGHT);
            wxPostEvent(mParent, eventScroll);
        }
        else
        {
            wxCommandEvent eventScroll(EVT_GSCROLL);
            eventScroll.SetInt(SCROLL_LEFT);
            wxPostEvent(mParent, eventScroll);
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
    _type = AUDIOSAMPLETYPE::RAW;
    _media = media;
    views.clear();
	if (_media != nullptr)
	{
        _media->SwitchTo(AUDIOSAMPLETYPE::RAW);
		float samplesPerLine = GetSamplesPerLineFromZoomLevel(mZoomLevel);
		WaveView wv(mZoomLevel, samplesPerLine, media, _type, _lowNote, _highNote);
		views.push_back(wv);
		mCurrentWaveView = 0;
		return media->LengthMS();
	}
	else
	{
        mCurrentWaveView = NO_WAVE_VIEW_SELECTED;
        SetZoomLevel(GetZoomLevel());
	    return 0;
	}
}

void Waveform::InitializeGLCanvas()
{
#ifdef __LINUX__
    if(!IsShownOnScreen()) return;
#endif
    mIsInitialized = true;
    SetZoomLevel(mZoomLevel);
}
void Waveform::InitializeGLContext()
{
    SetCurrentGLContext();
    LOG_GL_ERRORV(glClearColor(0.0f, 0.0f, 0.0f, 0.0f)); // Black Background
    LOG_GL_ERRORV(glDisable(GL_BLEND));
    LOG_GL_ERRORV(glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA));
    LOG_GL_ERRORV(glClear(GL_COLOR_BUFFER_BIT));
    prepare2DViewport(0,0,mWindowWidth, mWindowHeight);
}
void Waveform::renderGL( wxPaintEvent& event )
{
    renderGL();
}

void Waveform::renderGL()
{
    if(!IsShownOnScreen()) return;
    if(!mIsInitialized) { InitializeGLCanvas(); }
    InitializeGLContext();
	if (mCurrentWaveView >= 0) {
		DrawWaveView(views[mCurrentWaveView]);
	}
    LOG_GL_ERRORV(SwapBuffers());
}

void Waveform::DrawWaveView(const WaveView &wv)
{
    DrawGLUtils::SetLineWidth(1.0);

    DrawGLUtils::xlAccumulator vac;
    vac.PreAlloc(18);
    xlColor color = ColorManager::instance()->GetColor(ColorManager::COLOR_WAVEFORM_BACKGROUND);

    vac.AddVertex(0, 0, color);
    vac.AddVertex(mWindowWidth, 0, color);
    vac.AddVertex(mWindowWidth, mWindowHeight, color);
    vac.AddVertex(0, mWindowHeight, color);
    vac.Finish(GL_TRIANGLE_FAN);

    int max_wave_ht = mWindowHeight - VERTICAL_PADDING;

    // Draw Outside rectangle
    color.Set(128, 128, 128);
    vac.AddVertex(0.25, 0, color);
    vac.AddVertex(mWindowWidth, 0, color);
    vac.AddVertex(mWindowWidth, mWindowHeight-0.5, color);
    vac.AddVertex(0.25, mWindowHeight-0.5, color);
    vac.Finish(GL_LINE_LOOP, 0, 1);

    // Get selection positions from timeline
    int selected_x1 = mTimeline->GetSelectedPositionStart();
    int selected_x2 = mTimeline->GetSelectedPositionEnd();

    // draw shaded region if needed
    if (selected_x1 != -1 && selected_x2 != -1) {
        //color.Set(0, 0, 200, 45);
        color = xLightsApp::GetFrame()->color_mgr.GetColor(ColorManager::COLOR_WAVEFORM_SELECTED);
        color.SetAlpha(45);
        vac.AddVertex(selected_x1, 1, color);
        vac.AddVertex(selected_x2, 1, color);
        vac.AddVertex(selected_x2, mWindowHeight-1, color);
        vac.AddVertex(selected_x1, mWindowHeight-1, color);
        vac.Finish(GL_TRIANGLE_FAN, GL_BLEND);
    }

    if(_media != nullptr) {
        xlColor c(130,178,207,255);
        if (xLightsApp::GetFrame() != nullptr) {
            c = xLightsApp::GetFrame()->color_mgr.GetColor(ColorManager::COLOR_WAVEFORM);
        }

        int max = std::min(mWindowWidth, wv.MinMaxs.size());
        if (mStartPixelOffset != wv.lastRenderStart || max != wv.lastRenderSize) {
            wv.background.Reset();
            wv.outline.Reset();
            wv.background.PreAlloc((mWindowWidth + 2) * 2);
            wv.outline.PreAlloc((mWindowWidth + 2) + 4);

            std::vector<double> vertexes;
            vertexes.resize((mWindowWidth + 2));

            for (size_t x = 0; x < mWindowWidth && x < wv.MinMaxs.size(); x++)
            {
                int index = x;
                index += mStartPixelOffset;
                if (index >= 0 && index < wv.MinMaxs.size())
                {
                    double y1 = ((wv.MinMaxs[index].min * (float)(max_wave_ht / 2))+ (mWindowHeight / 2));
                    double y2 = ((wv.MinMaxs[index].max * (float)(max_wave_ht / 2))+ (mWindowHeight / 2));

                    wv.background.AddVertex(x, y1);
                    wv.background.AddVertex(x, y2);

                    wv.outline.AddVertex(x, y1);
                    vertexes[x] = y2;
                }
            }
            for (int x = mWindowWidth; x >= 0; x--) {
                int index = x;
                index += mStartPixelOffset;
                if (index >= 0 && index < wv.MinMaxs.size()) {
                    wv.outline.AddVertex(x, vertexes[x]);
                }
            }
            wv.lastRenderSize = max;
            wv.lastRenderStart = mStartPixelOffset;
        }
        vac.PreAlloc(wv.background.count + wv.outline.count + 6);
        vac.Load(wv.background, c);
        vac.Finish(GL_TRIANGLE_STRIP);
        vac.Load(wv.outline, xlWHITE);
        vac.Finish(GL_LINE_STRIP, 0, 1);
    }

    // draw selection line if not a range
    if (selected_x1 != -1 && selected_x2 == -1)
    {
        color.Set(0, 0, 0, 128);
        vac.AddVertex(selected_x1, 1, color);
        vac.AddVertex(selected_x1, mWindowHeight - 1, color);
    }

    // draw mouse position line
    int mouse_marker = mTimeline->GetMousePosition();
    if (mouse_marker != -1)
    {
        color.Set(0, 0, 255, 255);
        vac.AddVertex(mouse_marker, 1, color);
        vac.AddVertex(mouse_marker, mWindowHeight-1, color);
    }

    // draw play marker line
    int play_marker = mTimeline->GetPlayMarker();
    if (play_marker != -1)
    {
        color.Set(0, 0, 0, 255);
        vac.AddVertex(play_marker, 1, color);
        vac.AddVertex(play_marker, mWindowHeight-1, color);
    }

    if (xLightsApp::GetFrame() != nullptr)
    {
        Effect* selectedEffect = xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffect();
        if (selectedEffect != nullptr)
        {
            color = ColorManager::instance()->GetColor(ColorManager::COLOR_WAVEFORM_SELECTEDEFFECT);
            int start = mTimeline->GetPositionFromTimeMS(selectedEffect->GetStartTimeMS());
            int end = mTimeline->GetPositionFromTimeMS(selectedEffect->GetEndTimeMS());
            vac.AddVertex(start, 1, color);
            vac.AddVertex(start, (mWindowHeight - 1) / 4, color);
            vac.AddVertex(end, 1, color);
            vac.AddVertex(end, (mWindowHeight - 1) / 4, color);
            vac.AddVertex(start, (mWindowHeight - 1) / 8, color);
            vac.AddVertex(end, (mWindowHeight - 1) / 8, color);
        }
    }

    if (vac.HasMoreVertices()) {
        vac.Finish(GL_LINES, 0, 1);
    }
    if (vac.count > 0) {
        DrawGLUtils::Draw(vac);
    }
}

void Waveform::SetZoomLevel(int level)
{
    mZoomLevel = level;

    if (!mIsInitialized) return;

    mCurrentWaveView = NO_WAVE_VIEW_SELECTED;
    for (size_t i = 0; i < views.size(); i++)
    {
        if (views[i].GetZoomLevel() == mZoomLevel && views[i].GetType() == _type)
        {
            mCurrentWaveView = i;
        }
    }
    if (mCurrentWaveView == NO_WAVE_VIEW_SELECTED)
    {
        float samplesPerLine = GetSamplesPerLineFromZoomLevel(mZoomLevel);
        WaveView wv(mZoomLevel, samplesPerLine, _media, _type, _lowNote, _highNote);
        views.push_back(wv);
        mCurrentWaveView = views.size() - 1;
    }
}

int Waveform::GetZoomLevel() const
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

int Waveform::GetStartPixelOffset() const
{
    return  mStartPixelOffset;
}

void Waveform::SetTimeFrequency(int frequency)
{
    mFrequency = frequency;
}

int Waveform::GetTimeFrequency() const
{
    return  mFrequency;
}

float Waveform::GetSamplesPerLineFromZoomLevel(int ZoomLevel) const
{
    // The number of periods for each Zoomlevel is held in ZoomLevelValues array
    int periodsPerMajorHash = TimeLine::ZoomLevelValues[mZoomLevel];
    float timePerPixel = ((float)periodsPerMajorHash/(float)mFrequency)/(float)PIXELS_PER_MAJOR_HASH;
	if (_media != nullptr)
	{
		return (float)timePerPixel * (float)_media->GetRate();
	}
	else
	{
		return 0.0f;
	}
}

void Waveform::SetGLSize(int w, int h)
{

    SetMinSize(wxSize(w, h));

    wxSize size = GetSize();
    if (w == -1) w = size.GetWidth();
    if (h == -1) h = size.GetHeight();

    SetSize(w, h);
    mWindowHeight = h;
    mWindowWidth = w;
    mWindowResized = true;

    // All of our existing views are invalid ... so erase them
    views.clear();

    if (_media != nullptr)
    {
        float samplesPerLine = GetSamplesPerLineFromZoomLevel(mZoomLevel);
        WaveView wv(0, samplesPerLine, _media, _type, _lowNote, _highNote);
        views.push_back(wv);
    }

    mCurrentWaveView = NO_WAVE_VIEW_SELECTED;
    SetZoomLevel(mZoomLevel);

    Refresh(false);
}

void Waveform::WaveView::SetMinMaxSampleSet(float SamplesPerPixel, AudioManager* media, AUDIOSAMPLETYPE type, int lowNote, int highNote)
{
	MinMaxs.clear();

	if (media != nullptr)
	{
		float minimum=1;
		float maximum=-1;
		long trackSize = media->GetTrackSize();
		int totalMinMaxs = (int)((float)trackSize/SamplesPerPixel)+1;

		for (int i = 0; i < totalMinMaxs; i++) {
			// Use float calculation to minimize compounded rounding of position
			long start = (long)((float)i*SamplesPerPixel);
			if (start >= trackSize) {
				return;
			}
			long end = start + SamplesPerPixel;
			if (end >= trackSize) {
				end = trackSize;
			}
			minimum = 1;
			maximum = -1;
            media->GetLeftDataMinMax(start, end, minimum, maximum, type, lowNote, highNote);
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
