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

#include "graphics/xlGraphicsBase.h"
#include "sequencer/Effect.h"

class Model;  // forward declaration

class xlGridCanvas : public GRAPHICS_BASE_CLASS
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
        virtual void Copy() {}
        virtual void Paste() {}
        virtual void Cancel() {}

    protected:
        float getCellSize(bool logical = false) const;
    
        void DrawBaseGrid(xlGraphicsContext *ctx);
    
        float calcCellFromPercent(int value, int base) const;
        float GetRowCenter(int percent, bool logical = false);
        float GetColumnCenter(int percent, bool logical = false);

        int SetRowCenter(int position, bool logical = false);
        int SetColumnCenter(int position, bool logical = false);
        int GetCellFromPosition(int position, bool logical = false) const;
        float calcPercentFromCell(int value, int base, bool logical = false) const;

        Effect* mEffect;
        Model* mModel;
        xlColor* mGridlineColor;
        int mColumns;
        int mRows;
        bool mDragging = false;
        bool mRightDragging = false;
        const int mMinCornerSize = 15;
        DECLARE_EVENT_TABLE()
};
