#include "RowHeading.h"
#include "wx/wx.h"
#include "wx/brush.h"
#include "../xLightsMain.h"
#include "../xLightsVersion.h"
#include "../BitmapCache.h"
#include <wx/numdlg.h>
#include "models/ModelGroup.h"
#include "../SelectTimingsDialog.h"

#define ICON_SPACE 25

BEGIN_EVENT_TABLE(RowHeading, wxWindow)
EVT_LEFT_DOWN(RowHeading::mouseLeftDown)
EVT_RIGHT_DOWN(RowHeading::rightClick)
EVT_LEFT_DCLICK(RowHeading::leftDoubleClick)
EVT_ENTER_WINDOW(RowHeading::mouseEnter)
EVT_LEAVE_WINDOW(RowHeading::mouseLeave)
EVT_MOTION(RowHeading::mouseMove)
EVT_PAINT(RowHeading::render)
END_EVENT_TABLE()

// Menu constants
const long RowHeading::ID_ROW_MNU_INSERT_LAYER_ABOVE = wxNewId();
const long RowHeading::ID_ROW_MNU_INSERT_LAYER_BELOW = wxNewId();
const long RowHeading::ID_ROW_MNU_INSERT_LAYERS_BELOW = wxNewId();
const long RowHeading::ID_ROW_MNU_DELETE_LAYER = wxNewId();
const long RowHeading::ID_ROW_MNU_DELETE_LAYERS = wxNewId();
const long RowHeading::ID_ROW_MNU_DELETE_UNUSEDLAYERS = wxNewId();
const long RowHeading::ID_ROW_MNU_LAYER = wxNewId();
const long RowHeading::ID_ROW_MNU_PLAY_MODEL = wxNewId();
const long RowHeading::ID_ROW_MNU_EXPORT_MODEL = wxNewId();
const long RowHeading::ID_ROW_MNU_EXPORT_RENDERED_MODEL = wxNewId();
const long RowHeading::ID_ROW_MNU_EDIT_DISPLAY_ELEMENTS = wxNewId();
const long RowHeading::ID_ROW_MNU_TOGGLE_STRANDS = wxNewId();
const long RowHeading::ID_ROW_MNU_SHOW_EFFECTS = wxNewId();
const long RowHeading::ID_ROW_MNU_COLLAPSEALL = wxNewId();
const long RowHeading::ID_ROW_MNU_TOGGLE_NODES = wxNewId();
const long RowHeading::ID_ROW_MNU_CONVERT_TO_EFFECTS = wxNewId();
const long RowHeading::ID_ROW_MNU_PROMOTE_EFFECTS = wxNewId();
const long RowHeading::ID_ROW_MNU_COPY_ROW = wxNewId();
const long RowHeading::ID_ROW_MNU_PASTE_ROW = wxNewId();
const long RowHeading::ID_ROW_MNU_DELETE_ROW_EFFECTS = wxNewId();
const long RowHeading::ID_ROW_MNU_DELETE_MODEL_EFFECTS = wxNewId();
const long RowHeading::ID_ROW_MNU_SELECT_ROW_EFFECTS = wxNewId();

// Timing Track popup menu
const long RowHeading::ID_ROW_MNU_ADD_TIMING_TRACK = wxNewId();
const long RowHeading::ID_ROW_MNU_RENAME_TIMING_TRACK = wxNewId();
const long RowHeading::ID_ROW_MNU_DELETE_TIMING_TRACK = wxNewId();
const long RowHeading::ID_ROW_MNU_IMPORT_TIMING_TRACK = wxNewId();
const long RowHeading::ID_ROW_MNU_EXPORT_TIMING_TRACK = wxNewId();
const long RowHeading::ID_ROW_MNU_IMPORT_NOTES = wxNewId();
const long RowHeading::ID_ROW_MNU_IMPORT_LYRICS = wxNewId();
const long RowHeading::ID_ROW_MNU_BREAKDOWN_TIMING_PHRASES = wxNewId();
const long RowHeading::ID_ROW_MNU_BREAKDOWN_TIMING_WORDS = wxNewId();


int DEFAULT_ROW_HEADING_HEIGHT = 22;

