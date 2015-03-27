
#ifndef SequencePreview_H
#define SequencePreview_H
//#include <wx/app.h>

#include "wx/wx.h"
#include "xlGLCanvas.h"


class xlColor;

class SequencePreview : public xlGLCanvas
{

public:
	SequencePreview(wxPanel* parent);
	virtual ~SequencePreview();

    // Public Methods
    void InitializePreview();
    bool StartDrawing(wxDouble pointSize);
    void EndDrawing();

protected:
    virtual void InitializeGLCanvas();

private:
	void render(wxPaintEvent& evt);
	void mouseMoved(wxMouseEvent& event);
	void mouseDown(wxMouseEvent& event);
	void mouseWheelMoved(wxMouseEvent& event);
	void mouseReleased(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);
	void keyPressed(wxKeyEvent& event);
	void keyReleased(wxKeyEvent& event);

    bool mIsDrawing = false;

	DECLARE_EVENT_TABLE()
};
#endif
