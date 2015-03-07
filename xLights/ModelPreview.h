#ifndef MODELPREVIEW_H
#define MODELPREVIEW_H
#ifndef _glpane_
#define _glpane_

#include "wx/wx.h"
#include "wx/glcanvas.h"
#endif

#include "Image.h"
#include "XlightsDrawable.h"
#include "Color.h"

class ModelPreview : public wxGLCanvas
{

public:
	ModelPreview(wxPanel* parent, int* args);
	virtual ~ModelPreview();

    // Public Methods
	void resized(wxSizeEvent& event);
	int getWidth();
	int getHeight();
	void ClearBackground();
	void InitializePreview(wxString img,int brightness);
    bool StartDrawing(wxDouble pointSize);
    void SetPointSize(wxDouble pointSize);
    void DrawPoint(const xlColor &color, wxDouble x, wxDouble y);
    void DrawRectangle(const xlColor &color,bool dashed,int x1, int y1,int x2, int y2);
    void DrawFillRectangle(const xlColor &color, int x, int y,int width, int height);
    void DrawLine(const xlColor &color, wxDouble x1, wxDouble y1,wxDouble x2, wxDouble y2);
    void EndDrawing();
	void SetCanvasSize(int width,int height);
	void SetbackgroundImage(wxString image);
	void SetBackgroundBrightness(int brightness);

private:
	void render(wxPaintEvent& event);
	void SetOrigin();
    void prepare2DViewport(int x, int y, int w, int h);
	void mouseMoved(wxMouseEvent& event);
	void mouseLeftDown(wxMouseEvent& event);
	void mouseLeftUp(wxMouseEvent& event);
	void mouseWheelMoved(wxMouseEvent& event);
	void mouseReleased(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);
	void keyPressed(wxKeyEvent& event);
	void keyReleased(wxKeyEvent& event);

    bool mIsDrawing = false;
    bool mIsInitialized = false;
    bool mBackgroundImageExists = false;
    wxString mBackgroundImage;
    int  mBackgroundBrightness=100;
    wxDouble mPointSize = 2.0;
    wxGLContext*	m_context;

    Image* image = NULL;
    xLightsDrawable* sprite;

	DECLARE_EVENT_TABLE()
};


#endif // MODELPREVIEW_H
