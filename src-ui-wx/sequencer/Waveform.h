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

#include "wx/wx.h"

#include <vector>
#include <string>

#include "graphics/xlGraphicsBase.h"
#include "graphics/xlGraphicsContext.h"

#include "media/AudioManager.h"
#include "media/Spectrogram.h"

#define VERTICAL_PADDING            10
#define NO_WAVE_VIEW_SELECTED       -1
#define WAVEFORM_SIDE_MARGIN        25
wxDECLARE_EVENT(EVT_WAVE_FORM_MOVED, wxCommandEvent);
wxDECLARE_EVENT(EVT_WAVE_FORM_HIGHLIGHT, wxCommandEvent);
wxDECLARE_EVENT(EVT_TIME_SELECTED, wxCommandEvent);
wxDECLARE_EVENT(EVT_SCROLL_RIGHT, wxCommandEvent);
wxDECLARE_EVENT(EVT_ZOOM, wxCommandEvent);
wxDECLARE_EVENT(EVT_SCRUB, wxCommandEvent);
wxDECLARE_EVENT(EVT_GSCROLL, wxCommandEvent);
wxDECLARE_EVENT(EVT_MOUSE_POSITION, wxCommandEvent);

class TimeLine;

enum DRAG_MODE {
    DRAG_NORMAL,
    DRAG_LEFT_EDGE,
    DRAG_RIGHT_EDGE
};

class Waveform : public GRAPHICS_BASE_CLASS
{
    public:
		int OpenfileMedia(AudioManager* media, wxString& error);
		void CloseMedia();

        static int GetLargeSize() { return 75; };
        static int GetSmallSize() { return 37; };
        void SetZoomLevel(int level);
        int GetZoomLevel() const;
        void SetWaveFormSize(int h);

        int SetStartPixelOffset(int startPixel);
        int GetStartPixelOffset() const;
        void SetSelectedInterval(int startMS, int endMS);

        void SetTimeFrequency(int frequency);
        int GetTimeFrequency() const;

        void SetTimeline(TimeLine* timeLine);

        void UpdatePlayMarker();
        void CheckNeedToScroll() const;
        void ForceRedraw();
        // Reset all audio-analysis state (overlays, cached detector
        // results, view-mode toggles) — called on sequence close /
        // media change so the next sequence opens in default mode.
        void ResetAnalysisState();
#if defined(__APPLE__) || defined(HAVE_OPENVINO) || defined(HAVE_ORT)
        // A8: locate the HTDemucs CoreML model (or prompt the user
        // to pick a location and download it), then run the
        // separator and stash the result on `_media` via
        // `SetStemData`. Returns true if stem data is now available,
        // false on user cancel / download / inference failure.
        // ONNX Runtime/OpenVINO: download ONNX model from huggingface and put in ai-models/ folder in the show folder.
        bool PrepareStemData();
#endif

        int GetActiveAudioTrackIndex() const { return _activeAudioTrackIndex; }

        Waveform(wxPanel* parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition,
                const wxSize &size=wxDefaultSize,long style=0, const wxString &name=wxPanelNameStr);
		virtual ~Waveform();

        struct MINMAX
        {
            float min;
            float max;
            float rms;
        };

        virtual xlColor ClearBackgroundColor() const override;

        void render() override;


    protected:
        xlVertexAccumulator *border = nullptr;

