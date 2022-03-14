#pragma once

#include <wx/dialog.h>

#include <vector>

class wxPanel;

class SketchPathDialog: public wxDialog
{
public:
	SketchPathDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
    virtual ~SketchPathDialog() = default;

private:
	DECLARE_EVENT_TABLE()

    wxPanel* m_sketchPanel = nullptr;

	void OnSketchPaint(wxPaintEvent& event);
    void OnSketchKeyDown(wxKeyEvent& event);
    void OnSketchLeftDown(wxMouseEvent& event);
    void OnSketchLeftUp(wxMouseEvent& event);
    void OnSketchMouseMove(wxMouseEvent& event);

    void OnButton_StartPath(wxCommandEvent& event);

    void OnButton_Ok(wxCommandEvent& event);
    void OnButton_Cancel(wxCommandEvent& event);

    struct HandlePoint
    {
        HandlePoint(wxPoint _pt) :
            pt(_pt)
        {}
        wxPoint pt;
        bool state = false;
    };
    std::vector<HandlePoint> m_handles;
    std::vector<HandlePoint>::size_type m_grabbedHandleIndex = -1;
};
