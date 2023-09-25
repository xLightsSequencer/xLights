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
#include "ColorManager.h"
#include "../xLightsApp.h"
#include "../xLightsMain.h"
#include "MainSequencer.h"
#include "../NoteRangeDialog.h"

#include <log4cpp/Category.hh>

wxDEFINE_EVENT(EVT_WAVE_FORM_MOVED, wxCommandEvent);
wxDEFINE_EVENT(EVT_WAVE_FORM_HIGHLIGHT, wxCommandEvent);

BEGIN_EVENT_TABLE(Waveform, GRAPHICS_BASE_CLASS)
EVT_MOTION(Waveform::mouseMoved)
EVT_LEFT_DOWN(Waveform::mouseLeftDown)
EVT_LEFT_UP(Waveform::mouseLeftUp)
EVT_LEFT_DCLICK(Waveform::OnLeftDClick)
EVT_RIGHT_DOWN(Waveform::rightClick)
EVT_MOUSE_CAPTURE_LOST(Waveform::OnLostMouseCapture)
EVT_LEAVE_WINDOW(Waveform::mouseLeftWindow)
EVT_SIZE(Waveform::Resized)
EVT_MOUSEWHEEL(Waveform::mouseWheelMoved)
EVT_PAINT(Waveform::Paint)
END_EVENT_TABLE()

const long Waveform::ID_WAVE_MNU_RENDER = wxNewId();
const long Waveform::ID_WAVE_MNU_RAW = wxNewId();
const long Waveform::ID_WAVE_MNU_BASS = wxNewId();
const long Waveform::ID_WAVE_MNU_ALTO = wxNewId();
const long Waveform::ID_WAVE_MNU_TREBLE = wxNewId();
const long Waveform::ID_WAVE_MNU_CUSTOM = wxNewId();
const long Waveform::ID_WAVE_MNU_NONVOCALS = wxNewId();
const long Waveform::ID_WAVE_MNU_DOUBLEHEIGHT = wxNewId();

Waveform::Waveform(wxPanel* parent, wxWindowID id, const wxPoint &pos, const wxSize &size,
                   long style, const wxString &name):
    GRAPHICS_BASE_CLASS(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, "WaveForm")
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
    _type = AUDIOSAMPLETYPE::RAW;
    _lowNote = -1;
    _highNote = -1;
	_media = nullptr;
    mParent->Refresh();
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

    if (event.ShiftDown() || event.ControlDown()) {
        eventZoom.SetInt(ZOOM_OUT);
    } else {
        eventZoom.SetInt(ZOOM_IN);
    }

    wxPostEvent(GetParent(), eventZoom);
}

void Waveform::UpdatePlayMarker()
{
    render();
}

void Waveform::CheckNeedToScroll() const
{
    int StartTime;
    int EndTime;
    mTimeline->GetViewableTimeRange(StartTime, EndTime);
    int scroll_point = mTimeline->GetPositionFromTimeMS(EndTime) * 0.99;
    if (mTimeline->GetPlayMarker() > scroll_point) {
        wxCommandEvent eventScroll(EVT_SCROLL_RIGHT);
        wxPostEvent(mParent, eventScroll);
    }
}

void Waveform::mouseLeftDown( wxMouseEvent& event)
{
    if(!mIsInitialized) return;

    if (!m_dragging) {
        m_dragging = true;
        CaptureMouse();
    }
    if (m_drag_mode == DRAG_NORMAL) {
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
    if (m_dragging) {
        ReleaseMouse();
        m_dragging = false;
    }

    mTimeline->LatchSelectedPositions();
    Refresh(false);

    wxCommandEvent eventSelected(EVT_WAVE_FORM_HIGHLIGHT);
    if (mTimeline->GetNewEndTimeMS() == -1) {
        eventSelected.SetInt(0);
    } else {
        eventSelected.SetInt(abs(mTimeline->GetNewStartTimeMS() - mTimeline->GetNewEndTimeMS()));
    }
    wxPostEvent(mParent, eventSelected);
}

void Waveform::rightClick(wxMouseEvent& event)
{
    wxMenu mnuWave;
    if( (mTimeline->GetSelectedPositionStartMS() != -1 ) &&
        (mTimeline->GetSelectedPositionEndMS() != -1 ) ) {
        mnuWave.Append(ID_WAVE_MNU_RENDER,"Render Selected Region");
    }
    if (_media != nullptr) {
        if (mnuWave.GetMenuItemCount() > 0) {
            mnuWave.AppendSeparator();
        }

        mnuWave.AppendRadioItem(ID_WAVE_MNU_RAW, "Raw waveform")->Check(_type == AUDIOSAMPLETYPE::RAW);
        mnuWave.AppendRadioItem(ID_WAVE_MNU_BASS, "Bass waveform")->Check(_type == AUDIOSAMPLETYPE::BASS);
        mnuWave.AppendRadioItem(ID_WAVE_MNU_TREBLE, "Treble waveform")->Check(_type == AUDIOSAMPLETYPE::TREBLE);
        mnuWave.AppendRadioItem(ID_WAVE_MNU_ALTO, "Alto waveform")->Check(_type == AUDIOSAMPLETYPE::ALTO);
        mnuWave.AppendRadioItem(ID_WAVE_MNU_CUSTOM, "Custom filtered waveform")->Check(_type == AUDIOSAMPLETYPE::CUSTOM);
        mnuWave.AppendRadioItem(ID_WAVE_MNU_NONVOCALS, "Non Vocals waveform")->Check(_type == AUDIOSAMPLETYPE::NONVOCALS);
        mnuWave.AppendSeparator();
        mnuWave.AppendCheckItem(ID_WAVE_MNU_DOUBLEHEIGHT, "Double height waveform")->Check(_doubleHeight);
    }
    if (mnuWave.GetMenuItemCount() > 0) {
        mnuWave.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)& Waveform::OnGridPopup, nullptr, this);
        render();
        PopupMenu(&mnuWave);
    }
}