    private:
      	DECLARE_EVENT_TABLE()
        //void GetMinMaxSampleSet(int setSize, float*sampleData,int trackSize, MINMAX* minMax);
        float GetSamplesPerLineFromZoomLevel(int ZoomLevel) const;
		TimeLine* mTimeline;
        wxPanel* mParent;
        //wxWindow* mMainWindow;
        int mStartPixelOffset;
        int mCurrentWaveView;
        bool _doubleHeight = false;
        //int mMediaTrackSize;
        int mFrequency;
        int mZoomLevel;
        //bool mPointSize;
        bool m_dragging;
        DRAG_MODE m_drag_mode;
		AudioManager* _media;
        AUDIOSAMPLETYPE _type = AUDIOSAMPLETYPE::RAW;
        int _lowNote = -1;
        int _highNote = -1;
        int _activeAudioTrackIndex = 0; // 0 = main, 1..N = alt tracks
        // A2 onset overlay — spectral-flux peak picks across the whole
        // track, drawn as faint vertical lines. Computed lazily when
        // the user first toggles "Show Onsets".
        bool _showOnsets = false;
        bool _onsetsComputed = false;
        std::vector<long> _onsetMS;
        // A5 pitch-contour overlay. Per-frame (timeMS, freq, conf)
        // triples. Unvoiced frames keep freq=0 so the draw code can
        // break the polyline. Computed lazily on first toggle.
        bool _showPitchContour = false;
        bool _pitchComputed = false;
        std::vector<float> _pitchContour; // flat triples [t,f,c, t,f,c, ...]
        // A6 spectrogram view. STFT magnitude buffer is computed
        // lazily on first toggle; each draw resamples the cached
        // buffer into a viewport-sized BGRA image and uploads it to
        // a texture for rendering.
        bool _showSpectrogram = false;
        bool _spectrogramComputed = false;
        Spectrogram _spectrogram;
        xlTexture* _spectrogramTexture = nullptr;
        int _spectrogramTexW = 0;
        int _spectrogramTexH = 0;
        long _spectrogramRangeStartMS = -1;
        long _spectrogramRangeEndMS = -1;
#ifdef __APPLE__
        // A7 sound classification. Populated lazily when the user
        // picks "Classify Audio…" from the waveform menu. Each entry
        // is a per-window confidence array; `_soundClassTimeStep` is
        // the window size in seconds. `_selectedSoundClass` gates the
        // waveform — bucket peaks get multiplied by the confidence at
        // the bucket midpoint. Empty class ⇒ no gating.
        std::vector<std::pair<std::string, std::vector<float>>> _soundClasses;
        std::string _selectedSoundClass;
        float _soundClassTimeStep = 1.0f;
#endif
        static const long ID_WAVE_MNU_RENDER;
        static const long ID_WAVE_MNU_RAW;
        static const long ID_WAVE_MNU_BASS;
        static const long ID_WAVE_MNU_TREBLE;
        static const long ID_WAVE_MNU_ALTO;
        static const long ID_WAVE_MNU_CUSTOM;
        static const long ID_WAVE_MNU_NONVOCALS;
        static const long ID_WAVE_MNU_LUFS;
        static const long ID_WAVE_MNU_VOCALS;
        static const long ID_WAVE_MNU_DOUBLEHEIGHT;
        static const long ID_WAVE_MNU_SHOW_ONSETS;
        static const long ID_WAVE_MNU_SHOW_PITCH;
        static const long ID_WAVE_MNU_SHOW_SPECTROGRAM;
#if defined(__APPLE__) || defined(HAVE_OPENVINO) || defined(HAVE_ORT)
        // A8 — HTDemucs-based 4-stem separation. One radio item per
        // stem; selecting any triggers a first-run download flow if
        // the model isn't present.
        // Non-Apple: ONNX/OpenVINO ONNX model.
        static const long ID_WAVE_MNU_STEM_DRUMS;
        static const long ID_WAVE_MNU_STEM_BASS;
        static const long ID_WAVE_MNU_STEM_OTHER;
        static const long ID_WAVE_MNU_STEM_VOCALS;
#endif
#ifdef __APPLE__
        static const long ID_WAVE_MNU_CLASSIFY;
        static const long ID_WAVE_MNU_CLASSIFY_CLEAR;
#endif
        // Audio track submenu: IDs are ID_WAVE_MNU_AUDIO_TRACK_BASE + index
        static const long ID_WAVE_MNU_AUDIO_TRACK_BASE;

        class WaveView
        {
        private:
            float mSamplesPerPixel = 1;
            int mZoomLevel = 0;
            int _lowNote = -1;
            int _highNote = -1;
            AUDIOSAMPLETYPE _type = AUDIOSAMPLETYPE::RAW;

        public:

            mutable std::unique_ptr<xlVertexAccumulator> background = nullptr;
            mutable std::unique_ptr<xlVertexAccumulator> outline = nullptr;
            // A10 peak-vs-RMS overlay. Drawn after `background` so the
            // RMS band sits on top of the peak fill and gives a visual
            // cue to "how loud on average", independent of transient
            // peaks.
            mutable std::unique_ptr<xlVertexAccumulator> rmsFill = nullptr;
            mutable int lastRenderStart = -1;
            mutable int lastRenderSize = 0;
            std::vector<MINMAX> MinMaxs;
            mutable bool _doubleHeight = false;

            WaveView(int ZoomLevel, float SamplesPerPixel, AudioManager* media, AUDIOSAMPLETYPE type, int lowNote, int highNote)
            {
                mZoomLevel = ZoomLevel;
                mSamplesPerPixel = SamplesPerPixel;
                SetMinMaxSampleSet(SamplesPerPixel, media, type, lowNote, highNote);
                lastRenderStart = -1;
                lastRenderSize = 0;
                _type = type;
                _lowNote = lowNote;
                _highNote = highNote;
            }
            WaveView(int ZoomLevel)
            {
                mZoomLevel = ZoomLevel;
                lastRenderStart = -1;
                lastRenderSize = 0;
            }

            WaveView(WaveView && v) = default;

            virtual ~WaveView();

            void ForceRedraw()
            {
                lastRenderStart = -1;
            }
            int GetZoomLevel() const { return  mZoomLevel; }
            AUDIOSAMPLETYPE GetType() const { return _type; }
            int GetLowNote() const { return _lowNote; }
            int GetHighNote() const { return _highNote; }
            void SetMinMaxSampleSet(float SamplesPerPixel, AudioManager* media, AUDIOSAMPLETYPE type, int lowNote, int highNote);
        };


        void DrawWaveView(xlGraphicsContext *ctx, const WaveView &wv);
        void Paint( wxPaintEvent& event );
        void UpdateMousePosition(int time);
      	void mouseMoved(wxMouseEvent& event);
        void mouseWheelMoved(wxMouseEvent& event);
      	void mouseLeftDown(wxMouseEvent& event);
      	void mouseLeftUp( wxMouseEvent& event);
      	void OnLeftDClick(wxMouseEvent& event);
	    void rightClick(wxMouseEvent& event);
        void OnGridPopup(wxCommandEvent& event);
        void OnLostMouseCapture(wxMouseCaptureLostEvent& event);
        void mouseLeftWindow(wxMouseEvent& event);

        float translateOffset(float f);

        std::vector<WaveView> views;
};
