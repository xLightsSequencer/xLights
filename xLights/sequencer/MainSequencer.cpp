
#include "MainSequencer.h"
#include "RenderCommandEvent.h"
#include "TimeLine.h"
#include <wx/event.h>

//(*InternalHeaders(MainSequencer)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(MainSequencer)
const long MainSequencer::ID_STATICTEXT_Time = wxNewId();
const long MainSequencer::ID_PANEL5 = wxNewId();
const long MainSequencer::ID_PANEL1 = wxNewId();
const long MainSequencer::ID_PANEL3 = wxNewId();
const long MainSequencer::ID_PANEL6 = wxNewId();
const long MainSequencer::ID_PANEL2 = wxNewId();
const long MainSequencer::ID_SCROLLBAR_EFFECTS_VERTICAL = wxNewId();
const long MainSequencer::ID_SCROLLBAR_EFFECT_GRID_HORZ = wxNewId();
//*)

BEGIN_EVENT_TABLE(MainSequencer,wxPanel)
    EVT_MOUSEWHEEL(MainSequencer::mouseWheelMoved)

END_EVENT_TABLE()


void MainSequencer::SetHandlers(wxWindow *window)
{
    if (window) {
        window->Connect(wxID_ANY,
                        wxEVT_CHAR,
                        wxKeyEventHandler(MainSequencer::OnChar),
                        (wxObject*) NULL,
                        this);
        window->Connect(wxID_ANY,
                        wxEVT_CHAR_HOOK,
                        wxKeyEventHandler(MainSequencer::OnCharHook),
                        (wxObject*) NULL,
                        this);
        window->Connect(wxID_ANY,
                        wxEVT_KEY_DOWN,
                        wxKeyEventHandler(MainSequencer::OnKeyDown),
                        (wxObject*) NULL,
                        this);

        wxWindowListNode* pclNode = window->GetChildren().GetFirst();
        while(pclNode) {
            wxWindow* pclChild = pclNode->GetData();
            SetHandlers(pclChild);
            pclNode = pclNode->GetNext();
        }
    }
}



