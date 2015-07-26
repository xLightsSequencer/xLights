#include "RowHeading.h"
#include "Waveform.h"
#include "wx/wx.h"
#include "wx/brush.h"
#include "../xLightsMain.h"
#include "EffectDropTarget.h"
#include "BitmapCache.h"
#include "LyricsDialog.h"
#include "TimeLine.h"

BEGIN_EVENT_TABLE(RowHeading, wxWindow)
EVT_LEFT_DOWN(RowHeading::mouseLeftDown)
EVT_RIGHT_DOWN(RowHeading::rightClick)
EVT_LEFT_DCLICK(RowHeading::leftDoubleClick)
EVT_PAINT(RowHeading::render)
END_EVENT_TABLE()

// Menu constants
const long RowHeading::ID_ROW_MNU_INSERT_LAYER_ABOVE = wxNewId();
const long RowHeading::ID_ROW_MNU_INSERT_LAYER_BELOW = wxNewId();
const long RowHeading::ID_ROW_MNU_DELETE_LAYER = wxNewId();
const long RowHeading::ID_ROW_MNU_LAYER = wxNewId();
const long RowHeading::ID_ROW_MNU_PLAY_MODEL = wxNewId();
const long RowHeading::ID_ROW_MNU_EXPORT_MODEL = wxNewId();
const long RowHeading::ID_ROW_MNU_EDIT_DISPLAY_ELEMENTS = wxNewId();
const long RowHeading::ID_ROW_MNU_TOGGLE_STRANDS = wxNewId();
const long RowHeading::ID_ROW_MNU_TOGGLE_NODES = wxNewId();
const long RowHeading::ID_ROW_MNU_CONVERT_TO_EFFECTS = wxNewId();
const long RowHeading::ID_ROW_MNU_PROMOTE_EFFECTS = wxNewId();

// Timing Track popup menu
const long RowHeading::ID_ROW_MNU_ADD_TIMING_TRACK = wxNewId();
const long RowHeading::ID_ROW_MNU_DELETE_TIMING_TRACK = wxNewId();
const long RowHeading::ID_ROW_MNU_IMPORT_TIMING_TRACK = wxNewId();
const long RowHeading::ID_ROW_MNU_IMPORT_LYRICS = wxNewId();
const long RowHeading::ID_ROW_MNU_BREAKDOWN_TIMING_TRACK = wxNewId();


int DEFAULT_ROW_HEADING_HEIGHT = 22;

