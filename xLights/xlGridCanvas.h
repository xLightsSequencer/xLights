#ifndef XLGRIDCANVAS_H
#define XLGRIDCANVAS_H

#include "wx/wx.h"
#include "xlGLCanvas.h"
#include "Effect.h"
#include "Image.h"
#include "XlightsDrawable.h"

class ModelClass;  // forward declaration

wxDECLARE_EVENT(EVT_EFFECT_CHANGED, wxCommandEvent);

class xlGridCanvas : public xlGLCanvas
{
    public:
        xlGridCanvas(wxWindow* parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition,
                const wxSize &size=wxDefaultSize,long style=0, const wxString &name=wxPanelNameStr);
        virtual ~xlGridCanvas();

        void SetEffect(Effect* effect_);
        void SetModelClass(ModelClass* cls) {mModelClass = cls;}
        void SetNumColumns(int columns) {mColumns = columns;}
        void SetNumRows(int rows) {mRows = rows;}
        int GetCellSize() {return mCellSize;}
        void AdjustSize(wxSize& parent_size);
        void ForceRefresh();

        Effect* GetEffect() {return mEffect;}

    protected:
        virtual void InitializeGLCanvas();

    private:

        enum ImageDisplayMode
        {
            IMAGE_NONE,
            IMAGE_SINGLE_OVERSIZED,
            IMAGE_MULTIPLE_OVERSIZED,
            IMAGE_SINGLE_FITS,
            IMAGE_MULTIPLE_FITS
        };

        int CheckForCornerHit(int x, int y);
        void mouseMoved(wxMouseEvent& event);
        void mouseDown(wxMouseEvent& event);
        void mouseReleased(wxMouseEvent& event);
        void render(wxPaintEvent& event);
        void DrawBaseGrid();
        void DrawEffect();
        int GetRowCenter(int percent);
        int GetColumnCenter(int percent);
        int SetRowCenter(int position);
        int SetColumnCenter(int position);
        void CreateCornerTextures();
        void UpdateMorphPositionsFromEffect();
        void UpdateSelectedMorphCorner(int x, int y);
        void StoreUpdatedMorphPositions();
        int GetCellFromPosition(int position);

        Effect* mEffect;
        ModelClass* mModelClass;
        xlColor* mGridlineColor;
        int mCellSize;
        int mColumns;
        int mRows;
        int x1a, x1b, x2a, x2b, y1a, y1b, y2a, y2b;
        int mSelectedCorner;
        bool mDragging;
        bool mMorphStartLinked;
        bool mMorphEndLinked;
        wxBitmap corner_1a, corner_1b, corner_2a, corner_2b;
        GLuint mCornerTextures[6];

        // picture effect support
        wxString GetImageFilename();
        enum ImageDisplayMode img_mode;
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
        xLightsDrawable* sprite;
        wxString PictureName;
        wxString NewPictureName;

        DECLARE_EVENT_TABLE()
};

#endif // XLGRIDCANVAS_H
