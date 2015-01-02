#ifndef EFFECTSGRID_H
#define EFFECTSGRID_H

#ifndef _glpane_
#define _glpane_
#endif // EffectsGrid_H

#include "wx/wx.h"
#include "wx/glcanvas.h"
#include "SequenceElements.h"
#include "TimeLine.h"
#include "ElementEffects.h"

#define MINIMUM_EFFECT_WIDTH_FOR_ICON       25
#define MINIMUM_EFFECT_WIDTH_FOR_SMALL_RECT 4

#define EFFECT_RESIZE_NO                    0
#define EFFECT_RESIZE_LEFT                  1
#define EFFECT_RESIZE_RIGHT                 2

wxDECLARE_EVENT(EVT_ZOOM, wxCommandEvent);

class EffectsGrid : public wxGLCanvas
{

public:
	EffectsGrid(wxWindow* parent, int* args);
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

    void DrawLine(const wxColour &color, byte alpha,int x1, int y1,int x2, int y2,float width);
    void DrawRectangle(const wxColour &color, bool dashed, int x1, int y1,int x2, int y2);
    void DrawFillRectangle(const wxColour &color, byte alpha, int x, int y,int width, int height);
    void DrawEffectIcon(GLuint* texture,int x, int y);
    void CreateOrUpdateTexture(char** p_XPM, GLuint *texture);
    void CreateEffectIconTextures();
    void DeleteEffectIconTextures();
    void DrawHorizontalLines();
    void DrawVerticalLines();
    void DrawModelOrViewEffects(Element* element,int row);
    void DrawTimingEffects(Element* element,int row);
    void DrawEffects();
    void Resize(int position);
    void RunMouseOverHitTests(Element * element,int x,int y);
    void UpdateTimePosition(int position);
    SequenceElements* mSequenceElements;
    bool mIsDrawing = false;
    bool mIsInitialized = false;
    wxGLContext*	m_context;
    GLuint m_EffectTextures[NUMBER_OF_EFFECTS];
    int mStartPixelOffset;
    wxColour * mEffectColor;
    wxColour * mGridlineColor;
    wxColour * mTimingColor;
    wxColour * mTimingVerticalLine;

    wxWindow* mParent;
    TimeLine* mTimeline;

    int mResizingMode;
    bool mResizing;
    bool mDragging;
    int mDragStartX;
    int mDragStartY;
    int mDragEndX;
    int mDragEndY;
    ElementEffects* mElementEffects;
    int mResizeEffectIndex;
    int mSelectedRow;
    int mPaintOnIdleCounter;



	DECLARE_EVENT_TABLE()
};

#endif // EFFECTSGRID_H
