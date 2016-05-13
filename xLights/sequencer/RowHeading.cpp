#include "RowHeading.h"
#include "Waveform.h"
#include "wx/wx.h"
#include "wx/brush.h"
#include "../xLightsMain.h"
#include "EffectDropTarget.h"
#include "../BitmapCache.h"
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
const long RowHeading::ID_ROW_MNU_EXPORT_RENDERED_MODEL = wxNewId();
const long RowHeading::ID_ROW_MNU_EDIT_DISPLAY_ELEMENTS = wxNewId();
const long RowHeading::ID_ROW_MNU_TOGGLE_STRANDS = wxNewId();
const long RowHeading::ID_ROW_MNU_TOGGLE_NODES = wxNewId();
const long RowHeading::ID_ROW_MNU_CONVERT_TO_EFFECTS = wxNewId();
const long RowHeading::ID_ROW_MNU_PROMOTE_EFFECTS = wxNewId();
const long RowHeading::ID_ROW_MNU_COPY_ROW = wxNewId();
const long RowHeading::ID_ROW_MNU_PASTE_ROW = wxNewId();

// Timing Track popup menu
const long RowHeading::ID_ROW_MNU_ADD_TIMING_TRACK = wxNewId();
const long RowHeading::ID_ROW_MNU_DELETE_TIMING_TRACK = wxNewId();
const long RowHeading::ID_ROW_MNU_IMPORT_TIMING_TRACK = wxNewId();
const long RowHeading::ID_ROW_MNU_EXPORT_TIMING_TRACK = wxNewId();
const long RowHeading::ID_ROW_MNU_IMPORT_LYRICS = wxNewId();
const long RowHeading::ID_ROW_MNU_BREAKDOWN_TIMING_PHRASES = wxNewId();
const long RowHeading::ID_ROW_MNU_BREAKDOWN_TIMING_WORDS = wxNewId();


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
    papagayo_icon = BitmapCache::GetPapgayoIcon(tooltip, 16, false);
    model_group_icon = BitmapCache::GetModelGroupIcon(tooltip, 16, false);
    mCanPaste = false;
}