MainSequencer::MainSequencer(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(MainSequencer)
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxWANTS_CHARS, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(3, 3, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	FlexGridSizer1->AddGrowableRow(1);
	PanelPlayControls = new wxPanel(this, ID_PANEL5, wxDefaultPosition, wxSize(175,100), wxTAB_TRAVERSAL, _T("ID_PANEL5"));
	StaticText_SeqTime = new wxStaticText(PanelPlayControls, ID_STATICTEXT_Time, _("Time:"), wxPoint(80,64), wxDefaultSize, 0, _T("ID_STATICTEXT_Time"));
	FlexGridSizer1->Add(PanelPlayControls, 1, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 0);
	FlexGridSizer4 = new wxFlexGridSizer(2, 0, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	FlexGridSizer4->AddGrowableRow(1);
	PanelTimeLine = new TimeLine(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	PanelTimeLine->SetMinSize(wxSize(-1,25));
	PanelTimeLine->SetMaxSize(wxSize(-1,25));
	FlexGridSizer4->Add(PanelTimeLine, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	PanelWaveForm = new Waveform(this, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
	PanelWaveForm->SetMinSize(wxSize(-1,75));
	PanelWaveForm->SetMaxSize(wxSize(-1,75));
	FlexGridSizer4->Add(PanelWaveForm, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 0);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	PanelRowHeadings = new RowHeading(this, ID_PANEL6, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL6"));
	FlexGridSizer1->Add(PanelRowHeadings, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 0);
	PanelEffectGrid = new EffectsGrid(this, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxFULL_REPAINT_ON_RESIZE, _T("ID_PANEL2"));
	FlexGridSizer1->Add(PanelEffectGrid, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	ScrollBarEffectsVertical = new wxScrollBar(this, ID_SCROLLBAR_EFFECTS_VERTICAL, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL|wxALWAYS_SHOW_SB, wxDefaultValidator, _T("ID_SCROLLBAR_EFFECTS_VERTICAL"));
	ScrollBarEffectsVertical->SetScrollbar(0, 1, 10, 1);
	FlexGridSizer1->Add(ScrollBarEffectsVertical, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ScrollBarEffectsHorizontal = new wxScrollBar(this, ID_SCROLLBAR_EFFECT_GRID_HORZ, wxDefaultPosition, wxDefaultSize, wxSB_HORIZONTAL|wxALWAYS_SHOW_SB, wxDefaultValidator, _T("ID_SCROLLBAR_EFFECT_GRID_HORZ"));
	ScrollBarEffectsHorizontal->SetScrollbar(0, 1, 100, 1);
	FlexGridSizer1->Add(ScrollBarEffectsHorizontal, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_SCROLLBAR_EFFECTS_VERTICAL,wxEVT_SCROLL_TOP|wxEVT_SCROLL_BOTTOM|wxEVT_SCROLL_LINEUP|wxEVT_SCROLL_LINEDOWN|wxEVT_SCROLL_PAGEUP|wxEVT_SCROLL_PAGEDOWN|wxEVT_SCROLL_THUMBTRACK|wxEVT_SCROLL_THUMBRELEASE|wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectsVerticalScrollChanged);
	Connect(ID_SCROLLBAR_EFFECTS_VERTICAL,wxEVT_SCROLL_TOP,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectsVerticalScrollChanged);
	Connect(ID_SCROLLBAR_EFFECTS_VERTICAL,wxEVT_SCROLL_BOTTOM,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectsVerticalScrollChanged);
	Connect(ID_SCROLLBAR_EFFECTS_VERTICAL,wxEVT_SCROLL_LINEUP,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectsVerticalScrollChanged);
	Connect(ID_SCROLLBAR_EFFECTS_VERTICAL,wxEVT_SCROLL_LINEDOWN,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectsVerticalScrollChanged);
	Connect(ID_SCROLLBAR_EFFECTS_VERTICAL,wxEVT_SCROLL_PAGEUP,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectsVerticalScrollChanged);
	Connect(ID_SCROLLBAR_EFFECTS_VERTICAL,wxEVT_SCROLL_PAGEDOWN,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectsVerticalScrollChanged);
	Connect(ID_SCROLLBAR_EFFECTS_VERTICAL,wxEVT_SCROLL_THUMBTRACK,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectsVerticalScrollChanged);
	Connect(ID_SCROLLBAR_EFFECTS_VERTICAL,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectsVerticalScrollChanged);
	Connect(ID_SCROLLBAR_EFFECT_GRID_HORZ,wxEVT_SCROLL_TOP|wxEVT_SCROLL_BOTTOM|wxEVT_SCROLL_LINEUP|wxEVT_SCROLL_LINEDOWN|wxEVT_SCROLL_PAGEUP|wxEVT_SCROLL_PAGEDOWN|wxEVT_SCROLL_THUMBTRACK|wxEVT_SCROLL_THUMBRELEASE|wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectGridHorzScroll);
	Connect(ID_SCROLLBAR_EFFECT_GRID_HORZ,wxEVT_SCROLL_TOP,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectGridHorzScroll);
	Connect(ID_SCROLLBAR_EFFECT_GRID_HORZ,wxEVT_SCROLL_BOTTOM,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectGridHorzScroll);
	Connect(ID_SCROLLBAR_EFFECT_GRID_HORZ,wxEVT_SCROLL_LINEUP,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectGridHorzScroll);
	Connect(ID_SCROLLBAR_EFFECT_GRID_HORZ,wxEVT_SCROLL_LINEDOWN,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectGridHorzScroll);
	Connect(ID_SCROLLBAR_EFFECT_GRID_HORZ,wxEVT_SCROLL_PAGEUP,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectGridHorzScroll);
	Connect(ID_SCROLLBAR_EFFECT_GRID_HORZ,wxEVT_SCROLL_PAGEDOWN,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectGridHorzScroll);
	Connect(ID_SCROLLBAR_EFFECT_GRID_HORZ,wxEVT_SCROLL_THUMBTRACK,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectGridHorzScroll);
	Connect(ID_SCROLLBAR_EFFECT_GRID_HORZ,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectGridHorzScroll);
	//*)

    mParent = parent;
    SetHandlers(this);
}

MainSequencer::~MainSequencer()
{
	//(*Destroy(MainSequencer)
	//*)
}

void MainSequencer::SetSequenceElements(SequenceElements* elements)
{
    mSequenceElements = elements;
}

void MainSequencer::UpdateEffectGridVerticalScrollBar()
{
    int position = mSequenceElements->GetFirstVisibleModelRow();
    int range = mSequenceElements->GetTotalNumberOfModelRows();
    int pageSize = mSequenceElements->GetMaxModelsDisplayed();
    int thumbSize = mSequenceElements->GetMaxModelsDisplayed();
    ScrollBarEffectsVertical->SetScrollbar(position,thumbSize,range,pageSize);
    ScrollBarEffectsVertical->Refresh();
}

void MainSequencer::UpdateTimeDisplay(int time_ms)
{
    int time = time_ms >= 0 ? time_ms : 0;
    int msec=time % 1000;
    int seconds=time / 1000;
    int minutes=seconds / 60;
    seconds=seconds % 60;
    wxString play_time = wxString::Format("Time: %d:%02d.%02d",minutes,seconds,msec);
    StaticText_SeqTime->SetLabel(play_time);
}

void MainSequencer::OnPanelWaveFormPaint(wxPaintEvent& event)
{
}


void MainSequencer::OnScrollBarEffectGridHorzScroll(wxScrollEvent& event)
{
    wxCommandEvent eventScroll(EVT_HORIZ_SCROLL);
    wxPostEvent(mParent, eventScroll);
}

void MainSequencer::OnScrollBarEffectsVerticalScrollChanged(wxScrollEvent& event)
{
    int position = ScrollBarEffectsVertical->GetThumbPosition();
    mSequenceElements->SetFirstVisibleModelRow(position);
    Refresh();
}

void MainSequencer::mouseWheelMoved(wxMouseEvent& event)
{
    int i = event.GetWheelRotation();
    if (event.GetWheelAxis() == wxMOUSE_WHEEL_HORIZONTAL) {
        int position = ScrollBarEffectsHorizontal->GetThumbPosition();
        int ts = ScrollBarEffectsHorizontal->GetThumbSize() / 10;
        if (ts ==0) {
            ts = 1;
        }
        if (i < 0) {
            if (position > 0) {
                position -= ts;
            }
        } else {
            position += ts;
            if (position >= ScrollBarEffectsHorizontal->GetRange()) {
                position = ScrollBarEffectsHorizontal->GetRange() - 1;
            }
        }
        ScrollBarEffectsHorizontal->SetThumbPosition(position);
        wxCommandEvent eventScroll(EVT_HORIZ_SCROLL);
        wxPostEvent(mParent, eventScroll);
    } else {
        int position = ScrollBarEffectsVertical->GetThumbPosition();
        if(i<0)
        {
            if(position < ScrollBarEffectsVertical->GetRange()-1)
            {
                position++;
                ScrollBarEffectsVertical->SetThumbPosition(position);
                mSequenceElements->SetFirstVisibleModelRow(position);
            }
        }
        else
        {
            if(position > 0)
            {
                position--;
                ScrollBarEffectsVertical->SetThumbPosition(position);
                mSequenceElements->SetFirstVisibleModelRow(position);
            }
        }
        mSequenceElements->PopulateVisibleRowInformation();
        PanelEffectGrid->Refresh();
        PanelRowHeadings->Refresh();
    }
}

void MainSequencer::OnCharHook(wxKeyEvent& event)
{
    wxChar uc = event.GetKeyCode();
    //printf("OnCharHook %d   %c\n", uc, uc);
    switch(uc)
    {
        case WXK_BACK:
        case WXK_DELETE:
            DeleteAllSelectedEffects();
            event.StopPropagation();
            break;
        case WXK_SPACE:
            {
                wxCommandEvent playEvent(EVT_TOGGLE_PLAY);
                wxPostEvent(mParent, playEvent);
            }
            event.StopPropagation();
            break;
        default:
            event.Skip();
            break;
    }
}
void MainSequencer::OnKeyDown(wxKeyEvent& event)
{
    //wxChar uc = event.GetUnicodeKey();
    //printf("OnKeyDown %d   %c\n", uc, uc);
    event.Skip();
}
void MainSequencer::OnChar(wxKeyEvent& event)
{
    wxChar uc = event.GetUnicodeKey();
    //printf("OnChar %d   %c\n", uc, uc);
    switch(uc)
    {
        case '+':
            PanelTimeLine->ZoomIn();
            event.StopPropagation();
            break;
        case '-':
            PanelTimeLine->ZoomOut();
            event.StopPropagation();
            break;
            /*
        case WXK_F5:
        {
            wxCommandEvent eventEffectUpdated(EVT_EFFECT_UPDATED);
            wxPostEvent(this, eventEffectUpdated);
            break;
        }
             */
        case 't':
        case 'T':
            InsertTimingMarkFromRange();
            event.StopPropagation();
            break;
    }
}
void MainSequencer::DeleteAllSelectedEffects()
{
    for(int i=0;i<mSequenceElements->GetRowInformationSize();i++)
    {
        Element* element = mSequenceElements->GetRowInformation(i)->element;
        EffectLayer* el = element->GetEffectLayer(mSequenceElements->GetRowInformation(i)->layerIndex);
        double start = 99999999;
        double end = -1;
        for (int x = 0; x < el->GetEffectCount(); x++) {
            Effect *ef = el->GetEffect(x);
            if (ef->GetSelected() != EFFECT_NOT_SELECTED) {
                if (ef->GetStartTime() < start) {
                    start = ef->GetStartTime();
                }
                if (ef->GetEndTime() > end) {
                    end = ef->GetEndTime();
                }
            }
        }
        if (end > 0) {
            RenderCommandEvent event(element->GetName(), start, end, true, true);
            wxPostEvent(mParent, event);
            el->DeleteSelectedEffects();
        }
    }
    PanelEffectGrid->ForceRefresh();

}

void MainSequencer::InsertTimingMarkFromRange()
{
    int x1 = PanelTimeLine->GetSelectedPositionStart();
    int x2 = PanelTimeLine->GetSelectedPositionEnd();
    if( x2 == -1 ) x2 = x1;
    int selectedTiming = mSequenceElements->GetSelectedTimingRow();
    if(selectedTiming >= 0)
    {
        if(x1!=x2)
        {
            //Force x1 the smaller of two positions
            int tmp = x1;
            x1=x1<x2?x1:x2;
            x2=tmp<x2?x2:tmp;
            Element* e = mSequenceElements->GetRowInformation(selectedTiming)->element;
            EffectLayer* el = e->GetEffectLayer(mSequenceElements->GetRowInformation(selectedTiming)->layerIndex);
            int index,result;
            if(!el->HitTestEffect(x1,index,result) && !el->HitTestEffect(x2,index,result))
            {
                double t1 = PanelTimeLine->GetAbsoluteTimefromPosition(x1);
                double t2 = PanelTimeLine->GetAbsoluteTimefromPosition(x2);
                wxString name,settings;
                el->AddEffect(0,0,name,settings,-1,t1,t2,false,false);
                PanelEffectGrid->ForceRefresh();
            }
            else
            {
                wxMessageBox("Timing exist already in the selected region","Timing placement error");
            }
        }
        else
        {
            // x1 and x2 are the same. Insert from end time of timing to the left to x2
            Element* e = mSequenceElements->GetRowInformation(selectedTiming)->element;
            EffectLayer* el = e->GetEffectLayer(mSequenceElements->GetRowInformation(selectedTiming)->layerIndex);
            int index,result;
            if(!el->HitTestEffect(x2,index,result))
            {
                // if there is an effect to left
                wxString name,settings;
                Effect* effect = el->GetEffectBeforePosition(x2);
                if(effect!=nullptr)
                {
                    double t1 = PanelTimeLine->GetAbsoluteTimefromPosition(effect->GetEndPosition());
                    double t2 = PanelTimeLine->GetAbsoluteTimefromPosition(x2);
                    el->AddEffect(0,0,name,settings,-1,t1,t2,false,false);
                }
                // No effect to left start at time = 0
                else
                {
                    double t1 = 0;
                    double t2 = PanelTimeLine->GetAbsoluteTimefromPosition(x2);
                    el->AddEffect(0,0,name,settings,-1,t1,t2,false,false);
                }
                PanelEffectGrid->ForceRefresh();
            }
            else
            {
                wxMessageBox("Timing exist already in the selected region","Timing placement error");
            }
        }
    }
}


