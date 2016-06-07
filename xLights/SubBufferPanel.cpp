#include "SubBufferPanel.h"

#include <wx/dcbuffer.h>
#include <wx/settings.h>
#include <wx/arrstr.h>


BEGIN_EVENT_TABLE(SubBufferPanel, wxWindow)
EVT_MOTION(SubBufferPanel::mouseMoved)
EVT_LEFT_DOWN(SubBufferPanel::mouseLeftDown)
EVT_LEFT_UP(SubBufferPanel::mouseLeftUp)
EVT_LEFT_DCLICK(SubBufferPanel::mouseDblClick)
EVT_PAINT(SubBufferPanel::Paint)
EVT_CONTEXT_MENU(SubBufferPanel::ContextMenu)
END_EVENT_TABLE()


SubBufferPanel::SubBufferPanel(wxPanel* parent, wxWindowID id,
                               const wxPoint &pos,
                               const wxSize &size,
                               long style)
: wxWindow(parent, id, pos, size, style, "ID_CUSTOM_SubBuffer"), xlCustomControl()
{
    x1 = y1 = 0.0;
    x2 = y2 = 100.0;
    draggingHandle = -1;
    SetBackgroundStyle(wxBG_STYLE_SYSTEM);
}
SubBufferPanel::~SubBufferPanel()
{
}
void SubBufferPanel::SetDefaults() {
    x1 = y1 = 0.0;
    x2 = y2 = 100.0;
    Refresh();
}

void SubBufferPanel::SetValue(const std::string &val) {
    wxArrayString v = wxSplit(val, 'x');

    x1 = v.size() > 0 ? wxAtof(v[0]) : 0.0;
    y1 = v.size() > 1 ? wxAtof(v[1]) : 0.0;
    x2 = v.size() > 2 ? wxAtof(v[2]) : 100.0;
    y2 = v.size() > 3 ? wxAtof(v[3]) : 100.0;
    
    if (x1 > x2) std::swap(x1, x2);
    if (y1 > y2) std::swap(y1, y2);

    Refresh();
}
std::string SubBufferPanel::GetValue() {
    if (std::abs(x1) > 0.001
        || std::abs(y1) > 0.001
        || std::abs(100.0 - x2) > 0.001
        || std::abs(100.0 - y2) > 0.001) {
        return wxString::Format("%.2fx%.2fx%.2fx%.2f", x1, y1, x2, y2).ToStdString();
    }
    return "";
}
inline bool IsWithin(int mx, int my, int x, int y) {
    return mx >= (x - 2) && mx <= (x + 2)
    && my >= (y - 2) && my <= (y + 2);
}
void SubBufferPanel::Convert(float &x, float &y, wxMouseEvent& event) {
    wxSize size = GetSize();
    float startX = size.GetWidth()/10.0;
    float startY = size.GetHeight() / 10.0;
    float bw = size.GetWidth()*0.8;
    float bh = size.GetHeight()*0.8;
    
    x = (event.GetX() - startX) * 100.0/ bw ;
    y = 100.0 - (event.GetY() - startY) * 100.0/ bh ;
}
void SubBufferPanel::ContextMenu(wxContextMenuEvent& event) {
    wxMenu *menu = new wxMenu();
    menu->Append(wxNewId(), "Full Buffer");
    menu->AppendSeparator();
    menu->Append(wxNewId(), "Top Half");
    menu->Append(wxNewId(), "Bottom Half");
    menu->Append(wxNewId(), "Left Half");
    menu->Append(wxNewId(), "Right Half");
    menu->AppendSeparator();
    menu->Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&SubBufferPanel::MenuItemSelected, NULL, this);
    PopupMenu(menu);
    delete menu;
    
}
void SubBufferPanel::MenuItemSelected(wxCommandEvent &event) {
    wxMenu *m = (wxMenu*)event.GetEventObject();
    wxMenuItem *item = m->FindItem(event.GetId());
    if (item != nullptr) {
        wxString nm = item->GetItemLabel();
        if (nm == "Full Buffer") {
            x1 = y1 = 0.0;
            x2 = y2 = 100.0;
        } else if (nm == "Top Half") {
            x1 = 0.0;
            y1 = 50.0;
            x2 = y2 = 100.0;
        } else if (nm == "Bottom Half") {
            x1 = y1 = 0.0;
            y2 = 50.0;
            x2 = 100.0;
        } else if (nm == "Left Half") {
            x1 = y1 = 0.0;
            y2 = 100.0;
            x2 = 50.0;
        } else if (nm == "Right Half") {
            x1 = 50.0;
            y1 = 0.0;
            x2 = y2 = 100.0;
        }
        Refresh();
    }
}


