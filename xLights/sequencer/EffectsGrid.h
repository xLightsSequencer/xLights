#ifndef EFFECTSGRID_H
#define EFFECTSGRID_H

#ifndef _glpane_
#define _glpane_
#endif


#include "wx/wx.h"
#include "wx/glcanvas.h"
#include "xlGLCanvas.h"
#include "Color.h"
#include "SequenceElements.h"
#include "TimeLine.h"
#include "EffectLayer.h"
#include "Effect.h"
#include "Element.h"


#define MINIMUM_EFFECT_WIDTH_FOR_SMALL_RECT 4

#define EFFECT_RESIZE_NO                    0
#define EFFECT_RESIZE_LEFT                  1
#define EFFECT_RESIZE_RIGHT                 2

wxDECLARE_EVENT(EVT_ZOOM, wxCommandEvent);
wxDECLARE_EVENT(EVT_SELECTED_EFFECT_CHANGED, wxCommandEvent);
wxDECLARE_EVENT(EVT_PLAY_MODEL_EFFECT, wxCommandEvent);

wxDECLARE_EVENT(EVT_EFFECT_DROPPED, wxCommandEvent);

struct EffectDropData;

class MainSequencer;

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

    void Paste(const wxString &data);
    
    void SetRenderDataSources(xLightsFrame *xl, const SequenceData *data) {
        xlights = xl;  seqData = data;
    }
protected:
    virtual void InitializeGLCanvas();

private:
    void sendRenderEvent(const wxString &model, double start, double end, bool clear = true);
    void render(wxPaintEvent& evt);
	void mouseMoved(wxMouseEvent& event);
	void mouseDown(wxMouseEvent& event);
	void mouseWheelMoved(wxMouseEvent& event);
	void mouseReleased(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);
    void OnLostMouseCapture(wxMouseCaptureLostEvent& event);
	void keyPressed(wxKeyEvent& event);
	void keyReleased(wxKeyEvent& event);
	void Draw();

    void DrawEffectIcon(GLuint* texture,int x, int y);
    void CreateEffectIconTextures();
    void DeleteEffectIconTextures();
    void DrawHorizontalLines();
    void DrawVerticalLines();
    void DrawModelOrViewEffects(int row);
    bool DrawEffectBackground(const Effect *effect, int x1, int y1, int x2, int y2);
    void DrawTimingEffects(int row);
    void DrawEffects();
    void AdjustDropLocations(int x, EffectLayer* el);
    void Resize(int position);
    void RunMouseOverHitTests(int rowIndex, int x,int y);
    void UpdateTimePosition(int time);
    void CheckForSelectionRectangle();
    void RaiseSelectedEffectChanged(Effect* effect);
    void RaiseEffectDropped(int x, int y);
    void RaisePlayModelEffect(Element* element, Effect* effect,bool renderEffect);
    Element* GetActiveTimingElement();
    bool MultipleEffectsSelected();
    void ResizeSingleEffect(int position);
    void ResizeMoveMultipleEffects(int position);
    void GetRangeOfMovementForSelectedEffects(double &toLeft, double &toRight);
    void MoveAllSelectedEffects(double delta);
    int GetRow(int y);
    SequenceElements* mSequenceElements;
    bool mIsDrawing = false;
    bool mGridIconBackgrounds;

    //~ Need to see why I cannot access xLightsFrame::GB_EFFECTS_e from xLightsMain.h
    // for effect count
    GLuint m_EffectTextures[40];
    int mStartPixelOffset;
    xlColor * mEffectColor;
    xlColor * mGridlineColor;
    xlColor * mTimingColor;
    xlColor * mTimingVerticalLine;

    xlColor * mSelectionColor;
    wxWindow* mParent;
    TimeLine* mTimeline;

    int mSelectedRow;
    Effect* mSelectedEffect;

    int mResizingMode;
    int mStartResizeTime;
    bool mResizing;
    bool mDragging;
    bool mEffectDragging;
    int mDragStartX;
    int mDragStartX1;
    int mDragStartX2;
    int mDragStartY;
    int mDragEndX;
    int mDragEndY;

    EffectLayer* mEffectLayer;
    int mResizeEffectIndex;

    // Drag And Drop
    bool mDragDropping;
    int mDropStartX;
    int mDropEndX;
    int mDropRow;
    double mDropStartTime;
    double mDropEndTime;

    bool mEmptyCellSelected;
    int mSelectedTimingIndex;
    int mSelectedTimingRow;

    EventPlayEffectArgs* playArgs;
    
    const SequenceData *seqData;
    xLightsFrame *xlights;

	DECLARE_EVENT_TABLE()
};

#endif // EFFECTSGRID_H
