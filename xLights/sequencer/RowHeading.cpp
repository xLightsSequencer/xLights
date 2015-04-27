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
EVT_RIGHT_DOWN(RowHeading::rightClick)
//EVT_KEY_DOWN(RowHeading::keyPressed)
//EVT_KEY_UP(RowHeading::keyReleased)
//EVT_MOUSEWHEEL(RowHeading::mouseWheelMoved)
EVT_PAINT(RowHeading::render)
END_EVENT_TABLE()

// Menu constants
const long RowHeading::ID_ROW_MNU_ADD_LAYER = wxNewId();
const long RowHeading::ID_ROW_MNU_DELETE_LAYER = wxNewId();
const long RowHeading::ID_ROW_MNU_LAYER = wxNewId();
const long RowHeading::ID_ROW_MNU_PLAY_MODEL = wxNewId();
const long RowHeading::ID_ROW_MNU_EXPORT_MODEL = wxNewId();
const long RowHeading::ID_ROW_MNU_EDIT_DISPLAY_ELEMENTS = wxNewId();
const long RowHeading::ID_ROW_MNU_TOGGLE_STRANDS = wxNewId();
const long RowHeading::ID_ROW_MNU_TOGGLE_NODES = wxNewId();

// Timing Track popup menu
const long RowHeading::ID_ROW_MNU_ADD_TIMING_TRACK = wxNewId();
const long RowHeading::ID_ROW_MNU_DELETE_TIMING_TRACK = wxNewId();
const long RowHeading::ID_ROW_MNU_IMPORT_TIMING_TRACK = wxNewId();


int DEFAULT_ROW_HEADING_HEIGHT = 22;

RowHeading::RowHeading(MainSequencer* parent, wxWindowID id, const wxPoint &pos, const wxSize &size,
                       long style, const wxString &name):
                       wxWindow((wxWindow*)parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
{
    DOUBLE_BUFFER(this);
    mHeaderColorModel = new xlColor(212,208,200);
    mHeaderColorView = new xlColor(159,157,152);
    mHeaderColorTiming = new xlColor(130,178,207);
    mHeaderSelectedColor = new xlColor(130,178,207);
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
        Element* e = mSequenceElements->GetRowInformation(rowIndex)->element;
        if(e->GetType()=="model")
        {
            mSequenceElements->UnSelectAllElements();
            e->SetSelected(true);
            wxCommandEvent playEvent(EVT_MODEL_SELECTED);  // send model selection in case we need to switch playback to this model
            playEvent.SetString(e->GetName());
            wxPostEvent(GetParent(), playEvent);
            Refresh(false);
        }
        if(HitTestCollapseExpand(rowIndex,event.GetX(),&result))
        {
            e->SetCollapsed(!result);
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        }
        else if(HitTestTimingActive(rowIndex,event.GetX(),&result))
        {
            mSequenceElements->DeactivateAllTimingElements();
            e->SetActive(!result);
            // Set the selected timing row.
            int selectedTimingRow = result?rowIndex:-1;
            mSequenceElements->SetSelectedTimingRow(selectedTimingRow);
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        }
    }
}

void RowHeading::rightClick( wxMouseEvent& event)
{
    wxMenu *mnuLayer;
    mSelectedRow = event.GetY()/DEFAULT_ROW_HEADING_HEIGHT;
    if (mSelectedRow >= mSequenceElements->GetRowInformationSize()) {
        return;
    }

    Row_Information_Struct *ri =  mSequenceElements->GetRowInformation(mSelectedRow);
    Element* element = ri->element;
    if (element->GetType()=="model" || element->GetType()=="view")
    {
        mnuLayer = new wxMenu();
        if (ri->strandIndex < 0) {
            mnuLayer->Append(ID_ROW_MNU_ADD_LAYER,"Add Layer");
            if(element->GetEffectLayerCount() > 1)
            {
                mnuLayer->Append(ID_ROW_MNU_DELETE_LAYER,"Delete Layer");
            }
            if (element->GetType()=="model") {
                mnuLayer->AppendSeparator();
            }
        }
        if (element->GetType()=="model") {
            mnuLayer->Append(ID_ROW_MNU_PLAY_MODEL,"Play Model");
            mnuLayer->Append(ID_ROW_MNU_EXPORT_MODEL,"Export Model");
            mnuLayer->AppendSeparator();
            if (element->getStrandLayerCount() == 1) {
                mnuLayer->Append(ID_ROW_MNU_TOGGLE_NODES,"Toggle Nodes");
            } else if (element->getStrandLayerCount() > 1) {
                mnuLayer->Append(ID_ROW_MNU_TOGGLE_STRANDS,"Toggle Strands");
                if (ri->strandIndex >= 0) {
                    mnuLayer->Append(ID_ROW_MNU_TOGGLE_NODES,"Toggle Nodes");
                }
            }
        }
    }
    else
    {
        mnuLayer = new wxMenu();
        mnuLayer->Append(ID_ROW_MNU_ADD_TIMING_TRACK,"Add Timing Track");
        mnuLayer->Append(ID_ROW_MNU_DELETE_TIMING_TRACK,"Delete Timing Track");
        mnuLayer->Append(ID_ROW_MNU_IMPORT_TIMING_TRACK,"Import Timing Track");
    }

    mnuLayer->AppendSeparator();
    mnuLayer->Append(ID_ROW_MNU_EDIT_DISPLAY_ELEMENTS,"Edit Display Elements");
    mnuLayer->Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&RowHeading::OnLayerPopup, NULL, this);
    PopupMenu(mnuLayer);
}

