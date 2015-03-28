#ifndef WAVEFORM_H
#define WAVEFORM_H

#include "wx/wx.h"

#include "xlGLCanvas.h"
#include "mpg123.h"
#include <vector>

#define VERTICAL_PADDING            10
#define NO_WAVE_VIEW_SELECTED       -1
#define WAVEFORM_SIDE_MARGIN        25
wxDECLARE_EVENT(EVT_WAVE_FORM_MOVED, wxCommandEvent);
wxDECLARE_EVENT(EVT_TIME_SELECTED, wxCommandEvent);
wxDECLARE_EVENT(EVT_SCROLL_RIGHT, wxCommandEvent);
wxDECLARE_EVENT(EVT_ZOOM, wxCommandEvent);

class TimeLine;

class Waveform : public xlGLCanvas
{
    public:

        static int GetLengthOfMusicFileInMS(const char* filename);
        static int GetTrackSize(mpg123_handle *mh,int bits, int channels);

        int OpenfileMediaFile(const char* filename);
        void CloseMediaFile();

        void SetZoomLevel(int level);
        int GetZoomLevel();

        int SetStartPixelOffset(int startPixel);
        int GetStartPixelOffset();

        void SetTimeFrequency(int startPixel);
        int GetTimeFrequency();

        void SetTimeline(TimeLine* timeLine);

        void UpdatePlayMarker();
        void CheckNeedToScroll();

        Waveform(wxPanel* parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition,
                const wxSize &size=wxDefaultSize,long style=0, const wxString &name=wxPanelNameStr);
        virtual ~Waveform();


        struct MINMAX
        {
            float min;
            float max;
        };



    protected:
        virtual void InitializeGLCanvas();

    private:
      	DECLARE_EVENT_TABLE()
        void LoadTrackData(mpg123_handle *mh,char  * data, int maxSize);
        void SplitTrackDataAndNormalize(signed short* trackData,int trackSize,float* leftData,float* rightData);
        void NormalizeMonoTrackData(signed short* trackData,int trackSize,float* leftData);
        void cleanup(mpg123_handle *mh);
        void GetMinMaxSampleSet(int setSize, float*sampleData,int trackSize, MINMAX* minMax);
        float GetSamplesPerLineFromZoomLevel(int ZoomLevel);
        TimeLine* mTimeline;
        wxPanel* mParent;
        wxWindow* mMainWindow;
        bool mAudioIsLoaded;
        int mStartPixelOffset;
        int mCurrentWaveView;
        int mMediaTrackSize;
        int mFrequency;
        int mZoomLevel;
        int m_bits;
        int m_rate;
        int m_channels;
        bool mPointSize;
        bool m_dragging;
        float* m_left_data;
        float* m_right_data;

        class WaveView
        {
            private:
            float mSamplesPerPixel;
            int mZoomLevel;
            public:

            std::vector<MINMAX> MinMaxs;
            WaveView(int ZoomLevel,float SamplesPerPixel, float*sampleData, int trackSize)
            {
                mZoomLevel = ZoomLevel;
                mSamplesPerPixel = SamplesPerPixel;
                SetMinMaxSampleSet(SamplesPerPixel,sampleData,trackSize);
            }


            WaveView(int ZoomLevel)
            {

            }

            virtual ~WaveView()
            {
            }

            int GetZoomLevel()
            {
                return  mZoomLevel;
            }

            void SetMinMaxSampleSet(float SamplesPerPixel, float*sampleData, int trackSize)
            {
                if( sampleData != nullptr )
                {
                    int sampleIndex=0;
                    float minimum=1;
                    float maximum=-1;
                    int iSamplesPerPixel = (int)SamplesPerPixel;
                    int totalMinMaxs = (int)((float)trackSize/SamplesPerPixel)+1;
                    MinMaxs.clear();
                    for(int i=0;i<totalMinMaxs;i++)
                    {
                        // Use float calculation to minimize compounded rounding of position
                        sampleIndex = (int)((float)i*SamplesPerPixel);
                        minimum=1;
                        maximum=-1;
                        int j;
                        for(j=sampleIndex;j<sampleIndex+iSamplesPerPixel && j<trackSize;j++)
                        {
                            if(sampleData[j]< minimum)
                            {
                                minimum = sampleData[j];
                            }
                            if(sampleData[j]> maximum)
                            {
                                maximum = sampleData[j];
                            }
                        }
                        MINMAX mm;
                        mm.min = minimum;
                        mm.max = maximum;
                        MinMaxs.push_back(mm);
                        if (j>=trackSize)
                            break;
                    }
                }
                int l = MinMaxs.size();
                l++;
            }


        };

        void DrawWaveView(const WaveView &wv);
        void StartDrawing(wxDouble pointSize);
        void renderGL( wxPaintEvent& event );
      	void mouseMoved(wxMouseEvent& event);
        void mouseWheelMoved(wxMouseEvent& event);
      	void mouseLeftDown(wxMouseEvent& event);
      	void mouseLeftUp( wxMouseEvent& event);
      	void OnLeftDClick(wxMouseEvent& event);
        void OnLostMouseCapture(wxMouseCaptureLostEvent& event);
        void OutputText(GLfloat x, GLfloat y, char *text);
        void drawString (void * font, char *s, float x, float y, float z);
        std::vector<WaveView> views;


};

#endif // WAVEFORM_H