void Waveform::OnGridPopup(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    int id = event.GetId();
    if(id == ID_WAVE_MNU_RENDER) {
        logger_base.debug("OnGridPopup - ID_WAVE_MNU_RENDER");
        RenderCommandEvent rcEvent("", mTimeline->GetSelectedPositionStartMS(), mTimeline->GetSelectedPositionEndMS(), true, false);
        wxPostEvent(mParent, rcEvent);
    } else if (id == ID_WAVE_MNU_RAW) {
        _type = AUDIOSAMPLETYPE::RAW;
    } else if (id == ID_WAVE_MNU_BASS) {
        _type = AUDIOSAMPLETYPE::BASS;
    } else if (id == ID_WAVE_MNU_TREBLE) {
        _type = AUDIOSAMPLETYPE::TREBLE;
    } else if (id == ID_WAVE_MNU_ALTO) {
        _type = AUDIOSAMPLETYPE::ALTO;
    } else if (id == ID_WAVE_MNU_NONVOCALS) {
        _type = AUDIOSAMPLETYPE::NONVOCALS;
    } else if (id == ID_WAVE_MNU_CUSTOM) {
        int origLow = _lowNote;
        int origHigh = _highNote;
        if (_lowNote == -1) _lowNote = 0;
        if (_highNote == -1) _highNote = 127;
        NoteRangeDialog dlg(GetParent(), _lowNote, _highNote);
        if (dlg.ShowModal() == wxID_CANCEL) {
            _lowNote = origLow;
            _highNote = origHigh;
            return;
        }
        _type = AUDIOSAMPLETYPE::CUSTOM;
    } else if (id == ID_WAVE_MNU_DOUBLEHEIGHT) {
        _doubleHeight = !_doubleHeight;
    }

    wxSetCursor(wxCURSOR_WAIT);

    mCurrentWaveView = NO_WAVE_VIEW_SELECTED;
    for (size_t i = 0; i < views.size(); i++) {
        if (views[i].GetZoomLevel() == mZoomLevel && views[i].GetType() == _type) {
            if (_type == AUDIOSAMPLETYPE::CUSTOM) {
                if (views[i].GetLowNote() == _lowNote && views[i].GetHighNote() == _highNote) {
                    mCurrentWaveView = i;
                    break;
                }
            } else {
                mCurrentWaveView = i;
                break;
            }
        }
    }
    if (_media) {
        _media->SwitchTo(_type, _lowNote, _highNote);
    }
    if (mCurrentWaveView == NO_WAVE_VIEW_SELECTED) {
        float samplesPerLine = GetSamplesPerLineFromZoomLevel(mZoomLevel);
        views.emplace_back(mZoomLevel, samplesPerLine, _media, _type, _lowNote, _highNote);
        mCurrentWaveView = views.size() - 1;
    }

    wxSetCursor(wxCURSOR_ARROW);

    ForceRedraw();
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
        } else {
            mTimeline->SetSelectedPositionEnd(event.GetX());
        }
        Refresh(false);
        wxCommandEvent eventSelected(EVT_WAVE_FORM_HIGHLIGHT);
        eventSelected.SetInt(abs(mTimeline->GetNewStartTimeMS() - mTimeline->GetNewEndTimeMS()));
        wxPostEvent(mParent, eventSelected);
    } else {
        int selected_x1 = mTimeline->GetSelectedPositionStart();
        int selected_x2 = mTimeline->GetSelectedPositionEnd();
        if (event.GetX() >= selected_x1 && event.GetX() < selected_x1 + 6) {
            SetCursor(wxCURSOR_POINT_LEFT);
            m_drag_mode = DRAG_LEFT_EDGE;
        } else if (event.GetX() > selected_x2 - 6 && event.GetX() <= selected_x2) {
            SetCursor(wxCURSOR_POINT_RIGHT);
            m_drag_mode = DRAG_RIGHT_EDGE;
        } else {
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
    if(event.CmdDown()) {
        int i = event.GetWheelRotation();
        if (i < 0) {
            wxCommandEvent eventZoom(EVT_ZOOM);
            eventZoom.SetInt(ZOOM_OUT);
            wxPostEvent(mParent, eventZoom);
        } else {
            wxCommandEvent eventZoom(EVT_ZOOM);
            eventZoom.SetInt(ZOOM_IN);
            wxPostEvent(mParent, eventZoom);
        }
    } else if (event.ShiftDown()) {
        int i = event.GetWheelRotation();
        if (i < 0) {
            wxCommandEvent eventScroll(EVT_GSCROLL);
            eventScroll.SetInt(SCROLL_RIGHT);
            wxPostEvent(mParent, eventScroll);
        } else {
            wxCommandEvent eventScroll(EVT_GSCROLL);
            eventScroll.SetInt(SCROLL_LEFT);
            wxPostEvent(mParent, eventScroll);
        }
    } else {
        wxPostEvent(GetParent()->GetEventHandler(), event);
        event.Skip();
    }
}

// Open Media file and return elapsed time in milliseconds
int Waveform::OpenfileMedia(AudioManager* media, wxString& error)
{
    _type = AUDIOSAMPLETYPE::RAW;
    _lowNote = -1;
    _highNote = -1;
    _media = media;
    views.clear();
	if (_media != nullptr) {
        _media->SwitchTo(_type);
		float samplesPerLine = GetSamplesPerLineFromZoomLevel(mZoomLevel);
		views.emplace_back(mZoomLevel, samplesPerLine, media, _type, _lowNote, _highNote);
		mCurrentWaveView = 0;
		return media->LengthMS();
    } else {
        mCurrentWaveView = NO_WAVE_VIEW_SELECTED;
        SetZoomLevel(GetZoomLevel());
	    return 0;
	}
}


xlColor Waveform::ClearBackgroundColor() const {
    if (AudioManager::GetSDLManager()->IsNoAudio()) {
        return xlRED;
    }
    return ColorManager::instance()->GetColor(ColorManager::COLOR_WAVEFORM_BACKGROUND);
}

void Waveform::Paint(wxPaintEvent& event)
{
    wxPaintDC(this);
    render();
}

void Waveform::render()
{
    if(!IsShownOnScreen()) return;
    if(!mIsInitialized) {
        PrepareCanvas();
        SetZoomLevel(mZoomLevel);
    }

    xlGraphicsContext *ctx = PrepareContextForDrawing();
    if (ctx == nullptr) {
        return;
    }
    ctx->SetViewport(0, 0, mWindowWidth, mWindowHeight);

    if (mCurrentWaveView >= 0) {
		DrawWaveView(ctx, views[mCurrentWaveView]);
	}

    FinishDrawing(ctx);
}

float Waveform::translateOffset(float f) {
    if (drawingUsingLogicalSize()) {
        return f;
    }
    return translateToBacking(f);
}


void Waveform::ForceRedraw()
{
    if (mCurrentWaveView >= 0) {
        views[mCurrentWaveView].ForceRedraw();
    }
}

double DoubleHeight(double v, bool dh, int ht)
{
    if (dh) {
        return (v) * (double)ht;
    }
    return v * ((double)ht / 2.0);
}

void Waveform::DrawWaveView(xlGraphicsContext* ctx, const WaveView& wv)
{
    if (!border) {
        border = ctx->createVertexAccumulator();
        border->PreAlloc(5);
        border->AddVertex(0.25, 0.25, 0);
        border->AddVertex(mWindowWidth - 0.5, 0.25, 0);
        border->AddVertex(mWindowWidth - 0.5, mWindowHeight - 0.5, 0);
        border->AddVertex(0.25, mWindowHeight - 0.5, 0);
        border->AddVertex(0.25, 0.25, 0);
        border->Finalize(true);
    } else if (mWindowResized) {
        border->SetVertex(1, mWindowWidth - 0.5, 0.25, 0);
        border->SetVertex(2, mWindowWidth - 0.5, mWindowHeight - 0.5, 0);
        border->SetVertex(3, 0.25, mWindowHeight - 0.5, 0);
        border->FlushRange(1, 3);
    }
    xlColor color;
    color.Set(128, 128, 128);
    // Draw Outside rectangle
    ctx->drawLineStrip(border, color);

    // Get selection positions from timeline
    int selected_x1 = mTimeline->GetSelectedPositionStart();
    int selected_x2 = mTimeline->GetSelectedPositionEnd();

    // draw shaded region if needed
    if (selected_x1 != -1 && selected_x2 != -1) {
        float x1 = translateOffset(selected_x1);
        float x2 = translateOffset(selected_x2);
        color = xLightsApp::GetFrame()->color_mgr.GetColor(ColorManager::COLOR_WAVEFORM_SELECTED);
        color.SetAlpha(45);
        xlVertexAccumulator* selection = ctx->createVertexAccumulator();
        selection->PreAlloc(4);
        selection->AddVertex(x1, 1, 0);
        selection->AddVertex(x2, 1, 0);
        selection->AddVertex(x1, mWindowHeight - 1, 0);
        selection->AddVertex(x2, mWindowHeight - 1, 0);
        ctx->enableBlending();
        ctx->drawTriangleStrip(selection, color);
        ctx->disableBlending();
        delete selection;
    }

    int max_wave_ht = mWindowHeight - VERTICAL_PADDING;

    if (_media != nullptr) {
        xlColor c(130, 178, 207, 255);
        if (xLightsApp::GetFrame() != nullptr) {
            c = xLightsApp::GetFrame()->color_mgr.GetColor(ColorManager::COLOR_WAVEFORM);
        }

        int max = std::min(mWindowWidth, wv.MinMaxs.size());
        if (mStartPixelOffset != wv.lastRenderStart || max != wv.lastRenderSize || _doubleHeight != wv._doubleHeight) {
            float pixelOffset = translateOffset(mStartPixelOffset);

            if (wv.background.get() == nullptr) {
                wv.background = std::unique_ptr<xlVertexAccumulator>(ctx->createVertexAccumulator()->SetName("WaveFill"));
                wv.outline = std::unique_ptr<xlVertexAccumulator>(ctx->createVertexAccumulator()->SetName("WaveLines"));
            }
            wv.background->Reset();
            wv.outline->Reset();
            wv.background->PreAlloc((mWindowWidth + 2) * 2);
            wv.outline->PreAlloc((mWindowWidth + 2) + 4);

            std::vector<double> vertexes;
            vertexes.resize((mWindowWidth + 2));

            for (size_t x = 0; x < mWindowWidth && x < wv.MinMaxs.size(); x++) {
                int index = x;
                index += pixelOffset;
                if (index >= 0 && index < wv.MinMaxs.size()) {

                    double y1 = DoubleHeight(wv.MinMaxs[index].min, _doubleHeight, max_wave_ht) + (mWindowHeight / 2);
                    double y2 = DoubleHeight(wv.MinMaxs[index].max, _doubleHeight, max_wave_ht) + (mWindowHeight / 2);

                    wv.background->AddVertex(x, y1);
                    wv.background->AddVertex(x, y2);

                    wv.outline->AddVertex(x, y1);
                    vertexes[x] = y2;
                }
            }
            for (int x = std::min(mWindowWidth, wv.MinMaxs.size()) - 1; x >= 0; x--) {
                int index = x;
                index += pixelOffset;
                if (index >= 0 && index < wv.MinMaxs.size()) {
                    wv.outline->AddVertex(x, vertexes[x]);
                }
            }
            wv._doubleHeight = _doubleHeight;
            wv.lastRenderSize = max;
            wv.lastRenderStart = mStartPixelOffset;
            wv.background->FlushRange(0, wv.background->getCount());
            wv.outline->FlushRange(0, wv.outline->getCount());
        }
        if (wv.background.get() && wv.background->getCount()) {
            ctx->drawTriangleStrip(wv.background.get(), c);
        }
        if (wv.outline.get() && wv.outline->getCount()) {
            ctx->drawLineStrip(wv.outline.get(), xlWHITE);
        }
    }

    xlVertexColorAccumulator* vac = ctx->createVertexColorAccumulator();
    // draw selection line if not a range
    if (selected_x1 != -1 && selected_x2 == -1) {
        color.Set(0, 0, 0, 128);
        float f = translateOffset(selected_x1);
        vac->AddVertex(f, 1, 0, color);
        vac->AddVertex(f, mWindowHeight - 1, 0, color);
    }

    // draw mouse position line
    int mouse_marker = mTimeline->GetMousePosition();

    if (xLightsApp::GetFrame() != nullptr && xLightsApp::GetFrame()->_timelineZooming == 1) { // 1 = Mouse Position, 0 = Play Marker
        
        //
        // preference : use waveform mouse marker for centered zoom 
        //
        int aSelectedPlayMarkerStartMS = mTimeline->GetAbsoluteTimeMSfromPosition(mouse_marker);

        mTimeline->SetZoomMarkerMS(aSelectedPlayMarkerStartMS);

    }

    if (mouse_marker != -1) {
        color.Set(0, 0, 255, 255);
        
        if (xLightsApp::GetFrame() != nullptr) {
            color = xLightsApp::GetFrame()->color_mgr.GetColor(ColorManager::COLOR_WAVEFORM_MOUSE_MARKER);
        }        

        float f = translateOffset(mouse_marker);
        vac->AddVertex(f, 1, 0, color);
        vac->AddVertex(f, mWindowHeight - 1, 0, color);
    }

    // draw play marker line
    int play_marker = mTimeline->GetPlayMarker();
    if (play_marker != -1) {
        color.Set(0, 0, 0, 255);
        float f = translateOffset(play_marker);
        vac->AddVertex(f, 1, 0, color);
        vac->AddVertex(f, mWindowHeight - 1, 0, color);
    }

    if (xLightsApp::GetFrame() != nullptr) {
        Effect* selectedEffect = xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffect();
        if (selectedEffect != nullptr) {
            color = ColorManager::instance()->GetColor(ColorManager::COLOR_WAVEFORM_SELECTEDEFFECT);
            int start = translateOffset(mTimeline->GetPositionFromTimeMS(selectedEffect->GetStartTimeMS()));
            int end = translateOffset(mTimeline->GetPositionFromTimeMS(selectedEffect->GetEndTimeMS()));
            vac->AddVertex(start, 1, 0, color);
            vac->AddVertex(start, (mWindowHeight - 1) / 4, 0, color);
            vac->AddVertex(end, 1, 0, color);
            vac->AddVertex(end, (mWindowHeight - 1) / 4, 0, color);
            vac->AddVertex(start, (mWindowHeight - 1) / 8, 0, color);
            vac->AddVertex(end, (mWindowHeight - 1) / 8, 0, color);
        }
    }
    if (vac->getCount() > 0) {
        ctx->drawLines(vac);
    }
    delete vac;
    mWindowResized = false;
}

void Waveform::SetZoomLevel(int level)
{
    mZoomLevel = level;

    if (!mIsInitialized) return;

    mCurrentWaveView = NO_WAVE_VIEW_SELECTED;
    for (size_t i = 0; i < views.size(); i++) {
        if (views[i].GetZoomLevel() == mZoomLevel && views[i].GetType() == _type) {
            mCurrentWaveView = i;
        }
    }
    if (mCurrentWaveView == NO_WAVE_VIEW_SELECTED) {
        float samplesPerLine = GetSamplesPerLineFromZoomLevel(mZoomLevel);
        views.emplace_back(mZoomLevel, samplesPerLine, _media, _type, _lowNote, _highNote);
        mCurrentWaveView = views.size() - 1;
    }
}

int Waveform::GetZoomLevel() const
{
    return  mZoomLevel;
}

int Waveform::SetStartPixelOffset(int offset)
{
    if (mCurrentWaveView != NO_WAVE_VIEW_SELECTED) {
       mStartPixelOffset = offset;
    } else {
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
    if (!drawingUsingLogicalSize()) {
        timePerPixel /= GetContentScaleFactor();
    }
	if (_media != nullptr) {
		return (float)timePerPixel * (float)_media->GetRate();
    } else {
		return 0.0f;
	}
}

void Waveform::SetWaveFormSize(int h)
{
    int w = -1;
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

    if (_media != nullptr) {
        float samplesPerLine = GetSamplesPerLineFromZoomLevel(mZoomLevel);
        views.emplace_back(0, samplesPerLine, _media, _type, _lowNote, _highNote);
    }

    mCurrentWaveView = NO_WAVE_VIEW_SELECTED;
    SetZoomLevel(mZoomLevel);

    Refresh(false);
}

Waveform::WaveView::~WaveView() {
}

void Waveform::WaveView::SetMinMaxSampleSet(float SamplesPerPixel, AudioManager* media, AUDIOSAMPLETYPE type, int lowNote, int highNote)
{
	MinMaxs.clear();

	if (media != nullptr) {
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
