#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "wx/wx.h"

#include <vector>
#include <string>

#include "graphics/xlGraphicsBase.h"
#include "graphics/xlGraphicsContext.h"

#include "../AudioManager.h"

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

        Waveform(wxPanel* parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition,
                const wxSize &size=wxDefaultSize,long style=0, const wxString &name=wxPanelNameStr);
		virtual ~Waveform();

        struct MINMAX
        {
            float min;
            float max;
        };

        virtual xlColor ClearBackgroundColor() const override;

        void render();


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
        static const long ID_WAVE_MNU_RENDER;
        static const long ID_WAVE_MNU_RAW;
        static const long ID_WAVE_MNU_BASS;
        static const long ID_WAVE_MNU_TREBLE;
        static const long ID_WAVE_MNU_ALTO;
        static const long ID_WAVE_MNU_CUSTOM;
        static const long ID_WAVE_MNU_NONVOCALS;
        static const long ID_WAVE_MNU_DOUBLEHEIGHT;

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
