
#ifndef SequencePreview_H
#define SequencePreview_H
//#include <wx/app.h>
#ifndef _glpane_
#define _glpane_
#endif //

#include "wx/wx.h"
#include "wx/glcanvas.h"

class xlColor;

class SequencePreview : public wxGLCanvas
{

public:
	SequencePreview(wxPanel* parent, int* args);
	virtual ~SequencePreview();

    // Public Methods
	void resized(wxSizeEvent& evt);
	int getWidth();
	int getHeight();
	void ClearBackground();
    void InitializePreview();
    bool StartDrawing(wxDouble pointSize);
    void EndDrawing();
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

    bool mIsDrawing = false;
    bool mIsInitialized = false;

    wxGLContext*	m_context;

	DECLARE_EVENT_TABLE()
};
#endif
