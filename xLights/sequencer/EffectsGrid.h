#ifndef EFFECTSGRID_H
#define EFFECTSGRID_H

#ifndef _glpane_
#define _glpane_
#endif


#include "wx/wx.h"
#include "wx/glcanvas.h"
#include "../xlGLCanvas.h"
#include "../Color.h"
#include "SequenceElements.h"
#include "TimeLine.h"
#include "EffectLayer.h"
#include "Effect.h"
#include "Element.h"
#include "../EffectTreeDialog.h"

#include <map>

#define MINIMUM_EFFECT_WIDTH_FOR_SMALL_RECT 4


enum class HitLocation {
    NONE,
    LEFT_EDGE,
    LEFT_EDGE_DISCONNECT,
    LEFT,
    CENTER,
    RIGHT,
    RIGHT_EDGE_DISCONNECT,
    RIGHT_EDGE
};

enum EFF_ALIGN_MODE {
    ALIGN_START_TIMES,
    ALIGN_END_TIMES,
    ALIGN_BOTH_TIMES,
    ALIGN_CENTERPOINTS,
    ALIGN_MATCH_DURATION,
    ALIGN_START_TIMES_SHIFT,
    ALIGN_END_TIMES_SHIFT
};

wxDECLARE_EVENT(EVT_ZOOM, wxCommandEvent);
wxDECLARE_EVENT(EVT_GSCROLL, wxCommandEvent);
wxDECLARE_EVENT(EVT_PLAY_MODEL_EFFECT, wxCommandEvent);

wxDECLARE_EVENT(EVT_EFFECT_DROPPED, wxCommandEvent);

struct EffectDropData;

class MainSequencer;
class PixelBufferClass;
class SequenceData;

class EffectsGrid : public xlGLCanvas
{

public:
	EffectsGrid(MainSequencer* parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition,
                const wxSize &size=wxDefaultSize,long style=0, const wxString &name=wxPanelNameStr);

	virtual ~EffectsGrid();

    // Public Methods
    void SetStartPixelOffset(int offset);
    void SetSequenceElements(SequenceElements* elements);
    void SetTimeline(TimeLine* timeline);
    bool DragOver(int x, int y);
    void OnDrop(int x, int y);
    void ForceRefresh();
    void SetEffectIconBackground(bool mode) {mGridIconBackgrounds = mode;}
    void SetEffectNodeValues(bool mode) {mGridNodeValues = mode;}
    void MoveSelectedEffectUp(bool shift);
    void MoveSelectedEffectDown(bool shift);
    void MoveSelectedEffectLeft(bool shift);
    void MoveSelectedEffectRight(bool shift);
    void SetEffectStatusText(Effect* eff);

    void DeleteSelectedEffects();
    void SetEffectsDescription();
    void ProcessDroppedEffect(Effect* effect);
    void CopyModelEffects(int row_number);
    void PasteModelEffects(int row_number);

    void AlignSelectedEffects(EFF_ALIGN_MODE align_mode);

    void OldPaste(const wxString &data, const wxString &pasteDataVer);
    void Paste(const wxString &data, const wxString &pasteDataVer, bool row_paste = false);
    void SetCanPaste() { mCanPaste = true; }
    int GetStartColumn() { return mRangeStartCol < mRangeEndCol ? mRangeStartCol : mRangeEndCol; }

    void SetRenderDataSources(xLightsFrame *xl, const SequenceData *data) {
        seqData = data;
        xlights = xl;
    }

    void ClearSelection();

    static EffectLayer* FindOpenLayer(Element* elem, int startTimeMS, int endTimeMS);

protected:
    virtual void InitializeGLCanvas();


private:
    Effect* GetEffectAtRowAndTime(int row, int ms,int &index, HitLocation &selectionType);
    int GetClippedPositionFromTimeMS(int ms);

