#ifndef XLGRIDCANVASPICTURES_H
#define XLGRIDCANVASPICTURES_H

#include "wx/wx.h"
#include "../../xlGridCanvas.h"
#include "../../sequencer/Effect.h"
#include "../../Image.h"
#include "../../XlightsDrawable.h"

wxDECLARE_EVENT(EVT_EFFECT_CHANGED, wxCommandEvent);
wxDECLARE_EVENT(EVT_IMAGE_FILE_SELECTED, wxCommandEvent);
wxDECLARE_EVENT(EVT_IMAGE_SIZE, wxCommandEvent);
wxDECLARE_EVENT(EVT_EYEDROPPER_COLOR, wxCommandEvent);

class xlGridCanvasPictures : public xlGridCanvas
{
    public:

        enum PaintMode
        {
            PAINT_PENCIL,
            PAINT_ERASER,
            PAINT_EYEDROPPER,
            PAINT_SELECTCOPY
        };

        xlGridCanvasPictures(wxWindow* parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition,
                             const wxSize &size=wxDefaultSize,long style=0, const wxString &name=wxPanelNameStr);
        virtual ~xlGridCanvasPictures();

        virtual void SetEffect(Effect* effect_);
        virtual void ForceRefresh();
        void SetMessageParent(wxWindow* parent) { mMessageParent = parent; }
        void LoadImage();
        void SaveImage();
        void SaveAsImage();
        void ResizeImage();
        virtual void Copy();
        virtual void Paste();
        void CreateNewImage(wxString& image_dir);
        void SetPaintColor( xlColor& color ) { mPaintColor = color; }
        void SetPaintMode( PaintMode mode ) { mPaintMode = mode; Refresh(false); }

    protected:
        virtual void InitializeGLContext();

    private:

        enum ImageDisplayMode
        {
            IMAGE_NONE,
            IMAGE_SINGLE_OVERSIZED,
            IMAGE_MULTIPLE_OVERSIZED,
            IMAGE_SINGLE_FITS,
            IMAGE_MULTIPLE_FITS
        };

        void mouseMoved(wxMouseEvent& event);
        void mouseRightDown(wxMouseEvent& event);
        void mouseLeftDown(wxMouseEvent& event);
        void mouseDown(int x, int y);
        void mouseLeftUp(wxMouseEvent& event);
        void mouseRightUp(wxMouseEvent& event);
        void mouseUp();
        void render(wxPaintEvent& event);
        void DrawPicturesEffect();
        void DrawSelection();
        void LoadAndProcessImage();
        void ProcessNewImage();
        wxString GetImageFilename();
        void SaveImageToFile();
        void UpdateRenderedImage();
        void CalcSelection();
        void ProcessHoverDrag(int column, int row);

        bool mRightDown;
        bool mLeftDown;

        enum ImageDisplayMode img_mode;
        bool mModified;
        int imageCount;
        int imageIndex;
        int imageWidth;
        int imageHeight;
        int frame;
        int maxmovieframes;
        bool use_ping;
        double scaleh;
        double scalew;
        wxImage image;
        Image* imageGL_ping;
        Image* imageGL_pong;
        Image* mImage;
        wxImage image_copy;
        Image* mImageCopy;
        xLightsDrawable* sprite;
        xLightsDrawable* copy_sprite;
        wxString PictureName;
        wxString NewPictureName;
        xlColor mPaintColor;
        xlColor mEraseColor;
        PaintMode mPaintMode;
        xlColor* mSelectionColor;
        wxWindow* mMessageParent;

        int mDragStartX;
        int mDragStartY;
        int mDragEndX;
        int mDragEndY;
        int mHoverDragRow;
        int mHoverDragCol;
        int mStartRow;
        int mStartCol;
        int mEndRow;
        int mEndCol;
        bool mHoverSelection;
        bool mCopyAvailable;
        bool mPasteCopy;

        DECLARE_EVENT_TABLE()
};

#endif // XLGRIDCANVASPICTURES_H