int SubBufferPanel::OverMouseHandle(wxMouseEvent& event) {
    wxSize size = GetSize();
    float startX = size.GetWidth()/10.0;
    float startY = size.GetHeight() / 10.0;
    float bw = size.GetWidth()*0.8;
    float bh = size.GetHeight()*0.8;
    
    float x1b = (x1 * bw)/100.0 + startX;
    float x2b = (x2 * bw)/100.0 + startX;
    float y1b = ((100.0-y1) * bh)/100.0 + startY;
    float y2b = ((100.0-y2) * bh)/100.0 + startY;
    
    if (IsWithin(event.GetX(), event.GetY(), x1b, y1b)) {
        return 0;
    } else if (IsWithin(event.GetX(), event.GetY(), x1b, y2b)) {
        return 1;
    } else if (IsWithin(event.GetX(), event.GetY(), x2b, y2b)) {
        return 2;
    } else if (IsWithin(event.GetX(), event.GetY(), x2b, y1b)) {
        return 3;
    }
    return -1;
}

void SubBufferPanel::mouseDblClick( wxMouseEvent& event) {
    SetDefaults();
}

void SubBufferPanel::mouseLeftDown( wxMouseEvent& event) {
    if (!IsEnabled()) {
        return;
    }
    if (draggingHandle == -1) {
        draggingHandle = OverMouseHandle(event);
    }
    if (draggingHandle != -1) {
        CaptureMouse();
    }
}
void SubBufferPanel::mouseLeftUp( wxMouseEvent& event) {
    if (draggingHandle >= 0) {
        mouseMoved(event);
        ReleaseMouse();
        draggingHandle = -1;
        Refresh();
    }
    if (!IsEnabled()) {
        return;
    }
}

void SubBufferPanel::mouseMoved( wxMouseEvent& event) {
    if (!IsEnabled()) {
        return;
    }
    switch (draggingHandle) {
        case 0:
            Convert(x1, y1, event);
            break;
        case 1:
            Convert(x1, y2, event);
            break;
        case 2:
            Convert(x2, y2, event);
            break;
        case 3:
            Convert(x2, y1, event);
            break;
        default: {
            int i = OverMouseHandle(event);
            if (i > -1) {
                SetCursor(wxCURSOR_SIZING);
            } else {
                SetCursor(wxCURSOR_DEFAULT);
            }
        }
    }
    if (draggingHandle >= 0) {
        if (x2 < x1) std::swap(x1, x2);
        if (y2 < y1) std::swap(y1, y2);
        Refresh();
    }
}

void SubBufferPanel::Paint( wxPaintEvent& event ) {
    wxPaintDC dc(this);
    if (!IsShownOnScreen()) {
        return;
    }
    
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    wxSize size = GetSize();
    dc.DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());
    if (IsEnabled()) {
        dc.SetBrush(*wxBLACK_BRUSH);
        dc.SetPen(*wxBLACK_PEN);
    } else {
        dc.SetBrush(*wxGREY_BRUSH);
        dc.SetPen(*wxGREY_PEN);
    }
    float startX = size.GetWidth()/10.0;
    float startY = size.GetHeight() / 10.0;
    float bw = size.GetWidth()*0.8;
    float bh = size.GetHeight()*0.8;
    dc.DrawRectangle(startX, startY, bw, bh);
    
    if (!IsEnabled()) {
        return;
    }
    dc.SetBrush(*wxYELLOW_BRUSH);
    dc.SetPen(*wxYELLOW_PEN);
    
    float x1b = (x1 * bw)/100.0 + startX;
    float x2b = (x2 * bw)/100.0 + startX;
    
    float y1b = ((100.0-y1) * bh)/100.0 + startY;
    float y2b = ((100.0-y2) * bh)/100.0 + startY;
    
    dc.DrawCircle(x1b, y1b, 2);
    dc.DrawCircle(x1b, y2b, 2);
    dc.DrawCircle(x2b, y2b, 2);
    dc.DrawCircle(x2b, y1b, 2);
    
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(wxPen(*wxYELLOW, 1, wxPENSTYLE_DOT));
    dc.DrawRectangle(x1b, y1b, x2b-x1b,y2b-y1b);
    
    dc.SetTextForeground(*wxYELLOW);
    dc.SetFont(wxFont(wxSize(0, 8), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    wxString str = wxString::Format("%dx%d", (int)x2, (int)y2);
    wxSize sz = dc.GetTextExtent(str);
    dc.DrawText(wxString::Format("%dx%d", (int)x1, (int)y1), x1b + 2, y1b - sz.GetHeight() - 1);
    dc.DrawText(wxString::Format("%dx%d", (int)x1, (int)y2), x1b + 2, y2b);
    
    dc.DrawText(str, x2b - 3 - sz.GetWidth(), y2b);
    str = wxString::Format("%dx%d", (int)x2, (int)y1);
    sz = dc.GetTextExtent(str);
    dc.DrawText(str, x2b - 3 - sz.GetWidth(), y1b - sz.GetHeight() - 1);
}
