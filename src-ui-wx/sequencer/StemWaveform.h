#pragma once

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
#include <vector>
#include <string>
#include <memory>

#include "graphics/xlGraphicsBase.h"
#include "graphics/xlGraphicsContext.h"
#include "media/AudioManager.h"

class TimeLine;

class StemWaveform : public GRAPHICS_BASE_CLASS
{
public:
    StemWaveform(wxPanel* parent, wxWindowID id,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize);
    virtual ~StemWaveform();

    bool LoadMedia(const std::string& filepath, wxString& error);
    void CloseMedia();

    void SetStemName(const std::string& name) { _stemName = name; }
    std::string GetStemName() const { return _stemName; }

    void SetStemColor(const xlColor& color) { _stemColor = color; InvalidateCache(); }
    xlColor GetStemColor() const { return _stemColor; }

    void SetStemFilePath(const std::string& path) { _stemFilePath = path; }
    std::string GetStemFilePath() const { return _stemFilePath; }

    void SetTimeline(TimeLine* timeline) { _timeline = timeline; }
    void SetEventTarget(wxWindow* target) { _eventTarget = target; }
    void SetZoomLevel(int level);
    int SetStartPixelOffset(int offset);
    void SetTimeFrequency(int frequency);

    void SetRowHeight(int height);
    int GetRowHeight() const { return _rowHeight; }

    void ForceRedraw();
    void UpdatePlayMarker();

    AudioManager* GetMedia() const { return _media; }

    // Onset detection preview markers
    void SetOnsetMarkers(const std::vector<int>& timesMS) { _onsetMarkersMS = timesMS; }
    void ClearOnsetMarkers() { _onsetMarkersMS.clear(); }
    bool HasOnsetMarkers() const { return !_onsetMarkersMS.empty(); }

    virtual xlColor ClearBackgroundColor() const override;
    void render() override;

protected:
    DECLARE_EVENT_TABLE()

private:
    struct OverviewBucket {
        float min;
        float max;
    };

    void ComputeOverviewBuckets();
    void GetMinMaxForPixel(int pixel, float& minVal, float& maxVal);
    void DrawWaveform(xlGraphicsContext* ctx);
    void Paint(wxPaintEvent& event);
    void mouseLeftDown(wxMouseEvent& event);
    void mouseLeftUp(wxMouseEvent& event);
    void mouseMoved(wxMouseEvent& event);
    void mouseWheelMoved(wxMouseEvent& event);
    void OnLostMouseCapture(wxMouseCaptureLostEvent& event);
    void mouseLeftWindow(wxMouseEvent& event);
    void InvalidateCache();
    float GetSamplesPerLineFromZoomLevel(int zoomLevel) const;
    float translateOffset(float f);

    xlVertexAccumulator* _border = nullptr;
    std::unique_ptr<xlVertexAccumulator> _waveBackground;
    std::unique_ptr<xlVertexAccumulator> _waveOutline;
    int _cacheRenderStart = -1;
    int _cacheRenderSize = 0;

    TimeLine* _timeline = nullptr;
    wxPanel* _parent = nullptr;
    wxWindow* _eventTarget = nullptr;
    AudioManager* _media = nullptr;

    std::string _stemName;
    std::string _stemFilePath;
    xlColor _stemColor;

    int _zoomLevel = 0;
    int _startPixelOffset = 0;
    int _frequency = 40;
    int _rowHeight = 32;
    bool _dragging = false;

    // Async loading
    bool _pendingMediaInit = false;
    float _shimmerPhase = 0.0f;

    // Overview buckets for smooth zooming
    static const int OVERVIEW_BUCKET_COUNT = 32768;
    std::vector<OverviewBucket> _overviewBuckets;
    bool _overviewReady = false;
    float _samplesPerBucket = 0.0f;

    // Onset detection preview markers
    std::vector<int> _onsetMarkersMS;
};
