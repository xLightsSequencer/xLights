/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/wx.h>
#include <wx/event.h>

#include "StemWaveform.h"
#include "TimeLine.h"
#include "Waveform.h"
#include "xLightsApp.h"
#include "xLightsMain.h"
#include "color/ColorManager.h"


BEGIN_EVENT_TABLE(StemWaveform, GRAPHICS_BASE_CLASS)
EVT_LEFT_DOWN(StemWaveform::mouseLeftDown)
EVT_LEFT_UP(StemWaveform::mouseLeftUp)
EVT_MOTION(StemWaveform::mouseMoved)
EVT_MOUSE_CAPTURE_LOST(StemWaveform::OnLostMouseCapture)
EVT_LEAVE_WINDOW(StemWaveform::mouseLeftWindow)
EVT_MOUSEWHEEL(StemWaveform::mouseWheelMoved)
EVT_SIZE(StemWaveform::Resized)
EVT_PAINT(StemWaveform::Paint)
END_EVENT_TABLE()

StemWaveform::StemWaveform(wxPanel* parent, wxWindowID id,
                           const wxPoint& pos, const wxSize& size)
    : GRAPHICS_BASE_CLASS(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, "StemWaveform"),
      _parent(parent),
      _stemColor(130, 178, 207)
{
    _dragging = false;
    _rowHeight = 32;
}

StemWaveform::~StemWaveform()
{
    CloseMedia();
}

bool StemWaveform::LoadMedia(const std::string& filepath, wxString& error)
{
    spdlog::debug("[DEBUG: StemWaveform::LoadMedia starting '{}']", filepath);
    CloseMedia();

    spdlog::debug("[DEBUG: StemWaveform::LoadMedia creating AudioManager]");
    _media = new AudioManager(filepath);
    if (!_media->IsOk()) {
        error = wxString::Format("Failed to load stem audio: %s", filepath);
        delete _media;
        _media = nullptr;
        return false;
    }

    _stemFilePath = filepath;
    _pendingMediaInit = true;
    _shimmerPhase = 0.0f;
    spdlog::debug("[DEBUG: StemWaveform::LoadMedia done, pending async init]");
    return true;
}

void StemWaveform::CloseMedia()
{
    _overviewBuckets.clear();
    _overviewReady = false;
    _pendingMediaInit = false;
    _waveBackground.reset();
    _waveOutline.reset();
    _cacheRenderStart = -1;
    _cacheRenderSize = 0;
    if (_media != nullptr) {
        delete _media;
        _media = nullptr;
    }
}

void StemWaveform::ComputeOverviewBuckets()
{
    if (!_media || !_media->IsOk()) return;

    long trackSize = _media->GetTrackSize();
    if (trackSize <= 0) return;

    // Get raw data pointer directly — avoids 32768 GetLeftDataMinMax calls
    // each of which acquires a mutex and does a linear search
    FilteredAudioData* fad = _media->GetFilteredAudioData(AUDIOSAMPLETYPE::RAW, -1, -1);
    if (!fad || !fad->data0) return;

    float* data = fad->data0;
    _samplesPerBucket = (float)trackSize / (float)OVERVIEW_BUCKET_COUNT;
    _overviewBuckets.resize(OVERVIEW_BUCKET_COUNT);

    for (int b = 0; b < OVERVIEW_BUCKET_COUNT; b++) {
        long start = (long)((float)b * _samplesPerBucket);
        long end = (long)((float)(b + 1) * _samplesPerBucket);
        if (end > trackSize) end = trackSize;
        if (start >= trackSize) {
            _overviewBuckets[b].min = 0;
            _overviewBuckets[b].max = 0;
            continue;
        }

        float minVal = 0.0f;
        float maxVal = 0.0f;
        for (long j = start; j < end; j++) {
            float v = data[j];
            if (v < minVal) minVal = v;
            if (v > maxVal) maxVal = v;
        }
        _overviewBuckets[b].min = minVal;
        _overviewBuckets[b].max = maxVal;
    }

    _overviewReady = true;
}