void RowHeading::OnLayerPopup(wxCommandEvent& event)
{
    Element* element = mSequenceElements->GetRowInformation(mSelectedRow)->element;
    int id = event.GetId();
    if(id == ID_ROW_MNU_ADD_LAYER)
    {
        element->AddEffectLayer();
        wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
        wxPostEvent(GetParent(), eventRowHeaderChanged);
    }
    else if(id == ID_ROW_MNU_DELETE_LAYER)
    {
        int layerIndex = mSequenceElements->GetRowInformation(mSelectedRow)->layerIndex;
        wxString prompt = wxString::Format("Delete 'Layer %d' of '%s'?",
                                      layerIndex+1,element->GetName());
        wxString caption = "Comfirm Layer Deletion";

        int answer = wxMessageBox(prompt,caption,wxYES_NO);
        if(answer == wxYES)
        {
            element->RemoveEffectLayer(layerIndex);
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        }
    }
    else if(id == ID_ROW_MNU_ADD_TIMING_TRACK)
    {
        wxString name = wxGetTextFromUser("What is name of new timing track?", "Timing Track Name");
        if(name.size()>0)
        {
            // Deactivate active timing mark so new one is selected;
            mSequenceElements->DeactivateAllTimingElements();
            int timingCount = mSequenceElements->GetNumberOfTimingRows();
            wxString type = "timing";
            Element* e = mSequenceElements->AddElement(timingCount,name,type,true,false,true,false);
            e->AddEffectLayer();
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        }
    }
    else if(id == ID_ROW_MNU_DELETE_TIMING_TRACK)
    {
        wxString prompt = wxString::Format("Delete 'Timing Track '%s'?",element->GetName());
        wxString caption = "Comfirm Timing Track Deletion";

        int answer = wxMessageBox(prompt,caption,wxYES_NO);
        if(answer == wxYES)
        {
            mSequenceElements->DeleteElement(element->GetName());
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        }
    } else if(id == ID_ROW_MNU_IMPORT_TIMING_TRACK) {
        wxCommandEvent playEvent(EVT_IMPORT_TIMING);
        wxPostEvent(GetParent(), playEvent);
    } else if (id == ID_ROW_MNU_EXPORT_MODEL) {
        wxCommandEvent playEvent(EVT_EXPORT_MODEL);
        playEvent.SetString(element->GetName());
        wxPostEvent(GetParent(), playEvent);
    } else if (id == ID_ROW_MNU_PLAY_MODEL) {
        wxCommandEvent playEvent(EVT_PLAY_MODEL);
        playEvent.SetString(element->GetName());
        wxPostEvent(GetParent(), playEvent);
    } else if(id==ID_ROW_MNU_EDIT_DISPLAY_ELEMENTS) {
        wxCommandEvent displayElementEvent(EVT_SHOW_DISPLAY_ELEMENTS);
        wxPostEvent(GetParent(), displayElementEvent);
    } else if (id == ID_ROW_MNU_TOGGLE_STRANDS) {
        element->ShowStrands(!element->ShowStrands());
        wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
        wxPostEvent(GetParent(), eventRowHeaderChanged);
    } else if (id == ID_ROW_MNU_TOGGLE_NODES) {
        if (mSequenceElements->GetRowInformation(mSelectedRow)->strandIndex == -1) {
            element->GetStrandLayer(0)->ShowNodes(true);
            element->ShowStrands(!element->ShowStrands());
        } else {
            ((StrandLayer*)mSequenceElements->GetEffectLayer(mSelectedRow))->ShowNodes(!((StrandLayer*)mSequenceElements->GetEffectLayer(mSelectedRow))->ShowNodes());
        }
        wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
        wxPostEvent(GetParent(), eventRowHeaderChanged);
    }

    // Make sure message box is painted over by grid.
    wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
    wxPostEvent(GetParent(), eventForceRefresh);
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

int RowHeading::GetMaxRows()
{
    int max = (int)(getHeight()/DEFAULT_ROW_HEADING_HEIGHT);
    return max;
}

void RowHeading::render( wxPaintEvent& event )
{
    if(!IsShownOnScreen()) return;
    wxCoord w,h;
    wxPaintDC dc(this);
    wxPen penOutline(wxColor(32,32,32), .1);
    dc.GetSize(&w,&h);
    wxBrush brush(mHeaderColorModel->asWxColor(),wxBRUSHSTYLE_SOLID);
    dc.SetBrush(brush);
    dc.SetPen(penOutline);
    int row=0;
    int startY = 0,endY = 0;
    for(int i =0;i< mSequenceElements->GetRowInformationSize();i++)
    {
        wxBrush brush(GetHeaderColor(mSequenceElements->GetRowInformation(i))->asWxColor(),wxBRUSHSTYLE_SOLID);
        dc.SetBrush(brush);
        startY = DEFAULT_ROW_HEADING_HEIGHT*row;
        endY = DEFAULT_ROW_HEADING_HEIGHT*(row+1);
        dc.SetBackgroundMode(wxTRANSPARENT);
        dc.DrawRectangle(0,startY,w,DEFAULT_ROW_HEADING_HEIGHT);
        if(mSequenceElements->GetRowInformation(i)->layerIndex>0
           || mSequenceElements->GetRowInformation(i)->strandIndex >= 0)   // If effect layer = 0
        {
            dc.SetPen(*wxLIGHT_GREY_PEN);
            dc.DrawLine(1,startY,w-1,startY);
            dc.DrawLine(1,startY-1,w-1,startY-1);
            dc.SetPen(*wxBLACK_PEN);
            if (mSequenceElements->GetRowInformation(i)->strandIndex >= 0) {
                wxRect r(DEFAULT_ROW_HEADING_MARGIN,startY,w-DEFAULT_ROW_HEADING_MARGIN,22);
                wxString name = mSequenceElements->GetRowInformation(i)->displayName;
                if (name == "") {
                    if (mSequenceElements->GetRowInformation(i)->nodeIndex >= 0) {
                        name = wxString::Format("Node %d", mSequenceElements->GetRowInformation(i)->nodeIndex + 1);
                    } else {
                        name = wxString::Format("Strand %d", mSequenceElements->GetRowInformation(i)->strandIndex + 1);
                    }
                    
                }
                if (mSequenceElements->GetRowInformation(i)->nodeIndex >= 0) {
                    dc.DrawLabel("     " + name,r,wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT);
                } else {
                    dc.DrawLabel("  " + name,r,wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT);
                }
            }
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
            if(mSequenceElements->GetRowInformation(i)->element->GetEffectLayerCount() > 1 &&
               mSequenceElements->GetRowInformation(i)->layerIndex == 0 &&
               mSequenceElements->GetRowInformation(i)->strandIndex == -1)
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
    wxBrush b(mHeaderColorModel->asWxColor(),wxBRUSHSTYLE_SOLID);
    dc.SetBrush(b);
    dc.DrawRectangle(0,endY,w,h);

}

const xlColor* RowHeading::GetHeaderColor(Row_Information_Struct* info)
{
    if (info->element->GetType() == "model")
    {
        if(info->PartOfView)
        {
            return mHeaderColorView;
        }
        else
        {
            if (info->element->GetSelected())
            {
                return  mHeaderSelectedColor;
            }
            else
            {
                return mHeaderColorModel;
            }
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

const xlColour* RowHeading::GetTimingColor(int colorIndex)
{
    const xlColour* value;
    switch(colorIndex%5)
    {
        case 0:
            //
            value = &xlCYAN;
            break;
        case 1:
            value = &xlRED;
            break;
        case 2:
            value = &xlGREEN;
            break;
        case 3:
            value = &xlBLUE;
            break;
        default:
            value = &xlYELLOW;
            break;
    }
    return value;
}







