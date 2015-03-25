#ifndef MODELPREVIEW_H
#define MODELPREVIEW_H



#include "wx/wx.h"

#include "Image.h"
#include "XlightsDrawable.h"
#include "Color.h"
#include "xlGLCanvas.h"

class ModelPreview : public xlGLCanvas
{

public:
	ModelPreview(wxPanel* parent);
	virtual ~ModelPreview();

    // Public Methods
	void resized(wxSizeEvent& event);
	int getWidth();
	int getHeight();
	void ClearBackground();
	void InitializePreview(wxString img,int brightness);
    bool StartDrawing(wxDouble pointSize);
    void SetPointSize(wxDouble pointSize);
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

    Image* image = NULL;
    xLightsDrawable* sprite;

	DECLARE_EVENT_TABLE()
};


#endif // MODELPREVIEW_H
