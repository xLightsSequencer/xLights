#include "RowHeading.h"
#include "Waveform.h"
#include "wx/wx.h"
#include "wx/brush.h"

BEGIN_EVENT_TABLE(RowHeading, wxWindow)
//EVT_MOTION(RowHeading::mouseMoved)
//EVT_LEFT_DOWN(RowHeading::mouseLeftDown)
//EVT_LEFT_UP(RowHeading::mouseLeftUp)
//EVT_LEAVE_WINDOW(RowHeading::mouseLeftWindow)
//EVT_RIGHT_DOWN(RowHeading::rightClick)
//EVT_SIZE(RowHeading::resized)
//EVT_KEY_DOWN(RowHeading::keyPressed)
//EVT_KEY_UP(RowHeading::keyReleased)
//EVT_MOUSEWHEEL(RowHeading::mouseWheelMoved)
EVT_PAINT(RowHeading::render)
END_EVENT_TABLE()


RowHeading::RowHeading(wxWindow* parent) :
    wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
{
    SetDoubleBuffered(true);
    mHeaderColorModel = new wxColour(212,208,200);
    mHeaderColorView = new wxColour(159,157,152);
    mHeaderColorTiming = new wxColour(130,178,207);
}

RowHeading::~RowHeading()
{
}

void RowHeading::SetCanvasSize(int width,int height)
{
    SetSize(width,height);
    wxSize s;
    s.SetWidth(width);
    s.SetHeight(height);
    SetMaxSize(s);
    SetMinSize(s);
}


void RowHeading::render( wxPaintEvent& event )
{
    wxCoord w,h;
    float t;
    int labelCount=0;
    wxPaintDC dc(this);
    wxPen penOutline(wxColor(128,128,128), .1);
    dc.GetSize(&w,&h);
    wxBrush brush(*mHeaderColorModel,wxBRUSHSTYLE_SOLID);
    dc.SetBrush(brush);
    int row=0;
    int startY,endY;

    for(wxXmlNode*e=mElements->GetChildren(); e!=NULL; e=e->GetNext())
    {
        if (e->GetName() == "Element" && e->GetAttribute("visible")=="1")
        {
            wxBrush brush(*GetHeaderColor(e->GetAttribute("type")),wxBRUSHSTYLE_SOLID);
            dc.SetBrush(brush);
            startY = DEFAULT_ROW_HEADING_HEIGHT*row;
            endY = DEFAULT_ROW_HEADING_HEIGHT*(row+1);
            dc.DrawRectangle(0,startY,w,endY);
            wxRect r(DEFAULT_ROW_HEADING_MARGIN,startY,w-DEFAULT_ROW_HEADING_MARGIN,22);
            dc.DrawLabel(e->GetAttribute("name"),r,wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT);
            if(e->GetAttribute("type")=="view")
            {
                dc.SetPen(*wxBLACK_PEN);
                dc.DrawRectangle(2,startY+6,11,11);
                dc.DrawLine(4,startY+11,11,startY+11);
                dc.DrawLine(7,startY+8,7,startY+14);
                dc.SetPen(penOutline);
            }
            row++;
        }
    }
    dc.DrawRectangle(0,endY,w,h);

}

wxColour* RowHeading::GetHeaderColor(wxString headerType)
{
    if (headerType == "model")
    {
        return mHeaderColorModel;
    }
    else if (headerType == "view")
    {
        return mHeaderColorView;
    }
    else
    {
        return mHeaderColorTiming;
    }
}

void RowHeading::SetElements(wxXmlNode* displayElements)
{
    mElements = displayElements;
}

void RowHeading::DrawHeading(wxPaintDC* dc, wxXmlNode* model,int width,int row)
{
}







