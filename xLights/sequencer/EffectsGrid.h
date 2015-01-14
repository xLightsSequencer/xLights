#ifndef EFFECTSGRID_H
#define EFFECTSGRID_H

#ifndef _glpane_
#define _glpane_
#endif


#include "wx/wx.h"
#include "wx/glcanvas.h"
#include "SequenceElements.h"
#include "TimeLine.h"
#include "EffectLayer.h"
#include "Effect.h"
#include "Element.h"


#define MINIMUM_EFFECT_WIDTH_FOR_ICON       25
#define MINIMUM_EFFECT_WIDTH_FOR_SMALL_RECT 4
#define EFFECT_ICON_WIDTH                   16

#define EFFECT_RESIZE_NO                    0
#define EFFECT_RESIZE_LEFT                  1
#define EFFECT_RESIZE_RIGHT                 2

wxDECLARE_EVENT(EVT_ZOOM, wxCommandEvent);
wxDECLARE_EVENT(EVT_SELECTED_EFFECT_CHANGED, wxCommandEvent);

class EffectsGrid : public wxGLCanvas
{

public:
	EffectsGrid(wxScrolledWindow* parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition,
                const wxSize &size=wxDefaultSize,long style=0, const wxString &name=wxPanelNameStr);

	virtual ~EffectsGrid();

    // Public Methods
	void resized(wxSizeEvent& evt);
	int getWidth();
	int getHeight();
	void ClearBackground();
    void InitializeGrid();
    void StartDrawing(wxDouble pointSize);
    void DrawPoint(const wxColour &color, wxDouble x, wxDouble y);
    void EndDrawing();
    void SetCanvasSize(int w, int h);
    void SetStartPixelOffset(int offset);
    void SetSequenceElements(SequenceElements* elements);
    void SetTimeline(TimeLine* timeline);
    void DragOver(int x, int y);

private:
	void render(wxPaintEvent& evt);
    void prepare2DViewport(int x, int y, int w, int h);
	void mouseMoved(wxMouseEvent& event);
	void mouseDown(wxMouseEvent& event);
	void mouseWheelMoved(wxMouseEvent& event);
	void mouseReleased(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);
	void keyPressed(wxKeyEvent& event);
	void keyReleased(wxKeyEvent& event);
	void OnIdle(wxIdleEvent &event);

    void DrawLine(const wxColour &color, wxByte alpha,int x1, int y1,int x2, int y2,float width);
    void DrawRectangle(const wxColour &color, bool dashed, int x1, int y1,int x2, int y2);
    void DrawFillRectangle(const wxColour &color, wxByte alpha, int x, int y,int width, int height);
    void DrawEffectIcon(GLuint* texture,int x, int y);
    void CreateOrUpdateTexture(char** p_XPM, GLuint *texture);
    void CreateEffectIconTextures();
    void DeleteEffectIconTextures();
    void DrawHorizontalLines();
    void DrawVerticalLines();
    void DrawModelOrViewEffects(int row);
    void DrawTimingEffects(int row);
    void DrawEffects();
    void Resize(int position);
    void RunMouseOverHitTests(Element * element,int effectLayerIndex, int x,int y);
    void UpdateTimePosition(int position);
    void CheckForSelectionRectangle();
    void RaiseSelectedEffectChanged(Effect* effect);

    Element* GetActiveTimingElement();

    int GetRow(int y);
    SequenceElements* mSequenceElements;
    bool mIsDrawing = false;
    bool mIsInitialized = false;
    wxGLContext*	m_context;
    //~ Need to see why I cannot access xLightsFrame::GB_EFFECTS_e from xLightsMain.h
    // for effect count
    GLuint m_EffectTextures[40];
    int mStartPixelOffset;
    wxColour * mEffectColor;
    wxColour * mGridlineColor;
    wxColour * mTimingColor;
    wxColour * mTimingVerticalLine;

    wxColour * mSelectionColor;
    wxWindow* mParent;
    TimeLine* mTimeline;

    int mSelectedRow;
    int mSelectedEffectIndex;

    int mResizingMode;
    bool mResizing;
    bool mDragging;
    int mDragStartX;
    int mDragStartY;
    int mDragEndX;
    int mDragEndY;
    EffectLayer* mEffectLayer;
    int mResizeEffectIndex;
    int mPaintOnIdleCounter;



	DECLARE_EVENT_TABLE()
};

#endif // EFFECTSGRID_H