RowHeading::RowHeading(MainSequencer* parent, wxWindowID id, const wxPoint &pos, const wxSize &size,
                       long style, const wxString &name):
                       wxWindow((wxWindow*)parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
{
    DOUBLE_BUFFER(this);
    wxString tooltip;
#ifdef __WXOSX__
    bool exact = false;
#else
    bool exact = true;
#endif
    papagayo_icon = BitmapCache::GetPapgayoIcon(tooltip, 16, exact);
    papagayox_icon = BitmapCache::GetPapgayoXIcon(tooltip, 16, exact);
    model_group_icon = BitmapCache::GetModelGroupIcon(tooltip, 16, exact);
    mCanPaste = false;
}

RowHeading::~RowHeading()
{
}

void RowHeading::ProcessTooltip(wxMouseEvent& event)
{
    int mSelectedRow = event.GetY() / DEFAULT_ROW_HEADING_HEIGHT;
    if (mSelectedRow < mSequenceElements->GetVisibleRowInformationSize())
    {
        Element* e = mSequenceElements->GetVisibleRowInformation(mSelectedRow)->element;
        if (e != nullptr)
        {
            wxString layers;
            if (e->GetType() != ELEMENT_TYPE_TIMING && e->GetEffectLayerCount() > 1)
            {
                layers = wxString::Format(" [%d]", (int)e->GetEffectLayerCount());
            }

            wxClientDC dc(this);
            wxSize size = dc.GetTextExtent(e->GetName() + layers);

            // Only set tooltip if the text looks too big to display correctly
            if (size.x > getWidth() - DEFAULT_ROW_HEADING_MARGIN - ICON_SPACE)
            {
                auto s1 = GetToolTipText().ToStdString();
                auto s2 = e->GetName();
                if (GetToolTipText() != e->GetName() + layers)
                {
                    SetToolTip(e->GetName() + layers);
                }
            }
            else
            {
                SetToolTip("");
            }
        }
        else
        {
            SetToolTip("");
        }
    }
    else
    {
        SetToolTip("");
    }
}

void RowHeading::mouseEnter(wxMouseEvent& event)
{
    mouseMove(event);
}

void RowHeading::mouseLeave(wxMouseEvent& event)
{
    SetToolTip("");
}

void RowHeading::mouseMove(wxMouseEvent& event)
{
    ProcessTooltip(event);
}

void RowHeading::mouseLeftDown( wxMouseEvent& event)
{
    mSelectedRow = event.GetY()/DEFAULT_ROW_HEADING_HEIGHT;
    if(mSelectedRow < mSequenceElements->GetVisibleRowInformationSize())
    {
        bool result;
        Element* e = mSequenceElements->GetVisibleRowInformation(mSelectedRow)->element;
        if(e->GetType() == ELEMENT_TYPE_MODEL)
        {
            mSequenceElements->UnSelectAllElements();
            ModelElement *me = dynamic_cast<ModelElement *>(e);
            me->SetSelected(true);
            wxCommandEvent playEvent(EVT_MODEL_SELECTED);  // send model selection in case we need to switch playback to this model
            playEvent.SetString(e->GetModelName());
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
            TimingElement *te = dynamic_cast<TimingElement *>(e);
            te->SetActive(!result);
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

    if (element->GetType() == ELEMENT_TYPE_MODEL) {
        ModelElement *me = dynamic_cast<ModelElement *>(element);
        me->ShowStrands(!me->ShowStrands());
        wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
        eventRowHeaderChanged.SetString(element->GetModelName());
        wxPostEvent(GetParent(), eventRowHeaderChanged);
    } else if (element->GetType() == ELEMENT_TYPE_SUBMODEL) {

    } else if (element->GetType() == ELEMENT_TYPE_STRAND) {
        StrandElement *se = dynamic_cast<StrandElement *>(element);
        se->ShowNodes(!se->ShowNodes());

        wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
        eventRowHeaderChanged.SetString(element->GetName());
        wxPostEvent(GetParent(), eventRowHeaderChanged);
    } else if (element->GetType() == ELEMENT_TYPE_TIMING) {
        if (element->GetEffectLayerCount() > 1) {
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
    wxMenu mnuLayer;
    mSelectedRow = event.GetY()/DEFAULT_ROW_HEADING_HEIGHT;
    if (mSelectedRow >= mSequenceElements->GetVisibleRowInformationSize()) {
        return;
    }

    Row_Information_Struct *ri =  mSequenceElements->GetVisibleRowInformation(mSelectedRow);
    Element* element = ri->element;
    if (element->GetType()==ELEMENT_TYPE_MODEL
        || element->GetType()==ELEMENT_TYPE_SUBMODEL
        || element->GetType()==ELEMENT_TYPE_STRAND)
    {
        if (ri->nodeIndex < 0) {
            mnuLayer.Append(ID_ROW_MNU_INSERT_LAYER_ABOVE,"Insert Layer Above");
            mnuLayer.Append(ID_ROW_MNU_INSERT_LAYER_BELOW,"Insert Layer Below");
            mnuLayer.Append(ID_ROW_MNU_INSERT_LAYERS_BELOW, "Insert Multiple Layers Below");

            if(element->GetEffectLayerCount() > 1)
            {
                mnuLayer.Append(ID_ROW_MNU_DELETE_LAYER,"Delete Layer");

                mnuLayer.Append(ID_ROW_MNU_DELETE_LAYERS, "Delete Multiple Layers");
                mnuLayer.Append(ID_ROW_MNU_DELETE_UNUSEDLAYERS, "Delete Unused Layers");
            }
            mnuLayer.AppendSeparator();
        }
        mnuLayer.Append(ID_ROW_MNU_PLAY_MODEL,"Play Model");
        mnuLayer.Append(ID_ROW_MNU_EXPORT_MODEL,"Export Model");
        mnuLayer.Append(ID_ROW_MNU_EXPORT_RENDERED_MODEL, "Render and Export Model");
        mnuLayer.AppendSeparator();
        bool canPromote = false;
        ModelElement *me = dynamic_cast<ModelElement *>(element);
        if (element->GetType()==ELEMENT_TYPE_STRAND || element->GetType()==ELEMENT_TYPE_SUBMODEL) {
            me = dynamic_cast<SubModelElement *>(element)->GetModelElement();
        }
        if (me->GetSubModelAndStrandCount() > 0) {
            if (element->GetType() != ELEMENT_TYPE_SUBMODEL) {
                canPromote = true;
            }
            mnuLayer.Append(ID_ROW_MNU_TOGGLE_STRANDS,"Toggle Strands");
            if (ri->strandIndex >= 0) {
                mnuLayer.Append(ID_ROW_MNU_TOGGLE_NODES,"Toggle Nodes");
            }
        } else {
            mnuLayer.Append(ID_ROW_MNU_TOGGLE_STRANDS,"Toggle Models");
        }
        mnuLayer.Append(ID_ROW_MNU_SHOW_EFFECTS, "Show All Effects");
        mnuLayer.Append(ID_ROW_MNU_COLLAPSEALL, "Collapse All");
        if (ri->nodeIndex > -1) {
            StrandElement *se = dynamic_cast<StrandElement*>(element);
            if (se && se->GetNodeLayer(ri->nodeIndex)->GetEffectCount() == 0) {
                mnuLayer.Append(ID_ROW_MNU_CONVERT_TO_EFFECTS, "Convert To Effect");
            }
        }
        if (canPromote) {
            mnuLayer.Append(ID_ROW_MNU_PROMOTE_EFFECTS, "Promote Node Effects");
        }
        mnuLayer.AppendSeparator();
        mnuLayer.Append(ID_ROW_MNU_SELECT_ROW_EFFECTS, "Select Row Effects");
        mnuLayer.AppendSeparator();
        mnuLayer.Append(ID_ROW_MNU_COPY_ROW,"Copy Row");
        wxMenuItem* menu_paste = mnuLayer.Append(ID_ROW_MNU_PASTE_ROW,"Paste Row");
        if (!mCanPaste) {
            menu_paste->Enable(false);
        }
        mnuLayer.Append(ID_ROW_MNU_DELETE_ROW_EFFECTS, "Delete Row Effects");
        mnuLayer.Append(ID_ROW_MNU_DELETE_MODEL_EFFECTS, "Delete Model Effects");
    }
    else
    {
        mnuLayer.Append(ID_ROW_MNU_ADD_TIMING_TRACK,"Add Timing Track");
        mnuLayer.Append(ID_ROW_MNU_RENAME_TIMING_TRACK, "Rename Timing Track");
        mnuLayer.Append(ID_ROW_MNU_DELETE_TIMING_TRACK,"Delete Timing Track");
        mnuLayer.Append(ID_ROW_MNU_IMPORT_TIMING_TRACK, "Import Timing Track");
        mnuLayer.Append(ID_ROW_MNU_EXPORT_TIMING_TRACK, "Export Timing Track");
        mnuLayer.Append(ID_ROW_MNU_IMPORT_NOTES, "Import Notes");
        mnuLayer.AppendSeparator();
        mnuLayer.Append(ID_ROW_MNU_IMPORT_LYRICS,"Import Lyrics");
        mnuLayer.Append(ID_ROW_MNU_BREAKDOWN_TIMING_PHRASES,"Breakdown Phrases");
        if( element->GetEffectLayerCount() > 1 )
        {
            mnuLayer.Append(ID_ROW_MNU_BREAKDOWN_TIMING_WORDS,"Breakdown Words");
        }
        mnuLayer.AppendSeparator();
        mnuLayer.Append(ID_ROW_MNU_COPY_ROW,"Copy Row");
        wxMenuItem* menu_paste = mnuLayer.Append(ID_ROW_MNU_PASTE_ROW,"Paste Row");
        if( !mCanPaste ) {
            menu_paste->Enable(false);
        }
    }

    mnuLayer.AppendSeparator();
    mnuLayer.Append(ID_ROW_MNU_EDIT_DISPLAY_ELEMENTS,"Edit Display Elements");
    mnuLayer.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&RowHeading::OnLayerPopup, nullptr, this);
    Draw();
    PopupMenu(&mnuLayer);
}

void RowHeading::OnLayerPopup(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    Row_Information_Struct *ri = mSequenceElements->GetVisibleRowInformation(mSelectedRow);
    if (ri == nullptr || mSequenceElements == nullptr) return;

    Element* element = ri->element;
    if (element == nullptr) return;

    int layer_index = ri->layerIndex;
    int id = event.GetId();
    if (id == ID_ROW_MNU_INSERT_LAYER_ABOVE)
    {
        element->InsertEffectLayer(layer_index);
        wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
        wxPostEvent(GetParent(), eventRowHeaderChanged);
    }
    else if (id == ID_ROW_MNU_INSERT_LAYER_BELOW)
    {
        if ( layer_index < element->GetEffectLayerCount() - 1)
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
    else if (id == ID_ROW_MNU_INSERT_LAYERS_BELOW)
    {
        int numtoinsert = wxGetNumberFromUser("Enter number of layers to insert", "Layers", "Insert multiple layers", 2, 1, 100, this);

        if (numtoinsert > 0)
        {
            if (layer_index < element->GetEffectLayerCount() - 1)
            {
                for (int i = 0; i < numtoinsert; i++)
                {
                    element->InsertEffectLayer(layer_index + 1);
                }
            }
            else
            {
                for (int i = 0; i < numtoinsert; i++)
                {
                    element->AddEffectLayer();
                }
            }
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        }
    }
    else if (id == ID_ROW_MNU_DELETE_LAYER)
    {
        logger_base.debug("RowHeading::OnLayerPopup Deleting layer.");
        if (mSequenceElements->GetVisibleRowInformation(mSelectedRow) != nullptr)
        {
            int layerIndex = mSequenceElements->GetVisibleRowInformation(mSelectedRow)->layerIndex;

            int answer = wxYES;

            // only prompt the user if there are effects on the layer
            if (element->GetEffectLayer(layerIndex)->GetEffectCount() > 0)
            {
                wxString prompt = wxString::Format("Layer contains one or more effects. Are you sure you want to delete 'Layer %d' of '%s'?",
                    layerIndex + 1, element->GetModelName());
                wxString caption = "Confirm Layer Deletion";

                answer = wxMessageBox(prompt, caption, wxYES_NO);
            }

            if (answer == wxYES)
            {
                element->RemoveEffectLayer(layerIndex);
                wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
                wxPostEvent(GetParent(), eventRowHeaderChanged);
            }
        }
    }
	else if (id == ID_ROW_MNU_DELETE_LAYERS)
	{
        logger_base.debug("RowHeading::OnLayerPopup Deleting layers.");
        if (mSequenceElements->GetVisibleRowInformation(mSelectedRow) != nullptr)
        {
            int layerIndex = mSequenceElements->GetVisibleRowInformation(mSelectedRow)->layerIndex;
            int bottomLayer = element->GetEffectLayerCount();

            int numtoDelete = wxGetNumberFromUser("Enter number of layers to delete", "Layers", "Delete multiple layers", bottomLayer - layerIndex, 1, bottomLayer - layerIndex, this);


            if (numtoDelete > 0)
            {
                int startDeleteLayer = layerIndex + (numtoDelete - 1);

                bool containsEffects = false;
                bool deleteLayers = true;
                for (int deleteLayer = startDeleteLayer; deleteLayer >= layerIndex; deleteLayer--)
                {
                    //Check for effects
                    if (element->GetEffectLayer(deleteLayer)->GetEffectCount() > 0)
                    {
                        containsEffects = true;
                        break;
                    }
                }

                if (containsEffects == true)
                {
                    wxString prompt = wxString::Format("One or more layers contain effects.  Delete?");
                    wxString caption = "Confirm Layer Deletion";

                    int answer = wxMessageBox(prompt, caption, wxYES_NO);
                    if (answer == wxNO)
                    {
                        deleteLayers = false;
                    }
                }
                if (deleteLayers == true)
                {
                    for (int deleteLayer = startDeleteLayer; deleteLayer >= layerIndex; deleteLayer--)
                    {
                        element->RemoveEffectLayer(deleteLayer);
                    }

                    //Add a new layer if the topmost layer was removed
                    if (layerIndex == 0)element->AddEffectLayer();

                    wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
                    wxPostEvent(GetParent(), eventRowHeaderChanged);
                }
            }
        }
	}
    else if (id == ID_ROW_MNU_DELETE_UNUSEDLAYERS)
    {
        logger_base.debug("RowHeading::OnLayerPopup Deleting unused layers.");
        bool deleted = false;
        for (int i = 0; i < element->GetEffectLayerCount(); ++i)
        {
            if (element->GetEffectLayer(i)->GetEffectCount() > 0)
            {
                // dont delete this layer
            }
            else
            {
                if (element->GetEffectLayerCount() == 1)
                {
                    //last layer ... dont delete it
                }
                else
                {
                    element->RemoveEffectLayer(i);
                    --i;
                    deleted = true;
                }
            }
        }
        if (deleted)
        {
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

            int adjViews = (mSequenceElements->GetCurrentView() == MASTER_VIEW) ? -1 : -2;
            if (mSequenceElements->GetViewCount() + adjViews > 0 && mSequenceElements->GetCurrentView() == MASTER_VIEW)
            {
                if (wxMessageBox("Do you want to add this timing track to all views?", "Add to all views.", wxYES_NO) == wxYES)
                {
                    mSequenceElements->AddTimingToAllViews(name);
                }
                else
                {
                    mSequenceElements->AddTimingToCurrentView(name);
                }
            }
            else
            {
                mSequenceElements->AddTimingToCurrentView(name);
            }
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        }
    }
    else if (id == ID_ROW_MNU_RENAME_TIMING_TRACK)
    {
        std::string name = wxGetTextFromUser("What is the new name of the timing track?", "Timing Track Name").ToStdString();
        if (mSequenceElements->ElementExists(name))
        {
            wxMessageBox("Timing name already exists in sequence as a model or another timing.", "ERROR");
        }
        else if (name.size()>0)
        {
            std::string oldname = element->GetName();
            mSequenceElements->GetXLightsFrame()->RenameTimingElement(oldname, name);
        }
    }
    else if(id == ID_ROW_MNU_DELETE_TIMING_TRACK)
    {
        wxString prompt = wxString::Format("Delete 'Timing Track '%s'?",element->GetName());
        wxString caption = "Confirm Timing Track Deletion";

        int answer = wxMessageBox(prompt,caption,wxYES_NO);
        if(answer == wxYES)
        {
            mSequenceElements->DeleteElement(element->GetModelName());
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        }
    }
    else if (id == ID_ROW_MNU_EXPORT_TIMING_TRACK) {
        wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
        wxString filetypes;
        if (mSequenceElements->GetElement(element->GetName())->GetEffectLayerCount() == 3)
        {
            filetypes = "Timing files (*.xtiming)|*.xtiming|Papagayo files (*.pgo)|*.pgo";
        }
        else
        {
            filetypes = "Timing files (*.xtiming)|*.xtiming";
        }
        wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, element->GetModelName(), wxEmptyString, filetypes, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (filename.IsEmpty()) return;
        wxFileName fn(filename);
        TimingElement *te = dynamic_cast<TimingElement *>(element);
        if (fn.GetExt().Lower() == "xtiming")
        {
            SelectTimingsDialog dlg(this);

            for (int i = 0; i < mSequenceElements->GetNumberOfTimingElements(); i++)
            {
                TimingElement* te2 = mSequenceElements->GetTimingElement(i);
                dlg.CheckListBox_Timings->Insert(te2->GetName(), i);
                if (te2->GetName() == element->GetName())
                {
                    dlg.CheckListBox_Timings->Check(i);
                }
            }

            if (mSequenceElements->GetNumberOfTimingElements() == 1  || dlg.ShowModal() == wxID_OK)
            {
                wxArrayInt sel;
                dlg.CheckListBox_Timings->GetCheckedItems(sel);

                if (sel.size() > 0)
                {
                    wxFile f(filename);
                    logger_base.info("Saving to xtiming file %s.", (const char *)filename.c_str());
                    if (!f.Create(filename, true) || !f.IsOpened())
                    {
                        logger_base.info("Unable to create file %s. Error %d\n", (const char *)filename.c_str(), f.GetLastError());
                        wxMessageBox(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()));
                        return;
                    }
                    wxString v = xlights_version_string;
                    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
                    if (sel.size() > 1)
                    {
                        f.Write("<timings>\n");
                    }
                    for (int i = 0; i < dlg.CheckListBox_Timings->GetCount(); i++)
                    {
                        if (dlg.CheckListBox_Timings->IsChecked(i))
                        {
                            TimingElement *tee = dynamic_cast<TimingElement *>(mSequenceElements->GetElement(dlg.CheckListBox_Timings->GetString(i).ToStdString()));

                            wxString td = wxString(tee->GetExport().c_str());
                            f.Write("<timing ");
                            f.Write(wxString::Format("name=\"%s\" ", tee->GetName()));
                            f.Write(wxString::Format("SourceVersion=\"%s\">\n", v));
                            f.Write(td);
                            f.Write("</timing>\n");
                        }
                    }
                    if (sel.size() > 1)
                    {
                        f.Write("</timings>\n");
                    }
                    f.Close();
                }
            }
        }
        else if (fn.GetExt().Lower() == "pgo")
        {
            wxFile f(filename);
            logger_base.info("Saving to papagayo file %s.", (const char *)filename.c_str());
            if (!f.Create(filename, true) || !f.IsOpened())
            {
                logger_base.info("Unable to create file %s. Error %d\n", (const char *)filename.c_str(), f.GetLastError());
                wxMessageBox(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()));
                return;
            }
            wxString td = wxString(te->GetPapagayoExport(mSequenceElements->GetFrequency()).c_str());
            f.Write(td);
            f.Close();
        }
    } else if(id == ID_ROW_MNU_IMPORT_TIMING_TRACK) {
        wxCommandEvent playEvent(EVT_IMPORT_TIMING);
        wxPostEvent(GetParent(), playEvent);
    }
    else if (id == ID_ROW_MNU_IMPORT_NOTES) {
        wxCommandEvent importNotesEvent(EVT_IMPORT_NOTES);
        wxPostEvent(GetParent(), importNotesEvent);
    } else if(id == ID_ROW_MNU_IMPORT_LYRICS) {
        mSequenceElements->ImportLyrics(dynamic_cast<TimingElement *>(element), GetParent());
    } else if(id == ID_ROW_MNU_BREAKDOWN_TIMING_PHRASES) {
        int result = wxMessageBox("Breakdown phrases? Any existing words and phonemes will be deleted.", "Confirm Action", wxOK | wxCANCEL | wxCENTER);
        if (result == wxOK) {
            BreakdownTimingPhrases(dynamic_cast<TimingElement *>(element));
        }
    } else if(id == ID_ROW_MNU_BREAKDOWN_TIMING_WORDS) {
        int result = wxMessageBox("Breakdown words? Any existing phonemes will be deleted.", "Confirm Action", wxOK | wxCANCEL | wxCENTER);
        if (result == wxOK) {
            BreakdownTimingWords(dynamic_cast<TimingElement *>(element));
        }
    } else if (id == ID_ROW_MNU_EXPORT_MODEL) {
        wxCommandEvent playEvent(EVT_EXPORT_MODEL);
        playEvent.SetInt(0);
        playEvent.SetString(element->GetModelName());
        wxPostEvent(GetParent(), playEvent);
    } else if (id == ID_ROW_MNU_EXPORT_RENDERED_MODEL) {
        wxCommandEvent playEvent(EVT_EXPORT_MODEL);
        playEvent.SetInt(1);
        playEvent.SetString(element->GetModelName());
        wxPostEvent(GetParent(), playEvent);
    } else if (id == ID_ROW_MNU_PLAY_MODEL) {
        wxCommandEvent playEvent(EVT_PLAY_MODEL);
        playEvent.SetString(element->GetModelName());
        wxPostEvent(GetParent(), playEvent);
    } else if (id == ID_ROW_MNU_COPY_ROW) {
        wxCommandEvent copyRowEvent(EVT_COPY_MODEL_EFFECTS);
        copyRowEvent.SetInt(mSelectedRow);
        wxPostEvent(GetParent(), copyRowEvent);
        mCanPaste = true;
    } else if (id == ID_ROW_MNU_DELETE_ROW_EFFECTS) {
        wxCommandEvent eventUnSelected(EVT_UNSELECTED_EFFECT);
        m_parent->ProcessWindowEvent(eventUnSelected);
        mSequenceElements->get_undo_mgr().CreateUndoStep();
        if (layer_index < element->GetEffectLayerCount())
        {
            element->GetEffectLayer(layer_index)->RemoveAllEffects(&mSequenceElements->get_undo_mgr());
        }
    }
    else if (id == ID_ROW_MNU_SELECT_ROW_EFFECTS) {
        if (layer_index < element->GetEffectLayerCount())
        {
            element->GetEffectLayer(layer_index)->SelectAllEffects();
        }
    } else if (id == ID_ROW_MNU_DELETE_MODEL_EFFECTS) {
        wxCommandEvent eventUnSelected(EVT_UNSELECTED_EFFECT);
        m_parent->ProcessWindowEvent(eventUnSelected);
        mSequenceElements->get_undo_mgr().CreateUndoStep();
        for (int i = 0; i < element->GetEffectLayerCount(); ++i)
        {
            while (element->GetEffectLayer(i)->GetEffectCount() > 0)
            {
                element->GetEffectLayer(i)->RemoveAllEffects(&mSequenceElements->get_undo_mgr());
            }
        }
    } else if (id == ID_ROW_MNU_PASTE_ROW) {
        wxCommandEvent pasteRowEvent(EVT_PASTE_MODEL_EFFECTS);
        pasteRowEvent.SetInt(mSelectedRow);
        wxPostEvent(GetParent(), pasteRowEvent);
    } else if(id==ID_ROW_MNU_EDIT_DISPLAY_ELEMENTS) {
        wxCommandEvent displayElementEvent(EVT_SHOW_DISPLAY_ELEMENTS);
        wxPostEvent(GetParent(), displayElementEvent);
    }
    else if (id == ID_ROW_MNU_TOGGLE_STRANDS) {
        ModelElement *me = dynamic_cast<ModelElement *>(element);
        if (me == nullptr) {
            SubModelElement *se = dynamic_cast<SubModelElement *>(element);
            me = se->GetModelElement();
        }
        if (me != nullptr)
        {
            me->ShowStrands(!me->ShowStrands());
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            eventRowHeaderChanged.SetString(element->GetModelName());
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        }
    } else if (id == ID_ROW_MNU_SHOW_EFFECTS) {
        logger_base.debug("RowHeading::OnLayerPopup Show effects.");
        int view = mSequenceElements->GetCurrentView();
        for (int i = 0; i < mSequenceElements->GetElementCount(view); ++i)
        {
            Element* e = mSequenceElements->GetElement(i, view);
            if (e->GetType() != ELEMENT_TYPE_TIMING)
            {
                if (ExpandElementIfEffects(e))
                {
                    ModelElement* me = dynamic_cast<ModelElement*>(e);
                    if (me != nullptr)
                        me->ShowStrands(true);
                }
            }
        }
    }
    else if (id == ID_ROW_MNU_COLLAPSEALL) {
        logger_base.debug("RowHeading::OnLayerPopup Collapse all.");
        int view = mSequenceElements->GetCurrentView();

        for (int i = 0;  i < mSequenceElements->GetElementCount(view); ++i)
        {
            Element* e = mSequenceElements->GetElement(i, view);
            if (e->GetType() != ELEMENT_TYPE_TIMING)
            {
                ModelElement* me = dynamic_cast<ModelElement*>(e);
                if (me != nullptr)
                {
                    me->ShowStrands(false);
                }
            }
        }
        wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
        wxPostEvent(GetParent(), eventRowHeaderChanged);
    } else if (id == ID_ROW_MNU_TOGGLE_NODES) {
        StrandElement *se = dynamic_cast<StrandElement *>(element);
        if (se != nullptr)
        {
            se->ShowNodes(!se->ShowNodes());
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        }
    } else if (id == ID_ROW_MNU_CONVERT_TO_EFFECTS) {
        wxCommandEvent evt(EVT_CONVERT_DATA_TO_EFFECTS);
        evt.SetClientData(element);
        int i = ((ri->strandIndex << 16) & 0xFFFF0000) + ri->nodeIndex;
        evt.SetInt(i);
        wxPostEvent(GetParent(), evt);
    } else if (id == ID_ROW_MNU_PROMOTE_EFFECTS) {
        wxCommandEvent evt(EVT_PROMOTE_EFFECTS);
        SubModelElement *se = dynamic_cast<SubModelElement *>(element);
        if (se != nullptr) {
            evt.SetClientData(se->GetModelElement());
        } else {
            evt.SetClientData(element);
        }
        wxPostEvent(GetParent(), evt);
    }

    // Make sure message box is painted over by grid.
    wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
    wxPostEvent(GetParent(), eventForceRefresh);
}

bool RowHeading::ExpandElementIfEffects(Element* e)
{
    if (e == nullptr) return false;

    bool hasEffects = false;

	if (e->GetCollapsed())
	{
		for (int layer = 0; layer<e->GetEffectLayerCount(); layer++)
		{
			EffectLayer* el = e->GetEffectLayer(layer);
			if (el->GetEffectCount() > 0)
			{
				e->SetCollapsed(false);
				break;
			}
		}
	}

    if (e->GetType() == ELEMENT_TYPE_MODEL)
    {
        ModelElement *me = dynamic_cast<ModelElement *>(e);
        Model *m = mSequenceElements->GetXLightsFrame()->AllModels[me->GetModelName()];

        if (m->GetDisplayAs() == "ModelGroup")
        {
            int view = mSequenceElements->GetCurrentView();
            ModelGroup* mg = dynamic_cast<ModelGroup*>(m);
            auto models = mg->ModelNames();
            for (auto it = models.begin(); it != models.end(); ++it)
            {
                ModelElement* mm = dynamic_cast<ModelElement*>(mSequenceElements->GetElement(*it));

                if (mm != nullptr && !ModelInView(*it, view))
                {
                    hasEffects = mm->HasEffects();
                    hasEffects |= ExpandElementIfEffects(mSequenceElements->GetElement(*it));

                    if (hasEffects) me->ShowStrands(true);
                }
            }
        }
        else
        {
            for (int i = 0; i < me->GetStrandCount(); ++i)
            {
                hasEffects |= ExpandElementIfEffects(me->GetStrand(i));
            }
            for (int i = 0; i < me->GetSubModelAndStrandCount(); ++i)
            {
                hasEffects |= ExpandElementIfEffects(me->GetSubModel(i));
            }

            if (hasEffects) me->ShowStrands(true);
        }
    }
    else if (e->GetType() == ELEMENT_TYPE_STRAND)
    {
        StrandElement* se = dynamic_cast<StrandElement*>(e);
        hasEffects = se->HasEffects();
        for (int k = 0; k < se->GetNodeLayerCount(); ++k)
        {
            NodeLayer* nl = se->GetNodeLayer(k, false);
            if (nl != nullptr && nl->HasEffectsInTimeRange(0, 9999999))
            {
                se->ShowNodes(true);
                return true;
            }
        }
    }
    else
    {
        // Submodel
        SubModelElement* sme = dynamic_cast<SubModelElement*>(e);
        hasEffects = sme->HasEffects();
    }

    return hasEffects;
}

bool RowHeading::ModelInView(const std::string& model, int view) const
{
    for (int j = 0; j < mSequenceElements->GetElementCount(view); ++j)
    {
        if (model == mSequenceElements->GetElement(j, view)->GetName())
        {
            return true;
        }
    }

    return false;
}

void RowHeading::BreakdownTimingPhrases(TimingElement* element)
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

void RowHeading::BreakdownTimingWords(TimingElement* element)
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
    if(mSequenceElements->GetVisibleRowInformation(row)->element->GetType() != ELEMENT_TYPE_TIMING &&
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
    if (mSequenceElements->GetVisibleRowInformation(row)->element->GetType() == ELEMENT_TYPE_TIMING
        && x < DEFAULT_ROW_HEADING_MARGIN) {

        *IsActive = dynamic_cast<TimingElement*>(mSequenceElements->GetVisibleRowInformation(row)->element)->GetActive();
        return true;
    }
    return false;
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
    wxPen penOutline(wxColor(32,32,32), 0.1);
    dc.GetSize(&w,&h);
    wxBrush brush(ColorManager::instance()->GetColor(ColorManager::COLOR_ROW_HEADER).asWxColor(), wxBRUSHSTYLE_SOLID);
    dc.SetBrush(brush);
    dc.SetPen(penOutline);
    int row = 0;
    int endY = 0;

    for (int i = 0; i< mSequenceElements->GetVisibleRowInformationSize(); i++)
    {
        Row_Information_Struct* rowInfo = mSequenceElements->GetVisibleRowInformation(i);
        wxString prefix;
        wxString layers;
        if (rowInfo->submodel) {
            prefix = "  ";
        }
        if (rowInfo->element->GetType() != ELEMENT_TYPE_TIMING && rowInfo->element->GetEffectLayerCount() > 1)
        {
            layers = wxString::Format(" [%d]", (int)rowInfo->element->GetEffectLayerCount());
        }
        wxBrush brush2(GetHeaderColor(rowInfo).asWxColor(),wxBRUSHSTYLE_SOLID);
        dc.SetBrush(brush2);
        int startY = DEFAULT_ROW_HEADING_HEIGHT*row;
        endY = DEFAULT_ROW_HEADING_HEIGHT*(row+1);
        dc.SetBackgroundMode(wxTRANSPARENT);
        dc.DrawRectangle(0,startY,w,DEFAULT_ROW_HEADING_HEIGHT);
        if(rowInfo->layerIndex > 0 || rowInfo->strandIndex >= 0)   // If effect layer = 0
        {
            dc.SetPen(*wxLIGHT_GREY_PEN);
            dc.DrawLine(1,startY,w-1,startY);
            dc.DrawLine(1,startY-1,w-1,startY-1);
            dc.SetPen(*wxBLACK_PEN);
            if (rowInfo->strandIndex >= 0) {
                wxRect r(DEFAULT_ROW_HEADING_MARGIN,startY,w-DEFAULT_ROW_HEADING_MARGIN,DEFAULT_ROW_HEADING_HEIGHT);
                wxString name = rowInfo->displayName;
                if (name == "") {
                    if (rowInfo->nodeIndex >= 0) {
                        name = wxString::Format("Node %d", rowInfo->nodeIndex + 1);
                        layers = "";
                    } else {
                        name = wxString::Format("Strand %d", rowInfo->strandIndex + 1);
                    }
                }
                if (rowInfo->nodeIndex >= 0) {
                    dc.DrawLabel(prefix + "     " + name + layers,r,wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT);
                } else if (rowInfo->layerIndex == 0) {
                    dc.DrawLabel(prefix + "  " + name + layers,r,wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT);
                }
                else
                {
                    dc.SetPen(*wxBLUE_PEN);
                    wxString lay = wxString::Format("   [%d]", rowInfo->layerIndex + 1);
                    dc.DrawLabel(lay, r, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT);
                }
            }
            else
            {
                wxRect r(DEFAULT_ROW_HEADING_MARGIN, startY, w - DEFAULT_ROW_HEADING_MARGIN, DEFAULT_ROW_HEADING_HEIGHT);
                dc.SetPen(*wxBLUE_PEN);
                wxString lay = wxString::Format("   [%d]", rowInfo->layerIndex + 1);
                dc.DrawLabel(lay, r, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT);
            }
            dc.SetPen(penOutline);
            dc.SetBrush(brush2);
        }
        else        // Draw label
        {
            if (rowInfo->element->GetType() == ELEMENT_TYPE_SUBMODEL) {
                prefix += "  ";
            }
            wxRect r(DEFAULT_ROW_HEADING_MARGIN,startY,w-DEFAULT_ROW_HEADING_MARGIN,DEFAULT_ROW_HEADING_HEIGHT);
            dc.DrawLabel(prefix + rowInfo->element->GetName() + layers,r,wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT);
        }

        if (rowInfo->element->GetType() != ELEMENT_TYPE_TIMING)
        {
            if(rowInfo->element->GetEffectLayerCount() > 1
               && rowInfo->layerIndex == 0
               && rowInfo->nodeIndex == -1)
            {
                dc.SetBrush(*wxWHITE_BRUSH);
                dc.SetPen(*wxBLACK_PEN);
                dc.DrawRectangle(2,startY + DEFAULT_ROW_HEADING_HEIGHT/2 - 4,9,9);
                dc.DrawLine(2,startY + DEFAULT_ROW_HEADING_HEIGHT/2,9,startY + DEFAULT_ROW_HEADING_HEIGHT/2);
                if(rowInfo->Collapsed)
                {
                    dc.DrawLine(6,startY + DEFAULT_ROW_HEADING_HEIGHT/2 + 4,6,startY + DEFAULT_ROW_HEADING_HEIGHT/2 - 4);
                }
                dc.SetPen(penOutline);
                dc.SetBrush(brush2);
            }
            // draw Model Group icon if necessary
            Model *m = mSequenceElements->GetXLightsFrame()->AllModels[rowInfo->element->GetModelName()];
            if (m != nullptr)
            {
                if (m->GetDisplayAs() == "ModelGroup")
                {
                    dc.DrawBitmap(model_group_icon, getWidth() - ICON_SPACE, startY + 3, true);
                }
                else if (m->GetStringType().find("Single Color") == 0)
                {
                    if (m->GetNodeCount() > 0)
                    {
                        xlColor color;
                        if (m->GetDisplayAs() == "Channel Block")
                        {
                            StrandElement* se = dynamic_cast<StrandElement*>(rowInfo->element);
                            if (se != nullptr)
                            {
                                color = m->GetNodeMaskColor(se->GetStrand());
                            }
                            else
                            {
                                color = xlWHITE;
                            }
                        }
                        else
                        {
                            color = m->GetNodeMaskColor(0);
                        }
                        dc.SetPen(*wxBLACK_PEN);
                        dc.SetBrush(wxBrush(color.asWxColor()));
                        dc.DrawRectangle(getWidth() - 21, startY + 5, 12, 12);
                        dc.SetPen(penOutline);
                        dc.SetBrush(brush2);
                    }
                }
            }
        }
        else if(rowInfo->element->GetType()==ELEMENT_TYPE_TIMING)
        {
            if( rowInfo->layerIndex == 0 )
            {
                dc.SetPen(*wxBLACK_PEN);
                if(dynamic_cast<TimingElement*>(rowInfo->element)->GetActive())
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
                dc.SetBrush(brush2);
                if(rowInfo->element->GetEffectLayerCount() == 2)
                {
                    dc.DrawBitmap(papagayox_icon, getWidth() - ICON_SPACE, startY + 3, true);
                }
                else if(rowInfo->element->GetEffectLayerCount() > 2)
                {
                    dc.DrawBitmap(papagayo_icon, getWidth() - ICON_SPACE, startY + 3, true);
                }
            }
        }
        row++;
    }
    wxBrush b(ColorManager::instance()->GetColor(ColorManager::COLOR_ROW_HEADER).asWxColor(),wxBRUSHSTYLE_SOLID);
    dc.SetBrush(b);
    dc.DrawRectangle(0,endY,w,h);
}

xlColor RowHeading::GetHeaderColor(Row_Information_Struct* info) const
{
    if (info->element->GetType() == ELEMENT_TYPE_TIMING)
    {
        return ColorManager::instance()->GetTimingColor(info->colorIndex);
    }

    if (info->RowNumber == mSelectedRow )
    {
        return  ColorManager::instance()->GetColor(ColorManager::COLOR_ROW_HEADER_SELECTED);
    }
    else
    {
        return ColorManager::instance()->GetColor(ColorManager::COLOR_ROW_HEADER);
    }
}

void RowHeading::SetSequenceElements(SequenceElements* elements)
{
    mSequenceElements = elements;
}

void RowHeading::DrawHeading(wxPaintDC* dc, wxXmlNode* model,int width,int row)
{
}

int RowHeading::getWidth() const
{
    return GetSize().x;
}

int RowHeading::getHeight() const
{
    return GetSize().y;
}


