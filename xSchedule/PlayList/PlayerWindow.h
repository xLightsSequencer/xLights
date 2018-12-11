#ifndef PLAYERWINDOW_H
#define PLAYERWINDOW_H

#include <wx/window.h>
#include <wx/image.h>
#include <wx/frame.h>

class PlayerWindow: public wxFrame
{
    wxImage _image;
    wxImage _lastImage;
    wxPoint _startDragPos;
    wxPoint _startMousePos;
    bool _dragging;
    wxImageResizeQuality _quality;
    int _swsQuality;

	public:

		PlayerWindow(wxWindow* parent, bool topMost, wxImageResizeQuality quality = wxIMAGE_QUALITY_HIGH, int swsQuality = -1, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayerWindow();
        void SetImage(const wxImage& image);

	private:

        void OnMouseLeftUp(wxMouseEvent& event);
        void OnMouseMove(wxMouseEvent& event);
        void OnMouseLeftDown(wxMouseEvent& event);
        void Paint(wxPaintEvent& event);

		DECLARE_EVENT_TABLE()
};

#endif