void StemWaveform::GetMinMaxForPixel(int pixel, float& minVal, float& maxVal)
{
    float samplesPerPixel = GetSamplesPerLineFromZoomLevel(_zoomLevel);
    float pixelOffset = translateOffset(_startPixelOffset);
    float startSample = (pixel + pixelOffset) * samplesPerPixel;
    float endSample = startSample + samplesPerPixel;

    long trackSize = _media->GetTrackSize();
    if (startSample >= trackSize || endSample <= 0) {
        minVal = 0;
        maxVal = 0;
        return;
    }
    if (startSample < 0) startSample = 0;
    if (endSample > trackSize) endSample = (float)trackSize;

    if (_overviewReady && samplesPerPixel > _samplesPerBucket) {
        int startBucket = (int)(startSample / _samplesPerBucket);
        int endBucket = (int)(endSample / _samplesPerBucket);
        if (startBucket < 0) startBucket = 0;
        if (endBucket >= OVERVIEW_BUCKET_COUNT) endBucket = OVERVIEW_BUCKET_COUNT - 1;

        minVal = 0.0f;
        maxVal = 0.0f;
        for (int b = startBucket; b <= endBucket; b++) {
            if (_overviewBuckets[b].min < minVal) minVal = _overviewBuckets[b].min;
            if (_overviewBuckets[b].max > maxVal) maxVal = _overviewBuckets[b].max;
        }
    } else {
        // Direct data access for zoomed-in view (few samples per pixel)
        FilteredAudioData* fad = _media->GetFilteredAudioData(AUDIOSAMPLETYPE::RAW, -1, -1);
        if (!fad || !fad->data0) {
            minVal = 0;
            maxVal = 0;
            return;
        }
        minVal = 0.0f;
        maxVal = 0.0f;
        long s = (long)startSample;
        long e = std::min((long)endSample, trackSize);
        for (long j = s; j < e; j++) {
            float v = fad->data0[j];
            if (v < minVal) minVal = v;
            if (v > maxVal) maxVal = v;
        }
    }
}

void StemWaveform::SetZoomLevel(int level)
{
    _zoomLevel = level;
    if (_pendingMediaInit) return;
    InvalidateCache();
}

int StemWaveform::SetStartPixelOffset(int offset)
{
    if (_startPixelOffset != offset) {
        _startPixelOffset = offset;
        InvalidateCache();
    }
    return _startPixelOffset;
}

void StemWaveform::SetTimeFrequency(int frequency)
{
    _frequency = frequency;
}

void StemWaveform::SetRowHeight(int height)
{
    _rowHeight = height;
    SetMinSize(wxSize(-1, _rowHeight));
    SetMaxSize(wxSize(-1, _rowHeight));

    // Don't set mWindowHeight directly — base class Resized handler
    // applies GetContentScaleFactor() for Retina displays
    mWindowResized = true;

    InvalidateCache();
    Refresh(false);
}

void StemWaveform::ForceRedraw()
{
    InvalidateCache();
}

void StemWaveform::UpdatePlayMarker()
{
    render();
}

xlColor StemWaveform::ClearBackgroundColor() const
{
    return ColorManager::instance()->GetColor(ColorManager::COLOR_WAVEFORM_BACKGROUND);
}

void StemWaveform::Paint(wxPaintEvent& event)
{
    wxPaintDC(this);
    render();
}

float StemWaveform::translateOffset(float f)
{
    if (drawingUsingLogicalSize()) {
        return f;
    }
    return translateToBacking(f);
}

float StemWaveform::GetSamplesPerLineFromZoomLevel(int zoomLevel) const
{
    int periodsPerMajorHash = TimeLine::ZoomLevelValues[zoomLevel];
    float timePerPixel = ((float)periodsPerMajorHash / (float)_frequency) / (float)PIXELS_PER_MAJOR_HASH;
    if (!drawingUsingLogicalSize()) {
        timePerPixel /= GetContentScaleFactor();
    }
    if (_media != nullptr) {
        return timePerPixel * (float)_media->GetRate();
    }
    return 0.0f;
}

void StemWaveform::InvalidateCache()
{
    _cacheRenderStart = -1;
    _cacheRenderSize = 0;
    Refresh(false);
}