RowHeading::RowHeading(MainSequencer* parent, wxWindowID id, const wxPoint &pos, const wxSize &size,
                       long style, const wxString &name):
                       wxWindow((wxWindow*)parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
{
    DOUBLE_BUFFER(this);
    mHeaderColorModel = new xlColor(212,208,200);
    mHeaderColorTiming = new xlColor(130,178,207);
    mHeaderSelectedColor = new xlColor(130,178,207);
    SetDropTarget(new EffectDropTarget((wxWindow*)this,false));
    wxString tooltip;
    papagayo_icon = BitmapCache::GetPapgayoIcon(tooltip, 16, true);
}

RowHeading::~RowHeading()
{
}

void RowHeading::SetSequenceEnd(int ms)
{
    mSequenceEndMS = ms;
}

void RowHeading::mouseLeftDown( wxMouseEvent& event)
{
    mSelectedRow = event.GetY()/DEFAULT_ROW_HEADING_HEIGHT;
    if(mSelectedRow < mSequenceElements->GetVisibleRowInformationSize())
    {
        bool result;
        Element* e = mSequenceElements->GetVisibleRowInformation(mSelectedRow)->element;
        if(e->GetType()=="model")
        {
            mSequenceElements->UnSelectAllElements();
            e->SetSelected(true);
            wxCommandEvent playEvent(EVT_MODEL_SELECTED);  // send model selection in case we need to switch playback to this model
            playEvent.SetString(e->GetName());
            wxPostEvent(GetParent(), playEvent);
            Refresh(false);
        }
        if(HitTestCollapseExpand(mSelectedRow,event.GetX(),&result))
        {
            e->SetCollapsed(!result);
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        }
        else if(HitTestTimingActive(mSelectedRow,event.GetX(),&result))
        {
            mSequenceElements->DeactivateAllTimingElements();
            e->SetActive(!result);
            // Set the selected timing row.
            int selectedTimingRow = result?mSelectedRow:-1;
            mSequenceElements->SetSelectedTimingRow(selectedTimingRow);
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        }
    }
}
void RowHeading::leftDoubleClick(wxMouseEvent& event)
{
    mSelectedRow = event.GetY()/DEFAULT_ROW_HEADING_HEIGHT;
    if (mSelectedRow >= mSequenceElements->GetVisibleRowInformationSize()) {
        return;
    }
    Row_Information_Struct *ri =  mSequenceElements->GetVisibleRowInformation(mSelectedRow);
    Element* element = ri->element;

    if (element->GetType()=="model") {
        if (ri->strandIndex == -1) {
            //dbl click on model
            if (element->getStrandLayerCount() == 1) {
                element->GetStrandLayer(0)->ShowNodes(true);
                element->ShowStrands(!element->ShowStrands());
            } else {
                element->ShowStrands(!element->ShowStrands());
            }
        } else if (ri->nodeIndex >= 0) {
            //dbl click on node
            if (element->getStrandLayerCount() == 1) {
                element->ShowStrands(!element->ShowStrands());
            } else {
                element->GetStrandLayer(ri->strandIndex)->ShowNodes(!element->GetStrandLayer(ri->strandIndex)->ShowNodes());
            }
        } else {
            //dbl click on strand
            element->GetStrandLayer(ri->strandIndex)->ShowNodes(!element->GetStrandLayer(ri->strandIndex)->ShowNodes());
        }
        wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
        eventRowHeaderChanged.SetString(element->GetName());
        wxPostEvent(GetParent(), eventRowHeaderChanged);
    }
    else if(element->GetType()=="timing") {
        if(element->GetEffectLayerCount() > 1) {
            if(element->GetCollapsed()) {
                element->SetCollapsed(false);
            } else {
                element->SetCollapsed(true);
            }
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            eventRowHeaderChanged.SetString(element->GetName());
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        }
    }
}
void RowHeading::rightClick( wxMouseEvent& event)
{
    wxMenu *mnuLayer;
    mSelectedRow = event.GetY()/DEFAULT_ROW_HEADING_HEIGHT;
    if (mSelectedRow >= mSequenceElements->GetVisibleRowInformationSize()) {
        return;
    }

    Row_Information_Struct *ri =  mSequenceElements->GetVisibleRowInformation(mSelectedRow);
    Element* element = ri->element;
    if (element->GetType()=="model" || element->GetType()=="view")
    {
        mnuLayer = new wxMenu();
        if (ri->strandIndex < 0) {
            mnuLayer->Append(ID_ROW_MNU_INSERT_LAYER_ABOVE,"Insert Layer Above");
            mnuLayer->Append(ID_ROW_MNU_INSERT_LAYER_BELOW,"Insert Layer Below");
            //if( ri->Index > 0 )
            {
            }
            //if( ri->Index < element->GetEffectLayerCount()-1 )
            {
            }
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
            bool canPromote = false;
            if (element->getStrandLayerCount() == 1) {
                mnuLayer->Append(ID_ROW_MNU_TOGGLE_NODES,"Toggle Nodes");
                canPromote = true;
            } else if (element->getStrandLayerCount() > 1) {
                canPromote = true;
                mnuLayer->Append(ID_ROW_MNU_TOGGLE_STRANDS,"Toggle Strands");
                if (ri->strandIndex >= 0) {
                    mnuLayer->Append(ID_ROW_MNU_TOGGLE_NODES,"Toggle Nodes");
                }
            } else {
                mnuLayer->Append(ID_ROW_MNU_TOGGLE_STRANDS,"Toggle Models");
            }
            if (ri->nodeIndex > -1 && element->GetStrandLayer(ri->strandIndex)->GetNodeLayer(ri->nodeIndex)->GetEffectCount() == 0) {
                mnuLayer->Append(ID_ROW_MNU_CONVERT_TO_EFFECTS, "Convert To Effect");
            }
            if (canPromote) {
                mnuLayer->Append(ID_ROW_MNU_PROMOTE_EFFECTS, "Promote Node Effects");
            }
        }
    }
    else
    {
        mnuLayer = new wxMenu();
        mnuLayer->Append(ID_ROW_MNU_ADD_TIMING_TRACK,"Add Timing Track");
        mnuLayer->Append(ID_ROW_MNU_DELETE_TIMING_TRACK,"Delete Timing Track");
        mnuLayer->Append(ID_ROW_MNU_IMPORT_TIMING_TRACK,"Import Timing Track");
        mnuLayer->AppendSeparator();
        mnuLayer->Append(ID_ROW_MNU_IMPORT_LYRICS,"Import Lyrics");
        if(element->GetEffectLayerCount() == 1)
        {
            //mnuLayer->Append(ID_ROW_MNU_BREAKDOWN_TIMING_TRACK,"Breakdown Phrases");
        }
    }

    mnuLayer->AppendSeparator();
    mnuLayer->Append(ID_ROW_MNU_EDIT_DISPLAY_ELEMENTS,"Edit Display Elements");
    mnuLayer->Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&RowHeading::OnLayerPopup, NULL, this);
    Draw();
    PopupMenu(mnuLayer);
}

