#ifndef XLGRIDCANVAS_H
#define XLGRIDCANVAS_H

#include "wx/wx.h"
#include "xlGLCanvas.h"
#include "sequencer/Effect.h"

class Model;  // forward declaration

class xlGridCanvas : public xlGLCanvas
{
    public:

        xlGridCanvas(wxWindow* parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition,
                const wxSize &size=wxDefaultSize,long style=0, const wxString &name=wxPanelNameStr);
        virtual ~xlGridCanvas();

        virtual void SetEffect(Effect* effect_) = 0;
        Effect* GetEffect() {return mEffect;}
        void SetModel(Model* cls) {mModel = cls;}
        Model* GetModel() {return mModel;}
        void SetNumColumns(int columns) {mColumns = columns;}
        void SetNumRows(int rows) {mRows = rows;}
        //int GetCellSize() {return mCellSize;}
        void AdjustSize(wxSize& parent_size);
        virtual void ForceRefresh() = 0;
        virtual void Copy() {;}
        virtual void Paste() {;}

    protected:
        virtual void InitializeGLCanvas() = 0;

        void DrawBaseGrid();
        //void DrawEffect();
        int GetRowCenter(int percent);
        int GetColumnCenter(int percent);
        int SetRowCenter(int position);
        int SetColumnCenter(int position);
        int GetCellFromPosition(int position) const;
        int calcCellFromPercent(int value, int base);
        int calcPercentFromCell(int value, int base);

        Effect* mEffect;
        Model* mModel;
        xlColor* mGridlineColor;
        int mCellSize;
        int mColumns;
        int mRows;
        bool mDragging;
        const int mMinCornerSize = 15;
        DECLARE_EVENT_TABLE()
};

#endif // XLGRIDCANVAS_H
