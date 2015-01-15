#include "RowHeading.h"
#include "Waveform.h"
#include "wx/wx.h"
#include "wx/brush.h"
#include "../xLightsMain.h"
#include "EffectDropTarget.h"


BEGIN_EVENT_TABLE(RowHeading, wxWindow)
//EVT_MOTION(RowHeading::mouseMoved)
EVT_LEFT_DOWN(RowHeading::mouseLeftDown)
//EVT_LEFT_UP(RowHeading::mouseLeftUp)
//EVT_LEAVE_WINDOW(RowHeading::mouseLeftWindow)
//EVT_RIGHT_DOWN(RowHeading::rightClick)
//EVT_SIZE(RowHeading::resized)
//EVT_KEY_DOWN(RowHeading::keyPressed)
//EVT_KEY_UP(RowHeading::keyReleased)
//EVT_MOUSEWHEEL(RowHeading::mouseWheelMoved)
EVT_PAINT(RowHeading::render)
END_EVENT_TABLE()


RowHeading::RowHeading(wxScrolledWindow* parent, wxWindowID id, const wxPoint &pos, const wxSize &size,
                       long style, const wxString &name):
                       wxWindow((wxWindow*)parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
{
    DOUBLE_BUFFER(this);
    mHeaderColorModel = new wxColour(212,208,200);
    mHeaderColorView = new wxColour(159,157,152);
    mHeaderColorTiming = new wxColour(130,178,207);
    SetDropTarget(new EffectDropTarget((wxWindow*)this,false));

}

RowHeading::~RowHeading()
{
}

void RowHeading::mouseLeftDown( wxMouseEvent& event)
{
    int rowIndex = event.GetY()/DEFAULT_ROW_HEADING_HEIGHT;
    if(rowIndex < mSequenceElements->GetRowInformationSize())
    {
        bool result;
        if(HitTestCollapseExpand(rowIndex,event.GetX(),&result))
        {
            Element* e = mSequenceElements->GetRowInformation(rowIndex)->element;
            e->SetCollapsed(!result);
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        }
        else if(HitTestTimingActive(rowIndex,event.GetX(),&result))
        {
            mSequenceElements->DeactivateAllTimingElements();
            Element* e = mSequenceElements->GetRowInformation(rowIndex)->element;
            e->SetActive(!result);
            // Set the selected timing row.
            int selectedTimingRow = result?rowIndex:-1;
            mSequenceElements->SetSelectedTimingRow(selectedTimingRow);
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        }
    }
}

bool RowHeading::HitTestCollapseExpand(int row,int x, bool* IsCollapsed)
{
    if(mSequenceElements->GetRowInformation(row)->element->GetType() != "timing" &&
       x<DEFAULT_ROW_HEADING_MARGIN)
    {
        *IsCollapsed = mSequenceElements->GetRowInformation(row)->Collapsed;
        return true;
    }
    else
    {
        return false;
    }
}

bool RowHeading::HitTestTimingActive(int row,int x, bool* IsActive)
{
    if(mSequenceElements->GetRowInformation(row)->element->GetType() == "timing" &&
       x<DEFAULT_ROW_HEADING_MARGIN)
    {
        *IsActive = mSequenceElements->GetRowInformation(row)->Active;
        return true;
    }
    else
    {
        return false;
    }
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
    wxPen penOutline(wxColor(32,32,32), .1);
    dc.GetSize(&w,&h);
    wxBrush brush(*mHeaderColorModel,wxBRUSHSTYLE_SOLID);
    dc.SetBrush(brush);
    dc.SetPen(penOutline);
    int row=0;
    int startY,endY;
    for(int i =0;i< mSequenceElements->GetRowInformationSize();i++)
    {
        wxBrush brush(*GetHeaderColor(mSequenceElements->GetRowInformation(i)),wxBRUSHSTYLE_SOLID);
        dc.SetBrush(brush);
        startY = DEFAULT_ROW_HEADING_HEIGHT*row;
        endY = DEFAULT_ROW_HEADING_HEIGHT*(row+1);
        dc.DrawRectangle(0,startY,w,DEFAULT_ROW_HEADING_HEIGHT);
        if(mSequenceElements->GetRowInformation(i)->layerIndex>0)   // If effect layer = 0
        {
            dc.SetPen(*wxLIGHT_GREY_PEN);
            dc.DrawLine(1,startY,w-1,startY);
            dc.DrawLine(1,startY-1,w-1,startY-1);
            dc.SetPen(*wxBLACK_PEN);
        }
        else        // Draw label
        {
            if(mSequenceElements->GetRowInformation(i)->PartOfView)
            {
                wxRect r(INDENT_ROW_HEADING_MARGIN,startY,w-(INDENT_ROW_HEADING_MARGIN),22);
                dc.DrawLabel(mSequenceElements->GetRowInformation(i)->element->GetName(),r,wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT);
            }
            else
            {
                wxRect r(DEFAULT_ROW_HEADING_MARGIN,startY,w-DEFAULT_ROW_HEADING_MARGIN,22);
                dc.DrawLabel(mSequenceElements->GetRowInformation(i)->element->GetName(),r,wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT);
            }
        }

        if(mSequenceElements->GetRowInformation(i)->element->GetType()=="view")
        {
            dc.SetBrush(*wxWHITE_BRUSH);
            dc.SetPen(*wxBLACK_PEN);
            dc.DrawRectangle(2,startY+7,9,9);
            dc.DrawLine(4,startY+11,9,startY+11);
            if(mSequenceElements->GetRowInformation(i)->Collapsed)
            {
                dc.DrawLine(6,startY+9,6,startY+14);
            }
            dc.SetPen(penOutline);
            dc.SetBrush(brush);
        }
        else if (mSequenceElements->GetRowInformation(i)->element->GetType()=="model")
        {
            if(mSequenceElements->GetRowInformation(i)->element->GetEffectLayerCount()>0 &&
               mSequenceElements->GetRowInformation(i)->layerIndex == 0)
            {
                dc.SetBrush(*wxWHITE_BRUSH);
                dc.SetPen(*wxBLACK_PEN);
                dc.DrawRectangle(2,startY+7,9,9);
                dc.DrawLine(4,startY+11,9,startY+11);
                if(mSequenceElements->GetRowInformation(i)->Collapsed)
                {
                    dc.DrawLine(6,startY+9,6,startY+14);
                }
                dc.SetPen(penOutline);
                dc.SetBrush(brush);
            }
        }
        else if(mSequenceElements->GetRowInformation(i)->element->GetType()=="timing")
        {
            dc.SetPen(*wxBLACK_PEN);
            if(mSequenceElements->GetRowInformation(i)->Active)
            {
                dc.SetBrush(*wxWHITE_BRUSH);
                dc.DrawCircle(7,startY+11,5);

                dc.SetBrush(*wxGREY_BRUSH);
                dc.DrawCircle(7,startY+11,2);
            }
            else
            {
                dc.SetBrush(*wxWHITE_BRUSH);
                dc.DrawCircle(7,startY+11,5);
            }
            dc.SetPen(penOutline);
            dc.SetBrush(brush);
        }
        row++;
    }
    dc.DrawRectangle(0,endY,w,h);

}

const wxColour* RowHeading::GetHeaderColor(Row_Information_Struct* info)
{
    if (info->element->GetType() == "model")
    {
        if(info->PartOfView)
        {
            return mHeaderColorView;
        }
        else
        {
            return mHeaderColorModel;
        }
    }
    else if (info->element->GetType() == "view")
    {
        return mHeaderColorView;
    }
    else
    {
        return GetTimingColor(info->colorIndex);
    }
}

void RowHeading::SetSequenceElements(SequenceElements* elements)
{
    mSequenceElements = elements;
}

void RowHeading::DrawHeading(wxPaintDC* dc, wxXmlNode* model,int width,int row)
{
}

int RowHeading::getWidth()
{
    return GetSize().x;
}

int RowHeading::getHeight()
{
    return GetSize().y;
}

const wxColour* RowHeading::GetTimingColor(int colorIndex)
{
    const wxColour* value;
    switch(colorIndex%5)
    {
        case 0:
            //
            value = wxCYAN;
            break;
        case 1:
            value = wxRED;
            break;
        case 2:
            value = wxGREEN;
            break;
        case 3:
            value = wxBLUE;
            break;
        default:
            value = wxYELLOW;
            break;
    }
    return value;
}