void StemWaveform::render()
{
    if (!IsShown()) return;

    // Check if we're within visible bounds of the outer container
    wxWindow* outerPanel = GetParent() ? GetParent()->GetParent() : nullptr;
    if (outerPanel) {
        wxPoint pos = GetParent()->GetPosition();
        wxPoint myPos = GetPosition();
        int absY = pos.y + myPos.y;
        int outerH = outerPanel->GetSize().GetHeight();
        if (absY + _rowHeight < 0 || absY > outerH) return;
    }

    if (!mIsInitialized) {
        PrepareCanvas();
    }

    // Deferred media init — complete once audio data has finished loading
    if (_pendingMediaInit && _media != nullptr) {
        if (_media->IsDataLoaded()) {
            if (_media->IsOk()) {
                _media->SwitchTo(AUDIOSAMPLETYPE::RAW);
                ComputeOverviewBuckets();
            }
            _pendingMediaInit = false;
            _shimmerPhase = 0.0f;
        }
    }

    xlGraphicsContext* ctx = PrepareContextForDrawing();
    if (ctx == nullptr) return;
    ctx->SetViewport(0, 0, mWindowWidth, mWindowHeight);

    if (_pendingMediaInit) {
        // Draw loading shimmer while audio is being decoded
        float w = (float)mWindowWidth;
        float h = (float)mWindowHeight;

        float bandWidth = w * 0.25f;
        float bandCenter = _shimmerPhase * (w + bandWidth) - bandWidth * 0.5f;

        xlColor bgColor(30, 30, 35);
        xlColor shimmerColor(55, 55, 65);

        auto* vca = ctx->createVertexColorAccumulator();
        vca->PreAlloc(18);

        float leftEdge = std::max(0.0f, bandCenter - bandWidth * 0.5f);
        if (leftEdge > 0) {
            vca->AddVertex(0, 0, bgColor); vca->AddVertex(leftEdge, 0, bgColor); vca->AddVertex(0, h, bgColor);
            vca->AddVertex(leftEdge, 0, bgColor); vca->AddVertex(leftEdge, h, bgColor); vca->AddVertex(0, h, bgColor);
        }

        float sl = std::max(0.0f, bandCenter - bandWidth * 0.5f);
        float sm = std::min(w, std::max(0.0f, bandCenter));
        float sr = std::min(w, bandCenter + bandWidth * 0.5f);

        vca->AddVertex(sl, 0, bgColor); vca->AddVertex(sm, 0, shimmerColor); vca->AddVertex(sl, h, bgColor);
        vca->AddVertex(sm, 0, shimmerColor); vca->AddVertex(sm, h, shimmerColor); vca->AddVertex(sl, h, bgColor);
        vca->AddVertex(sm, 0, shimmerColor); vca->AddVertex(sr, 0, bgColor); vca->AddVertex(sm, h, shimmerColor);
        vca->AddVertex(sr, 0, bgColor); vca->AddVertex(sr, h, bgColor); vca->AddVertex(sm, h, shimmerColor);

        float rightEdge = std::min(w, bandCenter + bandWidth * 0.5f);
        if (rightEdge < w) {
            vca->AddVertex(rightEdge, 0, bgColor); vca->AddVertex(w, 0, bgColor); vca->AddVertex(rightEdge, h, bgColor);
            vca->AddVertex(w, 0, bgColor); vca->AddVertex(w, h, bgColor); vca->AddVertex(rightEdge, h, bgColor);
        }

        vca->Finalize(false, false);
        ctx->drawTriangles(vca);
        delete vca;

        _shimmerPhase += 0.02f;
        if (_shimmerPhase > 1.0f) _shimmerPhase = 0.0f;
        CallAfter([this]() { Refresh(); });
    } else if (_overviewReady) {
        DrawWaveform(ctx);
    }

    FinishDrawing(ctx);
}