void RowHeading::OnLayerPopup(wxCommandEvent& event)
{
    Row_Information_Struct *ri = mSequenceElements->GetVisibleRowInformation(mSelectedRow);
    Element* element = ri->element;
    int layer_index = ri->layerIndex;
    int id = event.GetId();
    if(id == ID_ROW_MNU_INSERT_LAYER_ABOVE)
    {
        element->InsertEffectLayer(layer_index);
        wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
        wxPostEvent(GetParent(), eventRowHeaderChanged);
    }
    else if(id == ID_ROW_MNU_INSERT_LAYER_BELOW)
    {
        if( layer_index < element->GetEffectLayerCount()-1)
        {
            element->InsertEffectLayer(layer_index+1);
        }
        else
        {
            element->AddEffectLayer();
        }
        wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
        wxPostEvent(GetParent(), eventRowHeaderChanged);
    }
    else if(id == ID_ROW_MNU_DELETE_LAYER)
    {
        int layerIndex = mSequenceElements->GetVisibleRowInformation(mSelectedRow)->layerIndex;
        wxString prompt = wxString::Format("Delete 'Layer %d' of '%s'?",
                                      layerIndex+1,element->GetName());
        wxString caption = "Confirm Layer Deletion";

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
        if( mSequenceElements->ElementExists(name) )
        {
            wxMessageBox("Timing name already exists in sequence as a model or another timing.", "ERROR");
        }
        else if(name.size()>0)
        {
            // Deactivate active timing mark so new one is selected;
            mSequenceElements->DeactivateAllTimingElements();
            int timingCount = mSequenceElements->GetNumberOfTimingRows();
            wxString type = "timing";
            Element* e = mSequenceElements->AddElement(timingCount,name,type,true,false,true,false);
            e->AddEffectLayer();
            wxArrayString timings;
            timings.push_back(name);
            mSequenceElements->AddViewToTimings(timings, mSequenceElements->GetViewName(mSequenceElements->GetCurrentView()));
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(GetParent(), eventRowHeaderChanged);
            timings.clear();
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
    } else if(id == ID_ROW_MNU_IMPORT_LYRICS) {
        ImportLyrics(element);
    } else if(id == ID_ROW_MNU_BREAKDOWN_TIMING_TRACK) {
        int result = wxMessageBox("Breakdown phrases into words and phonemes?", "Confirm Action", wxOK | wxCANCEL | wxCENTER);
        if (result == wxOK) {
            BreakdownTimingRow(element);
        }
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
        eventRowHeaderChanged.SetString(element->GetName());
        wxPostEvent(GetParent(), eventRowHeaderChanged);
    } else if (id == ID_ROW_MNU_TOGGLE_NODES) {
        if (mSequenceElements->GetVisibleRowInformation(mSelectedRow)->strandIndex == -1) {
            element->GetStrandLayer(0)->ShowNodes(true);
            element->ShowStrands(!element->ShowStrands());
        } else {
            ((StrandLayer*)mSequenceElements->GetVisibleEffectLayer(mSelectedRow))->ShowNodes(!((StrandLayer*)mSequenceElements->GetVisibleEffectLayer(mSelectedRow))->ShowNodes());
        }
        wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
        wxPostEvent(GetParent(), eventRowHeaderChanged);
    } else if (id == ID_ROW_MNU_CONVERT_TO_EFFECTS) {
        wxCommandEvent evt(EVT_CONVERT_DATA_TO_EFFECTS);
        evt.SetClientData(element);
        int i = ((ri->strandIndex << 16) & 0xFFFF0000) + ri->nodeIndex;
        evt.SetInt(i);
        wxPostEvent(GetParent(), evt);
    } else if (id == ID_ROW_MNU_PROMOTE_EFFECTS) {
        wxCommandEvent evt(EVT_PROMOTE_EFFECTS);
        evt.SetClientData(element);
        wxPostEvent(GetParent(), evt);
    }

    // Make sure message box is painted over by grid.
    wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
    wxPostEvent(GetParent(), eventForceRefresh);
}

void RowHeading::ImportLyrics(Element* element)
{
    LyricsDialog* dlgLyrics = new LyricsDialog(GetParent(), wxID_ANY, wxDefaultPosition, wxDefaultSize);

    if (dlgLyrics->ShowModal() == wxID_OK)
    {
        // remove all existing layers from timing track
        int num_layers = element->GetEffectLayerCount();
        for( int k = num_layers-1; k >= 0; k-- )
        {
            element->RemoveEffectLayer(k);
        }
        EffectLayer* phrase_layer = element->AddEffectLayer();

        int num_phrases = dlgLyrics->TextCtrlLyrics->GetNumberOfLines();
        for( int i = 0; i < dlgLyrics->TextCtrlLyrics->GetNumberOfLines(); i++ )
        {
            wxString line = dlgLyrics->TextCtrlLyrics->GetLineText(i);
            if( line == "" )
            {
                num_phrases--;
            }
        }
        int start_time = 0;
        int end_time = mSequenceEndMS;
        int interval_ms = (end_time-start_time) / num_phrases;
        for( int i = 0; i < num_phrases; i++ )
        {
            wxString line = dlgLyrics->TextCtrlLyrics->GetLineText(i);
            if( line != "" )
            {
                dictionary.InsertSpacesAfterPunctuation(line);
                end_time = TimeLine::RoundToMultipleOfPeriod(start_time+interval_ms, mSequenceElements->GetFrequency());
                phrase_layer->AddEffect(0,0,line,wxEmptyString,"",start_time,end_time,EFFECT_NOT_SELECTED,false);
                start_time = end_time;
            }
        }
    }
}

void RowHeading::BreakdownTimingRow(Element* element)
{
    dictionary.LoadDictionaries();
    EffectLayer* layer = element->GetEffectLayer(0);
    EffectLayer* word_layer = element->AddEffectLayer();
    EffectLayer* phoneme_layer = element->AddEffectLayer();
    for( int i = 0; i < layer->GetEffectCount(); i++ )
    {
        Effect* effect = layer->GetEffect(i);
        wxString phrase = effect->GetEffectName();
        BreakdownPhrase(word_layer, phoneme_layer, effect->GetStartTimeMS(), effect->GetEndTimeMS(), phrase);
    }
    wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
    wxPostEvent(GetParent(), eventRowHeaderChanged);
}

void RowHeading::BreakdownPhrase(EffectLayer* word_layer, EffectLayer* phoneme_layer, int start_time, int end_time, const wxString& phrase)
{
    if( phrase != "" )
    {
        wxArrayString words = wxSplit(phrase, ' ');
        int num_words = words.Count();
        int word_end_time = end_time;
        int interval_ms = (word_end_time-start_time) / num_words;
        for( int i = 0; i < num_words; i++ )
        {
            int word_end_time = TimeLine::RoundToMultipleOfPeriod(start_time+interval_ms, mSequenceElements->GetFrequency());
            if( i == num_words - 1 )
            {
                word_end_time = end_time;
            }
            word_layer->AddEffect(0,0,words[i],wxEmptyString,"",start_time,word_end_time,EFFECT_NOT_SELECTED,false);
            BreakdownWord(phoneme_layer, start_time, word_end_time, words[i]);
            start_time = word_end_time;
        }
    }
}

void RowHeading::BreakdownWord(EffectLayer* phoneme_layer, int start_time, int end_time, const wxString& word)
{
    wxArrayString phonemes;
    dictionary.BreakdownWord(word, phonemes);
    if( phonemes.Count() > 0 )
    {
        int phoneme_start_time = start_time;
        int phoneme_end_time = end_time;
        int phoneme_interval_ms = (phoneme_end_time-start_time) / phonemes.Count();
        for( int i = 0; i < phonemes.Count(); i++ )
        {
            phoneme_end_time = TimeLine::RoundToMultipleOfPeriod(phoneme_start_time+phoneme_interval_ms, mSequenceElements->GetFrequency());
            if( i == phonemes.Count() - 1 )
            {
                phoneme_end_time = end_time;
            }
            phoneme_layer->AddEffect(0,0,phonemes[i],wxEmptyString,"",phoneme_start_time,phoneme_end_time,EFFECT_NOT_SELECTED,false);
            phoneme_start_time = phoneme_end_time;
        }
    }
}

bool RowHeading::HitTestCollapseExpand(int row,int x, bool* IsCollapsed)
{
    if(mSequenceElements->GetVisibleRowInformation(row)->element->GetType() != "timing" &&
       x<DEFAULT_ROW_HEADING_MARGIN)
    {
        *IsCollapsed = mSequenceElements->GetVisibleRowInformation(row)->Collapsed;
        return true;
    }
    else
    {
        return false;
    }
}

bool RowHeading::HitTestTimingActive(int row,int x, bool* IsActive)
{
    if(mSequenceElements->GetVisibleRowInformation(row)->element->GetType() == "timing" &&
       x<DEFAULT_ROW_HEADING_MARGIN)
    {
        *IsActive = mSequenceElements->GetVisibleRowInformation(row)->Active;
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
    wxPaintDC dc(this);
    Draw();
}

void RowHeading::Draw()
{
    wxClientDC dc(this);
    wxCoord w,h;
    wxPen penOutline(wxColor(32,32,32), .1);
    dc.GetSize(&w,&h);
    wxBrush brush(mHeaderColorModel->asWxColor(),wxBRUSHSTYLE_SOLID);
    dc.SetBrush(brush);
    dc.SetPen(penOutline);
    int row=0;
    int startY = 0,endY = 0;
    for(int i =0;i< mSequenceElements->GetVisibleRowInformationSize();i++)
    {
        wxString prefix;
        if (mSequenceElements->GetVisibleRowInformation(i)->submodel) {
            prefix = "  ";
        }
        wxBrush brush(GetHeaderColor(mSequenceElements->GetVisibleRowInformation(i))->asWxColor(),wxBRUSHSTYLE_SOLID);
        dc.SetBrush(brush);
        startY = DEFAULT_ROW_HEADING_HEIGHT*row;
        endY = DEFAULT_ROW_HEADING_HEIGHT*(row+1);
        dc.SetBackgroundMode(wxTRANSPARENT);
        dc.DrawRectangle(0,startY,w,DEFAULT_ROW_HEADING_HEIGHT);
        if(mSequenceElements->GetVisibleRowInformation(i)->layerIndex>0
           || mSequenceElements->GetVisibleRowInformation(i)->strandIndex >= 0)   // If effect layer = 0
        {
            dc.SetPen(*wxLIGHT_GREY_PEN);
            dc.DrawLine(1,startY,w-1,startY);
            dc.DrawLine(1,startY-1,w-1,startY-1);
            dc.SetPen(*wxBLACK_PEN);
            if (mSequenceElements->GetVisibleRowInformation(i)->strandIndex >= 0) {
                wxRect r(DEFAULT_ROW_HEADING_MARGIN,startY,w-DEFAULT_ROW_HEADING_MARGIN,22);
                wxString name = mSequenceElements->GetVisibleRowInformation(i)->displayName;
                if (name == "") {
                    if (mSequenceElements->GetVisibleRowInformation(i)->nodeIndex >= 0) {
                        name = wxString::Format("Node %d", mSequenceElements->GetVisibleRowInformation(i)->nodeIndex + 1);
                    } else {
                        name = wxString::Format("Strand %d", mSequenceElements->GetVisibleRowInformation(i)->strandIndex + 1);
                    }

                }
                if (mSequenceElements->GetVisibleRowInformation(i)->nodeIndex >= 0) {
                    dc.DrawLabel(prefix + "     " + name,r,wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT);
                } else {
                    dc.DrawLabel(prefix + "  " + name,r,wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT);
                }
            }
        }
        else        // Draw label
        {
            wxRect r(DEFAULT_ROW_HEADING_MARGIN,startY,w-DEFAULT_ROW_HEADING_MARGIN,22);
            dc.DrawLabel(prefix + mSequenceElements->GetVisibleRowInformation(i)->element->GetName(),r,wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT);
        }

        if(mSequenceElements->GetVisibleRowInformation(i)->element->GetType()=="view")
        {
            dc.SetBrush(*wxWHITE_BRUSH);
            dc.SetPen(*wxBLACK_PEN);
            dc.DrawRectangle(2,startY+7,9,9);
            dc.DrawLine(4,startY+11,9,startY+11);
            if(mSequenceElements->GetVisibleRowInformation(i)->Collapsed)
            {
                dc.DrawLine(6,startY+9,6,startY+14);
            }
            dc.SetPen(penOutline);
            dc.SetBrush(brush);
        }
        else if (mSequenceElements->GetVisibleRowInformation(i)->element->GetType()=="model")
        {
            if(mSequenceElements->GetVisibleRowInformation(i)->element->GetEffectLayerCount() > 1 &&
               mSequenceElements->GetVisibleRowInformation(i)->layerIndex == 0 &&
               mSequenceElements->GetVisibleRowInformation(i)->strandIndex == -1)
            {
                dc.SetBrush(*wxWHITE_BRUSH);
                dc.SetPen(*wxBLACK_PEN);
                dc.DrawRectangle(2,startY+7,9,9);
                dc.DrawLine(4,startY+11,9,startY+11);
                if(mSequenceElements->GetVisibleRowInformation(i)->Collapsed)
                {
                    dc.DrawLine(6,startY+9,6,startY+14);
                }
                dc.SetPen(penOutline);
                dc.SetBrush(brush);
            }
        }
        else if(mSequenceElements->GetVisibleRowInformation(i)->element->GetType()=="timing")
        {
            if( mSequenceElements->GetVisibleRowInformation(i)->layerIndex == 0 )
            {
                dc.SetPen(*wxBLACK_PEN);
                if(mSequenceElements->GetVisibleRowInformation(i)->Active)
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
                if(mSequenceElements->GetVisibleRowInformation(i)->element->GetEffectLayerCount() > 1)
                {
                    dc.DrawBitmap(papagayo_icon, getWidth()-25, startY+3, true);
                }
            }
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
        if (info->RowNumber == mSelectedRow )
        //if (info->element->GetSelected())
        {
            return  mHeaderSelectedColor;
        }
        else
        {
            return mHeaderColorModel;
        }
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