    void sendRenderEvent(const std::string &model, int start, int end, bool clear = true);
    void sendRenderDirtyEvent();
    void render(wxPaintEvent& evt);
    void magnify(wxMouseEvent& event);
	void mouseMoved(wxMouseEvent& event);
	void mouseDown(wxMouseEvent& event);
	void mouseWheelMoved(wxMouseEvent& event);
	void mouseReleased(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
	void mouseLeftDClick(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);
    void OnLostMouseCapture(wxMouseCaptureLostEvent& event);
	void keyPressed(wxKeyEvent& event);
	void keyReleased(wxKeyEvent& event);
	void Draw();

    void CreateEffectIconTextures();
    void DeleteEffectIconTextures();
    void DrawLines();
    void DrawModelOrViewEffects(int row);
    void DrawSelectedCells();

    int DrawEffectBackground(const Row_Information_Struct* ri, const Effect *effect,
                             int x1, int y1, int x2, int y2,
                             DrawGLUtils::xlVertexColorAccumulator &backgrounds);

    void DrawTimingEffects(int row);
    void DrawEffects();
    void DrawPlayMarker();
    bool AdjustDropLocations(int x, EffectLayer* el);
    void Resize(int position, bool offset);
    void RunMouseOverHitTests(int rowIndex, int x,int y);
    void UpdateTimePosition(int time);
    void UpdateMousePosition(int time);
    void UpdateZoomPosition(int time);
    void EstablishSelectionRectangle();
    void UpdateSelectionRectangle();
    void UpdateSelectedEffects();
    void CheckForPartialCell(int x_pos);
    void RaiseSelectedEffectChanged(Effect* effect, bool isNew, bool updateUI = true);
    void RaiseEffectDropped(int x, int y);
    void RaisePlayModelEffect(Element* element, Effect* effect,bool renderEffect);
    Element* GetActiveTimingElement();
    bool MultipleEffectsSelected();
    std::list<Effect*> GetSelectedEffects();
    bool PapagayoEffectsSelected();
    void ResizeSingleEffect(int position);
    void ResizeMoveMultipleEffects(int position, bool offset);
    void ResizeMoveMultipleEffectsByTime(int delta);
    void GetRangeOfMovementForSelectedEffects(int &toLeft, int &toRight);
    void MoveAllSelectedEffects(int deltaMS, bool offset);
    int GetRow(int y);
    void OnGridPopup(wxCommandEvent& event);
    void FillRandomEffects();
    bool OneCellSelected();
    SequenceElements* mSequenceElements;
    bool mIsDrawing = false;
    bool mGridIconBackgrounds;
    bool mGridNodeValues = true;
    bool mCanPaste;

    //~ Need to see why I cannot access xLightsFrame::GB_EFFECTS_e from xLightsMain.h
    // for effect count
    std::vector<GLuint> m_EffectTextures;
    int mStartPixelOffset;
    xlColor * mEffectColor;
    xlColor * mGridlineColor;
    xlColor * mTimingColor;
    xlColor * mTimingVerticalLine;
    xlColor * mLabelColor;
    xlColor * mLabelOutlineColor;
    xlColor * mPhraseColor;
    xlColor * mWordColor;
    xlColor * mPhonemeColor;

    xlColor * mSelectionColor;
    xlColor * mSelectFocusColor;
    wxWindow* mParent;
    TimeLine* mTimeline;

    int mSelectedRow;
    Effect* mSelectedEffect;

    DrawGLUtils::xlVertexAccumulator lines;
    DrawGLUtils::xlVertexAccumulator timingEffLines;
    DrawGLUtils::xlVertexColorAccumulator timingLines;
    DrawGLUtils::xlVertexTextAccumulator texts;
    DrawGLUtils::xlVertexAccumulator selectedLines;
    DrawGLUtils::xlVertexAccumulator selectFocusLines;
    DrawGLUtils::xlVertexColorAccumulator backgrounds;
    DrawGLUtils::xlVertexColorAccumulator textBackgrounds;
    DrawGLUtils::xlVertexColorAccumulator selectedBoxes;
    std::map<GLuint, DrawGLUtils::xlVertexTextureAccumulator> textures;

    int mResizingMode;
    int mStartResizeTimeMS;
    bool mResizing;
    bool mDragging;
    int mDragStartRow;
    int mDragStartX;
    int mDragStartY;
    int mDragEndX;
    int mDragEndY;
    float magSinceLast;

    EffectLayer* mEffectLayer;
    int mResizeEffectIndex;

    // Drag And Drop
    bool mDragDropping;
    int mDropStartX;
    int mDropEndX;
    int mDropRow;
    int mDropStartTimeMS;
    int mDropEndTimeMS;

    bool mCellRangeSelected;
    bool mPartialCellSelected;
    int mRangeStartCol;
    int mRangeEndCol;
    int mRangeStartRow;
    int mRangeEndRow;

    static const long ID_GRID_MNU_COPY;
    static const long ID_GRID_MNU_PASTE;
    static const long ID_GRID_MNU_DELETE;
    static const long ID_GRID_MNU_RANDOM_EFFECTS;
    static const long ID_GRID_MNU_DESCRIPTION;
    static const long ID_GRID_MNU_UNDO;
    static const long ID_GRID_MNU_PRESETS;
    static const long ID_GRID_MNU_BREAKDOWN_PHRASE;
    static const long ID_GRID_MNU_BREAKDOWN_WORD;
    static const long ID_GRID_MNU_BREAKDOWN_WORDS;
    static const long ID_GRID_MNU_ALIGN_START_TIMES;
    static const long ID_GRID_MNU_ALIGN_END_TIMES;
    static const long ID_GRID_MNU_ALIGN_BOTH_TIMES;
    static const long ID_GRID_MNU_ALIGN_CENTERPOINTS;
    static const long ID_GRID_MNU_ALIGN_MATCH_DURATION;
    static const long ID_GRID_MNU_ALIGN_START_TIMES_SHIFT;
    static const long ID_GRID_MNU_ALIGN_END_TIMES_SHIFT;

    EventPlayEffectArgs* playArgs;

    const SequenceData *seqData;
    xLightsFrame *xlights;

	DECLARE_EVENT_TABLE()
};

#endif // EFFECTSGRID_H
