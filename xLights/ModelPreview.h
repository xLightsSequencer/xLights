#ifndef MODELPREVIEW_H
#define MODELPREVIEW_H
#ifndef _glpane_
#define _glpane_

#include "wx/wx.h"
#include "wx/glcanvas.h"
#endif

#include "Image.h"
#include "Drawable.h"

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
    void InitializePreview(wxString img);
    void StartDrawing(wxDouble pointSize);
    void SetPointSize(wxDouble pointSize);
    void DrawPoint(const wxColour &color, wxDouble x, wxDouble y);
    void EndDrawing();
	void SetCanvasSize(int width,int height);
	void SetbackgroundImage(wxString image);
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
    wxDouble mPointSize = 2.0;
    wxGLContext*	m_context;

    Image* image = NULL;
    Drawable* sprite;

	DECLARE_EVENT_TABLE()
};


#endif // MODELPREVIEW_H