void StemWaveform::DrawWaveform(xlGraphicsContext* ctx)
{
    // Draw border
    if (!_border) {
        _border = ctx->createVertexAccumulator();
        _border->PreAlloc(5);
        _border->AddVertex(0.25, 0.25, 0);
        _border->AddVertex(mWindowWidth - 0.5, 0.25, 0);
        _border->AddVertex(mWindowWidth - 0.5, mWindowHeight - 0.5, 0);
        _border->AddVertex(0.25, mWindowHeight - 0.5, 0);
        _border->AddVertex(0.25, 0.25, 0);
        _border->Finalize(true);
    } else if (mWindowResized) {
        _border->SetVertex(1, mWindowWidth - 0.5, 0.25, 0);
        _border->SetVertex(2, mWindowWidth - 0.5, mWindowHeight - 0.5, 0);
        _border->SetVertex(3, 0.25, mWindowHeight - 0.5, 0);
        _border->FlushRange(1, 3);
    }

    xlColor borderColor(80, 80, 80);
    ctx->drawLineStrip(_border, borderColor);

    int max_wave_ht = mWindowHeight - 4;

    if (_media != nullptr && _overviewReady) {
        if (_startPixelOffset != _cacheRenderStart || (int)mWindowWidth != _cacheRenderSize) {
            if (!_waveBackground) {
                _waveBackground.reset(ctx->createVertexAccumulator());
                _waveBackground->SetName("StemFill");
                _waveOutline.reset(ctx->createVertexAccumulator());
                _waveOutline->SetName("StemLines");
            }
            _waveBackground->Reset();
            _waveOutline->Reset();
            _waveBackground->PreAlloc((mWindowWidth + 2) * 2);
            _waveOutline->PreAlloc((mWindowWidth + 2) + 4);

            std::vector<double> vertexes(mWindowWidth + 2);

            for (int x = 0; x < (int)mWindowWidth; x++) {
                float minVal, maxVal;
                GetMinMaxForPixel(x, minVal, maxVal);

                double y1 = minVal * ((double)max_wave_ht / 2.0) + (mWindowHeight / 2.0);
                double y2 = maxVal * ((double)max_wave_ht / 2.0) + (mWindowHeight / 2.0);

                _waveBackground->AddVertex(x, y1);
                _waveBackground->AddVertex(x, y2);
                _waveOutline->AddVertex(x, y1);
                vertexes[x] = y2;
            }
            for (int x = (int)mWindowWidth - 1; x >= 0; x--) {
                _waveOutline->AddVertex(x, vertexes[x]);
            }

            _cacheRenderSize = (int)mWindowWidth;
            _cacheRenderStart = _startPixelOffset;
            _waveBackground->FlushRange(0, _waveBackground->getCount());
            _waveOutline->FlushRange(0, _waveOutline->getCount());
        }

        xlColor fillColor = _stemColor;
        fillColor.SetAlpha(180);
        if (_waveBackground && _waveBackground->getCount()) {
            ctx->enableBlending();
            ctx->drawTriangleStrip(_waveBackground.get(), fillColor);
            ctx->disableBlending();
        }

        xlColor outlineColor = _stemColor;
        if (_waveOutline && _waveOutline->getCount()) {
            ctx->drawLineStrip(_waveOutline.get(), outlineColor);
        }
    }

    // Draw onset preview markers
    if (!_onsetMarkersMS.empty() && _timeline != nullptr) {
        xlVertexColorAccumulator* onsetVac = ctx->createVertexColorAccumulator();
        xlColor onsetColor(0, 220, 220, 200);  // cyan
        for (int ms : _onsetMarkersMS) {
            int pos = _timeline->GetPositionFromTimeMS(ms);
            if (pos >= 0 && pos < (int)mWindowWidth) {
                float f = translateOffset(pos);
                onsetVac->AddVertex(f, 1, 0, onsetColor);
                onsetVac->AddVertex(f, mWindowHeight - 1, 0, onsetColor);
            }
        }
        if (onsetVac->getCount() > 0) {
            ctx->enableBlending();
            ctx->drawLines(onsetVac);
            ctx->disableBlending();
        }
        delete onsetVac;
    }

    // Draw play marker
    if (_timeline != nullptr) {
        xlVertexColorAccumulator* vac = ctx->createVertexColorAccumulator();
        int play_marker = _timeline->GetPlayMarker();
        if (play_marker != -1) {
            xlColor c(0, 0, 0, 255);
            float f = translateOffset(play_marker);
            vac->AddVertex(f, 1, 0, c);
            vac->AddVertex(f, mWindowHeight - 1, 0, c);
        }

        int mouse_marker = _timeline->GetMousePosition();
        if (mouse_marker != -1) {
            xlColor mc(0, 0, 255, 128);
            if (xLightsApp::GetFrame() != nullptr) {
                mc = xLightsApp::GetFrame()->color_mgr.GetColor(ColorManager::COLOR_WAVEFORM_MOUSE_MARKER);
                mc.SetAlpha(128);
            }
            float f = translateOffset(mouse_marker);
            vac->AddVertex(f, 1, 0, mc);
            vac->AddVertex(f, mWindowHeight - 1, 0, mc);
        }

        if (vac->getCount() > 0) {
            ctx->drawLines(vac);
        }
        delete vac;
    }

    mWindowResized = false;
}

