#ifndef XLCOLORCANVAS_H
#define XLCOLORCANVAS_H

#include "wx/wx.h"
#include "xlGLCanvas.h"
#include "Color.h"

wxDECLARE_EVENT(EVT_CP_SLIDER_CHANGED, wxCommandEvent);
wxDECLARE_EVENT(EVT_CP_PALETTE_CHANGED, wxCommandEvent);

class xlColorCanvas : public xlGLCanvas
{
    public:
        xlColorCanvas(wxWindow* parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition,
                const wxSize &size=wxDefaultSize,long style=0, const wxString &name=wxPanelNameStr);
        virtual ~xlColorCanvas();

        enum DisplayType
        {
            TYPE_PALETTE,
            TYPE_SLIDER
        };

        void SetType( DisplayType type) { mDisplayType = type; }
        void SetMode( ColorDisplayMode mode );
        HSVValue &GetHSV();
        void SetHSV( const HSVValue &hsv);
        wxColor GetRGB() { return mRGB.asWxColor(); }
        void SetRGB( xlColor rgb);

    
        virtual bool UsesVertexTextureAccumulator() { return false; }
        virtual bool UsesVertexColorAccumulator() {return false;}
        virtual bool UsesVertexAccumulator() {return false;}
        virtual bool UsesAddVertex() {return true;}
    protected:
        virtual void InitializeGLCanvas();

    private:

        void mouseMoved(wxMouseEvent& event);
        void mouseDown(wxMouseEvent& event);
        void mouseReleased(wxMouseEvent& event);
        void render(wxPaintEvent& event);
        void OnCanvasResize(wxSizeEvent& evt);
        void DrawSlider();
        void DrawPalette();
        void ProcessSliderClick(int row);
        void ProcessPaletteClick(int row, int column);
        int GetRGBColorFromRangeValue( int position, int range, int max_value, bool invert );

        DisplayType mDisplayType;
        ColorDisplayMode mDisplayMode;
        bool mDragging;
        HSVValue mHSV;
        xlColor mRGB;
        int iXrange;
        int iYrange;
        double dXrange;
        double dYrange;

        DECLARE_EVENT_TABLE()
};

#endif // XLGRIDCANVAS_H
