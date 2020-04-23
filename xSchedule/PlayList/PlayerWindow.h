#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/window.h>
#include <wx/image.h>
#include <wx/frame.h>

#include <mutex>
#include <atomic>

class PlayerWindow: public wxFrame
{
    wxImage _image;
    wxImage _inputImage;
    wxPoint _startDragPos;
    wxPoint _startMousePos;
    bool _dragging;
    wxImageResizeQuality _quality;
    int _swsQuality;
    std::timed_mutex _mutex;
    std::atomic_bool _imageChanged;

    bool PrepareImage();

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
