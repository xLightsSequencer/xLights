
#ifndef SequencePreviewAPP_H
#define SequencePreviewAPP_H
#include <wx/app.h>
#endif // SequencePreview_H
#ifndef _glpane_
#define _glpane_

#include "wx/wx.h"
#include "wx/glcanvas.h"

class SequencePreview : public wxGLCanvas
{
    wxGLContext*	m_context;

public:
	SequencePreview(wxPanel* parent, int* args);
	virtual ~SequencePreview();

	void resized(wxSizeEvent& evt);

	int getWidth();
	int getHeight();
	void ClearBackground();

    void InitializePreview();
    void StartDrawing(wxDouble pointSize);
    void DrawPoint(const wxColour &color, wxDouble x, wxDouble y);

    void EndDrawing();

	void render(wxPaintEvent& evt);
	void prepare3DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);
    void prepare2DViewport(int x, int y, int w, int h);

	// events
	void mouseMoved(wxMouseEvent& event);
	void mouseDown(wxMouseEvent& event);
	void mouseWheelMoved(wxMouseEvent& event);
	void mouseReleased(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);
	void keyPressed(wxKeyEvent& event);
	void keyReleased(wxKeyEvent& event);

private:
    bool mIsDrawing = false;

	DECLARE_EVENT_TABLE()
};
#endif
