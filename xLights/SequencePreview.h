
#ifndef SequencePreview_H
#define SequencePreview_H
//#include <wx/app.h>
#endif // SequencePreview_H
#ifndef _glpane_
#define _glpane_

#include "wx/wx.h"
#include "wx/glcanvas.h"

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
    void StartDrawing(wxDouble pointSize);
    void DrawPoint(const wxColour &color, wxDouble x, wxDouble y);
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
    wxGLContext*	m_context;

	DECLARE_EVENT_TABLE()
};
#endif