RowHeading::~RowHeading()
{
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
            mnuLayer->Append(ID_ROW_MNU_EXPORT_RENDERED_MODEL, "Render and Export Model");
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
            mnuLayer->AppendSeparator();
            mnuLayer->Append(ID_ROW_MNU_COPY_ROW,"Copy Row");
            wxMenuItem* menu_paste = mnuLayer->Append(ID_ROW_MNU_PASTE_ROW,"Paste Row");
            if( !mCanPaste ) {
                menu_paste->Enable(false);
            }
        }
    }
    else
    {
        mnuLayer = new wxMenu();
        mnuLayer->Append(ID_ROW_MNU_ADD_TIMING_TRACK,"Add Timing Track");
        mnuLayer->Append(ID_ROW_MNU_DELETE_TIMING_TRACK,"Delete Timing Track");
        mnuLayer->Append(ID_ROW_MNU_IMPORT_TIMING_TRACK, "Import Timing Track");
        mnuLayer->Append(ID_ROW_MNU_EXPORT_TIMING_TRACK, "Export Timing Track");
        mnuLayer->AppendSeparator();
        mnuLayer->Append(ID_ROW_MNU_IMPORT_LYRICS,"Import Lyrics");
        mnuLayer->Append(ID_ROW_MNU_BREAKDOWN_TIMING_PHRASES,"Breakdown Phrases");
        if( element->GetEffectLayerCount() > 1 )
        {
            mnuLayer->Append(ID_ROW_MNU_BREAKDOWN_TIMING_WORDS,"Breakdown Words");
        }
        mnuLayer->AppendSeparator();
        mnuLayer->Append(ID_ROW_MNU_COPY_ROW,"Copy Row");
        wxMenuItem* menu_paste = mnuLayer->Append(ID_ROW_MNU_PASTE_ROW,"Paste Row");
        if( !mCanPaste ) {
            menu_paste->Enable(false);
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
        std::string name = wxGetTextFromUser("What is name of new timing track?", "Timing Track Name").ToStdString();
        if( mSequenceElements->ElementExists(name) )
        {
            wxMessageBox("Timing name already exists in sequence as a model or another timing.", "ERROR");
        }
        else if(name.size()>0)
        {
            // Deactivate active timing mark so new one is selected;
            mSequenceElements->DeactivateAllTimingElements();
            int timingCount = mSequenceElements->GetNumberOfTimingElements();
            Element* e = mSequenceElements->AddElement(timingCount,name,
                                                       "timing",true,false,true,false);
            e->AddEffectLayer();
            mSequenceElements->AddTimingToAllViews(name);
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        }
    }
    else if(id == ID_ROW_MNU_DELETE_TIMING_TRACK)
    {
        wxString prompt = wxString::Format("Delete 'Timing Track '%s'?",element->GetName());
        wxString caption = "Confirm Timing Track Deletion";

        int answer = wxMessageBox(prompt,caption,wxYES_NO);
        if(answer == wxYES)
        {
            mSequenceElements->DeleteElement(element->GetName());
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        }
    }
    else if (id == ID_ROW_MNU_EXPORT_TIMING_TRACK) {
        wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
        wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, element->GetName(), wxEmptyString, "Timing files (*.xtiming)|*.xtiming", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (filename.IsEmpty()) return;
        wxFile f(filename);
        log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.info("Saving to xtiming file %s.", std::string(filename.c_str()).c_str());
        if (!f.Create(filename, true) || !f.IsOpened())
        {
            logger_base.info("Unable to create file %s. Error %d\n", std::string(filename.c_str()).c_str(), f.GetLastError());
            wxMessageBox(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()));
            return;
        }
        wxString name = wxFileName(filename).GetName();
        wxString td = wxString(mSequenceElements->GetElement(element->GetName())->GetExport().c_str());
        wxString v = xlights_version_string;
        f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<timing ");
        f.Write(wxString::Format("name=\"%s\" ", name));
        f.Write(wxString::Format("SourceVersion=\"%s\">\n", v));
        f.Write(td);
        f.Write("</timing>\n");
        f.Close();
    } else if(id == ID_ROW_MNU_IMPORT_TIMING_TRACK) {
        wxCommandEvent playEvent(EVT_IMPORT_TIMING);
        wxPostEvent(GetParent(), playEvent);
    } else if(id == ID_ROW_MNU_IMPORT_LYRICS) {
        mSequenceElements->ImportLyrics(element, GetParent());
    } else if(id == ID_ROW_MNU_BREAKDOWN_TIMING_PHRASES) {
        int result = wxMessageBox("Breakdown phrases? Any existing words and phonemes will be deleted.", "Confirm Action", wxOK | wxCANCEL | wxCENTER);
        if (result == wxOK) {
            BreakdownTimingPhrases(element);
        }
    } else if(id == ID_ROW_MNU_BREAKDOWN_TIMING_WORDS) {
        int result = wxMessageBox("Breakdown words? Any existing phonemes will be deleted.", "Confirm Action", wxOK | wxCANCEL | wxCENTER);
        if (result == wxOK) {
            BreakdownTimingWords(element);
        }
    } else if (id == ID_ROW_MNU_EXPORT_MODEL) {
        wxCommandEvent playEvent(EVT_EXPORT_MODEL);
        playEvent.SetInt(0);
        playEvent.SetString(element->GetName());
        wxPostEvent(GetParent(), playEvent);
    } else if (id == ID_ROW_MNU_EXPORT_RENDERED_MODEL) {
        wxCommandEvent playEvent(EVT_EXPORT_MODEL);
        playEvent.SetInt(1);
        playEvent.SetString(element->GetName());
        wxPostEvent(GetParent(), playEvent);
    } else if (id == ID_ROW_MNU_PLAY_MODEL) {
        wxCommandEvent playEvent(EVT_PLAY_MODEL);
        playEvent.SetString(element->GetName());
        wxPostEvent(GetParent(), playEvent);
    } else if (id == ID_ROW_MNU_COPY_ROW) {
        wxCommandEvent copyRowEvent(EVT_COPY_MODEL_EFFECTS);
        copyRowEvent.SetInt(mSelectedRow);
        wxPostEvent(GetParent(), copyRowEvent);
        mCanPaste = true;
    } else if (id == ID_ROW_MNU_PASTE_ROW) {
        wxCommandEvent pasteRowEvent(EVT_PASTE_MODEL_EFFECTS);
        pasteRowEvent.SetInt(mSelectedRow);
        wxPostEvent(GetParent(), pasteRowEvent);
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

void RowHeading::BreakdownTimingPhrases(Element* element)
{
    element->SetFixedTiming(0);
    EffectLayer* layer = element->GetEffectLayer(0);
    if( element->GetEffectLayerCount() > 1 )
    {
        for( int k = element->GetEffectLayerCount()-1; k > 0; k--)
        {
            element->RemoveEffectLayer(k);
        }
    }
    EffectLayer* word_layer = element->AddEffectLayer();
    for( int i = 0; i < layer->GetEffectCount(); i++ )
    {
        Effect* effect = layer->GetEffect(i);
        std::string phrase = effect->GetEffectName();
        mSequenceElements->BreakdownPhrase(word_layer, effect->GetStartTimeMS(), effect->GetEndTimeMS(), phrase);
    }
    wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
    wxPostEvent(GetParent(), eventRowHeaderChanged);
}

void RowHeading::BreakdownTimingWords(Element* element)
{
    if( element->GetEffectLayerCount() > 2 )
    {
        element->RemoveEffectLayer(2);
    }
    EffectLayer* word_layer = element->GetEffectLayer(1);
    EffectLayer* phoneme_layer = element->AddEffectLayer();
    for( int i = 0; i < word_layer->GetEffectCount(); i++ )
    {
        Effect* effect = word_layer->GetEffect(i);
        std::string word = effect->GetEffectName();
        mSequenceElements->BreakdownWord(phoneme_layer, effect->GetStartTimeMS(), effect->GetEndTimeMS(), word);
    }
    wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
    wxPostEvent(GetParent(), eventRowHeaderChanged);
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

void RowHeading::SetSelectedRow(int row_number)
{
    mSequenceElements->UnSelectAllElements();
    mSelectedRow = row_number;
    Refresh(false);
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
                wxRect r(DEFAULT_ROW_HEADING_MARGIN,startY,w-DEFAULT_ROW_HEADING_MARGIN,DEFAULT_ROW_HEADING_HEIGHT);
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
            wxRect r(DEFAULT_ROW_HEADING_MARGIN,startY,w-DEFAULT_ROW_HEADING_MARGIN,DEFAULT_ROW_HEADING_HEIGHT);
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
                dc.DrawRectangle(2,startY + DEFAULT_ROW_HEADING_HEIGHT/2 - 4,9,9);
                dc.DrawLine(2,startY + DEFAULT_ROW_HEADING_HEIGHT/2,9,startY + DEFAULT_ROW_HEADING_HEIGHT/2);
                if(mSequenceElements->GetVisibleRowInformation(i)->Collapsed)
                {
                    dc.DrawLine(6,startY + DEFAULT_ROW_HEADING_HEIGHT/2 + 4,6,startY + DEFAULT_ROW_HEADING_HEIGHT/2 - 4);
                }
                dc.SetPen(penOutline);
                dc.SetBrush(brush);
            }
            // draw Model Group icon if necessary
            Model *m = mSequenceElements->GetXLightsFrame()->AllModels[mSequenceElements->GetVisibleRowInformation(i)->element->GetName()];
            if ( m != nullptr &&  m->GetDisplayAs() == "ModelGroup" )
            {
                dc.DrawBitmap(model_group_icon, getWidth()-25, startY+3, true);
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
                    dc.DrawCircle(7,startY + DEFAULT_ROW_HEADING_HEIGHT/2,5);

                    dc.SetBrush(*wxGREY_BRUSH);
                    dc.DrawCircle(7,startY + DEFAULT_ROW_HEADING_HEIGHT/2,2);
                }
                else
                {
                    dc.SetBrush(*wxWHITE_BRUSH);
                    dc.DrawCircle(7,startY + DEFAULT_ROW_HEADING_HEIGHT/2,5);
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

const xlColor* RowHeading::GetTimingColor(int colorIndex)
{
    const xlColor* value;
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







