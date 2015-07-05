#ifndef XLGRIDCANVAS_H
#define XLGRIDCANVAS_H

#include "wx/wx.h"
#include <xlGLCanvas.h>
#include "Effect.h"

class ModelClass;  // forward declaration

wxDECLARE_EVENT(EVT_EFFECT_CHANGED, wxCommandEvent);

class xlGridCanvas : public xlGLCanvas
{
    public:
        xlGridCanvas(wxWindow* parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition,
                const wxSize &size=wxDefaultSize,long style=0, const wxString &name=wxPanelNameStr);
        virtual ~xlGridCanvas();

        void SetEffect(Effect* effect_) {mEffect = effect_;}
        void SetModelClass(ModelClass* cls) {mModelClass = cls;}
        void SetNumColumns(int columns) {mColumns = columns;}
        void SetNumRows(int rows) {mRows = rows;}
        int GetCellSize() {return mCellSize;}
        void AdjustSize(wxSize& parent_size);

        Effect* GetEffect() {return mEffect;}

    protected:
        virtual void InitializeGLCanvas();

    private:
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
        wxBitmap corner_1a, corner_1b, corner_2a, corner_2b;
        GLuint mCornerTextures[4];

        DECLARE_EVENT_TABLE()
};

#endif // XLGRIDCANVAS_H
