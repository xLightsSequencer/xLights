#include "PlayerWindow.h"
#include <wx/dcclient.h>
#include <log4cpp/Category.hh>

BEGIN_EVENT_TABLE(PlayerWindow, wxWindow)
EVT_MOTION(PlayerWindow::OnMouseMove)
EVT_LEFT_DOWN(PlayerWindow::OnMouseLeftDown)
EVT_LEFT_UP(PlayerWindow::OnMouseLeftUp)
EVT_PAINT(PlayerWindow::Paint)
END_EVENT_TABLE()

PlayerWindow::PlayerWindow(wxWindow* parent, bool topMost, wxImageResizeQuality quality = wxIMAGE_QUALITY_HIGH, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
    _quality = quality;
    _image = wxImage(size, true);
#ifndef __WXOSX__
    SetDoubleBuffered(true);
#endif
    _dragging = false;

    wxWindow *wind = FindFocus();

    if (topMost)
    {
        Create(parent, id, "Player Window", pos, size, wxBORDER_NONE | wxSTAY_ON_TOP, _T("id"));
    }
    else
    {
        Create(parent, id, "Player Window", pos, size, wxBORDER_NONE, _T("id"));
    }
    SetClientSize(size);
    Move(pos);
    Show();

    Connect(wxEVT_LEFT_DOWN, (wxObjectEventFunction)&PlayerWindow::OnMouseLeftDown, 0, this);
    Connect(wxEVT_LEFT_UP, (wxObjectEventFunction)&PlayerWindow::OnMouseLeftUp, 0, this);
    Connect(wxEVT_MOTION, (wxObjectEventFunction)&PlayerWindow::OnMouseMove, 0, this);
    Connect(wxEVT_PAINT, (wxObjectEventFunction)&PlayerWindow::Paint, 0, this);

    // prevent this window from stealing focus
    if (wind != nullptr)
    {
        wind->SetFocus();
    }

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    int w, h;
    GetSize(&w, &h);
    int x, y;
    GetPosition(&x, &y);
    logger_base.info("Player window created location (%d, %d) size (%d, %d).", x, y, w, h);
}

PlayerWindow::~PlayerWindow()
{
}

void PlayerWindow::SetImage(const wxImage& image)
{
    wxASSERT(image.IsOk());
    int width = 0;
    int height = 0;
    GetSize(&width, &height);
    _image = image.Copy();
    _image.Rescale(width, height, _quality);
    Refresh(false);
}

void PlayerWindow::Paint(wxPaintEvent& event)
{
    wxPaintDC dc(this);

    dc.DrawBitmap(_image, 0, 0);
}

void PlayerWindow::OnMouseLeftUp(wxMouseEvent& event) 
{
    if (_dragging)
    {
        int cwpx, cwpy;
        GetPosition(&cwpx, &cwpy);
        wxPoint currentWindowPos(cwpx, cwpy);

        int x = currentWindowPos.x + event.GetPosition().x - _startMousePos.x;
        int y = currentWindowPos.y + event.GetPosition().y - _startMousePos.y;

        Move(_startDragPos.x + x, _startDragPos.y + y);
        _dragging = false;
    }
}

void PlayerWindow::OnMouseLeftDown(wxMouseEvent& event)
{
    _dragging = true;
    int x, y;
    GetPosition(&x, &y);
    _startDragPos = wxPoint(x,y);
    _startMousePos = event.GetPosition() + _startDragPos;
}

void PlayerWindow::OnMouseMove(wxMouseEvent& event)
{
    if (_dragging)
    {
        int cwpx, cwpy;
        GetPosition(&cwpx, &cwpy);
        wxPoint currentWindowPos(cwpx, cwpy);

        int x = currentWindowPos.x + event.GetPosition().x - _startMousePos.x;
        int y = currentWindowPos.y + event.GetPosition().y - _startMousePos.y;

        Move(_startDragPos.x + x, _startDragPos.y + y);
    }
}