void StemWaveform::mouseLeftDown(wxMouseEvent& event)
{
    if (!mIsInitialized || _timeline == nullptr) return;

    if (!_dragging) {
        _dragging = true;
        CaptureMouse();
    }
    _timeline->SetSelectedPositionStart(event.GetX());
    SetFocus();
    Refresh(false);

    wxCommandEvent eventSelected(EVT_WAVE_FORM_HIGHLIGHT);
    eventSelected.SetInt(0);
    wxPostEvent(_eventTarget ? _eventTarget : _parent, eventSelected);
}

void StemWaveform::mouseLeftUp(wxMouseEvent& event)
{
    if (_dragging) {
        ReleaseMouse();
        _dragging = false;
    }
    if (_timeline != nullptr) {
        _timeline->LatchSelectedPositions();
    }
    Refresh(false);
}

void StemWaveform::mouseMoved(wxMouseEvent& event)
{
    if (!mIsInitialized || _timeline == nullptr) return;

    if (_dragging) {
        _timeline->SetSelectedPositionEnd(event.GetX());
        Refresh(false);

        wxCommandEvent eventSelected(EVT_WAVE_FORM_HIGHLIGHT);
        eventSelected.SetInt(abs(_timeline->GetNewStartTimeMS() - _timeline->GetNewEndTimeMS()));
        wxPostEvent(_eventTarget ? _eventTarget : _parent, eventSelected);
    }

    int mouseTimeMS = _timeline->GetAbsoluteTimeMSfromPosition(event.GetX());
    wxCommandEvent eventMousePos(EVT_MOUSE_POSITION);
    eventMousePos.SetInt(mouseTimeMS);
    wxPostEvent(_eventTarget ? _eventTarget : _parent, eventMousePos);
}

void StemWaveform::mouseWheelMoved(wxMouseEvent& event)
{
    if (event.CmdDown()) {
        int i = event.GetWheelRotation();
        wxCommandEvent eventZoom(EVT_ZOOM);
        eventZoom.SetInt(i < 0 ? ZOOM_OUT : ZOOM_IN);
        wxPostEvent(_eventTarget ? _eventTarget : _parent, eventZoom);
#ifdef __WXOSX__
    } else if (event.GetWheelAxis() == wxMOUSE_WHEEL_HORIZONTAL) {
        int i = event.GetWheelRotation();
        wxCommandEvent eventScroll(EVT_GSCROLL);
        eventScroll.SetInt(i > 0 ? SCROLL_RIGHT : SCROLL_LEFT);
        wxPostEvent(_eventTarget ? _eventTarget : _parent, eventScroll);
#endif
    } else if (event.ShiftDown()) {
        int i = event.GetWheelRotation();
        wxCommandEvent eventScroll(EVT_GSCROLL);
        eventScroll.SetInt(i < 0 ? SCROLL_RIGHT : SCROLL_LEFT);
        wxPostEvent(_eventTarget ? _eventTarget : _parent, eventScroll);
    } else {
        // Forward vertical scroll to StemsPanel for stem area scrolling
        // Go up from waveform → _waveformsInner → _stemWaveformsOuter → find handler
        wxWindow* target = _eventTarget ? _eventTarget : _parent;
        if (target) {
            target->GetEventHandler()->SafelyProcessEvent(event);
        }
    }
}

void StemWaveform::OnLostMouseCapture(wxMouseCaptureLostEvent& event)
{
    _dragging = false;
}

void StemWaveform::mouseLeftWindow(wxMouseEvent& event)
{
    wxCommandEvent eventMousePos(EVT_MOUSE_POSITION);
    eventMousePos.SetInt(-1);
    wxPostEvent(_eventTarget ? _eventTarget : _parent, eventMousePos);
}
