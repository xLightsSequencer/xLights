/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "wx/wx.h"
#include "wx/glcanvas.h"
#include <wx/textdlg.h>
#include <wx/numdlg.h>
#ifdef __WXMAC__
 #include "OpenGL/gl.h"
#else
  #include <GL/gl.h>
#endif

#include "EffectsGrid.h"
#include "MainSequencer.h"
#include "TimeLine.h"
#include  "RowHeading.h"
#include "../xLightsMain.h"
#include "SequenceElements.h"
#include "Effect.h"
#include "EffectLayer.h"
#include "EffectDropTarget.h"
#include "../DrawGLUtils.h"
#include "../RenderCommandEvent.h"
#include "../BitmapCache.h"
#include "../effects/RenderableEffect.h"
#include "../UtilFunctions.h"
#include "../xLightsXmlFile.h"
#include "EffectTimingDialog.h"
#include "effects/GlediatorEffect.h"
#include "effects/VideoEffect.h"
#include "effects/PicturesEffect.h"
#include "ColorManager.h"
#include "models/Model.h"
#include "PixelBuffer.h"
#include "../VideoReader.h"

#include <log4cpp/Category.hh>

#define EFFECT_RESIZE_NO                    0
#define EFFECT_RESIZE_LEFT                  1
#define EFFECT_RESIZE_RIGHT                 2
#define EFFECT_RESIZE_MOVE                  3
#define EFFECT_RESIZE_LEFT_EDGE             4
#define EFFECT_RESIZE_RIGHT_EDGE            5

BEGIN_EVENT_TABLE(EffectsGrid, xlGLCanvas)
EVT_MOTION(EffectsGrid::mouseMoved)
EVT_MAGNIFY(EffectsGrid::magnify)
EVT_MOUSEWHEEL(EffectsGrid::mouseWheelMoved)
EVT_LEFT_DOWN(EffectsGrid::mouseDown)
EVT_LEFT_UP(EffectsGrid::mouseReleased)
EVT_MOUSE_CAPTURE_LOST(EffectsGrid::OnLostMouseCapture)
EVT_RIGHT_DOWN(EffectsGrid::rightClick)
EVT_LEFT_DCLICK(EffectsGrid::mouseLeftDClick)
EVT_LEAVE_WINDOW(EffectsGrid::mouseLeftWindow)
EVT_PAINT(EffectsGrid::render)
END_EVENT_TABLE()

// Menu constants
const long EffectsGrid::ID_GRID_MNU_CUT = wxNewId();
const long EffectsGrid::ID_GRID_MNU_COPY = wxNewId();
const long EffectsGrid::ID_GRID_MNU_PASTE = wxNewId();
const long EffectsGrid::ID_GRID_MNU_DELETE = wxNewId();
const long EffectsGrid::ID_GRID_MNU_RANDOM_EFFECTS = wxNewId();
const long EffectsGrid::ID_GRID_MNU_DESCRIPTION = wxNewId();
const long EffectsGrid::ID_GRID_MNU_LOCK = wxNewId();
const long EffectsGrid::ID_GRID_MNU_UNLOCK = wxNewId();
const long EffectsGrid::ID_GRID_MNU_TIMING = wxNewId();
const long EffectsGrid::ID_GRID_MNU_UNDO = wxNewId();
const long EffectsGrid::ID_GRID_MNU_REDO = wxNewId();
const long EffectsGrid::ID_GRID_MNU_PRESETS = wxNewId();
const long EffectsGrid::ID_GRID_MNU_BREAKDOWN_PHRASE = wxNewId();
const long EffectsGrid::ID_GRID_MNU_HALVETIMINGS = wxNewId();
const long EffectsGrid::ID_GRID_MNU_BREAKDOWN_WORD = wxNewId();
const long EffectsGrid::ID_GRID_MNU_BREAKDOWN_WORDS = wxNewId();
const long EffectsGrid::ID_GRID_MNU_ALIGN_START_TIMES = wxNewId();
const long EffectsGrid::ID_GRID_MNU_ALIGN_END_TIMES = wxNewId();
const long EffectsGrid::ID_GRID_MNU_ALIGN_BOTH_TIMES = wxNewId();
const long EffectsGrid::ID_GRID_MNU_ALIGN_CENTERPOINTS = wxNewId();
const long EffectsGrid::ID_GRID_MNU_ALIGN_MATCH_DURATION = wxNewId();
const long EffectsGrid::ID_GRID_MNU_ALIGN_START_TIMES_SHIFT = wxNewId();
const long EffectsGrid::ID_GRID_MNU_ALIGN_END_TIMES_SHIFT = wxNewId();

EffectsGrid::EffectsGrid(MainSequencer* parent, wxWindowID id, const wxPoint &pos, const wxSize &size,
                       long style, const wxString &name)
    :xlGLCanvas(parent, id, pos, size, wxFULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN | wxCLIP_SIBLINGS, "EffectGrid")
{
    mParent = parent;
    mDragging = false;
    mResizing = false;
    mDragDropping = false;
    mDropStartX = 0;
    mDropEndX = 0;
    mCellRangeSelected = false;
    mPartialCellSelected = false;
    mDragStartRow = 0;
    mDragStartX = -1;
    mDragStartY = -1;
    mSelectedEffect = nullptr;
    mRangeStartRow = -1;
    mRangeEndRow = -1;
    mRangeStartCol = -1;
    mRangeEndCol = -1;
    mRangeCursorRow = -1;
    mRangeCursorCol = -1;
    mResizeEffectIndex = -1;
    mTimeline = nullptr;
    magSinceLast = 0.0f;
    mStartPixelOffset = 0;

    SetBackgroundStyle(wxBG_STYLE_CUSTOM);

    SetDropTarget(new EffectDropTarget(this));
    playArgs = new EventPlayEffectArgs();
    mSequenceElements = nullptr;
    xlights = nullptr;
    SetRCToolTip();
}

EffectsGrid::~EffectsGrid()
{
    delete playArgs;
}

void EffectsGrid::UnselectEffect(bool force) {
    if (mSelectedEffect != nullptr || force) {
        mSelectedEffect = nullptr;
        wxCommandEvent eventUnSelected(EVT_UNSELECTED_EFFECT);
        mParent->ProcessWindowEvent(eventUnSelected);
        Refresh(false);
    }
}

EffectLayer* EffectsGrid::FindOpenLayer(Element* elem, int startTimeMS, int endTimeMS)
{
    EffectLayer* layer;

    // need to search for open layer
    for( size_t i = 0; i < elem->GetEffectLayerCount(); i++ )
    {
            layer = elem->GetEffectLayer(i);
            if( layer->GetRangeIsClearMS(startTimeMS, endTimeMS) )
            {
                return layer;
            }
    }

    // empty layer not found so create a new one
    layer = elem->AddEffectLayer();

    return layer;
}

void EffectsGrid::mouseLeftDClick(wxMouseEvent& event)
{
    int update_time = -1;

    if (mSequenceElements == nullptr) {
        return;
    }
    int selectedTimeMS = mTimeline->GetAbsoluteTimeMSfromPosition(event.GetX());

    if (!event.ShiftDown()) {
        update_time = selectedTimeMS;
    }

    int row = GetRow(event.GetY());
    if(row>=mSequenceElements->GetVisibleRowInformationSize() || row < 0)
    {
        if( update_time > -1 )
        {
            UpdateTimePosition(update_time);
        }
        return;
    }
    int effectIndex;
    HitLocation selectionType = HitLocation::NONE;
    Effect* selectedEffect = GetEffectAtRowAndTime(row,selectedTimeMS,effectIndex,selectionType);
    if (selectedEffect != nullptr)
    {
        if ((mTimingPlayOnDClick && event.ShiftDown()) ||
             (!mTimingPlayOnDClick && !event.ShiftDown())) {
            if (selectedEffect->GetParentEffectLayer()->GetParentElement()->GetType() == ElementType::ELEMENT_TYPE_TIMING ){
                if (selectedEffect->GetParentEffectLayer()->IsFixedTimingLayer())
                {
                    if (wxMessageBox("Cannot Add Labels to a Fixed Timing Track.\nWould You Like to convert it to a Varible Timing Track First?", "Convert Fixed Timing Track First", wxYES_NO) == wxYES)
                    {
                        TimingElement* te = dynamic_cast<TimingElement*>(selectedEffect->GetParentEffectLayer()->GetParentElement());
                        te->SetFixedTiming(0);
                    }
                    else
                    {
                        if (update_time > -1)
                        {
                            UpdateTimePosition(update_time);
                        }
                        return;
                    }
                }
                wxString label = selectedEffect->GetEffectName();

                wxTextEntryDialog dlg(this, "Edit Label", "Enter new label:", label);
                OptimiseDialogPosition(&dlg);
                if (dlg.ShowModal()) {

                    wxString newLabel = dlg.GetValue();
                    newLabel = newLabel.Trim(true).Trim(false); // remove leading spaces
                    //newLabel.Replace("\"", ""); // remove illegal characters
                    //newLabel.Replace("<", ""); // remove illegal characters
                    //newLabel.Replace(">", ""); // remove illegal characters

                    selectedEffect->SetEffectName(newLabel.ToStdString());

                    // If we have a fixed timing track and we add a label then we can no longer store it as a fixed timing track
                    TimingElement* te = dynamic_cast<TimingElement*>(selectedEffect->GetParentEffectLayer()->GetParentElement());
                    if (dlg.GetValue().ToStdString() != "" && te->GetFixedTiming() != 0)
                    {
                        te->SetFixedTiming(0);
                    }
                }
                Refresh();
            }
            else
            {
                if( update_time > -1 )
                {
                    UpdateTimePosition(update_time);
                }
            }
        } else {
            // we have double clicked on an effect - highlight that part of the waveform
            ((MainSequencer*)mParent)->PanelWaveForm->SetSelectedInterval(selectedEffect->GetStartTimeMS(), selectedEffect->GetEndTimeMS());
            Refresh();
            // and play it play mode is active
            if( mTimingPlayOnDClick ) {
                wxCommandEvent playEvent(EVT_PLAY_SEQUENCE);
                wxPostEvent(mParent, playEvent);
            }
            else
            {
                if( update_time > -1 )
                {
                    UpdateTimePosition(update_time);
                }
            }
        }
    }
    else
    {
        if( update_time > -1 )
        {
            UpdateTimePosition(update_time);
        }
    }
}

void EffectsGrid::rightClick(wxMouseEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (mSequenceElements == nullptr) {
        return;
    }
    SetFocus();
    mSelectedRow = event.GetY()/DEFAULT_ROW_HEADING_HEIGHT;
    if (mSelectedRow >= mSequenceElements->GetVisibleRowInformationSize()) {
        return;
    }

    Row_Information_Struct *ri =  mSequenceElements->GetVisibleRowInformation(mSelectedRow);

    if (ri == nullptr)
        logger_base.crit("EffectsGrid::rightClick No row information ... this is not going to end well.");

    Element* element = ri->element;
    if (element == nullptr)
        logger_base.crit("EffectsGrid::rightClick No row element ... this is not going to end well.");
    if (element->GetType() != ElementType::ELEMENT_TYPE_TIMING)
    {
        wxMenu mnuLayer;
        // Copy / Paste / Delete
        wxMenuItem* menu_cut = mnuLayer.Append(ID_GRID_MNU_CUT, "Cut");
        wxMenuItem* menu_copy = mnuLayer.Append(ID_GRID_MNU_COPY, "Copy");
        wxMenuItem* menu_paste = mnuLayer.Append(ID_GRID_MNU_PASTE,"Paste");
        wxMenuItem* menu_delete = mnuLayer.Append(ID_GRID_MNU_DELETE,"Delete");
        if( (mSelectedEffect == nullptr && !AtLeastOneEffectSelected()) &&
            !(IsACActive() && mCellRangeSelected)) {
            menu_cut->Enable(false);
            menu_copy->Enable(false);
            menu_delete->Enable(false);
        }
        if (!(mCellRangeSelected || mPartialCellSelected) ) {
            menu_paste->Enable(false);
        }

        // Undo
        mnuLayer.AppendSeparator();
        wxString undo_string = mSequenceElements->get_undo_mgr().GetUndoString();
        wxMenuItem* menu_undo = mnuLayer.Append(ID_GRID_MNU_UNDO,undo_string);
        if( !mSequenceElements->get_undo_mgr().CanUndo() ) {
            menu_undo->Enable(false);
        }

        // Redo
        wxString redo_string = mSequenceElements->get_undo_mgr().GetRedoString();
        wxMenuItem* menu_redo = mnuLayer.Append(ID_GRID_MNU_REDO,redo_string);
        if( !mSequenceElements->get_undo_mgr().CanRedo() ) {
            menu_redo->Enable(false);
        }

        // Alignment
        mnuLayer.AppendSeparator();
        wxMenu *mnuAlignment = new wxMenu();
        wxMenuItem* menu_align_start_times = mnuAlignment->Append(ID_GRID_MNU_ALIGN_START_TIMES,"Align Start Times");
        wxMenuItem* menu_align_end_times = mnuAlignment->Append(ID_GRID_MNU_ALIGN_END_TIMES,"Align End Times");
        wxMenuItem* menu_align_both_times = mnuAlignment->Append(ID_GRID_MNU_ALIGN_BOTH_TIMES,"Align Both Times");
        wxMenuItem* menu_align_centerpoints = mnuAlignment->Append(ID_GRID_MNU_ALIGN_CENTERPOINTS,"Align Centerpoints");
        wxMenuItem* menu_align_match_duration = mnuAlignment->Append(ID_GRID_MNU_ALIGN_MATCH_DURATION,"Align Match Duration");
        wxMenuItem* menu_align_start_times_shift = mnuAlignment->Append(ID_GRID_MNU_ALIGN_START_TIMES_SHIFT,"Shift Align Start Times");
        wxMenuItem* menu_align_end_times_shift = mnuAlignment->Append(ID_GRID_MNU_ALIGN_END_TIMES_SHIFT,"Shift Align End Times");
        mnuAlignment->Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EffectsGrid::OnGridPopup, nullptr, this);
        mnuLayer.AppendSubMenu(mnuAlignment, "Alignment" );
        if( (mSelectedEffect == nullptr) || !MultipleEffectsSelected() ) {
            menu_align_start_times->Enable(false);
            menu_align_end_times->Enable(false);
            menu_align_both_times->Enable(false);
            menu_align_centerpoints->Enable(false);
            menu_align_match_duration->Enable(false);
            menu_align_start_times_shift->Enable(false);
            menu_align_end_times_shift->Enable(false);
        }

        // Miscellaneous
        mnuLayer.AppendSeparator();
        mnuLayer.Append(ID_GRID_MNU_PRESETS,"Effect Presets");
        wxMenuItem* menu_random = mnuLayer.Append(ID_GRID_MNU_RANDOM_EFFECTS,"Create Random Effects");
        if( !(mCellRangeSelected || mPartialCellSelected) ) {
            menu_random->Enable(false);
        }

        wxMenuItem* menu_effect_description = mnuLayer.Append(ID_GRID_MNU_DESCRIPTION, "Description");
        wxMenuItem* menu_effect_lock = mnuLayer.Append(ID_GRID_MNU_LOCK, "Lock");
        wxMenuItem* menu_effect_unlock = mnuLayer.Append(ID_GRID_MNU_UNLOCK, "Unlock");
        if (mSelectedEffect == nullptr && !AtLeastOneEffectSelected())
        {
            menu_effect_description->Enable(false);
            menu_effect_unlock->Enable(false);
            menu_effect_lock->Enable(false);
        }

        wxMenuItem* menu_effect_timing = mnuLayer.Append(ID_GRID_MNU_TIMING, "Timing");
        if (mSelectedEffect == nullptr ||
            MultipleEffectsSelected() ||
            (mSelectedEffect != nullptr && mSelectedEffect->IsLocked()))
        {
            menu_effect_timing->Enable(false);
        }

        mnuLayer.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EffectsGrid::OnGridPopup, nullptr, this);
        Draw();
        PopupMenu(&mnuLayer);
    }
    else
    {
        wxMenu mnuLayer;
        int effectIndex;
        HitLocation selectionType;
        int startTime = mTimeline->GetAbsoluteTimeMSfromPosition(event.GetX());
        Effect* selectedEffect = GetEffectAtRowAndTime(mSelectedRow,startTime,effectIndex,selectionType);
        if (selectedEffect != nullptr && selectedEffect->GetParentEffectLayer()->GetParentElement()->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
            if( ri->layerIndex == 0 )
            {
                mnuLayer.Append(ID_GRID_MNU_BREAKDOWN_PHRASE,"Breakdown Phrase");
            }
            else if( ri->layerIndex == 1 )
            {
                mnuLayer.Append(ID_GRID_MNU_BREAKDOWN_WORD,"Breakdown Word");
                if (selectedEffect->GetParentEffectLayer()->GetSelectedEffectCount() > 1) {
                    mnuLayer.Append(ID_GRID_MNU_BREAKDOWN_WORDS,"Breakdown Selected Words");
                }
            }
            mnuLayer.Append(ID_GRID_MNU_HALVETIMINGS, "Divide Timings")->Enable(!selectedEffect->GetParentEffectLayer()->IsFixedTimingLayer());
            mSelectedEffect = selectedEffect;
        }
        mnuLayer.AppendSeparator();
        wxMenuItem* menu_effect_lock = mnuLayer.Append(ID_GRID_MNU_LOCK, "Lock");
        wxMenuItem* menu_effect_unlock = mnuLayer.Append(ID_GRID_MNU_UNLOCK, "Unlock");
        if (mSelectedEffect == nullptr && !AtLeastOneEffectSelected())
        {
            menu_effect_unlock->Enable(false);
            menu_effect_lock->Enable(false);
        }
        mnuLayer.AppendSeparator();
        wxMenuItem* menu_cut = mnuLayer.Append(ID_GRID_MNU_CUT,"Cut");
        wxMenuItem* menu_copy = mnuLayer.Append(ID_GRID_MNU_COPY,"Copy");
        wxMenuItem* menu_paste = mnuLayer.Append(ID_GRID_MNU_PASTE,"Paste");
        wxMenuItem* menu_delete = mnuLayer.Append(ID_GRID_MNU_DELETE,"Delete");
        if( mSelectedEffect == nullptr && !AtLeastOneEffectSelected()) {
            menu_cut->Enable(false);
            menu_copy->Enable(false);
            menu_delete->Enable(false);
        }
        if (!(mCellRangeSelected || mPartialCellSelected) ) {
            menu_paste->Enable(false);
        }
        mnuLayer.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EffectsGrid::OnGridPopup, nullptr, this);
        Draw();
        PopupMenu(&mnuLayer);
    }
}

void EffectsGrid::keyReleased(wxKeyEvent& event){}
void EffectsGrid::keyPressed(wxKeyEvent& event){}

void EffectsGrid::sendRenderDirtyEvent() {
    sendRenderEvent("", -1, -1, true);
}

void EffectsGrid::sendRenderEvent(const std::string &model, int start, int end, bool clear) {
    RenderCommandEvent event(model, start, end, clear, false);
    wxPostEvent(mParent, event);
}

void EffectsGrid::OnGridPopup(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    int id = event.GetId();
    if(id == ID_GRID_MNU_CUT)
    {
        logger_base.debug("OnGridPopup - CUT");
        ((MainSequencer*)mParent)->Cut();
    }
    else if(id == ID_GRID_MNU_COPY)
    {
        logger_base.debug("OnGridPopup - COPY");
        ((MainSequencer*)mParent)->CopySelectedEffects();
    }
    else if(id == ID_GRID_MNU_PASTE)
    {
        logger_base.debug("OnGridPopup - PASTE");
        ((MainSequencer*)mParent)->Paste();
    }
    else if(id == ID_GRID_MNU_DELETE)
    {
        logger_base.debug("OnGridPopup - DELETE");
        DeleteSelectedEffects();
    }
    else if (id == ID_GRID_MNU_DESCRIPTION)
    {
        logger_base.debug("OnGridPopup - DESCRIPTION");
        SetEffectsDescription();
    }
    else if (id == ID_GRID_MNU_LOCK)
    {
        logger_base.debug("OnGridPopup - LOCK");
        LockEffects(true);
    }
    else if (id == ID_GRID_MNU_UNLOCK)
    {
        logger_base.debug("OnGridPopup - UNLOCK");
        LockEffects(false);
    }
    else if (id == ID_GRID_MNU_TIMING)
    {
        logger_base.debug("OnGridPopup - TIMING");
        SetEffectsTiming();
    }
    else if(id == ID_GRID_MNU_RANDOM_EFFECTS)
    {
        logger_base.debug("OnGridPopup - RANDOM");
        Effect* ef = FillRandomEffects();
        SelectEffect(ef);
    }
    else if(id == ID_GRID_MNU_UNDO)
    {
        logger_base.debug("OnGridPopup - UNDO");
        mSelectedEffect = nullptr; // lets clear it as the undo may delete that effect ... and i cant be sure
        mSequenceElements->get_undo_mgr().UndoLastStep();
        sendRenderDirtyEvent();
    }
    else if(id == ID_GRID_MNU_REDO)
    {
        logger_base.debug("OnGridPopup - REDO");
        mSelectedEffect = nullptr; // lets clear it as the redo may delete that effect ... and i cant be sure
        mSequenceElements->get_undo_mgr().RedoLastStep();
        sendRenderDirtyEvent();
    }
    else if( id == ID_GRID_MNU_ALIGN_START_TIMES )
    {
        logger_base.debug("OnGridPopup - ALIGN_START");
        AlignSelectedEffects(EFF_ALIGN_MODE::ALIGN_START_TIMES);
    }
    else if( id == ID_GRID_MNU_ALIGN_END_TIMES )
    {
        logger_base.debug("OnGridPopup - ALIGN_END");
        AlignSelectedEffects(EFF_ALIGN_MODE::ALIGN_END_TIMES);
    }
    else if( id == ID_GRID_MNU_ALIGN_BOTH_TIMES )
    {
        logger_base.debug("OnGridPopup - ALIGN_BOTH");
        AlignSelectedEffects(EFF_ALIGN_MODE::ALIGN_BOTH_TIMES);
    }
    else if( id == ID_GRID_MNU_ALIGN_CENTERPOINTS )
    {
        logger_base.debug("OnGridPopup - ALIGN_CENTER");
        AlignSelectedEffects(EFF_ALIGN_MODE::ALIGN_CENTERPOINTS);
    }
    else if( id == ID_GRID_MNU_ALIGN_MATCH_DURATION )
    {
        logger_base.debug("OnGridPopup - MATCH_DURATION");
        AlignSelectedEffects(EFF_ALIGN_MODE::ALIGN_MATCH_DURATION);
    }
    else if( id == ID_GRID_MNU_ALIGN_START_TIMES_SHIFT )
    {
        logger_base.debug("OnGridPopup - ALIGN_START_TIMES_SHIFT");
        AlignSelectedEffects(EFF_ALIGN_MODE::ALIGN_START_TIMES_SHIFT);
    }
    else if( id == ID_GRID_MNU_ALIGN_END_TIMES_SHIFT )
    {
        logger_base.debug("OnGridPopup - ALIGN_END_TIMES_SHIFT");
        AlignSelectedEffects(EFF_ALIGN_MODE::ALIGN_END_TIMES_SHIFT);
    }
    else if(id == ID_GRID_MNU_PRESETS)
    {
        logger_base.debug("OnGridPopup - PRESETS");
        xlights->ShowPresetsPanel();
    }
    else if(id == ID_GRID_MNU_BREAKDOWN_PHRASE)
    {
        logger_base.debug("OnGridPopup - ID_GRID_MNU_BREAKDOWN_PHRASE");
        Effect* phrase_effect = mSelectedEffect;
        EffectLayer* word_layer;
        TimingElement* element = dynamic_cast<TimingElement*>(phrase_effect->GetParentEffectLayer()->GetParentElement());
        bool found_locked = false;
        element->SetFixedTiming(0);
        if( element->GetEffectLayerCount() == 1 )
        {
            word_layer = element->AddEffectLayer();
        }
        else
        {
            word_layer = element->GetEffectLayer(1);
        }

        for (auto&& e : word_layer->GetAllEffectsByTime(phrase_effect->GetStartTimeMS(), phrase_effect->GetEndTimeMS()))
        {
            if (e->IsLocked())
            {
                found_locked = true;
                break;
            }
        }

        if (found_locked)
        {
            wxMessageBox("Locked words in the way - Can not break down phrase", "Error", wxOK);
        }
        else
        {
            mSequenceElements->get_undo_mgr().CreateUndoStep();
            word_layer->UnSelectAllEffects();
            word_layer->SelectEffectsInTimeRange(phrase_effect->GetStartTimeMS(), phrase_effect->GetEndTimeMS());
            word_layer->DeleteSelectedEffects(mSequenceElements->get_undo_mgr());
            mSequenceElements->BreakdownPhrase(word_layer, phrase_effect->GetStartTimeMS(), phrase_effect->GetEndTimeMS(), phrase_effect->GetEffectName());
            element->SetCollapsed(false);
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(mParent, eventRowHeaderChanged);
        }
    }
    else if (id == ID_GRID_MNU_HALVETIMINGS)
    {
        wxNumberEntryDialog dlg(this, "", "Divide timings by:", "Divide Timings", 2, 2, 100);

        if (dlg.ShowModal() == wxID_OK) {
            int by = dlg.GetValue();
            mSequenceElements->get_undo_mgr().CreateUndoStep();
            auto el = mSelectedEffect->GetParentEffectLayer();
            auto frequency = mTimeline->GetTimeFrequency();
            int base_timing = 1000.0 / frequency;
            for (int i = 0; i < el->GetEffectCount(); i++) {
                auto ef = el->GetEffect(i);
                if (ef->GetSelected() || ef->GetID() == mSelectedEffect->GetID()) {
                    long s = ef->GetStartTimeMS();
                    long e = ef->GetEndTimeMS();
                    if (e - s > base_timing) {
                        float splitf = (float)(e - s) / (float)by;
                        mSequenceElements->get_undo_mgr().CaptureModifiedEffect(el->GetParentElement()->GetModelName(), el->GetIndex(), ef->GetID(), ef->GetSettingsAsString(), ef->GetPaletteAsString());
                        mSequenceElements->get_undo_mgr().CaptureEffectToBeMoved(el->GetParentElement()->GetModelName(), el->GetIndex(), ef->GetID(), ef->GetStartTimeMS(), ef->GetEndTimeMS());
                        long newend = TimeLine::RoundToMultipleOfPeriod((float)s + splitf, frequency);
                        ef->SetEndTimeMS(newend);

                        for (int j = 1; j < by; j++) {

                            long newstart = newend;
                            if (j == by - 1) {
                                newend = e;
                            }
                            else {
                                newend = TimeLine::RoundToMultipleOfPeriod((long)((float)s + splitf * (float)(j + 1)), frequency);
                            }

                            if (newstart != newend)
                            {
                                Effect* newef = el->AddEffect(0, "", "", "", newstart, newend, EFFECT_SELECTED, false);
                                mSequenceElements->get_undo_mgr().CaptureAddedEffect(el->GetParentElement()->GetName(), el->GetIndex(), newef->GetID());
                                i++; // jump over the one we just inserted
                            }
                        }
                        ef->SetSelected(EFFECT_SELECTED);
                    }
                }
            }
        }
    }
    else if(id == ID_GRID_MNU_BREAKDOWN_WORD)
    {
        logger_base.debug("OnGridPopup - ID_GRID_MNU_BREAKDOWN_WORD");
        Effect* word_effect = mSelectedEffect;
        EffectLayer* phoneme_layer;
        Element* element = word_effect->GetParentEffectLayer()->GetParentElement();
        bool found_locked = false;

        if( element->GetEffectLayerCount() < 3 )
        {
            phoneme_layer = element->AddEffectLayer();
        }
        else
        {
            phoneme_layer = element->GetEffectLayer(2);
        }

        for (auto&& e : phoneme_layer->GetAllEffectsByTime(word_effect->GetStartTimeMS(), word_effect->GetEndTimeMS()))
        {
            if (e->IsLocked())
            {
                found_locked = true;
                break;
            }
        }

        if (found_locked)
        {
            wxMessageBox("Locked phonemes in the way - Can not break down word", "Error", wxOK);
        }
        else
        {
            mSequenceElements->get_undo_mgr().CreateUndoStep();
            phoneme_layer->UnSelectAllEffects();
            phoneme_layer->SelectEffectsInTimeRange(word_effect->GetStartTimeMS(), word_effect->GetEndTimeMS());
            phoneme_layer->DeleteSelectedEffects(mSequenceElements->get_undo_mgr());
            mSequenceElements->BreakdownWord(phoneme_layer, word_effect->GetStartTimeMS(), word_effect->GetEndTimeMS(), word_effect->GetEffectName());
            element->SetCollapsed(false);
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(mParent, eventRowHeaderChanged);
        }
    }
    else if(id == ID_GRID_MNU_BREAKDOWN_WORDS)
    {
        logger_base.debug("OnGridPopup - ID_GRID_MNU_BREAKDOWN_WORDS");
        Effect* word_effect = mSelectedEffect;
        EffectLayer* phoneme_layer;
        Element* element = word_effect->GetParentEffectLayer()->GetParentElement();
        bool found_locked = false;

        if( element->GetEffectLayerCount() < 3 )
        {
            phoneme_layer = element->AddEffectLayer();
        }
        else
        {
            phoneme_layer = element->GetEffectLayer(2);
        }
        mSequenceElements->get_undo_mgr().CreateUndoStep();
        phoneme_layer->UnSelectAllEffects();
        phoneme_layer->SelectEffectsInTimeRange(word_effect->GetStartTimeMS(), word_effect->GetEndTimeMS());
        EffectLayer *layer = word_effect->GetParentEffectLayer();
        for (int x = 0; x < layer->GetEffectCount(); x++) {
            word_effect = layer->GetEffect(x);
            if (word_effect->GetSelected() != EFFECT_NOT_SELECTED) {
                for (auto&& e : phoneme_layer->GetAllEffectsByTime(word_effect->GetStartTimeMS(), word_effect->GetEndTimeMS()))
                {
                    if (e->IsLocked())
                    {
                        found_locked = true;
                        break;
                    }
                }
                phoneme_layer->SelectEffectsInTimeRange(word_effect->GetStartTimeMS(), word_effect->GetEndTimeMS());
            }
            if (found_locked)
            {
                break;
            }
        }

        if (found_locked)
        {
            wxMessageBox("Locked phonemes in the way - Can not break down words", "Error", wxOK);
            phoneme_layer->UnSelectAllEffects();
        }
        else
        {
            phoneme_layer->DeleteSelectedEffects(mSequenceElements->get_undo_mgr());
            for (int x = 0; x < layer->GetEffectCount(); x++) {
                word_effect = layer->GetEffect(x);
                if (word_effect->GetSelected() != EFFECT_NOT_SELECTED) {
                    mSequenceElements->BreakdownWord(phoneme_layer, word_effect->GetStartTimeMS(), word_effect->GetEndTimeMS(), word_effect->GetEffectName());
                }
            }
            element->SetCollapsed(false);
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(mParent, eventRowHeaderChanged);
        }
    }
    Refresh();
}

Effect* EffectsGrid::FillRandomEffects()
{
    Effect* res = nullptr;
    SetCursor(wxCURSOR_WAIT);
    int row1 = mRangeStartRow;
    int row2 = mRangeEndRow;
    if( row1 > row2 ) {
        std::swap(row1, row2);
    }
    int selected_timing_row = mSequenceElements->GetSelectedTimingRow();
    if (selected_timing_row >= 0 ) {
        EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(selected_timing_row);
        int timingIndex1 = mRangeStartCol;
        int timingIndex2 = mRangeEndCol;
        if( timingIndex1 > timingIndex2 ) {
            std::swap(timingIndex1, timingIndex2);
        }
        Effect *lastEffect = nullptr;
        if (timingIndex1 != -1 && timingIndex2 != -1) {
            wxProgressDialog prog("Generating random effects", "This may take some time", 100, this, wxPD_APP_MODAL | wxPD_AUTO_HIDE);
            prog.Show();
            float progValue = 0;
            float den = (float)((row2 - row1 + 1) * (timingIndex2 - timingIndex1 + 1));
            float per = 0.01f;
            if (den != 0) per = 100.0 / den;
            mSequenceElements->get_undo_mgr().CreateUndoStep();
            for( int row = row1; row <= row2; row++)
            {
                EffectLayer* effectLayer = mSequenceElements->GetEffectLayer(row);
                for(int i = timingIndex1; i <= timingIndex2; i++)
                {
                    Effect* eff = tel->GetEffect(i);
                    if( effectLayer->GetRangeIsClearMS(eff->GetStartTimeMS(), eff->GetEndTimeMS()) )
                    {
                        Effect* ef = effectLayer->AddEffect(0,
                                                            "Random",
                                                            "",
                                                            "",
                                                            eff->GetStartTimeMS(),
                                                            eff->GetEndTimeMS(),
                                                            EFFECT_SELECTED,
                                                            false);
                        if (res == nullptr) res = ef;
                        if (ef != nullptr)
                        {
                            lastEffect = ef;
                            mSequenceElements->get_undo_mgr().CaptureAddedEffect(effectLayer->GetParentElement()->GetModelName(), effectLayer->GetIndex(), ef->GetID());
                            RaiseSelectedEffectChanged(ef, true, false);
                            mSelectedEffect = ef;
                        }
                    }
                    progValue += per;
                    prog.Update(progValue);
                }
            }
            prog.Update(100);
            if (!xlights->IsACActive())
            {
                mCellRangeSelected = false;
            }
        }
    } else if (mSequenceElements->GetVisibleEffectLayer(mDropRow) != nullptr) {
        EffectLayer* el = mSequenceElements->GetVisibleEffectLayer(mDropRow);
        int end_time = mDropEndTimeMS;
        if( el->GetRangeIsClearMS(mDropStartTimeMS, end_time) )
        {
            Effect* ef = el->AddEffect(0,
                                       "Random",
                                       "",
                                       "",
                                       mDropStartTimeMS,
                                       mDropEndTimeMS,
                                       EFFECT_SELECTED,
                                       false);

            if (res == nullptr) res = ef;

            if (ef != nullptr)
            {
                mSequenceElements->get_undo_mgr().CreateUndoStep();
                mSequenceElements->get_undo_mgr().CaptureAddedEffect(el->GetParentElement()->GetModelName(), el->GetIndex(), ef->GetID());
                RaiseSelectedEffectChanged(ef, true);
                mSelectedEffect = ef;
                if (!ef->GetPaletteMap().empty()) {
                    sendRenderEvent(el->GetParentElement()->GetModelName(),
                        mDropStartTimeMS,
                        mDropEndTimeMS, true);
                }
                mPartialCellSelected = false;
            }
        }
    }
    SetCursor(wxCURSOR_ARROW);
    return res;
}

void EffectsGrid::ProcessDroppedEffect(Effect* effect)
{
    mSelectedEffect = effect;
    mSelectedRow = mDropRow;
    RaiseSelectedEffectChanged(effect, true);
}

void EffectsGrid::OnLostMouseCapture(wxMouseCaptureLostEvent& event)
{
    mDragging = false;
    mResizing = false;
    mDragDropping = false;
    mResizingMode = EFFECT_RESIZE_NO;
}

bool EffectsGrid::AdjustDropLocations(int x, EffectLayer* el)
{
    bool return_val = false;
    int time = mTimeline->GetAbsoluteTimeMSfromPosition(x);

    int startT = mTimeline->GetAbsoluteTimeMSfromPosition(mDropStartX);
    int endT = mTimeline->GetAbsoluteTimeMSfromPosition(mDropEndX);

    if( !el->GetRangeIsClearMS(startT, endT) )
    {
        Effect* before_eff = el->GetEffectBeforeEmptyTime(time);
        if( before_eff != nullptr )
        {
            int endPos = mTimeline->GetPositionFromTimeMS(before_eff->GetEndTimeMS());
            if( endPos > mDropStartX ) {
                mDropStartX = endPos;
                mDropStartTimeMS = before_eff->GetEndTimeMS();
                return_val = true;
            }
        }
        Effect* after_eff = el->GetEffectAfterEmptyTime(time);
        if( after_eff != nullptr )
        {
            int sPos = mTimeline->GetPositionFromTimeMS(after_eff->GetStartTimeMS());
            if (sPos < mDropEndX ) {
                mDropEndX = sPos;
                mDropEndTimeMS = after_eff->GetStartTimeMS();
                return_val = true;
            }
        }
    }
    return return_val;
}

bool EffectsGrid::DragOver(int x, int y)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (mSequenceElements == nullptr) {
        return false;
    }

    mDragDropping = false;
    int row = GetRow(y);
    if( row < mSequenceElements->GetVisibleRowInformationSize() )
    {
        int effectIndex;
        HitLocation selectionType;
        int time = mTimeline->GetRawTimeMSfromPosition(x);

        Effect* selectedEffect = GetEffectAtRowAndTime(row,time,effectIndex,selectionType);
        if(selectedEffect != nullptr)
        {
            mDragDropping = true;
            mDropStartX = mTimeline->GetPositionFromTimeMS(selectedEffect->GetStartTimeMS());
            mDropEndX = mTimeline->GetPositionFromTimeMS(selectedEffect->GetEndTimeMS());
            mDropStartTimeMS = selectedEffect->GetStartTimeMS();
            mDropEndTimeMS = selectedEffect->GetEndTimeMS();
            mDropRow = row;
        }
        else
        {
            int selectedTimingIndex = mSequenceElements->GetSelectedTimingRow();
            bool freeform_drop = false;
            if(selectedTimingIndex >= 0)
            {
                EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(selectedTimingIndex);
                if (tel == nullptr)
                {
                    logger_base.crit("EffectsGrid::DragOver tel is nullptr ... this is going to crash.");
                }

                int timingIndex = 0;
                if(tel->HitTestEffectByTime(time, timingIndex))
                {
                    EffectLayer* el = mSequenceElements->GetVisibleEffectLayer(row);
                    if( el != nullptr )
                    {
                        mDropStartX = mTimeline->GetPositionFromTimeMS(tel->GetEffect(timingIndex)->GetStartTimeMS());
                        mDropEndX = mTimeline->GetPositionFromTimeMS(tel->GetEffect(timingIndex)->GetEndTimeMS());
                        mDropStartTimeMS = tel->GetEffect(timingIndex)->GetStartTimeMS();
                        mDropEndTimeMS = tel->GetEffect(timingIndex)->GetEndTimeMS();
                        AdjustDropLocations(x, el);
                        mDragDropping = true;
                        mDropRow = row;
                    }
                }
                else
                {
                    freeform_drop = true;
                }
            }
            else
            {
                    freeform_drop = true;
            }
            if( freeform_drop )
            {
                EffectLayer* el = mSequenceElements->GetVisibleEffectLayer(row);
                if( el != nullptr )
                {
                    mDropStartTimeMS = mTimeline->GetAbsoluteTimeMSfromPosition(x);
                    mDropStartTimeMS -= 500;
                    if( mDropStartTimeMS < 0 ) mDropStartTimeMS = 0;
                    mDropEndTimeMS = mDropStartTimeMS + 1000;
                    mDropStartX = mTimeline->GetPositionFromTimeMS(mDropStartTimeMS);
                    mDropEndX = mTimeline->GetPositionFromTimeMS(mDropEndTimeMS);
                    AdjustDropLocations(x, el);
                    mDragDropping = true;
                    mDropRow = row;
                }
            }
        }
    }
    Refresh(false);
    return mDragDropping;
}

void EffectsGrid::OnDrop(int x, int y)
{
    if (mDragDropping)
    {
        RaiseEffectDropped(x,y);
        mDragDropping = false;
    }

    Refresh(false);
}

bool EffectsGrid::IsDroppingBetweenTimingMarks() const
{
    // is any timing mark active
    if (mSequenceElements->GetSelectedTimingRow() == -1) {
        return false;
    }

    EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
    if (tel == nullptr) return false; // this should never happen

    Effect* e = tel->GetEffectAtTime((mDropStartTimeMS + mDropEndTimeMS) / 2);
    if (e == nullptr) return false;

    return (e->GetStartTimeMS() == mDropStartTimeMS && e->GetEndTimeMS() == mDropEndTimeMS);
}

void EffectsGrid::OnDropFiles(int x, int y, const wxArrayString& files)
{
    if (files.size() > 0)
    {
        long tlEnd = mTimeline->GetSequenceEnd();

        int row = GetRow(y);
        if (row >= mSequenceElements->GetVisibleRowInformationSize()) {
            return; // we are not one a row
        }
        EffectLayer* effectLayer = mSequenceElements->GetVisibleEffectLayer(row);
        if (effectLayer == nullptr) return;
        Effect* e = effectLayer->GetEffectAfterEmptyTime(mDropStartTimeMS);

        long nextEffectStart = 999999999;
        if (e != nullptr)
        {
            nextEffectStart = e->GetStartTimeMS();
        }

        wxString file = files.front();
        std::string effectName = "";
        if (VideoEffect::IsVideoFile(file))
        {
            effectName = "Video";

            if (!IsDroppingBetweenTimingMarks())
            {
                long videoEnd = mDropStartTimeMS + VideoReader::GetVideoLength(file);
                mDropEndTimeMS = std::min(std::min(tlEnd, videoEnd), nextEffectStart);
            }
        }
        else if (PicturesEffect::IsPictureFile(file))
        {
            effectName = "Pictures";
        }
        else if (GlediatorEffect::IsGlediatorFile(file))
        {
            effectName = "Glediator";

            if (IsDroppingBetweenTimingMarks())
            {
                Model* m = xlights->GetModel(effectLayer->GetParentElement()->GetModelName());
                if (m != nullptr)
                {
                    wxSize size(m->GetDefaultBufferWi(), m->GetDefaultBufferHt());
                    GlediatorReader gr(file, size);
                    long glediatorEnd = mDropStartTimeMS + gr.GetFrames() * mTimeline->GetFrameMS();

                    mDropEndTimeMS = std::min(std::min(tlEnd, glediatorEnd), nextEffectStart);
                }
            }
        }

        if (effectName != "")
        {
            if (mRangeStartCol == -1)
            {
                if (mDropEndTimeMS <= mDropStartTimeMS) mDropEndTimeMS = mDropStartTimeMS + 1000;
            }
            CreateEffectForFile(x, y, effectName, file.ToStdString());
        }
    }
}

void EffectsGrid::mouseMoved(wxMouseEvent& event)
{

    if (!mIsInitialized || mSequenceElements == nullptr) {
        return;
    }

    int rowIndex = GetRow(event.GetY());
    bool out_of_bounds =  rowIndex < 0 || (rowIndex >= mSequenceElements->GetVisibleRowInformationSize());

    if(mResizing)
    {
        //static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        //logger_base.debug("EffectsGrid::mouseMoved sizing or moving effects.");
        Resize(event.GetX(), event.AltDown(), event.ControlDown());
        Refresh(false);
        Update();
    }
    else if (mDragging)
    {
        mDragEndX = event.GetX();
        mDragEndY = event.GetY();
        UpdateSelectionRectangle();
        Refresh(false);
        Update();
    }
    else
    {
        if( !xlights->IsACActive() || rowIndex < mSequenceElements->GetNumberOfTimingRows() ) {
            if(!out_of_bounds)
            {
                Element* element = mSequenceElements->GetVisibleRowInformation(rowIndex)->element;
                if( element != nullptr )
                {
                    RunMouseOverHitTests(rowIndex,event.GetX(),event.GetY());
                }
            }
        } else {
            SetCursor(wxCURSOR_DEFAULT);
            mResizingMode = EFFECT_RESIZE_NO;
        }
    }

    int mouseTimeMS = mTimeline->GetAbsoluteTimeMSfromPosition(event.GetX());
    UpdateMousePosition(mouseTimeMS);
}

void adjustMS(int timeMS, int &min, int &max) {
    if (timeMS < min) {
        min = timeMS;
    }
    if (timeMS > max) {
        max = timeMS;
    }
}

void EffectsGrid::mouseLeftWindow(wxMouseEvent& event)
{
    UpdateMousePosition(-1);
}

int EffectsGrid::GetClippedPositionFromTimeMS(int ms) const
{
    int i = mTimeline->GetPositionFromTimeMS(ms);
    if (i < -10) {
        i = -10;
    }
    if (i > GetSize().x) {
        i = GetSize().x;
    }
    return i;
}

void EffectsGrid::CreateEffectForFile(int x, int y, const std::string& effectName, const std::string& filename)
{
    int row = GetRow(y);

    if (row >= mSequenceElements->GetVisibleRowInformationSize()) {
        return;
    }

    mSequenceElements->ClearSelectedRanges();

    EffectLayer* effectLayer = mSequenceElements->GetVisibleEffectLayer(row);

    EffectRange effectRange;
    effectRange.Layer = effectLayer;
    // Store start and end time. The effect text will be supplied by parent class
    effectRange.StartTime = mDropStartTimeMS;
    effectRange.EndTime = mDropEndTimeMS;
    mSequenceElements->AddSelectedRange(&effectRange);

    // Raise event
    wxCommandEvent eventDropped(EVT_EFFECTFILE_DROPPED);
    eventDropped.SetString(effectName + "|" + filename);
    wxPostEvent(GetParent(), eventDropped);
}

int MapHitLocationToEffectSelection(HitLocation location) {
    switch (location) {
        case HitLocation::NONE:
            return EFFECT_NOT_SELECTED;
        case HitLocation::LEFT:
        case HitLocation::LEFT_EDGE:
        case HitLocation::LEFT_EDGE_DISCONNECT:
            return EFFECT_LT_SELECTED;
        case HitLocation::RIGHT:
        case HitLocation::RIGHT_EDGE:
        case HitLocation::RIGHT_EDGE_DISCONNECT:
            return EFFECT_RT_SELECTED;
        case HitLocation::CENTER:
            return EFFECT_SELECTED;
    }
    return EFFECT_NOT_SELECTED;
}

Effect* EffectsGrid::GetEffectAtRowAndTime(int row, int ms,int &index, HitLocation &selectionType) {

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    EffectLayer* effectLayer = mSequenceElements->GetVisibleEffectLayer(row);

    if (effectLayer == nullptr)
    {
        logger_base.crit("EffectsGrid::GetEffectAtRowAndTime effectLayer is nullptr ... this is going to crash.");
    }

    Effect *eff = nullptr;
    selectionType = HitLocation::NONE;
    if (effectLayer->HitTestEffectByTime(ms, index)) {
        eff = effectLayer->GetEffect(index);
        int startPos = GetClippedPositionFromTimeMS(eff->GetStartTimeMS());
        int endPos = GetClippedPositionFromTimeMS(eff->GetEndTimeMS());
        int position = GetClippedPositionFromTimeMS(ms);
        int mid = (startPos + endPos) / 2;

        if (effectLayer->IsFixedTimingLayer())
        {
            selectionType = HitLocation::NONE;
        }
        else if (!eff->IsLocked())
        {
            if ((endPos - startPos) < 8) {
                //too small to really differentiate, just
                //provide ability to make the effect larger
                if (position < mid) {
                    selectionType = HitLocation::LEFT_EDGE;
                }
                else {
                    selectionType = HitLocation::RIGHT_EDGE;
                }
            }
            else if (position > endPos - 6) {
                selectionType = HitLocation::RIGHT_EDGE;
            }
            else if (position > endPos - 12) {
                selectionType = HitLocation::RIGHT_EDGE_DISCONNECT;
            }
            else if (position > mid + 8) {
                selectionType = HitLocation::RIGHT;
            }
            else if (position < startPos + 6) {
                selectionType = HitLocation::LEFT_EDGE;
            }
            else if (position < startPos + 12) {
                selectionType = HitLocation::LEFT_EDGE_DISCONNECT;
            }
            else if (position < mid - 8) {
                selectionType = HitLocation::LEFT;
            }
            else {
                selectionType = HitLocation::CENTER;
            }
        }
        else
        {
            selectionType = HitLocation::CENTER;
        }
    }
    return eff;
}

void EffectsGrid::ClearSelection()
{
    mDragging = false;
    mResizing = false;
    mDragDropping = false;
    mDropStartX = 0;
    mDropEndX = 0;
    mCellRangeSelected = false;
    mPartialCellSelected = false;
    mDragStartRow = 0;
    mDragStartX = -1;
    mDragStartY = -1;
    UnselectEffect();
    mSelectedEffect = nullptr;
    mRangeCursorRow = mRangeStartRow;
    mRangeCursorCol = mRangeStartCol;
    mRangeStartRow = -1;
    mRangeEndRow = -1;
    mRangeStartCol = -1;
    mRangeEndCol = -1;
    mResizeEffectIndex = -1;
    SetRCToolTip();
}

void EffectsGrid::mouseDown(wxMouseEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    mPartialCellSelected = false;

    // if no shift key clear any cell range selections
    if (!event.ShiftDown()) {
        mCellRangeSelected = false;
        mRangeStartCol = mRangeEndCol = mRangeStartRow = mRangeEndRow = -1;
        Refresh();
    }
    if (mSequenceElements == nullptr) {
        return;
    }
    SetFocus();
    if(!(event.ShiftDown() || event.ControlDown() || event.AltDown()) && mResizingMode == EFFECT_RESIZE_NO)
    {
        mSequenceElements->UnSelectAllEffects();
    }
    int selectedTimeMS = mTimeline->GetAbsoluteTimeMSfromPosition(event.GetX());
    mStartResizeTimeMS = selectedTimeMS;
    int row = GetRow(event.GetY());
    if(row>=mSequenceElements->GetVisibleRowInformationSize() || row < 0)
        return;
    mSequenceElements->get_undo_mgr().CreateUndoStep();
    mSequenceElements->get_undo_mgr().SetCaptureUndo(true);
    int effectIndex;
    HitLocation selectionType;
    int time = mTimeline->GetRawTimeMSfromPosition(event.GetX());
    Effect* selectedEffect = GetEffectAtRowAndTime(row,time,effectIndex,selectionType);
    if(selectedEffect!= nullptr)
    {
        logger_base.debug("EffectsGrid::mouseDown effect selected %s.", (const char *)selectedEffect->GetEffectName().c_str());
        switch (selectionType) {
            case HitLocation::NONE:
                break;
            case HitLocation::LEFT_EDGE:
            case HitLocation::LEFT_EDGE_DISCONNECT:
                selectedTimeMS = selectedEffect->GetStartTimeMS();
                break;
            case HitLocation::LEFT:
            case HitLocation::CENTER:
            case HitLocation::RIGHT:
                if( mResizingMode == EFFECT_RESIZE_MOVE )
                {
                    int spos = GetClippedPositionFromTimeMS(selectedEffect->GetStartTimeMS());
                    int epos = GetClippedPositionFromTimeMS(selectedEffect->GetEndTimeMS());
                    int selected_pos = spos + (epos-spos)/2;
                    selectedTimeMS = mTimeline->GetAbsoluteTimeMSfromPosition(selected_pos);
                }
                break;
            case HitLocation::RIGHT_EDGE:
            case HitLocation::RIGHT_EDGE_DISCONNECT:
                selectedTimeMS = selectedEffect->GetEndTimeMS();
                break;
        }

        if(!(event.ShiftDown() || event.ControlDown() || event.AltDown()))
        {
            mSequenceElements->UnSelectAllEffects();
            if(( selectionType != HitLocation::CENTER ) || ((selectionType == HitLocation::CENTER) && ( mResizingMode == EFFECT_RESIZE_MOVE )))
            {
                selectedEffect->SetSelected(MapHitLocationToEffectSelection(selectionType));
            }
        }
        if( event.ControlDown() )
        {
            if( selectionType != HitLocation::NONE )
            {
                if( selectedEffect->GetSelected() == EFFECT_NOT_SELECTED )
                {
                    selectedEffect->SetSelected(MapHitLocationToEffectSelection(selectionType));
                }
                else
                {
                    selectedEffect->SetSelected(EFFECT_NOT_SELECTED);
                    selectedEffect = nullptr;
                    mSelectedEffect = nullptr;
                    mSelectedRow = -1;
                }
            }
        }

        if( mSequenceElements->GetSelectedTimingRow() >= 0 ) {
            switch (selectionType) {
                case HitLocation::NONE:
                case HitLocation::LEFT:
                case HitLocation::RIGHT:
                    selectedEffect = nullptr;
                    break;
                default:
                    break;
            }
        }

        if( selectedEffect != nullptr )
        {
            mEffectLayer = mSequenceElements->GetVisibleEffectLayer(row);
            Element* element = mEffectLayer->GetParentElement();

            if(element->GetType() != ElementType::ELEMENT_TYPE_TIMING)
            {
                if (selectedEffect != mSelectedEffect)
                {
                    mSelectedEffect = selectedEffect;
                    RaiseSelectedEffectChanged(mSelectedEffect, false);
                }
                RaisePlayModelEffect(element,mSelectedEffect,false);
                if (row != mSelectedRow)
                {
                    mSelectedRow = row;
                    wxCommandEvent eventRowChanged(EVT_SELECTED_ROW_CHANGED);
                    eventRowChanged.SetInt(mSelectedRow);
                    eventRowChanged.SetString(element->GetModelName());
                    wxPostEvent(GetParent(), eventRowChanged);
                }
            }
            else
            {
                mSelectedEffect = selectedEffect;
            }
        }
        Refresh(false);
    }

    if(mResizingMode!=EFFECT_RESIZE_NO)
    {
        if(selectedEffect != nullptr)
        {
            mResizing = true;
            mResizeEffectIndex = effectIndex;
            CaptureMouse();
            Refresh(false);
        }
    }
    else
    {
        if( !mDragging)
        {
            if( !event.ShiftDown() || (mDragStartX == -1) )
            {
                mDragStartX = event.GetX();
                mDragStartY = event.GetY();
                mDragStartRow = mSequenceElements->GetFirstVisibleModelRow();
                if (selectedEffect == nullptr)
                {
                    UnselectEffect();
                    mSelectedEffect = nullptr;
                    mSelectedRow = -1;
                }
            }
            mDragging = true;
            mDragEndX = event.GetX();
            mDragEndY = event.GetY();
            if( event.ShiftDown() ) {
                UpdateSelectionRectangle();
            } else {
                EstablishSelectionRectangle();
            }
            CaptureMouse();
            Refresh(false);
        }
    }
    UpdateZoomPosition(selectedTimeMS);
}

Effect* EffectsGrid::GetSelectedEffect() const
{
    return mSelectedEffect;
}

bool EffectsGrid::AreAllSelectedEffectsOnTheSameElement() const
{
    Element* selected = nullptr;

    for (int row = 0; row<mSequenceElements->GetRowInformationSize(); row++)
    {
        EffectLayer* el = mSequenceElements->GetEffectLayer(row);
        if (el->GetSelectedEffectCount() > 0)
        {
            if (selected == nullptr)
            {
                selected = el->GetParentElement();
            }
            else
            {
                if (selected != el->GetParentElement())
                {
                    return false;
                }
            }
        }
    }

    return true;
}

int EffectsGrid::GetSelectedEffectCount(const std::string& effectName) const
{
    int count = 0;

    for (int row = 0; row<mSequenceElements->GetRowInformationSize(); row++)
    {
        EffectLayer* el = mSequenceElements->GetEffectLayer(row);
        count += el->GetSelectedEffectCount(effectName);
    }

    return count;
}

void EffectsGrid::ApplyEffectSettingToSelected(const std::string& effectName, const std::string id, const std::string value, ValueCurve* vc, const std::string& vcid)
{
    Element* lastModel = nullptr;
    RangeAccumulator rangeAccumulator;

    mSequenceElements->get_undo_mgr().CreateUndoStep();
    for (int row = 0; row < mSequenceElements->GetRowInformationSize(); row++)
    {
        Row_Information_Struct* ri = mSequenceElements->GetRowInformationFromRow(row);

        if (ri->element != lastModel && lastModel != nullptr && rangeAccumulator.size() > 0)
        {
            // send render events for all the ranges that changed
            // 1000 means if there is less than a second gap between ranges then we will just merge them
            //rangeAccumulator.Consolidate(1000);
            //for (auto it = rangeAccumulator.begin(); it != rangeAccumulator.end(); ++it)
            //{
            //    sendRenderEvent(lastModel->GetModelName(), it->first, it->second);
            //}

            // just send one render event for the whole model range
            // if there are big gaps this massively over-renders compared to the commented out code above
            rangeAccumulator.Consolidate(); // still need to consolidate before we access it
            sendRenderEvent(lastModel->GetModelName(), rangeAccumulator.front().first, rangeAccumulator.back().second);

            rangeAccumulator.clear();
        }
        lastModel = ri->element;

        if (ri->element->GetType() == ElementType::ELEMENT_TYPE_TIMING)
        {
            // skip timing rows
        }
        else
        {
            mSequenceElements->GetEffectLayer(row)->ApplyEffectSettingToSelected(this, mSequenceElements->get_undo_mgr(), effectName, id, value, vc, vcid, xlights->GetEffectManager(), rangeAccumulator);
        }
    }

    if (lastModel != nullptr && rangeAccumulator.size() > 0)
    {
        // send render events for all the ranges that changed
        // 1000 means if there is less than a second gap between ranges then we will just merge them
        //rangeAccumulator.Consolidate(1000);
        //for (auto it = rangeAccumulator.begin(); it != rangeAccumulator.end(); ++it)
        //{
        //    sendRenderEvent(lastModel->GetModelName(), it->first, it->second);
        //}

        // just send one render event for the whole model range
        // if there are big gaps this massively over-renders compared to the commented out code above
        rangeAccumulator.Consolidate(); // still need to consolidate before we access it
        sendRenderEvent(lastModel->GetModelName(), rangeAccumulator.front().first, rangeAccumulator.back().second);

        rangeAccumulator.clear();
    }
}

void EffectsGrid::ApplyButtonPressToSelected(const std::string& effectName, const std::string id)
{
    Element* lastModel = nullptr;
    RangeAccumulator rangeAccumulator;

    mSequenceElements->get_undo_mgr().CreateUndoStep();
    for (int row = 0; row < mSequenceElements->GetRowInformationSize(); row++)
    {
        Row_Information_Struct* ri = mSequenceElements->GetRowInformationFromRow(row);

        if (ri->element != lastModel && lastModel != nullptr && rangeAccumulator.size() > 0)
        {
            // send render events for all the ranges that changed
            // 1000 means if there is less than a second gap between ranges then we will just merge them
            //rangeAccumulator.Consolidate(1000);
            //for (auto it = rangeAccumulator.begin(); it != rangeAccumulator.end(); ++it)
            //{
            //    sendRenderEvent(lastModel->GetModelName(), it->first, it->second);
            //}

            // just send one render event for the whole model range
            // if there are big gaps this massively over-renders compared to the commented out code above
            rangeAccumulator.Consolidate(); // still need to consolidate before we access it
            sendRenderEvent(lastModel->GetModelName(), rangeAccumulator.front().first, rangeAccumulator.back().second);

            rangeAccumulator.clear();
        }
        lastModel = ri->element;

        if (ri->element->GetType() == ElementType::ELEMENT_TYPE_TIMING)
        {
            // skip timing rows
        }
        else
        {
            mSequenceElements->GetEffectLayer(row)->ApplyButtonPressToSelected(this, mSequenceElements->get_undo_mgr(), effectName, id, xlights->GetEffectManager(), rangeAccumulator);
        }
    }

    if (lastModel != nullptr && rangeAccumulator.size() > 0)
    {
        // send render events for all the ranges that changed
        // 1000 means if there is less than a second gap between ranges then we will just merge them
        //rangeAccumulator.Consolidate(1000);
        //for (auto it = rangeAccumulator.begin(); it != rangeAccumulator.end(); ++it)
        //{
        //    sendRenderEvent(lastModel->GetModelName(), it->first, it->second);
        //}

        // just send one render event for the whole model range
        // if there are big gaps this massively over-renders compared to the commented out code above
        rangeAccumulator.Consolidate(); // still need to consolidate before we access it
        sendRenderEvent(lastModel->GetModelName(), rangeAccumulator.front().first, rangeAccumulator.back().second);

        rangeAccumulator.clear();
    }
}

void EffectsGrid::RemapSelectedDMXEffectValues(const std::vector<std::pair<int, int>>& pairs)
{
    Element* lastModel = nullptr;
    RangeAccumulator rangeAccumulator;

    mSequenceElements->get_undo_mgr().CreateUndoStep();
    for (int row = 0; row < mSequenceElements->GetRowInformationSize(); row++)
    {
        Row_Information_Struct* ri = mSequenceElements->GetRowInformationFromRow(row);

        if (ri->element != lastModel && lastModel != nullptr && rangeAccumulator.size() > 0)
        {
            // send render events for all the ranges that changed
            // 1000 means if there is less than a second gap between ranges then we will just merge them
            //rangeAccumulator.Consolidate(1000);
            //for (auto it = rangeAccumulator.begin(); it != rangeAccumulator.end(); ++it)
            //{
            //    sendRenderEvent(lastModel->GetModelName(), it->first, it->second);
            //}

            // just send one render event for the whole model range
            // if there are big gaps this massively over-renders compared to the commented out code above
            rangeAccumulator.Consolidate(); // still need to consolidate before we access it
            sendRenderEvent(lastModel->GetModelName(), rangeAccumulator.front().first, rangeAccumulator.back().second);

            rangeAccumulator.clear();
        }
        lastModel = ri->element;

        if (ri->element->GetType() == ElementType::ELEMENT_TYPE_TIMING)
        {
            // skip timing rows
        }
        else
        {
            mSequenceElements->GetEffectLayer(row)->RemapSelectedDMXEffectValues(this, mSequenceElements->get_undo_mgr(), pairs, xlights->GetEffectManager(), rangeAccumulator);
        }
    }

    if (lastModel != nullptr && rangeAccumulator.size() > 0)
    {
        // send render events for all the ranges that changed
        // 1000 means if there is less than a second gap between ranges then we will just merge them
        //rangeAccumulator.Consolidate(1000);
        //for (auto it = rangeAccumulator.begin(); it != rangeAccumulator.end(); ++it)
        //{
        //    sendRenderEvent(lastModel->GetModelName(), it->first, it->second);
        //}

        // just send one render event for the whole model range
        // if there are big gaps this massively over-renders compared to the commented out code above
        rangeAccumulator.Consolidate(); // still need to consolidate before we access it
        sendRenderEvent(lastModel->GetModelName(), rangeAccumulator.front().first, rangeAccumulator.back().second);

        rangeAccumulator.clear();
    }
}

void EffectsGrid::ConvertSelectedEffectsTo(const std::string& effectName)
{
    Element* lastModel = nullptr;
    RangeAccumulator rangeAccumulator;

    mSequenceElements->get_undo_mgr().CreateUndoStep();
    for (int row = 0; row<mSequenceElements->GetRowInformationSize(); row++)
    {
        Row_Information_Struct* ri = mSequenceElements->GetRowInformationFromRow(row);

        if (ri->element != lastModel && lastModel != nullptr && rangeAccumulator.size() > 0)
        {
            // send render events for all the ranges that changed
            // 1000 means if there is less than a second gap between ranges then we will just merge them
            //rangeAccumulator.Consolidate(1000);
            //for (auto it = rangeAccumulator.begin(); it != rangeAccumulator.end(); ++it)
            //{
            //    sendRenderEvent(lastModel->GetModelName(), it->first, it->second);
            //}

            // just send one render event for the whole model range
            // if there are big gaps this massively over-renders compared to the commented out code above
            rangeAccumulator.Consolidate(); // still need to consolidate before we access it
            sendRenderEvent(lastModel->GetModelName(), rangeAccumulator.front().first, rangeAccumulator.back().second);

            rangeAccumulator.clear();
        }
        lastModel = ri->element;

        if (ri->element->GetType() == ElementType::ELEMENT_TYPE_TIMING)
        {
            // skip timing rows
        }
        else
        {
            mSequenceElements->GetEffectLayer(row)->ConvertSelectedEffectsTo(this, mSequenceElements->get_undo_mgr(), effectName, xlights->GetEffectManager(), rangeAccumulator);
        }
    }

    if (lastModel != nullptr && rangeAccumulator.size() > 0)
    {
        // send render events for all the ranges that changed
        // 1000 means if there is less than a second gap between ranges then we will just merge them
        //rangeAccumulator.Consolidate(1000);
        //for (auto it = rangeAccumulator.begin(); it != rangeAccumulator.end(); ++it)
        //{
        //    sendRenderEvent(lastModel->GetModelName(), it->first, it->second);
        //}

        // just send one render event for the whole model range
        // if there are big gaps this massively over-renders compared to the commented out code above
        rangeAccumulator.Consolidate(); // still need to consolidate before we access it
        sendRenderEvent(lastModel->GetModelName(), rangeAccumulator.front().first, rangeAccumulator.back().second);

        rangeAccumulator.clear();
    }
}

Effect* EffectsGrid::ACDraw(ACTYPE type, ACSTYLE style, ACMODE mode, int intensity, int a, int b, int startMS, int endMS, int startRow, int endRow)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    Effect* res = nullptr;
    bool first = false; // true; - if i change this back to true then first drawn effects will be selected

    if (type != ACTYPE::OFF && mode == ACMODE::MODENIL)
    {
        res = ACDraw(ACTYPE::OFF, style, mode, intensity, a, b, startMS, endMS, startRow, endRow);
    }

    for (int r = startRow; r <= endRow; ++r)
    {
        EffectLayer* el = mSequenceElements->GetVisibleEffectLayer(r - mSequenceElements->GetFirstVisibleModelRow());
        if (el == nullptr)
        {
            logger_base.crit("AAA GetVisibleEffectLayer %d about to crash ... so continuing", r - mSequenceElements->GetFirstVisibleModelRow());
            continue;
        }
        Element *e = el->GetParentElement();
        if (e == nullptr) logger_base.crit("BBB GetParentElement about to crash");

        if (e->GetType() != ElementType::ELEMENT_TYPE_TIMING)
        {
            switch (type)
            {
            case ACTYPE::OFF:
                // I can ignore mode ...
                // ... if it is already off then it wont do anything.
                // ... if it is on then this will already delete it
                for (auto i = 0; i < el->GetEffectCount(); ++i)
                {
                    Effect* eff = el->GetEffect(i);
                    if (eff == nullptr) logger_base.crit("CCC GetEffect about to crash %d", i);

                    if (eff->GetStartTimeMS() >= endMS || eff->GetEndTimeMS() <= startMS)
                    {
                        // can ignore these
                    }
                    else if (eff->GetStartTimeMS() < startMS && eff->GetEndTimeMS() > endMS)
                    {
                        if (!eff->IsLocked())
                        {
                            SettingsMap settings = eff->GetSettings();

                            int start = eff->GetStartTimeMS();
                            int end = eff->GetEndTimeMS();
                            TruncateEffect(el, eff, start, startMS);
                            Effect* ef = DuplicateAndTruncateEffect(el, settings, eff->GetPaletteAsString(), eff->GetEffectName(), start, end, endMS, end);
                            if (res == nullptr) res = ef;
                        }
                    }
                    else if (eff->GetStartTimeMS() < startMS)
                    {
                        if (!eff->IsLocked())
                        {
                            // need to truncate it
                            TruncateEffect(el, eff, eff->GetStartTimeMS(), startMS);
                        }
                    }
                    else if (eff->GetEndTimeMS() > endMS)
                    {
                        if (!eff->IsLocked())
                        {
                            TruncateEffect(el, eff, endMS, eff->GetEndTimeMS());
                        }
                    }
                    else
                    {
                        mSequenceElements->get_undo_mgr().CaptureEffectToBeDeleted(el->GetParentElement()->GetModelName(), el->GetIndex(), eff->GetEffectName(), eff->GetSettingsAsString(), eff->GetPaletteAsString(), eff->GetStartTimeMS(), eff->GetEndTimeMS(), EFFECT_NOT_SELECTED, false);
                        el->RemoveEffect(i);
                        if (eff == mSelectedEffect) {
                            UnselectEffect();
                            mSelectedEffect = nullptr;
                        }
                        --i;
                    }
                }
                sendRenderEvent(el->GetParentElement()->GetModelName(), startMS, endMS);

                break;
            case ACTYPE::ON:
            case ACTYPE::SHIMMER:
            case ACTYPE::TWINKLE:
            {
                if (mode == ACMODE::FOREGROUND)
                {
                    for (auto i = 0; i < el->GetEffectCount(); ++i)
                    {
                        Effect* eff = el->GetEffect(i);
                        if (eff == nullptr) logger_base.crit("DDD GetEffect about to crash %d", i);

                        if (eff->GetStartTimeMS() < endMS && eff->GetEndTimeMS() > startMS)
                        {
                            int start = std::max(eff->GetStartTimeMS(), startMS);
                            int end = std::min(eff->GetEndTimeMS(), endMS);

                            // remove the effect we are about to replace
                            mSequenceElements->get_undo_mgr().CaptureEffectToBeDeleted(el->GetParentElement()->GetModelName(), el->GetIndex(), eff->GetEffectName(), eff->GetSettingsAsString(), eff->GetPaletteAsString(), eff->GetStartTimeMS(), eff->GetEndTimeMS(), EFFECT_NOT_SELECTED, false);
                            el->RemoveEffect(i);
                            if (eff == mSelectedEffect) {
                                UnselectEffect();
                                mSelectedEffect = nullptr;
                            }

                            if (style == ACSTYLE::RAMPUPDOWN)
                            {
                                Effect* ef = CreatePartialACEffect(el, type, startMS, endMS, start, end, a, b, a, first);
                                if (res == nullptr) res = ef;
                            }
                            else if (style == ACSTYLE::INTENSITY)
                            {
                                Effect* ef = CreatePartialACEffect(el, type, startMS, endMS, start, end, intensity, -1, intensity, first);
                                if (res == nullptr) res = ef;
                            }
                            else if (style == ACSTYLE::RAMPUP)
                            {
                                Effect* ef = CreatePartialACEffect(el, type, startMS, endMS, start, end, a, -1, b, first);
                                if (res == nullptr) res = ef;
                            }
                            else if (style == ACSTYLE::RAMPDOWN)
                            {
                                Effect* ef = CreatePartialACEffect(el, type, startMS, endMS, start, end, a, -1, b, first);
                                if (res == nullptr) res = ef;
                            }
                        }
                    }
                }
                else if (mode == ACMODE::BACKGROUND)
                {
                    int start = -1;
                    int end = -1;
                    for (auto i = 0; i < el->GetEffectCount(); ++i)
                    {
                        Effect* eff = el->GetEffect(i);
                        if (eff == nullptr) logger_base.crit("EEE GetEffect about to crash %d", i);

                        if (eff->GetStartTimeMS() < end)
                        {
                            // this is an effect we just added
                        }
                        else if (eff->GetStartTimeMS() <= startMS && eff->GetEndTimeMS() < endMS)
                        {
                            start = 1; // anything but -1
                            end = eff->GetEndTimeMS();
                        }
                        else if (eff->GetStartTimeMS() > startMS && eff->GetStartTimeMS() <= endMS && start == -1)
                        {
                            start = startMS;
                            end = eff->GetEndTimeMS();
                            if (style == ACSTYLE::RAMPUPDOWN)
                            {
                                Effect* ef = CreatePartialACEffect(el, type, startMS, endMS, start, eff->GetStartTimeMS(), a, b, a, first);
                                if (res == nullptr) res = ef;
                            }
                            else if (style == ACSTYLE::INTENSITY)
                            {
                                Effect* ef = CreatePartialACEffect(el, type, startMS, endMS, start, eff->GetStartTimeMS(), intensity, -1, intensity, first);
                                if (res == nullptr) res = ef;
                            }
                            else if (style == ACSTYLE::RAMPUP)
                            {
                                Effect* ef = CreatePartialACEffect(el, type, startMS, endMS, start, eff->GetStartTimeMS(), a, -1, b, first);
                                if (res == nullptr) res = ef;
                            }
                            else if (style == ACSTYLE::RAMPDOWN)
                            {
                                Effect* ef = CreatePartialACEffect(el, type, startMS, endMS, start, eff->GetStartTimeMS(), a, -1, b, first);
                                if (res == nullptr) res = ef;
                            }
                        }
                        else if (eff->GetStartTimeMS() > startMS && eff->GetStartTimeMS() <= endMS)
                        {
                            start = end;
                            if (eff->GetStartTimeMS() == end)
                            {
                                // no gap
                            }
                            else
                            {
                                if (style == ACSTYLE::RAMPUPDOWN)
                                {
                                    Effect* ef = CreatePartialACEffect(el, type, startMS, endMS, start, eff->GetStartTimeMS(), a, b, a, first);
                                    if (res == nullptr) res = ef;
                                }
                                else if (style == ACSTYLE::INTENSITY)
                                {
                                    Effect* ef = CreatePartialACEffect(el, type, startMS, endMS, start, eff->GetStartTimeMS(), intensity, -1, intensity, first);
                                    if (res == nullptr) res = ef;
                                }
                                else if (style == ACSTYLE::RAMPUP)
                                {
                                    Effect* ef = CreatePartialACEffect(el, type, startMS, endMS, start, eff->GetStartTimeMS(), a, -1, b, first);
                                    if (res == nullptr) res = ef;
                                }
                                else if (style == ACSTYLE::RAMPDOWN)
                                {
                                    Effect* ef = CreatePartialACEffect(el, type, startMS, endMS, start, eff->GetStartTimeMS(), a, -1, b, first);
                                    if (res == nullptr) res = ef;
                                }
                            }
                            end = eff->GetEndTimeMS();
                        }
                        else if (eff->GetStartTimeMS() >= endMS && end != -1)
                        {
                            start = end;
                            if (style == ACSTYLE::RAMPUPDOWN)
                            {
                                Effect* ef = CreatePartialACEffect(el, type, startMS, endMS, start, endMS, a, b, a, first);
                                if (res == nullptr) res = ef;
                            }
                            else if (style == ACSTYLE::INTENSITY)
                            {
                                Effect* ef = CreatePartialACEffect(el, type, startMS, endMS, start, endMS, intensity, -1, intensity, first);
                                if (res == nullptr) res = ef;
                            }
                            else if (style == ACSTYLE::RAMPUP)
                            {
                                Effect* ef = CreatePartialACEffect(el, type, startMS, endMS, start, endMS, a, -1, b, first);
                                if (res == nullptr) res = ef;
                            }
                            else if (style == ACSTYLE::RAMPDOWN)
                            {
                                Effect* ef = CreatePartialACEffect(el, type, startMS, endMS, start, endMS, a, -1, b, first);
                                if (res == nullptr) res = ef;
                            }
                            end = -1;
                        }
                    }

                    if (end == -1 && start == -1)
                    {
                        if (style == ACSTYLE::RAMPUPDOWN)
                        {
                            Effect* ef = CreatePartialACEffect(el, type, startMS, endMS, startMS, endMS, a, b, a, first);
                            if (res == nullptr) res = ef;
                        }
                        else if (style == ACSTYLE::INTENSITY)
                        {
                            Effect* ef = CreatePartialACEffect(el, type, startMS, endMS, startMS, endMS, intensity, -1, intensity, first);
                            if (res == nullptr) res = ef;
                        }
                        else if (style == ACSTYLE::RAMPUP)
                        {
                            Effect* ef = CreatePartialACEffect(el, type, startMS, endMS, startMS, endMS, a, -1, b, first);
                            if (res == nullptr) res = ef;
                        }
                        else if (style == ACSTYLE::RAMPDOWN)
                        {
                            Effect* ef = CreatePartialACEffect(el, type, startMS, endMS, startMS, endMS, a, -1, b, first);
                            if (res == nullptr) res = ef;
                        }
                    }
                    else if (end != -1)
                    {
                        start = end;
                        if (style == ACSTYLE::RAMPUPDOWN)
                        {
                            Effect* ef = CreatePartialACEffect(el, type, startMS, endMS, start, endMS, a, b, a, first);
                            if (res == nullptr) res = ef;
                        }
                        else if (style == ACSTYLE::INTENSITY)
                        {
                            Effect* ef = CreatePartialACEffect(el, type, startMS, endMS, start, endMS, intensity, -1, intensity, first);
                            if (res == nullptr) res = ef;
                        }
                        else if (style == ACSTYLE::RAMPUP)
                        {
                            Effect* ef = CreatePartialACEffect(el, type, startMS, endMS, start, endMS, a, -1, b, first);
                            if (res == nullptr) res = ef;
                        }
                        else if (style == ACSTYLE::RAMPDOWN)
                        {
                            Effect* ef = CreatePartialACEffect(el, type, startMS, endMS, start, endMS, a, -1, b, first);
                            if (res == nullptr) res = ef;
                        }
                    }
                }
                else
                {
                    if (style == ACSTYLE::RAMPUPDOWN)
                    {
                        Effect* ef = CreateACEffect(el, type, startMS, endMS, a, b, a, first);
                        if (res == nullptr) res = ef;
                    }
                    else if (style == ACSTYLE::INTENSITY)
                    {
                        Effect* ef = CreateACEffect(el, type, startMS, endMS, intensity, -1, intensity, first);
                        if (res == nullptr) res = ef;
                    }
                    else if (style == ACSTYLE::RAMPUP)
                    {
                        Effect* ef = CreateACEffect(el, type, startMS, endMS, a, -1, b, first);
                        if (res == nullptr) res = ef;
                    }
                    else if (style == ACSTYLE::RAMPDOWN)
                    {
                        Effect* ef = CreateACEffect(el, type, startMS, endMS, a, -1, b, first);
                        if (res == nullptr) res = ef;
                    }
                }
                sendRenderEvent(el->GetParentElement()->GetModelName(), startMS, endMS);
            }
            break;
            case SELECT:
            case NILTYPEOVERRIDE:
            break;
            }

            first = false;
        }
    }

    return res;
}

void EffectsGrid::ACCascade(int startMS, int endMS, int startCol, int endCol, int startRow, int endRow)
{
    EffectLayer* el = mSequenceElements->GetVisibleEffectLayer(startRow - mSequenceElements->GetFirstVisibleModelRow());
    if (el == nullptr) return;
    Element *e = el->GetParentElement();

    int inc = 1;
    if (startRow > endRow) inc = -1;

    if (e->GetType() == ElementType::ELEMENT_TYPE_TIMING) return;

    // exclude timing from end rows
    EffectLayer* eler = mSequenceElements->GetVisibleEffectLayer(endRow - mSequenceElements->GetFirstVisibleModelRow());
    if (eler == nullptr) return;
    Element *eer = eler->GetParentElement();
    while (eer->GetType() == ElementType::ELEMENT_TYPE_TIMING)
    {
        endRow -= inc;

        if (startRow == endRow) return;

        eler = mSequenceElements->GetVisibleEffectLayer(endRow - mSequenceElements->GetFirstVisibleModelRow());
        if (eler == nullptr) return;
        eer = eler->GetParentElement();
    }

    int extraLayers = 0;
    std::list<EffectLayer*> uniqueLayers;
    for (int i = std::min(startRow, endRow); i <= std::max(startRow, endRow); i++)
    {
        if (i != startRow)
        {
            EffectLayer* elTarget = mSequenceElements->GetVisibleEffectLayer(i - mSequenceElements->GetFirstVisibleModelRow());
            if (std::find(uniqueLayers.begin(), uniqueLayers.end(), elTarget) == uniqueLayers.end() && elTarget != el)
            {
                uniqueLayers.push_back(elTarget);
            }
            else
            {
                extraLayers++;
            }
        }
    }

    int actualStart = -1;
    int actualEnd = -1;

    for (auto i = 0; i < el->GetEffectCount(); ++i)
    {
        Effect* eff = el->GetEffect(i);

        if (eff->GetStartTimeMS() <= startMS && eff->GetEndTimeMS() > startMS)
        {
            actualStart = startMS;
        }
        else if (eff->GetStartTimeMS() >= startMS && eff->GetStartTimeMS() < endMS && actualStart == -1)
        {
            actualStart = eff->GetStartTimeMS();
        }

        if (eff->GetStartTimeMS() < endMS && eff->GetEndTimeMS() >= endMS)
        {
            actualEnd = endMS;
        }
        else if (eff->GetEndTimeMS() >= startMS && eff->GetEndTimeMS() <= endMS && actualEnd < eff->GetEndTimeMS())
        {
            actualEnd = eff->GetEndTimeMS();
        }
    }

    if ((actualStart == -1 && actualEnd == -1) || startRow == endRow) return;

    int spaceToCascade;
    int dirFactor = 1;
    if (startCol > endCol)
    {
        // cascade to left
        spaceToCascade = actualStart - startMS;
        dirFactor = -1;
    }
    else
    {
        // cascade right ... the default
        spaceToCascade = endMS - actualEnd;
    }

    int perRowOffsetMS = 0;
    if (startRow != endRow)
    {
        perRowOffsetMS = spaceToCascade / (abs(startRow - endRow) - extraLayers);
    }

    std::list<EffectLayer*> layerUsed;
    int seenExtraLayers = 0;
    if (spaceToCascade == 0)
    {
        // just do a copy
        for (int i = std::min(startRow, endRow); i <= std::max(startRow, endRow); i++)
        {
            if (i != startRow)
            {
                EffectLayer* elTarget = mSequenceElements->GetVisibleEffectLayer(i - mSequenceElements->GetFirstVisibleModelRow());
                Element *eTarget = elTarget->GetParentElement();

                if (eTarget->GetType() != ElementType::ELEMENT_TYPE_TIMING && el != elTarget)
                {
                    if (std::find(layerUsed.begin(), layerUsed.end(), elTarget) == layerUsed.end())
                    {
                        layerUsed.push_back(elTarget);
                        // erase everything in the target first
                        ACDraw(ACTYPE::OFF, ACSTYLE::INTENSITY, ACMODE::MODENIL, 0, 0, 0, startMS, endMS, i, i);

                        if (eTarget->GetType() != ElementType::ELEMENT_TYPE_TIMING)
                        {
                            for (auto j = 0; j < el->GetEffectCount(); ++j)
                            {
                                Effect* eff = el->GetEffect(j);

                                if (eff->GetStartTimeMS() < startMS && eff->GetEndTimeMS() > startMS)
                                {
                                    // copy end
                                    DuplicateAndTruncateEffect(elTarget, eff->GetSettings(), eff->GetPaletteAsString(), eff->GetEffectName(), eff->GetStartTimeMS(), eff->GetEndTimeMS(), startMS, std::min(endMS, eff->GetEndTimeMS()));
                                }
                                else if (eff->GetStartTimeMS() >= startMS && eff->GetStartTimeMS() < endMS && eff->GetEndTimeMS() > endMS)
                                {
                                    // copy start
                                    DuplicateAndTruncateEffect(elTarget, eff->GetSettings(), eff->GetPaletteAsString(), eff->GetEffectName(), eff->GetStartTimeMS(), eff->GetEndTimeMS(), eff->GetStartTimeMS(), endMS);
                                }
                                else if (eff->GetStartTimeMS() <= startMS && eff->GetEndTimeMS() >= endMS)
                                {
                                    DuplicateAndTruncateEffect(elTarget, eff->GetSettings(), eff->GetPaletteAsString(), eff->GetEffectName(), eff->GetStartTimeMS(), eff->GetEndTimeMS(), startMS, endMS);
                                }
                                else if (eff->GetStartTimeMS() >= startMS && eff->GetEndTimeMS() <= endMS)
                                {
                                    // copy whole
                                    Effect* effNew = elTarget->AddEffect(0, eff->GetEffectName(), eff->GetSettingsAsString(), eff->GetPaletteAsString(), eff->GetStartTimeMS(), eff->GetEndTimeMS(), EFFECT_NOT_SELECTED, false);
                                    if (effNew != nullptr)
                                    {
                                        effNew->SetLocked(false);
                                        mSequenceElements->get_undo_mgr().CaptureAddedEffect(elTarget->GetParentElement()->GetModelName(), elTarget->GetIndex(), effNew->GetID());
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        for (int r = startRow + inc; r != endRow + inc; r += inc)
        {
            int cascades = dirFactor * abs(r - startRow - seenExtraLayers);
            int cascadeMS = TimeLine::RoundToMultipleOfPeriod(cascades * perRowOffsetMS, mSequenceElements->GetFrequency());

            EffectLayer* elTarget = mSequenceElements->GetVisibleEffectLayer(r - mSequenceElements->GetFirstVisibleModelRow());
            Element *eTarget = elTarget->GetParentElement();

            if (eTarget->GetType() != ElementType::ELEMENT_TYPE_TIMING && el != elTarget)
            {
                if (std::find(layerUsed.begin(), layerUsed.end(), elTarget) == layerUsed.end())
                {
                    layerUsed.push_back(elTarget);

                    // erase everything in the target first
                    ACDraw(ACTYPE::OFF, ACSTYLE::INTENSITY, ACMODE::MODENIL, 0, 0, 0, startMS, endMS, r, r);

                    for (auto j = 0; j < el->GetEffectCount(); ++j)
                    {
                        Effect* eff = el->GetEffect(j);

                        if (eff->GetStartTimeMS() < startMS && eff->GetEndTimeMS() > startMS)
                        {
                            // copy end
                            DuplicateAndTruncateEffect(elTarget, eff->GetSettings(), eff->GetPaletteAsString(), eff->GetEffectName(), eff->GetStartTimeMS(), eff->GetEndTimeMS(), startMS, eff->GetEndTimeMS(), cascadeMS);
                        }
                        else if (eff->GetStartTimeMS() >= startMS && eff->GetStartTimeMS() < endMS && eff->GetEndTimeMS() > endMS)
                        {
                            // copy start
                            DuplicateAndTruncateEffect(elTarget, eff->GetSettings(), eff->GetPaletteAsString(), eff->GetEffectName(), eff->GetStartTimeMS(), eff->GetEndTimeMS(), eff->GetStartTimeMS(), endMS, cascadeMS);
                        }
                        else if (eff->GetStartTimeMS() >= startMS && eff->GetEndTimeMS() <= endMS)
                        {
                            // copy whole
                            Effect* effNew = elTarget->AddEffect(0, eff->GetEffectName(), eff->GetSettingsAsString(), eff->GetPaletteAsString(), eff->GetStartTimeMS() + cascadeMS, eff->GetEndTimeMS() + cascadeMS, EFFECT_NOT_SELECTED, false);
                            if (effNew != nullptr)
                            {
                                effNew->SetLocked(false);
                                mSequenceElements->get_undo_mgr().CaptureAddedEffect(elTarget->GetParentElement()->GetModelName(), elTarget->GetIndex(), effNew->GetID());
                            }
                        }
                    }
                }
            }
        }
    }
}

int EffectsGrid::GetEffectBrightnessAt(std::string effName, SettingsMap settings, float pos, long startMS, long endMS)
{
    if (effName == "On")
    {
        return (settings.GetInt("E_TEXTCTRL_Eff_On_End", 100) - settings.GetInt("E_TEXTCTRL_Eff_On_Start", 100)) * pos + settings.GetInt("E_TEXTCTRL_Eff_On_Start", 100);
    }
    else if (effName == "Twinkle")
    {
        if (wxString(settings.Get("C_VALUECURVE_Brightness", "")).Contains("Active=TRUE"))
        {
            ValueCurve vc(settings.Get("C_VALUECURVE_Brightness", ""));
            vc.SetLimits(0, 400);
            return vc.GetOutputValueAt(pos, startMS, endMS);
        }
        else
        {
            return settings.GetInt("C_SLIDER_Brightness", 100);
        }
    }

    return 100;
}

std::string EffectsGrid::TruncateEffectSettings(SettingsMap settings, std::string name, int originalStartMS, int originalEndMS, int startMS, int endMS)
{
    int originalLength = originalEndMS - originalStartMS;
    double startPos = ((double)startMS - (double)originalStartMS) / (double)originalLength;
    double endPos = ((double)endMS - (double)originalStartMS) / (double)originalLength;

    if (name == "On")
    {
        int startBrightness = GetEffectBrightnessAt(name, settings, 0.0, startMS, endMS);
        int endBrightness = GetEffectBrightnessAt(name, settings, 1.0, startMS, endMS);

        if (startBrightness != endBrightness)
        {
            int newStartBrightness = (endBrightness - startBrightness) * startPos + startBrightness;
            int newEndBrightness = (endBrightness - startBrightness) * endPos + startBrightness;
            settings["E_TEXTCTRL_Eff_On_Start"] = wxString::Format("%i", newStartBrightness);
            settings["E_TEXTCTRL_Eff_On_End"] = wxString::Format("%i", newEndBrightness);
        }
    }
    else if (name == "Twinkle")
    {
        if (wxString(settings.Get("C_VALUECURVE_Brightness", "")).Contains("Active=TRUE"))
        {
            ValueCurve vc(settings.Get("C_VALUECURVE_Brightness", ""));
            vc.SetLimits(0, 400);

            TruncateBrightnessValueCurve(vc, startPos, endPos, startMS, endMS, originalLength);

            settings["C_VALUECURVE_Brightness"] = vc.Serialise();
        }
    }

    return settings.AsString();
}

Effect* EffectsGrid::DuplicateAndTruncateEffect(EffectLayer* el, SettingsMap settings, std::string palette, std::string name, int originalStartMS, int originalEndMS, int startMS, int endMS, int offsetMS)
{
    Effect* res = nullptr;
    std::string ss = TruncateEffectSettings(settings, name, originalStartMS, originalEndMS, startMS, endMS);

    if (name == "On")
    {
        Effect* eff = el->AddEffect(0, name, ss, palette, startMS + offsetMS, endMS + offsetMS, EFFECT_NOT_SELECTED, false);
        if (res == nullptr) res = eff;
        if (eff != nullptr)
        {
            eff->SetLocked(false);
            mSequenceElements->get_undo_mgr().CaptureAddedEffect(el->GetParentElement()->GetModelName(), el->GetIndex(), eff->GetID());
        }
    }
    else if (name == "Twinkle")
    {
        Effect* eff = el->AddEffect(0, name, ss, palette, startMS + offsetMS, endMS + offsetMS, EFFECT_NOT_SELECTED, false);
        if (res == nullptr) res = eff;
        if (eff != nullptr)
        {
            eff->SetLocked(false);
            mSequenceElements->get_undo_mgr().CaptureAddedEffect(el->GetParentElement()->GetModelName(), el->GetIndex(), eff->GetID());
        }
    }
    else
    {
        Effect* eff = el->AddEffect(0, name, settings.AsString(), palette, startMS + offsetMS, endMS + offsetMS, EFFECT_NOT_SELECTED, false);
        if (res == nullptr) res = eff;
        if (eff != nullptr)
        {
            eff->SetLocked(false);
            mSequenceElements->get_undo_mgr().CaptureAddedEffect(el->GetParentElement()->GetModelName(), el->GetIndex(), eff->GetID());
        }
    }
    return res;
}

void EffectsGrid::TruncateBrightnessValueCurve(ValueCurve& vc, double startPos, double endPos, int startMS, int endMS, int originalLength)
{
    if (vc.GetType() == "Ramp")
    {
        if (startPos != 0)
        {
            int newStartBrightness = vc.GetOutputValueAt(startPos, startMS, endMS);
            vc.SetParameter1(newStartBrightness / 4);
        }
        if (endPos != 1.0)
        {
            int newEndBrightness = vc.GetOutputValueAt(endPos, startMS, endMS);
            vc.SetParameter2(newEndBrightness / 4);
        }
    }
}

void EffectsGrid::TruncateEffect(EffectLayer* el, Effect* eff, int startMS, int endMS)
{
    wxASSERT(!eff->IsLocked());

    int originalLength = eff->GetEndTimeMS() - eff->GetStartTimeMS();
    double startPos = ((double)startMS - (double)eff->GetStartTimeMS()) / (double)originalLength;
    double endPos = ((double)endMS - (double)eff->GetStartTimeMS()) / (double)originalLength;
    std::string name = eff->GetEffectName();

    if (eff->GetStartTimeMS() == startMS && endMS < eff->GetEndTimeMS())
    {
        // chop off the end
        mSequenceElements->get_undo_mgr().CaptureModifiedEffect(el->GetParentElement()->GetModelName(), el->GetIndex(), eff->GetID(), eff->GetSettingsAsString(), eff->GetPaletteAsString());
        mSequenceElements->get_undo_mgr().CaptureEffectToBeMoved(el->GetParentElement()->GetModelName(), el->GetIndex(), eff->GetID(), eff->GetStartTimeMS(), eff->GetEndTimeMS());
        eff->SetEndTimeMS(endMS);

        // now fix the brightness ... the hard part
        if (name == "On")
        {
            int startBrightness = GetEffectBrightnessAt(eff->GetEffectName(), eff->GetSettings(), 0.0, startMS, endMS);
            int endBrightness = GetEffectBrightnessAt(eff->GetEffectName(), eff->GetSettings(), 1.0, startMS, endMS);

            if (startBrightness != endBrightness)
            {
                int newEndBrightness = (endBrightness - startBrightness) * endPos + startBrightness;
                eff->GetSettings()["E_TEXTCTRL_Eff_On_End"] = wxString::Format("%i", newEndBrightness);
                eff->IncrementChangeCount();
                RaiseSelectedEffectChanged(eff, false, true);
            }
        }
        else if (name == "Twinkle")
        {
            if (wxString(eff->GetSettings().Get("C_VALUECURVE_Brightness", "")).Contains("Active=TRUE"))
            {
                ValueCurve vc(eff->GetSettings().Get("C_VALUECURVE_Brightness", ""));
                vc.SetLimits(0, 400);

                TruncateBrightnessValueCurve(vc, startPos, endPos, startMS, endMS, originalLength);

                eff->GetSettings()["C_VALUECURVE_Brightness"] = vc.Serialise();
                eff->IncrementChangeCount();
                RaiseSelectedEffectChanged(eff, false, true);
            }
        }
    }
    else if (eff->GetEndTimeMS() == endMS && startMS > eff->GetStartTimeMS())
    {
        // chop off the start
        mSequenceElements->get_undo_mgr().CaptureModifiedEffect(el->GetParentElement()->GetModelName(), el->GetIndex(), eff->GetID(), eff->GetSettingsAsString(), eff->GetPaletteAsString());
        mSequenceElements->get_undo_mgr().CaptureEffectToBeMoved(el->GetParentElement()->GetModelName(), el->GetIndex(), eff->GetID(), eff->GetStartTimeMS(), eff->GetEndTimeMS());
        eff->SetStartTimeMS(startMS);

        // now fix the brightness ... the hard part
        if (name == "On")
        {
            int startBrightness = GetEffectBrightnessAt(eff->GetEffectName(), eff->GetSettings(), 0.0, startMS, endMS);
            int endBrightness = GetEffectBrightnessAt(eff->GetEffectName(), eff->GetSettings(), 1.0, startMS, endMS);

            if (startBrightness != endBrightness)
            {
                int newStartBrightness = (endBrightness - startBrightness) * startPos + startBrightness;
                eff->GetSettings()["E_TEXTCTRL_Eff_On_Start"] = wxString::Format("%i", newStartBrightness);
                eff->IncrementChangeCount();
                RaiseSelectedEffectChanged(eff, false, true);
            }
        }
        else if (name == "Twinkle")
        {
            if (wxString(eff->GetSettings().Get("C_VALUECURVE_Brightness", "")).Contains("Active=TRUE"))
            {
                ValueCurve vc(eff->GetSettings().Get("C_VALUECURVE_Brightness", ""));
                vc.SetLimits(0, 400);

                TruncateBrightnessValueCurve(vc, startPos, endPos, startMS, endMS, originalLength);

                eff->GetSettings()["C_VALUECURVE_Brightness"] = vc.Serialise();
                eff->IncrementChangeCount();
                RaiseSelectedEffectChanged(eff, false, true);
            }
        }
    }
    else if (eff->GetStartTimeMS() < startMS && eff->GetEndTimeMS() > endMS)
    {
        // chop start and end
        // I dont think I need this
        wxASSERT(false);
    }
    else if (eff->GetStartTimeMS() == startMS && eff->GetEndTimeMS() == endMS)
    {
        // dont need to do anything
    }
    else
    {
        // this case cant be handled
        wxASSERT(false);
    }
}

Effect* EffectsGrid::CreateACEffect(EffectLayer* el, std::string name, std::string settings, int startMS, int endMS, bool select, std::string pal)
{
    Effect* res = nullptr;
    // because an element may appear multiple times when drawing this stops an error being reported
    // it assumes we always correctly pre-clear the area we are drawing in
    // if we dont this will prevent the effect from being created.
    if (!el->HasEffectsInTimeRange(startMS, endMS))
    {
        std::string palette = "C_BUTTON_Palette1=#FFFFFF,C_CHECKBOX_Palette1=1";
        if (pal != "")
        {
            palette += "," + pal;
        }
        Effect* eff = el->AddEffect(0, name, settings, palette, startMS, endMS, (select ? EFFECT_SELECTED : EFFECT_NOT_SELECTED), false);
        if (res == nullptr) res = eff;
        if (eff != nullptr) {
            mSequenceElements->get_undo_mgr().CaptureAddedEffect(el->GetParentElement()->GetModelName(), el->GetIndex(), eff->GetID());
        }
    }
    return res;
}

Effect* EffectsGrid::CreateACEffect(EffectLayer* el, ACTYPE type, int startMS, int endMS, int startBrightness, int midBrightness, int endBrightness, bool select)
{
    Effect* res = nullptr;
    std::string settings = "";
    std::string pal = "";

    if (type == ACTYPE::ON)
    {
        if (midBrightness == -1)
        {
            if (startBrightness != 100 || endBrightness != 100)
            {
                settings = wxString::Format("E_TEXTCTRL_Eff_On_End=%i,E_TEXTCTRL_Eff_On_Start=%i", endBrightness, startBrightness).ToStdString();
            }
            Effect* ef = CreateACEffect(el, "On", settings, startMS, endMS, select);
            if (res == nullptr) res = ef;
        }
        else
        {
            int mid = TimeLine::RoundToMultipleOfPeriod((startMS + endMS) / 2, mSequenceElements->GetFrequency());
            if (startBrightness != 100 || midBrightness != 100)
            {
                settings = wxString::Format("E_TEXTCTRL_Eff_On_End=%i,E_TEXTCTRL_Eff_On_Start=%i", midBrightness, startBrightness).ToStdString();
            }
            Effect* ef = CreateACEffect(el, "On", settings, startMS, mid, select);
            if (res == nullptr) res = ef;

            if (endBrightness != 100 || midBrightness != 100)
            {
                settings = wxString::Format("E_TEXTCTRL_Eff_On_End=%i,E_TEXTCTRL_Eff_On_Start=%i", endBrightness, midBrightness).ToStdString();
            }
            ef = CreateACEffect(el, "On", settings, mid, endMS, select);
            if (res == nullptr) res = ef;
        }
    }
    else if (type == ACTYPE::SHIMMER)
    {
        if (midBrightness == -1)
        {
            if (startBrightness != 100 || endBrightness != 100)
            {
                settings = wxString::Format("E_CHECKBOX_On_Shimmer=1,E_TEXTCTRL_Eff_On_End=%i,E_TEXTCTRL_Eff_On_Start=%i", endBrightness, startBrightness).ToStdString();
            }
            else
            {
                settings = "E_CHECKBOX_On_Shimmer=1";
            }
            Effect* ef = CreateACEffect(el, "On", settings, startMS, endMS, select);
            if (res == nullptr) res = ef;
        }
        else
        {
            int mid = TimeLine::RoundToMultipleOfPeriod((startMS + endMS) / 2, mSequenceElements->GetFrequency());
            if (startBrightness != 100 || midBrightness != 100)
            {
                settings = wxString::Format("E_CHECKBOX_On_Shimmer=1,E_TEXTCTRL_Eff_On_End=%i,E_TEXTCTRL_Eff_On_Start=%i", midBrightness, startBrightness).ToStdString();
            }
            else
            {
                settings = "E_CHECKBOX_On_Shimmer=1";
            }
            Effect* ef = CreateACEffect(el, "On", settings, startMS, mid, select);
            if (res == nullptr) res = ef;

            if (endBrightness != 100 || midBrightness != 100)
            {
                settings = wxString::Format("E_CHECKBOX_On_Shimmer=1,E_TEXTCTRL_Eff_On_End=%i,E_TEXTCTRL_Eff_On_Start=%i", endBrightness, midBrightness).ToStdString();
            }
            else
            {
                settings = "E_CHECKBOX_On_Shimmer=1";
            }
            ef = CreateACEffect(el, "On", settings, mid, endMS, select);
            if (res == nullptr) res = ef;
        }
    }
    else if (type == ACTYPE::TWINKLE)
    {
        if (midBrightness == -1 || (midBrightness == startBrightness && midBrightness == endBrightness))
        {
            if (startBrightness == endBrightness)
            {
                // Intensity
                if (startBrightness != 100)
                {
                    settings = wxString::Format("C_SLIDER_Brightness=%i", startBrightness).ToStdString();
                }
            }
            else
            {
                // Ramp
                pal = wxString::Format("C_VALUECURVE_Brightness=Active=TRUE|Id=ID_VALUECURVE_Brightness|Type=Ramp|Min=0.00|Max=400.00|P1=%i|P2=%i|", startBrightness / 4, endBrightness / 4).ToStdString();
            }
            Effect* ef = CreateACEffect(el, "Twinkle", settings, startMS, endMS, select, pal);
            if (res == nullptr) res = ef;
        }
        else
        {
            // ramp up/down
            int mid = TimeLine::RoundToMultipleOfPeriod((startMS + endMS) / 2, mSequenceElements->GetFrequency());

            pal = wxString::Format("C_VALUECURVE_Brightness=Active=TRUE|Id=ID_VALUECURVE_Brightness|Type=Ramp|Min=0.00|Max=400.00|P1=%i|P2=%i|", startBrightness / 4, midBrightness / 4).ToStdString();

            Effect* ef = CreateACEffect(el, "Twinkle", settings, startMS, mid, select, pal);
            if (res == nullptr) res = ef;

            pal = wxString::Format("C_VALUECURVE_Brightness=Active=TRUE|Id=ID_VALUECURVE_Brightness|Type=Ramp|Min=0.00|Max=400.00|P1=%i|P2=%i|", midBrightness / 4, endBrightness / 4).ToStdString();

            ef = CreateACEffect(el, "Twinkle", settings, mid, endMS, select, pal);
            if (res == nullptr) res = ef;
        }
    }

    return res;
}

Effect* EffectsGrid::CreatePartialACEffect(EffectLayer* el, ACTYPE type, int startMS, int endMS, int partialStart, int partialEnd, int startBrightness, int midBrightness, int endBrightness, bool select)
{
    Effect* res = nullptr;
    std::string palette = "C_BUTTON_Palette1=#FFFFFF,C_CHECKBOX_Palette1=1";

    if (type == ACTYPE::ON)
    {
        if (midBrightness == -1)
        {
            SettingsMap settings;
            if (startBrightness != 100 || endBrightness != 100)
            {
                settings["E_TEXTCTRL_Eff_On_End"] = wxString::Format("%i", endBrightness);
                settings["E_TEXTCTRL_Eff_On_Start"] = wxString::Format("%i", startBrightness);
            }
            Effect* ef = DuplicateAndTruncateEffect(el, settings, palette, "On", startMS, endMS, partialStart, partialEnd);
            if (res == nullptr) res = ef;
        }
        else
        {
            SettingsMap settings;
            int mid = TimeLine::RoundToMultipleOfPeriod((startMS + endMS) / 2, mSequenceElements->GetFrequency());
            if (startBrightness != 100 || midBrightness != 100)
            {
                settings["E_TEXTCTRL_Eff_On_End"] = wxString::Format("%i", midBrightness);
                settings["E_TEXTCTRL_Eff_On_Start"] = wxString::Format("%i", startBrightness);
            }
            if (partialStart < mid)
            {
                Effect* ef = DuplicateAndTruncateEffect(el, settings, palette, "On", startMS, endMS, partialStart, std::min(mid, partialEnd));
                if (res == nullptr) res = ef;
            }

            settings.clear();
            if (endBrightness != 100 || midBrightness != 100)
            {
                settings["E_TEXTCTRL_Eff_On_End"] = wxString::Format("%i", endBrightness);
                settings["E_TEXTCTRL_Eff_On_Start"] = wxString::Format("%i", midBrightness);
            }
            if (partialEnd > mid)
            {
                Effect* ef = DuplicateAndTruncateEffect(el, settings, palette, "On", startMS, endMS, std::max(mid, partialStart), partialEnd);
                if (res == nullptr) res = ef;
            }
        }
    }
    else if (type == ACTYPE::SHIMMER)
    {
        if (midBrightness == -1)
        {
            SettingsMap settings;
            settings["E_CHECKBOX_On_Shimmer"] = "1";
            if (startBrightness != 100 || endBrightness != 100)
            {
                settings["E_TEXTCTRL_Eff_On_End"] = wxString::Format("%i", endBrightness);
                settings["E_TEXTCTRL_Eff_On_Start"] = wxString::Format("%i", startBrightness);
            }
            Effect* ef = DuplicateAndTruncateEffect(el, settings, palette, "On", startMS, endMS, partialStart, partialEnd);
            if (res == nullptr) res = ef;
        }
        else
        {
            SettingsMap settings;
            settings["E_CHECKBOX_On_Shimmer"] = "1";
            int mid = TimeLine::RoundToMultipleOfPeriod((startMS + endMS) / 2, mSequenceElements->GetFrequency());
            if (startBrightness != 100 || midBrightness != 100)
            {
                settings["E_TEXTCTRL_Eff_On_End"] = wxString::Format("%i", midBrightness);
                settings["E_TEXTCTRL_Eff_On_Start"] = wxString::Format("%i", startBrightness);
            }
            if (partialStart < mid)
            {
                Effect* ef = DuplicateAndTruncateEffect(el, settings, palette, "On", startMS, endMS, partialStart, std::min(mid, partialEnd));
                if (res == nullptr) res = ef;
            }

            settings.clear();
            settings["E_CHECKBOX_On_Shimmer"] = "1";
            if (endBrightness != 100 || midBrightness != 100)
            {
                settings["E_TEXTCTRL_Eff_On_End"] = wxString::Format("%i", endBrightness);
                settings["E_TEXTCTRL_Eff_On_Start"] = wxString::Format("%i", midBrightness);
            }
            if (partialEnd > mid)
            {
                Effect* ef = DuplicateAndTruncateEffect(el, settings, palette, "On", startMS, endMS, std::max(mid, partialStart), partialEnd);
                if (res == nullptr) res = ef;
            }
        }
    }
    else if (type == ACTYPE::TWINKLE)
    {
        if (midBrightness == -1 || (midBrightness == startBrightness && midBrightness == endBrightness))
        {
            SettingsMap settings;
            if (startBrightness == endBrightness)
            {
                // Intensity
                if (startBrightness != 100)
                {
                    settings["C_SLIDER_Brightness"] = wxString::Format("%i", startBrightness);
                }
            }
            else
            {
                // Ramp
                settings["C_VALUECURVE_Brightness"] = wxString::Format("Active=TRUE|Id=ID_VALUECURVE_Brightness|Type=Ramp|Min=0.00|Max=400.00|P1=%i|P2=%i|", startBrightness / 4, endBrightness / 4);
            }
            Effect* ef = DuplicateAndTruncateEffect(el, settings, palette, "Twinkle", startMS, endMS, partialStart, partialEnd);
            if (res == nullptr) res = ef;
        }
        else
        {
            SettingsMap settings;
            // ramp up/down
            int mid = TimeLine::RoundToMultipleOfPeriod((startMS + endMS) / 2, mSequenceElements->GetFrequency());

            settings["C_VALUECURVE_Brightness"] = wxString::Format("Active=TRUE|Id=ID_VALUECURVE_Brightness|Type=Ramp|Min=0.00|Max=400.00|P1=%i|P2=%i|", startBrightness / 4, midBrightness / 4);

            if (partialStart < mid)
            {
                Effect* ef = DuplicateAndTruncateEffect(el, settings, palette, "Twinkle", startMS, endMS, partialStart, std::min(mid, partialEnd));
                if (res == nullptr) res = ef;
            }

            settings.clear();
            settings["C_VALUECURVE_Brightness"] =wxString::Format("Active=TRUE|Id=ID_VALUECURVE_Brightness|Type=Ramp|Min=0.00|Max=400.00|P1=%i|P2=%i|", midBrightness / 4, endBrightness / 4);

            if (partialEnd > mid)
            {
                Effect* ef = DuplicateAndTruncateEffect(el, settings, palette, "Twinkle", startMS, endMS, std::max(mid, partialStart), partialEnd);
                if (res == nullptr) res = ef;
            }
        }
    }
    return res;
}

void EffectsGrid::ACFill(ACTYPE type, int startMS, int endMS, int startRow, int endRow)
{
    if (type == ACTYPE::SELECT)
    {
        type = ACTYPE::ON;
    }

    for (int r = std::min(startRow, endRow); r <= std::max(startRow, endRow); r++)
    {
        EffectLayer* el = mSequenceElements->GetVisibleEffectLayer(r - mSequenceElements->GetFirstVisibleModelRow());
        if (el == nullptr) return;

        int startBrightness = 0;
        int startTime = startMS;
        bool done = false;

        for (auto j = 0; j < el->GetEffectCount(); ++j)
        {
            Effect* eff = el->GetEffect(j);

            if (eff->GetEndTimeMS() >= startMS && eff->GetStartTimeMS() <= startMS && eff->GetEndTimeMS() < endMS)
            {
                startBrightness = GetEffectBrightnessAt(eff->GetEffectName(), eff->GetSettings(), 1.0, startMS, endMS);
                if (eff->GetEndTimeMS() > startTime)
                {
                    startTime = eff->GetEndTimeMS();
                }
            }
            else if (eff->GetStartTimeMS() > startMS && eff->GetStartTimeMS() <= endMS)
            {
                int endBrightness = GetEffectBrightnessAt(eff->GetEffectName(), eff->GetSettings(), 0.0, startMS, endMS);

                if (startTime < eff->GetStartTimeMS() && (startBrightness != 0 || endBrightness != 0))
                {
                    CreateACEffect(el, type, startTime, eff->GetStartTimeMS(), startBrightness, -1, endBrightness, false);
                }

                startTime = eff->GetEndTimeMS();

                if (startTime >= endMS)
                {
                    done = true;
                    break;
                }

                startBrightness = GetEffectBrightnessAt(eff->GetEffectName(), eff->GetSettings(), 1.0, startMS, endMS);
            }
            else if (eff->GetStartTimeMS() > endMS)
            {
                if (startTime < endMS && startBrightness != 0)
                {
                    CreateACEffect(el, type, startTime, endMS, startBrightness, -1, 0, false);
                }

                done = true;
                break;
            }
            else if (eff->GetStartTimeMS() <= startMS && eff->GetEndTimeMS() >= endMS)
            {
                done = true;
                break;
            }
        }

        if (!done)
        {
            // ramp to end
            if (startBrightness != 0)
            {
                CreateACEffect(el, type, startTime, endMS, startBrightness, -1, 0, false);
            }
        }
        sendRenderEvent(el->GetParentElement()->GetModelName(), startMS, endMS);
    }
}

bool EffectsGrid::IsACActive()
{
    return xlights->IsACActive();
}

void EffectsGrid::MakeRowVisible(int row)
{
    if (row < mSequenceElements->GetFirstVisibleModelRow())
    {
        ((MainSequencer*)mParent)->ScrollToRow(row);
    }
    else if (row > mSequenceElements->GetFirstVisibleModelRow() + mSequenceElements->GetMaxModelsDisplayed())
    {
        ((MainSequencer*)mParent)->ScrollToRow(std::max(0, row - mSequenceElements->GetMaxModelsDisplayed()));
    }
}

void EffectsGrid::MakeColVisible(int col)
{
    EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
    if (tel == nullptr) return;
    Effect* eff = tel->GetEffect(col);
    if (eff == nullptr) return;

    int startTime, endTime;
    ((MainSequencer*)mParent)->PanelTimeLine->GetViewableTimeRange(startTime, endTime);

    if (endTime - startTime < eff->GetEndTimeMS() - eff->GetStartTimeMS())
    {
        // full column is not visible
        ((MainSequencer*)mParent)->PanelTimeLine->SetStartTimeMS(eff->GetStartTimeMS());
    }
    else if (startTime > eff->GetStartTimeMS())
    {
        ((MainSequencer*)mParent)->PanelTimeLine->SetStartTimeMS(eff->GetStartTimeMS());
    }
    else if (endTime < eff->GetEndTimeMS())
    {
        ((MainSequencer*)mParent)->PanelTimeLine->SetStartTimeMS(eff->GetEndTimeMS() - (endTime - startTime));
    }
    ((MainSequencer*)mParent)->UpdateEffectGridHorizontalScrollBar();
}

bool EffectsGrid::HandleACKey(wxChar key, bool shift)
{
    if (HasCapture()) return false;

    ACTYPE old_type;
    ACSTYLE old_style;
    ACTOOL old_tool;
    ACMODE old_mode;
    xlights->GetACSettings(old_type, old_style, old_tool, old_mode);

    if (mRangeStartRow == -1 || mRangeStartCol == -1 || mRangeEndRow == -1 || mRangeEndCol == -1)
    {
        SetRCToolTip();
        // nothing selected
        return false;
    }

    if (key == 'h' || key == 'H')
    {
        xlights->SetACSettings(ACTOOL::CASCADE);
        DoACDraw(true);
        if (old_type == ACTYPE::SELECT) xlights->SetACSettings(old_type);
        return true;
    }
    else if (key == 'f' || key == 'F')
    {
        xlights->SetACSettings(ACTOOL::FILL);
        DoACDraw(true);
        if (old_type == ACTYPE::SELECT) xlights->SetACSettings(old_type);
        return true;
    }
    else if (key == 'n' || key == 'N')
    {
        xlights->SetACSettings(ACTYPE::ON);
        DoACDraw(true);
        if (old_type == ACTYPE::SELECT) xlights->SetACSettings(old_type);
        return true;
    }
    else if (key == (wxChar)WXK_DELETE)
    {
        xlights->SetACSettings(ACTYPE::OFF);
        DoACDraw(true);
        if (old_type == ACTYPE::SELECT) xlights->SetACSettings(old_type);
        return true;
    }
    else if (key == 'k' || key == 'K')
    {
        xlights->SetACSettings(ACTYPE::TWINKLE);
        DoACDraw(true);
        if (old_type == ACTYPE::SELECT) xlights->SetACSettings(old_type);
        return true;
    }
    else if (key == 'l' || key == 'L')
    {
        xlights->SetACSettings(ACTYPE::SELECT);
        DoACDraw(true);
        return true;
    }
    else if (key == 's' || key == 'S')
    {
        xlights->SetACSettings(ACTYPE::SHIMMER);
        DoACDraw(true);
        if (old_type == ACTYPE::SELECT) xlights->SetACSettings(old_type);
        return true;
    }
    else if (key == 'i' || key == 'I')
    {
        xlights->SetACSettings(ACSTYLE::INTENSITY);
        DoACDraw(true);
        if (old_type == ACTYPE::SELECT) xlights->SetACSettings(old_type);
        return true;
    }
    else if (key == 'u' || key == 'U')
    {
        xlights->SetACSettings(ACSTYLE::RAMPUP);
        DoACDraw(true);
        if (old_type == ACTYPE::SELECT) xlights->SetACSettings(old_type);
        return true;
    }
    else if (key == 'd' || key == 'D')
    {
        xlights->SetACSettings(ACSTYLE::RAMPDOWN);
        DoACDraw(true);
        if (old_type == ACTYPE::SELECT) xlights->SetACSettings(old_type);
        return true;
    }
    else if (key == 'a' || key == 'A')
    {
        xlights->SetACSettings(ACSTYLE::RAMPUPDOWN);
        DoACDraw(true);
        if (old_type == ACTYPE::SELECT) xlights->SetACSettings(old_type);
        return true;
    }
    else if (key == 'g' || key == 'G')
    {
        xlights->SetACSettings(ACMODE::FOREGROUND);
        if (old_type == ACTYPE::SELECT) xlights->SetACSettings(old_type);
        return true;
    }
    else if (key == 'b' || key == 'B')
    {
        xlights->SetACSettings(ACMODE::BACKGROUND);
        if (old_type == ACTYPE::SELECT) xlights->SetACSettings(old_type);
        return true;
    }
    else if (key == (wxChar)WXK_UP)
    {
        if (mSequenceElements->GetSelectedTimingRow() == -1) {
            mCellRangeSelected = false;
        }
        else
        {
            EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
            if (tel == nullptr) return true;
            Effect* eff = tel->GetEffect(0);
            if (eff != nullptr && mSequenceElements->GetFirstVisibleModelRow() != -1 && ((mRangeCursorRow == -1 || mRangeCursorCol == -1)))
            {
                mRangeStartCol = 0;
                mRangeEndCol = 0;
                mRangeStartRow = mSequenceElements->GetFirstVisibleModelRow();
                mRangeEndRow = mRangeStartRow;
                mRangeCursorCol = mRangeStartCol;
                mRangeCursorRow = mRangeStartRow;
                mCellRangeSelected = true;
            }
        }

        if (mCellRangeSelected)
        {
            if (shift)
            {
                if (mRangeCursorRow > mRangeStartRow)
                {
                    mRangeCursorRow--;
                    mRangeEndRow = mRangeCursorRow;
                }
                else
                {
                    if (mRangeStartRow > mSequenceElements->GetNumberOfTimingRows()) {
                        mRangeCursorRow--;
                        mRangeStartRow = mRangeCursorRow;
                    }
                }
            }
            else
            {
                if (mRangeCursorRow > mSequenceElements->GetNumberOfTimingRows()) {
                    mRangeCursorRow--;
                }
                mRangeStartRow = mRangeCursorRow;
                mRangeEndRow = mRangeCursorRow;
                mRangeStartCol = mRangeCursorCol;
                mRangeEndCol = mRangeCursorCol;
            }
            Refresh(false);
        }
        mDropStartTimeMS = GetMSFromColumn(mRangeCursorCol);
        mDropRow = mRangeCursorRow;
        SetRCToolTip();
        MakeRowVisible(mRangeCursorRow - mSequenceElements->GetNumberOfTimingRows());
        return true;
    }
    else if (key == (wxChar)WXK_DOWN)
    {
        if (mSequenceElements->GetSelectedTimingRow() == -1) {
            mCellRangeSelected = false;
        }
        else
        {
            EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
            if (tel == nullptr) return true;
            Effect* eff = tel->GetEffect(0);
            if (eff != nullptr && mSequenceElements->GetFirstVisibleModelRow() != -1 && ((mRangeEndRow == -1 || mRangeStartRow == -1 || mRangeStartCol == -1 || mRangeEndRow == -1)))
            {
                mRangeStartCol = 0;
                mRangeEndCol = 0;
                mRangeStartRow = mSequenceElements->GetFirstVisibleModelRow();
                mRangeEndRow = mRangeStartRow;
                mCellRangeSelected = true;
            }

            if (mRangeCursorCol == -1)
            {
                mRangeCursorCol = mRangeStartCol;
                mRangeCursorRow = mRangeStartRow;
            }
        }

        if (mCellRangeSelected)
        {
            if (shift)
            {
                if (mRangeCursorRow < mRangeEndRow)
                {
                    mRangeCursorRow++;
                    mRangeStartRow = mRangeCursorRow;
                }
                else
                {
                    if (mRangeEndRow < mSequenceElements->GetRowInformationSize()-1)
                    {
                        mRangeCursorRow++;
                        mRangeEndRow = mRangeCursorRow;
                    }
                }
            }
            else
            {
                if (mRangeCursorRow < mSequenceElements->GetRowInformationSize() - 1) {
                    mRangeCursorRow++;
                }
                mRangeStartRow = mRangeCursorRow;
                mRangeEndRow = mRangeCursorRow;
                mRangeEndCol = mRangeCursorCol;
                mRangeStartCol = mRangeCursorCol;
            }
            Refresh(false);
        }
        mDropStartTimeMS = GetMSFromColumn(mRangeCursorCol);
        mDropRow = mRangeCursorRow;

        SetRCToolTip();
        MakeRowVisible(mRangeCursorRow- mSequenceElements->GetNumberOfTimingRows()+1);
        return true;
    }
    else if (key == (wxChar)WXK_LEFT)
    {
        if (mSequenceElements->GetSelectedTimingRow() == -1) {
            mCellRangeSelected = false;
        }
        else
        {
            EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
            if (tel == nullptr) return true;
            Effect* eff = tel->GetEffect(0);
            if (eff != nullptr && mSequenceElements->GetFirstVisibleModelRow() != -1 && ((mRangeEndRow == -1 || mRangeStartRow == -1 || mRangeStartCol == -1 || mRangeEndRow == -1)))
            {
                mRangeStartCol = 0;
                mRangeEndCol = 0;
                mRangeStartRow = mSequenceElements->GetFirstVisibleModelRow();
                mRangeEndRow = mRangeStartRow;
                mCellRangeSelected = true;
            }

            if (mRangeCursorCol == -1)
            {
                mRangeCursorCol = mRangeStartCol;
                mRangeCursorRow = mRangeStartRow;
            }
        }

        if (mCellRangeSelected)
        {
            EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
            if (tel == nullptr) return true;
            Effect* eff = tel->GetEffect(mRangeEndCol - 1);
            if (eff != nullptr)
            {
                if (shift)
                {
                    if (mRangeCursorCol > mRangeStartCol)
                    {
                        mRangeCursorCol--;
                        mRangeEndCol = mRangeCursorCol;
                    }
                    else
                    {
                        if (mRangeCursorCol > 0)
                        {
                            mRangeCursorCol--;
                            mRangeStartCol = mRangeCursorCol;
                        }
                    }
                }
                else
                {
                    if (mRangeCursorCol > 0)
                    {
                        mRangeCursorCol--;
                    }
                    mRangeStartCol = mRangeCursorCol;
                    mRangeEndCol = mRangeStartCol;
                    mRangeStartRow = mRangeCursorRow;
                    mRangeEndRow = mRangeCursorRow;
                }
                Refresh(false);
            }
        }
        mDropStartTimeMS = GetMSFromColumn(mRangeCursorCol);
        mDropRow = mRangeCursorRow;

        SetRCToolTip();
        MakeColVisible(mRangeCursorCol);
        return true;
    }
    else if (key == (wxChar)WXK_RIGHT)
    {
        if (mSequenceElements->GetSelectedTimingRow() == -1) {
            mCellRangeSelected = false;
        }
        else
        {
            EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
            if (tel == nullptr) return true;
            Effect* eff = tel->GetEffect(0);
            if (eff != nullptr && mSequenceElements->GetFirstVisibleModelRow() != -1 && ((mRangeEndRow == -1 || mRangeStartRow == -1 || mRangeStartCol == -1 || mRangeEndRow == -1)))
            {
                mRangeStartCol = 0;
                mRangeEndCol = 0;
                mRangeStartRow = mSequenceElements->GetFirstVisibleModelRow();
                mRangeEndRow = mRangeStartRow;
                mCellRangeSelected = true;
            }
            if (mRangeCursorCol == -1)
            {
                mRangeCursorCol = mRangeStartCol;
                mRangeCursorRow = mRangeStartRow;
            }
        }

        if (mCellRangeSelected)
        {
            EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
            if (tel == nullptr) return true;
            Effect* eff1 = tel->GetEffect(mRangeStartCol + 1);
            Effect* eff2 = tel->GetEffect(mRangeEndCol + 1);
            if (eff1 != nullptr && eff2 != nullptr)
            {
                if (mRangeCursorCol < mRangeEndCol)
                {
                    mRangeCursorCol++;
                    mRangeStartCol = mRangeCursorCol;
                }
                else
                {
                    mRangeCursorCol++;
                    mRangeEndCol = mRangeCursorCol;
                }

                if (!shift)
                {
                    mRangeEndCol = mRangeCursorCol;
                    mRangeStartCol = mRangeCursorCol;
                    mRangeEndRow = mRangeCursorRow;
                    mRangeStartRow = mRangeCursorRow;
                }
            }
            else
            {
                if (!shift)
                {
                    mRangeEndCol = mRangeCursorCol;
                    mRangeStartCol = mRangeCursorCol;
                    mRangeEndRow = mRangeCursorRow;
                    mRangeStartRow = mRangeCursorRow;
                }
            }
            Refresh(false);
        }
        mDropStartTimeMS = GetMSFromColumn(mRangeCursorCol);
        mDropRow = mRangeCursorRow;

        SetRCToolTip();
        MakeColVisible(mRangeCursorCol);
        return true;
    }
    return false;
}

bool EffectsGrid::DoACDraw(bool keyboard, ACTYPE typeOverride, ACSTYLE styleOverride, ACTOOL toolOverride, ACMODE modeOverride)
{
    if (mSequenceElements == nullptr) {
        return false;
    }

    // dont do AC if there is no timing selected
    if (mSequenceElements->GetSelectedTimingRow() == -1) {
        return false;
    }

    if (mRangeStartRow < 0 || mRangeEndRow < 0 || mRangeStartCol < 0 || mRangeEndCol < 0)
    {
        return false;
    }

    ACTYPE type;
    ACSTYLE style;
    ACTOOL tool;
    ACMODE mode;
    xlights->GetACSettings(type, style, tool, mode);

    if (typeOverride != ACTYPE::NILTYPEOVERRIDE)
    {
        type = typeOverride;
    }
    if (styleOverride != ACSTYLE::NILSTYLEOVERRIDE)
    {
        style = styleOverride;
    }
    if (toolOverride != ACTOOL::NILTOOLOVERRIDE)
    {
        tool = toolOverride;
    }
    if (modeOverride != ACMODE::NILMODEOVERRIDE)
    {
        mode = modeOverride;
    }

    if (type == ACTYPE::SELECT && tool != ACTOOL::CASCADE && tool != ACTOOL::FILL)
    {
        return false;
    }

    int intensity = xlights->GetACIntensity();
    int a, b;
    xlights->GetACRampValues(a, b);

    mSequenceElements->get_undo_mgr().CreateUndoStep();

    int startMS = 0;
    int endMS = 0;
    EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
    if (tel == nullptr) return false;
    Effect* eff1 = tel->GetEffect(mRangeStartCol);
    Effect* eff2 = tel->GetEffect(mRangeEndCol);
    if (eff1 != nullptr)
    {
        if (mRangeStartCol <= mRangeEndCol)
        {
            startMS = eff1->GetStartTimeMS();
        }
        else
        {
            startMS = eff1->GetEndTimeMS();
        }
    }
    if (eff2 != nullptr)
    {
        if (mRangeStartCol <= mRangeEndCol)
        {
            endMS = eff2->GetEndTimeMS();
        }
        else
        {
            endMS = eff2->GetStartTimeMS();
        }
    }

    if (tool == ACTOOL::FILL)
    {
        ACFill(type, std::min(startMS, endMS), std::max(startMS, endMS), mRangeStartRow, mRangeEndRow);
    }
    else if (tool == ACTOOL::CASCADE)
    {
        int startr = mRangeStartRow;
        int endr = mRangeEndRow;

        int startc = mRangeStartCol;
        int endc = mRangeEndCol;

        if (keyboard)
        {
            if (startr == mRangeCursorRow)
            {
                startr = mRangeEndRow;
                endr = mRangeStartRow;
            }
            if (startc == mRangeCursorCol)
            {
                startc = mRangeEndCol;
                endc = mRangeStartCol;
            }
        }

        ACCascade(std::min(startMS, endMS), std::max(startMS, endMS), startc, endc, startr, endr);
    }
    else
    {
        ACDraw(type, style, mode, intensity, a, b, std::min(startMS, endMS), std::max(startMS, endMS), std::min(mRangeStartRow, mRangeEndRow), std::max(mRangeStartRow, mRangeEndRow));
    }

    Refresh();
    sendRenderDirtyEvent();

    return true;
}

void EffectsGrid::mouseReleased(wxMouseEvent& event)
{
    if (mSequenceElements == nullptr) return;

    if (mDragging && xlights->IsACActive())
    {
        ReleaseMouse();
        mDragging = false;

        if (DoACDraw())
        {
            mRangeCursorCol = mRangeStartCol;
            mRangeCursorRow = mRangeStartRow;
            mRangeStartRow = -1;
            mRangeStartCol = -1;
            mRangeEndRow = -1;
            mRangeEndCol = -1;
            mCellRangeSelected = false;
            SetRCToolTip();
        }
    }
    else
    {
        bool checkForEmptyCell = false;
        if (HasCapture()) {
            ReleaseMouse();
        }
        if (mResizing)
        {
            if (mEffectLayer->GetParentElement()->GetType() != ElementType::ELEMENT_TYPE_TIMING)
            {
                if (MultipleEffectsSelected()) {
                    std::string lastModel;
                    int startMS = 99999999;
                    int endMS = -1;
                    for (int row = 0; row < mSequenceElements->GetRowInformationSize(); row++)
                    {
                        EffectLayer* el = mSequenceElements->GetEffectLayer(row);
                        if (el->GetParentElement()->GetModelName() != lastModel) {
                            if (endMS != -1) {
                                sendRenderEvent(lastModel, startMS, endMS);
                            }
                            startMS = 99999999;
                            endMS = -1;
                            lastModel = el->GetParentElement()->GetModelName();
                        }
                        if (el->GetSelectedEffectCount() > 0) {
                            int startDirty, endDirty;
                            el->GetParentElement()->GetDirtyRange(startDirty, endDirty);
                            if (startDirty != -1) {
                                adjustMS(startDirty, startMS, endMS);
                                adjustMS(endDirty, startMS, endMS);
                            }
                        }
                    }
                    if (endMS != -1) {
                        sendRenderEvent(lastModel, startMS, endMS);
                    }
                }
                else {
                    int stime = mStartResizeTimeMS;
                    int timeMS = mTimeline->GetAbsoluteTimeMSfromPosition(event.GetX());
                    int min = stime;
                    int max = stime;
                    adjustMS(timeMS, min, max);
                    Effect* effect = mEffectLayer->GetEffect(mResizeEffectIndex);
                    if (effect)
                    {
                        adjustMS(mEffectLayer->GetEffect(mResizeEffectIndex)->GetStartTimeMS(), min, max);
                        adjustMS(mEffectLayer->GetEffect(mResizeEffectIndex)->GetEndTimeMS(), min, max);
                        if (mSelectedEffect != nullptr &&
                            mSelectedEffect->GetSelected() == EFFECT_LT_SELECTED &&
                            mResizeEffectIndex > 0) {
                            //also have to re-render the effect to the left
                            adjustMS(mEffectLayer->GetEffect(mResizeEffectIndex - 1)->GetStartTimeMS(), min, max);
                            adjustMS(mEffectLayer->GetEffect(mResizeEffectIndex - 1)->GetEndTimeMS(), min, max);
                        }
                        else if (mSelectedEffect != nullptr &&
                            mSelectedEffect->GetSelected() == EFFECT_RT_SELECTED &&
                            mResizeEffectIndex < (mEffectLayer->GetEffectCount() - 1)) {
                            adjustMS(mEffectLayer->GetEffect(mResizeEffectIndex + 1)->GetStartTimeMS(), min, max);
                            adjustMS(mEffectLayer->GetEffect(mResizeEffectIndex + 1)->GetEndTimeMS(), min, max);
                        }
                        int startDirty, endDirty;
                        mEffectLayer->GetParentElement()->GetDirtyRange(startDirty, endDirty);
                        if (startDirty != -1) {
                            adjustMS(startDirty, min, max);
                            adjustMS(endDirty, min, max);
                        }
                        sendRenderEvent(mEffectLayer->GetParentElement()->GetModelName(), min, max);
                        RaisePlayModelEffect(mEffectLayer->GetParentElement(), effect, false);
                    }
                }
            }

            // if dragging an effect endpoint move the selection point with it so it will
            // focus on that spot if you zoom afterwards.
            if (mSelectedEffect != nullptr && mSelectedEffect->GetSelected() == EFFECT_LT_SELECTED)
            {
                int selected_time = (int)(mSelectedEffect->GetStartTimeMS());
                UpdateZoomPosition(selected_time);
            }
            else if (mSelectedEffect != nullptr && mSelectedEffect->GetSelected() == EFFECT_RT_SELECTED)
            {
                int selected_time = (int)(mSelectedEffect->GetEndTimeMS());
                UpdateZoomPosition(selected_time);
            }
        }
        else if (mDragging) {
            UnsetToolTip();
            mDragging = false;
            if ((mDragStartX == event.GetX() && mDragStartY == event.GetY()) || (mSequenceElements->GetNumberOfActiveTimingEffects() > 0)) {
                checkForEmptyCell = true;
            }
        }
        else if (!event.ControlDown()) {
            checkForEmptyCell = true;
        }

        if (checkForEmptyCell) {
            int row = GetRow(event.GetY());
            if (mSequenceElements->GetSelectedTimingRow() >= 0 && row < mSequenceElements->GetVisibleRowInformationSize())
            {
                CheckForPartialCell(event.GetX());
                if (mPartialCellSelected)
                {
                    mSelectedRow = -1;
                    mSequenceElements->UnSelectAllEffects();
                    UnselectEffect(true);
                }
                else if (!mCellRangeSelected)
                {
                    EffectLayer* el = mSequenceElements->GetVisibleEffectLayer(row);
                    if (el != nullptr) {
                        mDropStartTimeMS = mTimeline->GetAbsoluteTimeMSfromPosition(event.GetX());
                        mDropEndTimeMS = mDropStartTimeMS + 1000;
                        mDropStartX = mTimeline->GetPositionFromTimeMS(mDropStartTimeMS);
                        mDropEndX = mTimeline->GetPositionFromTimeMS(mDropEndTimeMS);
                        AdjustDropLocations(event.GetX(), el);
                        mPartialCellSelected = true;
                        mDropRow = row;
                        mSequenceElements->UnSelectAllEffects();
                        UnselectEffect(true);
                    }
                }
            }
            else if (row >= 0 && row < mSequenceElements->GetVisibleRowInformationSize()) {
                EffectLayer* el = mSequenceElements->GetVisibleEffectLayer(row);
                int startTime = mTimeline->GetAbsoluteTimeMSfromPosition(event.GetX());
                int effectIndex = 0;
                if (el != nullptr) {
                    if (el->HitTestEffectByTime(startTime, effectIndex)) {
                        el->GetEffect(effectIndex)->SetSelected(EFFECT_SELECTED);
                    }
                    else {
                        mDropStartTimeMS = mTimeline->GetAbsoluteTimeMSfromPosition(event.GetX());
                        mDropEndTimeMS = mDropStartTimeMS + 1000;
                        mDropStartX = mTimeline->GetPositionFromTimeMS(mDropStartTimeMS);
                        mDropEndX = mTimeline->GetPositionFromTimeMS(mDropEndTimeMS);
                        AdjustDropLocations(event.GetX(), el);
                        mPartialCellSelected = true;
                        mDropRow = row;
                        mSequenceElements->UnSelectAllEffects();
                        UnselectEffect(true);
                    }
                }
            }
        }

        mResizing = false;
        mDragDropping = false;
        Refresh(false);
        mSequenceElements->get_undo_mgr().SetCaptureUndo(false);
        mSequenceElements->get_undo_mgr().RemoveUnusedMarkers();
    }
}

void EffectsGrid::CheckForPartialCell(int x_pos)
{
    //static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    mPartialCellSelected = false;
    // make sure a valid row and column is selected
    if( mRangeStartCol >= 0 && mRangeStartRow >= 0 )
    {
        // check for only single cell selection
        if( mRangeStartRow == mRangeEndRow && mRangeStartCol == mRangeEndCol )
        {
            EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
            EffectLayer* el = mSequenceElements->GetVisibleEffectLayer(mRangeStartRow - mSequenceElements->GetFirstVisibleModelRow());

            if (el == nullptr)
            {
                // I have seen a log where this happened so now skipping code below - KW
                //logger_base.crit("EffectsGrid::CheckForPartialCell el is nullptr ... this is going to crash.");
            }
            else
            {

                int startTime = mTimeline->GetAbsoluteTimeMSfromPosition(x_pos);
                int effectIndex = 0;
                if (!el->HitTestEffectByTime(startTime, effectIndex)) {
                    Effect* eff = tel->GetEffect(mRangeStartCol);

                    if (eff != nullptr)
                    {
                        mDropStartX = mTimeline->GetPositionFromTimeMS(eff->GetStartTimeMS());
                        mDropEndX = mTimeline->GetPositionFromTimeMS(eff->GetEndTimeMS());
                        mDropStartTimeMS = eff->GetStartTimeMS();
                        mDropEndTimeMS = eff->GetEndTimeMS();
                        mDropRow = mRangeStartRow - mSequenceElements->GetFirstVisibleModelRow();
                        if (AdjustDropLocations(x_pos, el)) {
                            mPartialCellSelected = true;
                        }
                    }
                }
            }
        }
    }
}

void EffectsGrid::Resize(int position, bool offset, bool control)
{

    if (mSequenceElements == nullptr) return;

    int new_time = -1;

    // Snap to timing marks logic
    if ((xlights->GetSnapToTimingMarks() && !control) || (!xlights->GetSnapToTimingMarks() && control))
    {
        if (mSequenceElements->GetSelectedTimingRow() >= 0)
        {
            int time = mTimeline->GetRawTimeMSfromPosition(position);
            int time_plus_one = mTimeline->GetRawTimeMSfromPosition(position+1);
            int times = 2;
            while ( time_plus_one == time && times < 11 ) {
                time_plus_one = mTimeline->GetAbsoluteTimeMSfromPosition(position+times);
                times++;
            }
            int time_delta = (time_plus_one - time) * (10 / (times-1));  // snap within 10 pixels
            if( time_delta == 0 ) time_delta = 25;
            EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());

            if (tel != nullptr)
            {
                if( tel->GetEffectCount() > 0 )
                {
                    int time1 = -1000;
                    int time2 = 100000000;
                    // see if inside an effect
                    Effect* eff1 = tel->GetEffectAtTime(time);
                    if (eff1 != nullptr)
                    {
                        time1 = eff1->GetStartTimeMS();
                        time2 = eff1->GetEndTimeMS();

                        if( (time - time1) <= time_delta )
                        {
                            new_time = time1;
                        }
                        else
                        {
                            if( (time2-time) <= time_delta )
                            {
                                new_time = time2;
                            }
                        }
                    }
                    else
                    {
                        eff1 = tel->GetEffectBeforeTime(time);
                        Effect* eff2 = tel->GetEffectAfterTime(time);

                        if (eff1 != nullptr )
                        {
                            time1 = eff1->GetEndTimeMS();
                        }
                        if (eff2 != nullptr )
                        {
                            time2 = eff2->GetStartTimeMS();
                        }

                        if( (time - time1) <= time_delta )
                        {
                            new_time = time1;
                        }
                        else
                        {
                            if( (time2-time) <= time_delta )
                            {
                                new_time = time2;
                            }
                        }
                    }
                }
            }
        }
    }

    if( new_time != -1 ) {
        if(MultipleEffectsSelected() || mResizingMode == EFFECT_RESIZE_MOVE) {
            ResizeMoveMultipleEffectsMS(new_time, offset);
        }
        else {
            ResizeSingleEffectMS(new_time);
        }
    } else {
        if(MultipleEffectsSelected() || mResizingMode == EFFECT_RESIZE_MOVE) {
            ResizeMoveMultipleEffects(position, offset);
        }
        else {
            ResizeSingleEffect(position);
        }
    }

    if( mSequenceElements->get_undo_mgr().ChangeCaptured() )
    {
        mSequenceElements->get_undo_mgr().SetCaptureUndo(false);
    }

    // KW - I am commenting this out because it is causing a large number of render events
    // when you drag or resize an effect. I have tested some scenarios and it seems to be ok
    // If we encounter scenarios where it isnt it would be better to send the render dirty
    // event from the calling functions. I have temporarily added logging for all click and
    // drag events to try to help us identify why we miss rendering when we do
    // static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    //logger_base.debug("EffectsGrid::Resize would have sent render dirty event");
    //sendRenderDirtyEvent();
}

void EffectsGrid::ScrollBy(int by)
{
    mSelectedRow -= by;
}

void EffectsGrid::MoveSelectedEffectUp(bool shift)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (mSequenceElements == nullptr) return;

    if (mSequenceElements->GetSelectedTimingRow() == -1) {
        mCellRangeSelected = false;
    }
    if (mCellRangeSelected)
    {
        if (shift)
        {
            if (mRangeEndRow > mRangeStartRow)
            {
                mRangeEndRow--;
            }
        }
        else
        {
            if (mRangeStartRow > mSequenceElements->GetNumberOfTimingRows()) {
                mRangeStartRow--;
                mRangeEndRow--;
            }
        }
        SetRCToolTip();
        UpdateSelectedEffects();
        MakeRowVisible(mRangeEndRow - mSequenceElements->GetNumberOfTimingRows());
        Refresh(false);
    }
    else if (!MultipleEffectsSelected() && mSelectedEffect != nullptr && !mSelectedEffect->IsLocked() && mSelectedRow > 0)
    {
        logger_base.debug("EffectsGrid::MoveSelectedEffectUp moving single effect.");
        int row = mSelectedRow - 1;
        EffectLayer* el = mSelectedEffect->GetParentEffectLayer();
        while (row + mSequenceElements->GetFirstVisibleModelRow() >= mSequenceElements->GetNumberOfTimingRows())
        {
            EffectLayer* new_el = mSequenceElements->GetEffectLayer(mSequenceElements->GetFirstVisibleModelRow() + row);
            if (new_el != nullptr)
            {
                if (new_el->GetRangeIsClearMS(mSelectedEffect->GetStartTimeMS(), mSelectedEffect->GetEndTimeMS()))
                {
                    mSequenceElements->get_undo_mgr().CreateUndoStep();
                    Effect* ef = new_el->AddEffect(0,
                        mSelectedEffect->GetEffectName(),
                        mSelectedEffect->GetSettingsAsString(),
                        mSelectedEffect->GetPaletteAsString(),
                        mSelectedEffect->GetStartTimeMS(),
                        mSelectedEffect->GetEndTimeMS(),
                        EFFECT_SELECTED,
                        false);
                    if (ef != nullptr)
                    {
                        mSelectedRow = row;
                        mSelectedEffect = ef;
                        el->DeleteSelectedEffects(mSequenceElements->get_undo_mgr());
                        mSequenceElements->get_undo_mgr().CaptureAddedEffect(new_el->GetParentElement()->GetModelName(), new_el->GetIndex(), ef->GetID());
                        RaiseSelectedEffectChanged(ef, false, true);
                        sendRenderDirtyEvent();
                        MakeRowVisible(mSelectedRow + mSequenceElements->GetFirstVisibleModelRow() - mSequenceElements->GetNumberOfTimingRows());
                    }
                    else
                    {
                        logger_base.warn("Problem adding effect when moving effect up %s", (const char *)mSelectedEffect->GetEffectName().c_str());
                    }
                    Refresh(false);
                    return;
                }
            }
            row--;
        }
    }
    else if (MultipleEffectsSelected())
    {
        logger_base.debug("EffectsGrid::MoveSelectedEffectUp moving multiple effects.");

        // check if its clear for all effects
        bool all_clear = true;
        int first_model_row = mSequenceElements->GetNumberOfTimingRows();
        int num_effects = mSequenceElements->GetEffectLayer(first_model_row)->GetSelectedEffectCount();
        if (num_effects > 0)
        {
            all_clear = false;
        }
        for (int row = first_model_row + 1; row < mSequenceElements->GetRowInformationSize() && all_clear; row++)
        {
            EffectLayer* el1 = mSequenceElements->GetEffectLayer(row - 1);
            EffectLayer* el2 = mSequenceElements->GetEffectLayer(row);
            if (mSequenceElements->GetEffectLayer(row)->GetSelectedEffectCount() > 0)
            {
                num_effects = mSequenceElements->GetEffectLayer(row)->GetEffectCount();
                for (int i = 0; (i < num_effects) && all_clear; ++i)
                {
                    Effect* eff = el2->GetEffect(i);
                    if (eff->GetSelected() && !eff->IsLocked())
                    {
                        if (!el1->GetRangeIsClearMS(eff->GetStartTimeMS(), eff->GetEndTimeMS(), true))
                        {
                            all_clear = false;
                        }
                    }
                }
            }
        }
        if (all_clear) // all clear so now move them all up
        {
            // Tag all selected effects so we don't move them twice
            ((MainSequencer*)mParent)->TagAllSelectedEffects();

            mSequenceElements->get_undo_mgr().CreateUndoStep();
            for (int row = first_model_row + 1; row < mSequenceElements->GetRowInformationSize(); row++)
            {
                EffectLayer* el1 = mSequenceElements->GetEffectLayer(row - 1);
                EffectLayer* el2 = mSequenceElements->GetEffectLayer(row);
                if (mSequenceElements->GetEffectLayer(row)->GetTaggedEffectCount() > 0)
                {
                    num_effects = mSequenceElements->GetEffectLayer(row)->GetEffectCount();
                    for (int i = 0; (i < num_effects) && all_clear; ++i)
                    {
                        Effect* eff = el2->GetEffect(i);
                        if (eff->GetSelected() && eff->GetTagged() && !eff->IsLocked())
                        {
                            eff->SetTagged(false);
                            Effect* ef = el1->AddEffect(0,
                                eff->GetEffectName(),
                                eff->GetSettingsAsString(),
                                eff->GetPaletteAsString(),
                                eff->GetStartTimeMS(),
                                eff->GetEndTimeMS(),
                                EFFECT_SELECTED,
                                false);
                            if (ef != nullptr)
                            {
                                mSequenceElements->get_undo_mgr().CaptureAddedEffect(el1->GetParentElement()->GetModelName(), el1->GetIndex(), ef->GetID());
                                mSelectedEffect = ef;
                            }
                            else
                            {
                                logger_base.warn("Error adding effect when moving multiple effects up %s", (const char*)eff->GetEffectName().c_str());
                            }
                        }
                    }
                    mSelectedRow = row;
                    el2->DeleteSelectedEffects(mSequenceElements->get_undo_mgr());
                }
            }
            mCellRangeSelected = false;
            mRangeStartCol = mRangeEndCol = mRangeStartRow = mRangeEndRow = -1;
            RaiseSelectedEffectChanged(mSelectedEffect, false, false);
            MakeRowVisible(mSelectedRow - mSequenceElements->GetNumberOfTimingRows() - 1);
            sendRenderDirtyEvent();
            Refresh(false);
        }
    }
}

void EffectsGrid::MoveSelectedEffectDown(bool shift)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (mSequenceElements == nullptr) return;

    if( mSequenceElements->GetSelectedTimingRow() == -1 ) {
        mCellRangeSelected = false;
    }
    if( mCellRangeSelected )
    {
        if( shift )
        {
            if (mRangeEndRow < mSequenceElements->GetRowInformationSize() - 1)
            {
                mRangeEndRow++;
            }
        }
        else
        {
            if (mRangeStartRow < mSequenceElements->GetRowInformationSize() - 1) {
                mRangeStartRow++;
                mRangeEndRow++;
            }
        }
        SetRCToolTip();
        UpdateSelectedEffects();
        MakeRowVisible(mRangeEndRow- mSequenceElements->GetNumberOfTimingRows()+1);
        Refresh(false);
    }
    else if( !MultipleEffectsSelected() && mSelectedEffect != nullptr && !mSelectedEffect->IsLocked() && mSelectedRow >= 0)
    {
        logger_base.debug("EffectsGrid::MoveSelectedEffectDown moving single effect.");

        int row = mSelectedRow+1;
        EffectLayer* el = mSelectedEffect->GetParentEffectLayer();
        while( row < mSequenceElements->GetRowInformationSize())
        {
            EffectLayer* new_el = mSequenceElements->GetEffectLayer(mSequenceElements->GetFirstVisibleModelRow() + row);
            if( new_el != nullptr )
            {
                if( new_el->GetRangeIsClearMS( mSelectedEffect->GetStartTimeMS(), mSelectedEffect->GetEndTimeMS()))
                {
                    mSequenceElements->get_undo_mgr().CreateUndoStep();
                    Effect* ef = new_el->AddEffect(0,
                                                   mSelectedEffect->GetEffectName(),
                                                   mSelectedEffect->GetSettingsAsString(),
                                                   mSelectedEffect->GetPaletteAsString(),
                                                   mSelectedEffect->GetStartTimeMS(),
                                                   mSelectedEffect->GetEndTimeMS(),
                                                   EFFECT_SELECTED,
                                                   false);
                    if (ef != nullptr)
                    {
                        mSelectedRow = row;
                        mSelectedEffect = ef;
                        el->DeleteSelectedEffects(mSequenceElements->get_undo_mgr());
                        mSequenceElements->get_undo_mgr().CaptureAddedEffect(new_el->GetParentElement()->GetModelName(), new_el->GetIndex(), ef->GetID());
                        RaiseSelectedEffectChanged(ef, false, true);
                        sendRenderDirtyEvent();
                        MakeRowVisible(mSelectedRow + mSequenceElements->GetFirstVisibleModelRow() - mSequenceElements->GetNumberOfTimingRows() + 1);
                    }
                    else
                    {
                        logger_base.warn("Error adding effect when moving effects down %s", (const char*)mSelectedEffect->GetEffectName().c_str());
                    }
                    Refresh(false);
                    return;
                }
            }
            row++;
        }
    }
    else if( MultipleEffectsSelected() )
    {
        logger_base.debug("EffectsGrid::MoveSelectedEffectDown moving mulitple effects.");

        // check if its clear for all effects
        bool all_clear = true;
        int first_model_row = mSequenceElements->GetNumberOfTimingRows();
        int num_effects = mSequenceElements->GetEffectLayer(mSequenceElements->GetRowInformationSize()-1)->GetSelectedEffectCount();
        if (num_effects > 0)
        {
            all_clear = false;
        }
        for (int row=mSequenceElements->GetRowInformationSize()-1;row>first_model_row && all_clear;row--)
        {
            EffectLayer* el1 = mSequenceElements->GetEffectLayer(row-1);
            EffectLayer* el2 = mSequenceElements->GetEffectLayer(row);
            if (mSequenceElements->GetEffectLayer(row-1)->GetSelectedEffectCount() > 0)
            {
                int num_effects = mSequenceElements->GetEffectLayer(row-1)->GetEffectCount();
                for( int i = 0; (i < num_effects) && all_clear; ++i )
                {
                    Effect* eff = el1->GetEffect(i);
                    if (eff->GetSelected() && !eff->IsLocked())
                    {
                        if( !el2->GetRangeIsClearMS( eff->GetStartTimeMS(), eff->GetEndTimeMS(), true) )
                        {
                            all_clear = false;
                        }
                    }
                }
            }
        }
        if( all_clear ) // all clear so now move them all down
        {
            // Tag all selected effects so we don't move them twice
            ((MainSequencer*)mParent)->TagAllSelectedEffects();

            mSequenceElements->get_undo_mgr().CreateUndoStep();
            for(int row=mSequenceElements->GetRowInformationSize()-1;row>first_model_row;row--)
            {
                EffectLayer* el1 = mSequenceElements->GetEffectLayer(row-1);
                EffectLayer* el2 = mSequenceElements->GetEffectLayer(row);
                if( mSequenceElements->GetEffectLayer(row-1)->GetTaggedEffectCount() > 0 )
                {
                    num_effects = mSequenceElements->GetEffectLayer(row-1)->GetEffectCount();
                    for( int i = 0; (i < num_effects) && all_clear; ++i )
                    {
                        Effect* eff = el1->GetEffect(i);
                        if (eff->GetSelected() && eff->GetTagged() && !eff->IsLocked())
                        {
                            eff->SetTagged(false);
                            Effect* ef = el2->AddEffect(0,
                                                    eff->GetEffectName(),
                                                    eff->GetSettingsAsString(),
                                                    eff->GetPaletteAsString(),
                                                    eff->GetStartTimeMS(),
                                                    eff->GetEndTimeMS(),
                                                    EFFECT_SELECTED,
                                                    false);
                            if (ef != nullptr)
                            {
                                mSequenceElements->get_undo_mgr().CaptureAddedEffect( el2->GetParentElement()->GetModelName(), el2->GetIndex(), ef->GetID() );
                                mSelectedEffect = ef;
                            }
                            else
                            {
                                logger_base.warn("Error adding effect when moving multiple effects down %s", (const char*)eff->GetEffectName().c_str());
                            }
                        }
                    }
                    mSelectedRow = row;
                    el1->DeleteSelectedEffects(mSequenceElements->get_undo_mgr());
                }
            }
            mCellRangeSelected = false;
            mRangeStartCol = mRangeEndCol = mRangeStartRow = mRangeEndRow = -1;
            RaiseSelectedEffectChanged(mSelectedEffect, false, false);
            sendRenderDirtyEvent();
            MakeRowVisible(mSelectedRow-mSequenceElements->GetNumberOfTimingRows() + 1);
            Refresh(false);
        }
    }
}

void EffectsGrid::MoveSelectedEffectRight(bool shift, bool control, bool alt)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (mSequenceElements == nullptr) return; // a sequence must be open

    logger_base.debug("EffectsGrid::MoveSelectedEffectRight.");

    if( mSequenceElements->GetSelectedTimingRow() == -1 ) {
        mCellRangeSelected = false;
    }

    bool force = false;
    if (mCellRangeSelected)
    {
        EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
        Effect* eff1 = tel->GetEffect(mRangeStartCol+1);
        Effect* eff2 = tel->GetEffect(mRangeEndCol+1);
        if (eff1 != nullptr && eff2 != nullptr)
        {
            mRangeEndCol++;
            if( !shift )
            {
                mRangeStartCol++;
            }
            SetRCToolTip();
            UpdateSelectedEffects();
            MakeColVisible(mRangeEndCol);
            Refresh(false);
        }
    }
    else
    {
        int by = mSequenceElements->GetMinPeriod();

        bool buttup = false;

        if (control && mSelectedEffect != nullptr && mSequenceElements->GetSelectedTimingRow() >= 0)
        {
            EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
            if (tel != nullptr)
            {
                Effect * teff = tel->GetEffectAtTime(mSelectedEffect->GetEndTimeMS());
                if( teff != nullptr && (mSelectedEffect->GetEndTimeMS() == teff->GetEndTimeMS()) )
                {
                    teff = tel->GetEffectAtTime(mSelectedEffect->GetEndTimeMS()+1);
                }
                if (teff == nullptr)
                {
                    teff = tel->GetEffectAfterTime(mSelectedEffect->GetEndTimeMS());
                    if (teff == nullptr)
                    {
                        by = mTimeline->GetTimeLength() - mSelectedEffect->GetEndTimeMS();
                    }
                    else
                    {
                        by = teff->GetStartTimeMS() - mSelectedEffect->GetEndTimeMS();
                    }
                }
                else
                {
                    by = teff->GetEndTimeMS() - mSelectedEffect->GetEndTimeMS();
                }
                if( !MultipleEffectsSelected() && !shift )
                {
                    int col = -1;
                    for (int index = 0; index < tel->GetEffectCount(); index++)
                    {
                        Effect* tim_ef = tel->GetEffect(index);
                        if (mSelectedEffect->GetEndTimeMS() >= tim_ef->GetStartTimeMS() && mSelectedEffect->GetEndTimeMS() < tim_ef->GetEndTimeMS())
                        {
                            col = index;
                            break;
                        }
                    }
                    if (col >= 0)
                    {
                        int effect_length_ms = mSelectedEffect->GetEndTimeMS() - mSelectedEffect->GetStartTimeMS();
                        EffectLayer* el = mSelectedEffect->GetParentEffectLayer();
                        Effect* timing_effect = tel->GetEffect(col);
                        while (timing_effect != nullptr && col < tel->GetEffectCount())
                        {
                            int new_end_time_ms = timing_effect->GetEndTimeMS();
                            int start_time_for_check = new_end_time_ms - effect_length_ms;
                            if (start_time_for_check < mSelectedEffect->GetEndTimeMS())
                            {
                                start_time_for_check = mSelectedEffect->GetEndTimeMS();
                            }
                            if (el->GetRangeIsClearMS(start_time_for_check, new_end_time_ms))
                            {
                                by = timing_effect->GetEndTimeMS() - mSelectedEffect->GetEndTimeMS();
                                force = true;
                                break;
                            }
                            col++;
                            if (col < tel->GetEffectCount())
                            {
                                timing_effect = tel->GetEffect(col);
                            }
                        }
                    }
                }
            }
        }
        else
        {
            buttup = true;
        }

        mSequenceElements->get_undo_mgr().CreateUndoStep();
        mSequenceElements->get_undo_mgr().SetCaptureUndo(true);
        if (control && shift)
        {
            if (buttup)
            {
                ButtUpStretchMultipleEffects(true);
            }
            else
            {
                StretchMultipleEffectsByTime(by);
            }
        }
        else if (control)
        {
            if (buttup)
            {
                ButtUpResizeMoveMultipleEffects(true);
            }
            else
            {
                ResizeMoveMultipleEffectsByTime(by, force);
            }
        }
        else
        {
            ResizeMoveMultipleEffectsByTime(by, force);
        }
        mSequenceElements->get_undo_mgr().SetCaptureUndo(false);

        sendRenderDirtyEvent();
        Refresh(false);
    }
}

void EffectsGrid::MoveSelectedEffectLeft(bool shift, bool control, bool alt)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (mSequenceElements == nullptr) return;

    logger_base.debug("EffectsGrid::MoveSelectedEffectLeft.");

    if( mSequenceElements->GetSelectedTimingRow() == -1 ) {
        mCellRangeSelected = false;
    }

    bool force = false;
    if( mCellRangeSelected )
    {
        EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
        Effect* eff = tel->GetEffect(mRangeEndCol - 1);
        if( eff != nullptr )
        {
            if( shift )
            {
                if( mRangeEndCol > mRangeStartCol )
                {
                    mRangeEndCol--;
                }
            }
            else if( mRangeStartCol > 0 )
            {
                mRangeStartCol--;
                mRangeEndCol--;
            }
            SetRCToolTip();
            UpdateSelectedEffects();
            MakeColVisible(mRangeStartCol);
            Refresh(false);
        }
    }
    else
    {
        int by = -1 * mSequenceElements->GetMinPeriod();
        bool buttup = false;

        if (control && mSelectedEffect != nullptr && mSequenceElements->GetSelectedTimingRow() >= 0)
        {
            EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
            if (tel != nullptr)
            {
                Effect * teff = tel->GetEffectAtTime(mSelectedEffect->GetStartTimeMS());
                if( teff != nullptr && (mSelectedEffect->GetStartTimeMS() == teff->GetStartTimeMS()) )
                {
                    teff = tel->GetEffectAtTime(mSelectedEffect->GetStartTimeMS()-1);
                }
                if (teff == nullptr)
                {
                    teff = tel->GetEffectBeforeTime(mSelectedEffect->GetStartTimeMS());
                    if (teff == nullptr)
                    {
                        by = -1 * mSelectedEffect->GetStartTimeMS();
                    }
                    else
                    {
                        by = -1 * (mSelectedEffect->GetStartTimeMS() - teff->GetEndTimeMS());
                    }
                }
                else
                {
                    by = -1 * (mSelectedEffect->GetStartTimeMS() - teff->GetStartTimeMS());
                }
                if( !MultipleEffectsSelected() && !shift )
                {
                    int col = -1;
                    for (int index = 0; index < tel->GetEffectCount(); index++)
                    {
                        Effect* tim_ef = tel->GetEffect(index);
						if (mSelectedEffect->GetStartTimeMS() == tim_ef->GetStartTimeMS() && mSelectedEffect->GetStartTimeMS() < tim_ef->GetEndTimeMS())
						{
							col = index - 1;
							break;
						}
						else if (mSelectedEffect->GetStartTimeMS() > tim_ef->GetStartTimeMS() && mSelectedEffect->GetStartTimeMS() < tim_ef->GetEndTimeMS())
						{
							col = index;
							break;
						}
					}
                    if (col >= 0)
                    {
                        int effect_length_ms = mSelectedEffect->GetEndTimeMS() - mSelectedEffect->GetStartTimeMS();
                        EffectLayer* el = mSelectedEffect->GetParentEffectLayer();
                        Effect* timing_effect = tel->GetEffect(col);
                        while (timing_effect != nullptr && col >= 0)
                        {
                            int new_end_time_ms = timing_effect->GetStartTimeMS() + effect_length_ms;
                            int end_time_for_check = new_end_time_ms;
                            if (end_time_for_check > mSelectedEffect->GetStartTimeMS())
                            {
                                end_time_for_check = mSelectedEffect->GetStartTimeMS();
                            }
                            if (el->GetRangeIsClearMS(timing_effect->GetStartTimeMS(), end_time_for_check))
                            {
                                by = timing_effect->GetStartTimeMS() - mSelectedEffect->GetStartTimeMS();
                                force = true;
                                break;
                            }
                            col--;
                            if (col >= 0)
                            {
                                timing_effect = tel->GetEffect(col);
                            }
                        }
                    }
                }
            }
        }

        else
        {
            buttup = true;
        }

        mSequenceElements->get_undo_mgr().CreateUndoStep();
        mSequenceElements->get_undo_mgr().SetCaptureUndo(true);
        if (control && shift)
        {
            if (buttup)
            {
                ButtUpStretchMultipleEffects(false);
            }
            else
            {
                StretchMultipleEffectsByTime(by);
            }
        }
        else if (control)
        {
            if (buttup)
            {
                ButtUpResizeMoveMultipleEffects(false);
            }
            else
            {
                ResizeMoveMultipleEffectsByTime(by, force);
            }
        }
        else
        {
            ResizeMoveMultipleEffectsByTime(by, force);
        }
        mSequenceElements->get_undo_mgr().SetCaptureUndo(false);

        Refresh(false);
        sendRenderDirtyEvent();
    }
}

std::list<Effect*> EffectsGrid::GetSelectedEffects() const
{
    std::list<Effect*> res;

    for (int i = 0; i < mSequenceElements->GetRowInformationSize(); i++)
    {
        EffectLayer* el = mSequenceElements->GetEffectLayer(i);

        for (int x = 0; x < el->GetEffectCount(); x++) {
            Effect *ef = el->GetEffect(x);
            if (ef->GetSelected() != EFFECT_NOT_SELECTED) {
                res.push_back(ef);
            }
        }
    }

    return res;
}

void EffectsGrid::LockEffects(bool lock)
{
    if (mSequenceElements == nullptr) return;

    if (mSelectedEffect != nullptr || AtLeastOneEffectSelected())
    {
        auto efs = GetSelectedEffects();
        // add in the selected effect if we didnt get it
        if (mSelectedEffect != nullptr)
        {
            bool found = false;
            for (auto it = efs.begin(); it != efs.end(); ++it)
            {
                if ((*it) == mSelectedEffect)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                efs.push_back(mSelectedEffect);
            }
        }

        if (efs.size() > 0)
        {
            for (auto it = efs.begin(); it != efs.end(); ++it)
            {
                (*it)->SetLocked(lock);
            }
        }
    }
}

void EffectsGrid::SetEffectsDescription()
{
    if (mSequenceElements == nullptr) return;

    bool oktocont = true;
    if (mSelectedEffect != nullptr || AtLeastOneEffectSelected())
    {
        auto efs = GetSelectedEffects();
        // add in the selected effect if we didnt get it
        if (mSelectedEffect != nullptr)
        {
            bool found = false;
            for (auto it = efs.begin(); it != efs.end(); ++it)
            {
                if ((*it) == mSelectedEffect)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                efs.push_back(mSelectedEffect);
            }
        }

        // check if any have a different description
        wxString description = "";
        SettingsMap& sm = efs.front()->GetSettings();
        if (sm.Contains("X_Effect_Description"))
        {
            description = sm["X_Effect_Description"];
        }

        for (auto it = efs.begin(); it != efs.end(); ++it)
        {
            SettingsMap& sma = (*it)->GetSettings();
            wxString thisdescription = "";
            if (sma.Contains("X_Effect_Description"))
            {
                thisdescription = sma["X_Effect_Description"];
            }

            if (description != thisdescription)
            {
                if (wxMessageBox("Effects already have differing descriptions. Are you sure you want to change them all to the same value", "Overwrite description", wxYES_NO) == wxYES)
                {
                    if (description == "") description = thisdescription;
                    break;
                }
                else
                {
                    oktocont = false;
                    break;
                }
            }
        }

        if (oktocont && efs.size() > 0)
        {
            wxTextEntryDialog dlg(this, "Enter a description to associate with this effect", "Description", description);
            OptimiseDialogPosition(&dlg);
            if (dlg.ShowModal() == wxID_OK)
            {
                mSequenceElements->get_undo_mgr().CreateUndoStep();
                description = dlg.GetValue();

                for (auto it = efs.begin(); it != efs.end(); ++it)
                {
                    SettingsMap& smt = (*it)->GetSettings();
                    if (description == "" && smt.Contains("X_Effect_Description"))
                    {
                        smt.erase("X_Effect_Description");
                    }
                    else if (description != "")
                    {
                        smt["X_Effect_Description"] = description;
                    }
                }
            }
        }
    }
}

void EffectsGrid::SetEffectsTiming()
{
    if (mSequenceElements == nullptr ||
        mSelectedEffect == nullptr ||
        MultipleEffectsSelected() ||
        (mSelectedEffect != nullptr && mSelectedEffect->IsLocked())) {
        return;
    }

    EffectLayer* el = mSelectedEffect->GetParentEffectLayer();

    if (el->IsFixedTimingLayer()) return;

    EffectTimingDialog dlg(this, mSelectedEffect, el, mTimeline->GetTimeFrequency());
    OptimiseDialogPosition(&dlg);
    if (dlg.ShowModal() == wxID_OK)
    {
        mSequenceElements->get_undo_mgr().CreateUndoStep();

        mSelectedEffect->SetStartTimeMS(dlg.GetStartTime());
        mSelectedEffect->SetEndTimeMS(dlg.GetEndTime());
        ForceRefresh();
    }
}

void EffectsGrid::DeleteSelectedEffects()
{
    if (mSequenceElements == nullptr) {
        return;
    }

    mSequenceElements->get_undo_mgr().CreateUndoStep();
    for(int i=0;i<mSequenceElements->GetRowInformationSize();i++)
    {
        Element* element = mSequenceElements->GetRowInformation(i)->element;
        EffectLayer* el = mSequenceElements->GetEffectLayer(i);
        int start = 99999999;
        int end = -1;
        for (int x = 0; x < el->GetEffectCount(); x++) {
            Effect *ef = el->GetEffect(x);
            if (ef->GetSelected() != EFFECT_NOT_SELECTED && !ef->IsLocked() && !el->IsFixedTimingLayer()) {
                if (ef->GetStartTimeMS() < start) {
                    start = ef->GetStartTimeMS();
                }
                if (ef->GetEndTimeMS() > end) {
                    end = ef->GetEndTimeMS();
                }
            }
        }
        if (end > 0) {
            RenderCommandEvent event(element->GetModelName(), start, end, true, true);
            wxPostEvent(mParent, event);
            el->DeleteSelectedEffects(mSequenceElements->get_undo_mgr());
        }
    }
    UnselectEffect();
    mSelectedEffect = nullptr;
    mSelectedRow = -1;
    mResizing = false;
    mDragging = false;
    ForceRefresh();
}

void EffectsGrid::AlignSelectedEffects(EFF_ALIGN_MODE align_mode)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    //TODO :  Fix so that rendering occurs where effects used to exist and their new location
    if (mSequenceElements == nullptr || mSelectedEffect == nullptr) {
        return;
    }

    logger_base.debug("EffectsGrid::AlignSelectedEffects.");

    int sel_eff_start = mSelectedEffect->GetStartTimeMS();
    int sel_eff_end = mSelectedEffect->GetEndTimeMS();
    int sel_eff_center = 0;
    if (align_mode == ALIGN_CENTERPOINTS) {
        sel_eff_center = sel_eff_end - sel_eff_start;
        if( sel_eff_center < mSequenceElements->GetMinPeriod() ) {
            sel_eff_center = sel_eff_start;
        } else {
            sel_eff_center = sel_eff_start + sel_eff_center/2;
        }
    }
    mSequenceElements->get_undo_mgr().CreateUndoStep();
    int rows_to_process = mSequenceElements->GetRowInformationSize();
    std::vector<bool> reserved;
    int first_model_row = mSequenceElements->GetNumberOfTimingRows();
    for (int i = first_model_row; i < rows_to_process; i++)
    {
        Element* element = mSequenceElements->GetRowInformation(i)->element;
        EffectLayer* el = mSequenceElements->GetEffectLayer(i);
        for (int x = 0; x < el->GetEffectCount(); x++) {
            Effect *ef = el->GetEffect(x);
            if (ef->GetSelected() != EFFECT_NOT_SELECTED && !ef->IsLocked() && !el->IsFixedTimingLayer()) {
                int align_start, align_end, align_delta;
                if( align_mode == ALIGN_START_TIMES ) {
                    align_start = sel_eff_start;
                    if( ef->GetEndTimeMS() > sel_eff_start ) {
                        align_end = ef->GetEndTimeMS();
                    } else {
                        align_delta = align_start - ef->GetStartTimeMS();
                        align_end = ef->GetEndTimeMS() + align_delta;
                    }
                } else if( align_mode == ALIGN_END_TIMES ) {
                    align_end = sel_eff_end;
                    if( ef->GetStartTimeMS() < sel_eff_end ) {
                        align_start = ef->GetStartTimeMS();
                    } else {
                        align_delta = align_end - ef->GetEndTimeMS();
                        align_start = ef->GetStartTimeMS() + align_delta;
                    }
                } else if( align_mode == ALIGN_BOTH_TIMES ) {
                    align_start = sel_eff_start;
                    align_end = sel_eff_end;
                } else if( align_mode == ALIGN_CENTERPOINTS ) {
                    int eff_center = ef->GetEndTimeMS() - ef->GetStartTimeMS();
                    if( eff_center < mSequenceElements->GetMinPeriod() ) {
                        eff_center = ef->GetStartTimeMS();
                    } else {
                        eff_center = ef->GetStartTimeMS() + eff_center/2;
                    }
                    align_delta = sel_eff_center - eff_center;
                    align_start = ef->GetStartTimeMS() + align_delta;
                    align_end = ef->GetEndTimeMS() + align_delta;
                } else if( align_mode == ALIGN_MATCH_DURATION ) {
                    align_delta = sel_eff_end - sel_eff_start;
                    align_start = ef->GetStartTimeMS();
                    align_end = align_start + align_delta;
                } else if ( align_mode == ALIGN_START_TIMES_SHIFT ) {
                    align_start = sel_eff_start;
                    align_delta = align_start - ef->GetStartTimeMS();
                    align_end = ef->GetEndTimeMS() + align_delta;
                } else if( align_mode == ALIGN_END_TIMES_SHIFT ) {
                    align_end = sel_eff_end;
                    align_delta = align_end - ef->GetEndTimeMS();
                    align_start = ef->GetStartTimeMS() + align_delta;
                } else {
                    return;
                }
                if( align_start < 0 ) {
                    align_start = 0;
                }

                // adjust times so effect doesn't block himself
                bool all_clear = false;
                int str_time_for_check = align_start;
                int end_time_for_check = align_end;
                if( align_mode == ALIGN_BOTH_TIMES ) {
                    if( align_start < ef->GetStartTimeMS() && align_end > ef->GetEndTimeMS() ) {
                        if( el->GetRangeIsClearMS( align_start, ef->GetStartTimeMS()) &&
                            el->GetRangeIsClearMS( ef->GetEndTimeMS(), align_end) ) {
                            all_clear = true;
                        }
                    }
                }
                if( !all_clear ) {
                    if( align_start >= ef->GetStartTimeMS() && align_end <= ef->GetEndTimeMS() ) {
                        all_clear = true;
                    } else {
                        // time for check excludes the time of the effect we are adjusting
                        // otherwise the check for a clash always fails
                        if( align_end > ef->GetStartTimeMS() && align_end <= ef->GetEndTimeMS() ) {
                            end_time_for_check = std::min(mSelectedEffect->GetEndTimeMS(), ef->GetStartTimeMS());
                        } else if( align_start >= ef->GetStartTimeMS() && align_start < ef->GetEndTimeMS() ) {
                            str_time_for_check = std::max(mSelectedEffect->GetStartTimeMS(), ef->GetEndTimeMS());
                        }
                    }
                }

                if( all_clear || el->GetRangeIsClearMS( str_time_for_check, end_time_for_check) ) {
                    if (!ef->IsLocked() && !el->IsFixedTimingLayer())
                    {
                        mSequenceElements->get_undo_mgr().CaptureEffectToBeMoved(el->GetParentElement()->GetModelName(), el->GetIndex(), ef->GetID(),
                            ef->GetStartTimeMS(), ef->GetEndTimeMS());
                        ef->SetStartTimeMS(align_start);
                        ef->SetEndTimeMS(align_end);
                    }
                } else if (mSequenceElements->GetRowInformation(i)->nodeIndex != -1) {
                    std::string name = ef->GetEffectName();
                    std::string settings = ef->GetSettingsAsString();
                    std::string palette = ef->GetPaletteAsString();
                    mSequenceElements->get_undo_mgr().CaptureEffectToBeDeleted( el->GetParentElement()->GetModelName(), el->GetIndex(), ef->GetEffectName(),
                                                                                ef->GetSettingsAsString(), ef->GetPaletteAsString(),
                                                                                ef->GetStartTimeMS(), ef->GetEndTimeMS(),
                                                                                ef->GetSelected(), ef->GetProtected() );
                    el->DeleteEffect(ef->GetID());
                    if (ef == mSelectedEffect) {
                        mSelectedEffect = nullptr;
                    }
                    EffectLayer* new_el = EffectsGrid::FindOpenLayer(element, align_start, align_end);
                    element->SetCollapsed(false);
                    Effect* new_ef = new_el->AddEffect(0,
                                                       name,
                                                       settings,
                                                       palette,
                                                       align_start,
                                                       align_end,
                                                       EFFECT_NOT_SELECTED,
                                                       false);
                    new_ef->SetStartTimeMS(align_start);
                    new_ef->SetEndTimeMS(align_end);
                    mSequenceElements->get_undo_mgr().CaptureAddedEffect( new_el->GetParentElement()->GetName(), new_el->GetIndex(), new_ef->GetID() );
                    mSequenceElements->PopulateRowInformation();
                    rows_to_process = mSequenceElements->GetRowInformationSize();
                    x--; // need to reprocess this index since effect was added.
                }
            }
        }
    }
    sendRenderDirtyEvent();
    xlights->DoForceSequencerRefresh();
}

bool EffectsGrid::PapagayoEffectsSelected() const
{
    for(int i = 0; i<mSequenceElements->GetVisibleRowInformationSize(); i++)
    {
        EffectLayer* el = mSequenceElements->GetVisibleEffectLayer(i);
        if( el->GetParentElement()->GetType() == ElementType::ELEMENT_TYPE_TIMING )
        {
            if( el->GetParentElement()->GetEffectLayerCount() > 1 )
            {
                if(el->GetSelectedEffectCount() > 0)
                {
                     return true;
                }
            }
        }
    }
    return false;
}

bool EffectsGrid::AtLeastOneEffectSelected() const
{
    for (int i = 0; i < mSequenceElements->GetRowInformationSize(); i++)
    {
        EffectLayer* el = mSequenceElements->GetEffectLayer(i);
        if (el->GetSelectedEffectCount() > 0)
        {
            return true;
        }
    }
    return false;
}

std::set<EffectLayer *> EffectsGrid::GetLayersWithSelectedEffects() const {
    std::set<EffectLayer *> layers;
    for (int i = 0; i < mSequenceElements->GetRowInformationSize(); i++) {
        EffectLayer* el = mSequenceElements->GetEffectLayer(i);
        if (el->GetSelectedEffectCount() > 0) {
            layers.insert(el);
        }
    }
    return layers;
}

bool EffectsGrid::MultipleEffectsSelected() const
{
    int count = 0;
    for (auto layer : GetLayersWithSelectedEffects()) {
        count += layer->GetSelectedEffectCount();
        if (count > 1) {
            return true;
        }
    }
    return false;
}

bool EffectsGrid::OneCellSelected()
{
    if( mCellRangeSelected ) {
        if( mRangeStartCol == mRangeEndCol && mRangeStartRow == mRangeEndRow ) {
            mDropRow = mRangeStartRow - mSequenceElements->GetFirstVisibleModelRow();
            return true;
        }
    }
    return false;
}

Effect* EffectsGrid::OldPaste(const wxString &data, const wxString &pasteDataVersion) {

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    Effect* res = nullptr;

    if (mSequenceElements == nullptr) return res;

    logger_base.debug("EffectsGrid::OldPaste.");

    bool paste_by_cell = ((MainSequencer*)mParent)->PasteByCellActive();
    ((MainSequencer*)mParent)->PanelRowHeadings->SetCanPaste(false);
    if (mPartialCellSelected || OneCellSelected()) {
        wxArrayString all_efdata = wxSplit(data, '\n');
        if (all_efdata.size() == 0) {
            return res;
        }
        if( all_efdata.size() > 2 )  // multi-effect paste
        {
            wxArrayString eff1data = wxSplit(all_efdata[0], '\t');
			if (eff1data.size() < 7)
			{
				// this code assumes at least 7 so dont go there
                wxASSERT(false);
			}
			else
			{
			    int column_start_time = wxAtoi(eff1data[6]);
				int drop_time_offset = wxAtoi(eff1data[3]);
				EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
				if (column_start_time < 0 || tel == nullptr)
				{
					drop_time_offset = mDropStartTimeMS - drop_time_offset;
				}
				else
				{
					drop_time_offset = mDropStartTimeMS - column_start_time;
				}
				int drop_row = mDropRow;
				int start_row = wxAtoi(eff1data[5]);
				int drop_row_offset = drop_row - start_row;
				mSequenceElements->get_undo_mgr().CreateUndoStep();
				for (size_t i = 0; i < all_efdata.size() - 1; i++)
				{
					wxArrayString efdata = wxSplit(all_efdata[i], '\t');
					if (efdata.size() < 7) {
						break;
					}
					int new_start_time = wxAtoi(efdata[3]);
					int new_end_time = wxAtoi(efdata[4]);
					new_start_time += drop_time_offset;
					new_end_time += drop_time_offset;
					int eff_row = wxAtoi(efdata[5]);
					drop_row = eff_row + drop_row_offset + mSequenceElements->GetFirstVisibleModelRow();
					Row_Information_Struct* row_info = mSequenceElements->GetRowInformationFromRow(drop_row);
					if (row_info == nullptr) break;
					Element* elem = row_info->element;
					if (elem == nullptr) break;
					EffectLayer* el = mSequenceElements->GetEffectLayer(row_info);
					if (el == nullptr) break;
					if (el->GetRangeIsClearMS(new_start_time, new_end_time))
					{
						int effectIndex = xlights->GetEffectManager().GetEffectIndex(efdata[0].ToStdString());
						if (effectIndex >= 0) {
							Effect* ef = el->AddEffect(0,
								efdata[0].ToStdString(),
								efdata[1].ToStdString(),
								efdata[2].ToStdString(),
								new_start_time,
								new_end_time,
								EFFECT_NOT_SELECTED,
								false);
                            if (res == nullptr) res = ef;

                            if (ef != nullptr)
                            {
                                ef->SetLocked(false);
                                if (xlights->GetEffectManager().GetEffect(efdata[0].ToStdString())->needToAdjustSettings(pasteDataVersion.ToStdString())) {
                                    xlights->GetEffectManager().GetEffect(efdata[0].ToStdString())->adjustSettings(pasteDataVersion.ToStdString(), ef, false);
                                }
                                mSequenceElements->get_undo_mgr().CaptureAddedEffect(el->GetParentElement()->GetModelName(), el->GetIndex(), ef->GetID());
                                if (!ef->GetPaletteMap().empty()) {
                                    sendRenderEvent(el->GetParentElement()->GetModelName(),
                                        new_start_time,
                                        new_end_time, true);
                                }
                            }
						}
					}
				}
				mPartialCellSelected = false;
			}
        }
        else
        {
            wxArrayString efdata = wxSplit(all_efdata[0], '\t');
            if (efdata.size() < 3) {
                return res;
            }
            if( efdata[0] == "Random" )
            {
                Effect* ef = FillRandomEffects();
                if (res == nullptr) res = ef;
            }
            else
            {
                if(mCellRangeSelected && !mPartialCellSelected)
                {
                    EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
                    mDropStartTimeMS = tel->GetEffect(mRangeStartCol)->GetStartTimeMS();
                    mDropEndTimeMS = tel->GetEffect(mRangeEndCol)->GetEndTimeMS();
                    int first_row = mSequenceElements->GetFirstVisibleModelRow();
                    mDropRow = mRangeStartRow - first_row;
                }
                int effectIndex = xlights->GetEffectManager().GetEffectIndex(efdata[0].ToStdString());
                if (effectIndex >= 0) {
                    int end_time = mDropEndTimeMS;
                    if( (efdata.size() == 7) && GetActiveTimingElement() == nullptr )  // use original effect length if no timing track is active
                    {
                        int drop_time_offset = wxAtoi(efdata[3]);
                        drop_time_offset = mDropStartTimeMS - drop_time_offset;
                        end_time = wxAtoi(efdata[4]);
                        end_time += drop_time_offset;
                    }
                    EffectLayer* el = mSequenceElements->GetVisibleEffectLayer(mDropRow);
                    if (el != nullptr && el->GetRangeIsClearMS(mDropStartTimeMS, end_time) )
                    {
                        Effect* ef = el->AddEffect(0,
                                      efdata[0].ToStdString(),
                                      efdata[1].ToStdString(),
                                      efdata[2].ToStdString(),
                                      mDropStartTimeMS,
                                      end_time,
                                      EFFECT_SELECTED,
                                      false);
                        if (res == nullptr) res = ef;

                        if (ef != nullptr)
                        {
                            ef->SetLocked(false);
                            if (xlights->GetEffectManager().GetEffect(efdata[0].ToStdString())->needToAdjustSettings(pasteDataVersion.ToStdString())) {
                                xlights->GetEffectManager().GetEffect(efdata[0].ToStdString())->adjustSettings(pasteDataVersion.ToStdString(), ef, false);
                            }
                            mSequenceElements->get_undo_mgr().CreateUndoStep();
                            mSequenceElements->get_undo_mgr().CaptureAddedEffect(el->GetParentElement()->GetModelName(), el->GetIndex(), ef->GetID());
                            if (!ef->GetPaletteMap().empty()) {
                                sendRenderEvent(el->GetParentElement()->GetModelName(),
                                    mDropStartTimeMS,
                                    mDropEndTimeMS, true);
                            }
                            RaiseSelectedEffectChanged(ef, true);
                            mSelectedEffect = ef;
                            mPartialCellSelected = false;
                            mSelectedRow = mDropRow;
                        }
                    }
                }
            }
        }
    }
    else if (mCellRangeSelected) {
        wxArrayString all_efdata = wxSplit(data, '\n');
        if (all_efdata.size() == 0) {
            return res;
        }
        if( all_efdata.size() <= 2 )  // only single effect paste allowed for range
        {
            wxArrayString efdata = wxSplit(all_efdata[0], '\t');
            if (efdata.size() < 3) {
                return res;
            }
            if( efdata[0] == "Random" )
            {
                Effect* ef = FillRandomEffects();
                if (res == nullptr) res = ef;
            }
            else
            {
                int row1 = mRangeStartRow;
                int row2 = mRangeEndRow;
                if( row1 > row2 ) {
                    std::swap(row1, row2);
                }
                int col1 = mRangeStartCol;
                int col2 = mRangeEndCol;
                if( col1 > col2 ) {
                    std::swap(col1, col2);
                }
                for( int row = row1; row <= row2; row++ )
                {
                    EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
                    if( paste_by_cell )
                    {
                        EffectLayer* tel1 = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
                        int timingIndex1 = mRangeStartCol;
                        int timingIndex2 = mRangeEndCol;
                        if( timingIndex1 > timingIndex2 ) {
                            std::swap(timingIndex1, timingIndex2);
                        }
                        if (timingIndex1 != -1 && timingIndex2 != -1) {
                            mSequenceElements->get_undo_mgr().CreateUndoStep();
                            for( int row4 = row1; row4 <= row2; row4++)
                            {
                                EffectLayer* effectLayer = mSequenceElements->GetEffectLayer(row4);
                                for(int i = timingIndex1; i <= timingIndex2; i++)
                                {
                                    Effect* eff = tel1->GetEffect(i);
                                    if( effectLayer != nullptr && effectLayer->GetRangeIsClearMS(eff->GetStartTimeMS(), eff->GetEndTimeMS()) )
                                    {
                                        Effect* ef = effectLayer->AddEffect(0,
                                                                  efdata[0].ToStdString(),
                                                                  efdata[1].ToStdString(),
                                                                  efdata[2].ToStdString(),
                                                                  eff->GetStartTimeMS(),
                                                                  eff->GetEndTimeMS(),
                                                                  EFFECT_SELECTED,
                                                                  false);
                                        if (res == nullptr) res = ef;
                                        if (ef != nullptr)
                                        {
                                            ef->SetLocked(false);
                                            if (xlights->GetEffectManager().GetEffect(efdata[0].ToStdString())->needToAdjustSettings(pasteDataVersion.ToStdString())) {
                                                xlights->GetEffectManager().GetEffect(efdata[0].ToStdString())->adjustSettings(pasteDataVersion.ToStdString(), ef, false);
                                            }
                                            mSequenceElements->get_undo_mgr().CaptureAddedEffect(effectLayer->GetParentElement()->GetModelName(), effectLayer->GetIndex(), ef->GetID());
                                            RaiseSelectedEffectChanged(ef, true);
                                            mSelectedEffect = ef;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        mSequenceElements->get_undo_mgr().CreateUndoStep();
                        int start_time = tel->GetEffect(col1)->GetStartTimeMS();
                        int end_time = tel->GetEffect(col2)->GetEndTimeMS();
                        EffectLayer* el = mSequenceElements->GetEffectLayer(row);
                        if(el != nullptr && el->GetRangeIsClearMS(start_time, end_time) )
                        {
                            int effectIndex = xlights->GetEffectManager().GetEffectIndex(efdata[0].ToStdString());
                            if (effectIndex >= 0) {
                                Effect* ef = el->AddEffect(0,
                                          efdata[0].ToStdString(),
                                          efdata[1].ToStdString(),
                                          efdata[2].ToStdString(),
                                          start_time,
                                          end_time,
                                          EFFECT_SELECTED,
                                          false);
                                if (res == nullptr) res = ef;
                                if (ef != nullptr)
                                {
                                    ef->SetLocked(false);
                                    if (xlights->GetEffectManager().GetEffect(efdata[0].ToStdString())->needToAdjustSettings(pasteDataVersion.ToStdString())) {
                                        xlights->GetEffectManager().GetEffect(efdata[0].ToStdString())->adjustSettings(pasteDataVersion.ToStdString(), ef, false);
                                    }
                                    mSequenceElements->get_undo_mgr().CaptureAddedEffect(el->GetParentElement()->GetModelName(), el->GetIndex(), ef->GetID());
                                    if (!ef->GetPaletteMap().empty()) {
                                        sendRenderEvent(el->GetParentElement()->GetModelName(),
                                            start_time,
                                            end_time, true);
                                    }
                                    RaiseSelectedEffectChanged(ef, true);
                                    mSelectedEffect = ef;
                                }
                             }
                        }
                    }
                }
            }
            if (!xlights->IsACActive())
            {
                mCellRangeSelected = false;
            }
        }
    }

    Refresh();
    return res;
}

int EffectsGrid::GetMSFromColumn(int col) const
{
    if (col < 0) return 0;
    EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
    if (tel != nullptr)
    {
        return tel->GetEffect(col)->GetStartTimeMS();
    }
    return 0;
}

Effect* EffectsGrid::Paste(const wxString& data, const wxString& pasteDataVersion, bool row_paste) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    Effect* res = nullptr;

    if (mSequenceElements == nullptr) return res;

    logger_base.info("Pasting data: %s", (const char*)data.c_str());

    wxArrayString all_efdata = wxSplit(data, '\n');
    if (all_efdata.size() == 0)  return res;

    wxArrayString banner_data = wxSplit(all_efdata[0], '\t');
    if (banner_data.size() == 0) return res;

    if (banner_data[0] != "CopyFormat1" && banner_data[0] != "CopyFormatAC")
    {
        res = OldPaste(data, pasteDataVersion);
        return res;
    }

    if (banner_data[0] == "CopyFormatAC" && !xlights->IsACActive())
    {
        DisplayWarning("Cannot paste AC data in non-AC mode");
        return res;
    }

    if (banner_data[0] != "CopyFormatAC" && xlights->IsACActive())
    {
        DisplayWarning("Only AC data may be pasted in AC mode");
        return res;
    }

    if (banner_data.size() < 7) return res;
    bool contains_cell_info = (banner_data[6] != "NO_PASTE_BY_CELL");
    bool paste_by_cell = ((MainSequencer*)mParent)->PasteByCellActive();
    if (paste_by_cell && !row_paste)
    {
        if (!contains_cell_info)
        {
            DisplayWarning("Paste By Cell information missing.\nYou can only Paste By Time with this data.");
            return res;
        }
        if (mSequenceElements->GetSelectedTimingRow() < 0)
        {
            DisplayWarning("Paste By Cell requires an active timing track.");
            return res;
        }
    }

    int number_of_timings = wxAtoi(banner_data[1]);
    int number_of_effects = wxAtoi(banner_data[2]);
    int number_of_original_timing_rows = wxAtoi(banner_data[3]);
    int last_timing_row = wxAtoi(banner_data[4]);
    int start_column = wxAtoi(banner_data[5]);
    int selected_start_column = 0;
    int number_of_timing_rows = mSequenceElements->GetNumberOfTimingRows();
    int selectedrows = mRangeEndRow - mRangeStartRow + 1;
    int timestopaste = 1;

    if (number_of_timings > 0 && number_of_effects > 0)
    {
        if (number_of_original_timing_rows != number_of_timing_rows)
        {
            DisplayWarning("Number of timing rows does not match how many existed when copied.");
            return res;
        }
    }

    ((MainSequencer*)mParent)->PanelRowHeadings->SetCanPaste(false);

    logger_base.info("mPartialCellSelected %d,   OneCellSelected: %d    paste_by_cell:  %d", (int)mPartialCellSelected, (int)OneCellSelected(), paste_by_cell);

    if (mPartialCellSelected || OneCellSelected() || xlights->IsACActive() || row_paste) {
        if (((number_of_timings + number_of_effects) > 1) || xlights->IsACActive() || paste_by_cell)  // multi-effect paste or row_paste or paste_by_cell one effect
        {
            std::set<std::string> modelsToRender;

            // we refer to all_efdata[1] below so make sure it is there
            if (all_efdata.size() < 2) return res;

            wxArrayString eff1data = wxSplit(all_efdata[1], '\t');
            if (eff1data.size() < 7) return res;

            int column_start_time = wxAtoi(eff1data[6]);
            if (xlights->IsACActive()) {
                if (banner_data.size() < 10) return res;
                column_start_time = wxAtoi(banner_data[9]);
            }
            int drop_time_offset = wxAtoi(eff1data[3]);
            EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
            if (row_paste) {
                mDropStartTimeMS = drop_time_offset;
            }
            if (column_start_time < 0 || tel == nullptr)
            {
                drop_time_offset = mDropStartTimeMS - drop_time_offset;
            }
            else
            {
                drop_time_offset = mDropStartTimeMS - column_start_time;
            }
            int drop_row = mDropRow;
            int start_row = wxAtoi(eff1data[5]);
            if (xlights->IsACActive()) {
                start_row = wxAtoi(banner_data[7]);
            }
            int drop_row_offset = drop_row - start_row;
            if (number_of_timings > 0 && number_of_effects > 0)  // only allow timing and model effects to be pasted on same rows
            {
                drop_row = 0;
                drop_row_offset = 0;
            }
            else if (number_of_timings > 0 && number_of_effects == 0) // if only timing effects make sure enough rows remain to receive effects
            {
                if (last_timing_row + mDropRow >= number_of_timing_rows)
                {
                    DisplayWarning("Not enough timing rows to paste timing effects starting on this row.");
                    return res;
                }
            }
            else if (number_of_timings == 0 && number_of_effects > 0) // if only model effects make sure target row isn't in timing tracks
            {
                if (mDropRow < number_of_timing_rows) {
                    DisplayWarning("Cannot paste model effects into timing tracks.");
                    return res;
                }
            }
            if (paste_by_cell && !row_paste) {
                if (tel == nullptr) {
                    logger_base.crit("EffectsGrid::Paste tel is nullptr ... this is going to crash.");
                }

                bool found_selected_start_column = tel->HitTestEffectByTime(mDropStartTimeMS + 1, selected_start_column);
                if (!found_selected_start_column)
                {
                    DisplayWarning("Unable to find a selected timing start location for Paste By Cell.");
                    return res;
                }
            }

            // clear the region if AC mode
            if (xlights->IsACActive()) {
                if (banner_data.size() < 11) return res;
                int drop_end_time = mDropStartTimeMS + wxAtoi(banner_data[10]) - wxAtoi(banner_data[9]);
                int rowstopaste = wxAtoi(banner_data[8]) - wxAtoi(banner_data[7]) + 1;
                if (rowstopaste == 1)
                {
                    timestopaste = selectedrows;
                    mDropRow = std::min(mRangeStartRow, mRangeEndRow);
                    if (mRangeCursorRow == mRangeEndRow)
                    {
                        drop_row_offset -= timestopaste - 1;
                    }
                    res = ACDraw(ACTYPE::OFF, ACSTYLE::NILSTYLEOVERRIDE, ACMODE::MODENIL, 0, 0, 0, mDropStartTimeMS, drop_end_time, mRangeStartRow, mRangeEndRow);
                }
                else
                {
                    // force paste to top left of selection
                    mDropRow = std::min(mRangeStartRow, mRangeEndRow);
                    int drop_end_row = mDropRow + rowstopaste - 1 + mSequenceElements->GetFirstVisibleModelRow();

                    // This gets it to the right row ... but doesnt fix the right column so leaving it out until i can fix that
                    // It isnt great the way it is ... but it is better than being inconsistent
                    //if (mRangeCursorRow == mRangeEndRow)
                    //{
                    //    drop_row_offset -= selectedrows - 1;
                    //}
                    res = ACDraw(ACTYPE::OFF, ACSTYLE::NILSTYLEOVERRIDE, ACMODE::MODENIL, 0, 0, 0, mDropStartTimeMS, drop_end_time, mDropRow + mSequenceElements->GetFirstVisibleModelRow(), drop_end_row);
                }
            }

            mSequenceElements->get_undo_mgr().CreateUndoStep();

            for (int rpts = 0; rpts < timestopaste; ++rpts)
            {
                for (size_t i = 1; i < all_efdata.size() - 1; i++)
                {
                    wxArrayString efdata = wxSplit(all_efdata[i], '\t');
                    if (efdata.size() < 8)  break;
                    bool is_timing_effect = (efdata[7] == "TIMING_EFFECT");
                    int new_start_time = wxAtoi(efdata[3]);
                    int new_end_time = wxAtoi(efdata[4]);
                    if (paste_by_cell && !is_timing_effect && !row_paste)
                    {
                        if (efdata.size() < 11)  break;
                        int eff_start_column = wxAtoi(efdata[7]);
                        int eff_end_column = wxAtoi(efdata[8]);
                        int eff_start_pct = wxAtoi(efdata[9]);
                        int eff_end_pct = wxAtoi(efdata[10]);
                        int column_offset = selected_start_column - start_column;
                        eff_start_column += column_offset;
                        eff_end_column += column_offset;
                        Effect* te_start = tel->GetEffect(eff_start_column);
                        Effect* te_end = tel->GetEffect(eff_end_column);
                        if (te_start == nullptr || te_end == nullptr)
                        {
                            break;
                        }
                        new_start_time = te_start->GetStartTimeMS() + (((te_start->GetEndTimeMS() - te_start->GetStartTimeMS()) * eff_start_pct) / 100);
                        new_end_time = te_end->GetStartTimeMS() + (((te_end->GetEndTimeMS() - te_end->GetStartTimeMS()) * eff_end_pct) / 100);
                    }
                    else
                    {
                        new_start_time += drop_time_offset;
                        new_end_time += drop_time_offset;
                    }
                    int eff_row = wxAtoi(efdata[5]);
                    drop_row = eff_row + drop_row_offset + rpts;
                    if (!is_timing_effect)
                    {
                        drop_row += mSequenceElements->GetFirstVisibleModelRow();
                    }
                    Row_Information_Struct* row_info = mSequenceElements->GetRowInformationFromRow(drop_row);
                    if (row_info == nullptr) break;
                    Element* elem = row_info->element;
                    if (elem == nullptr) break;
                    EffectLayer* el = mSequenceElements->GetEffectLayer(row_info);
                    if (el == nullptr) break;
                    if (el->GetRangeIsClearMS(new_start_time, new_end_time))
                    {
                        int effectIndex = xlights->GetEffectManager().GetEffectIndex(efdata[0].ToStdString());
                        if (effectIndex >= 0 || is_timing_effect) {
                            Effect* ef = el->AddEffect(0,
                                efdata[0].ToStdString(),
                                efdata[1].ToStdString(),
                                efdata[2].ToStdString(),
                                new_start_time,
                                new_end_time,
                                EFFECT_NOT_SELECTED,
                                false);

                            if (res == nullptr) res = ef;

                            if (ef != nullptr)
                            {
                                ef->SetLocked(false);
                                logger_base.info("(1) Created effect %s  %s  %s  %d %d -->  %X",
                                    (const char*)efdata[0].c_str(),
                                    (const char*)efdata[1].Left(128).c_str(),
                                    (const char*)efdata[2].c_str(),
                                    new_start_time,
                                    new_end_time, ef);
                                if (!is_timing_effect && xlights->GetEffectManager().GetEffect(efdata[0].ToStdString())->needToAdjustSettings(pasteDataVersion.ToStdString())) {
                                    xlights->GetEffectManager().GetEffect(efdata[0].ToStdString())->adjustSettings(pasteDataVersion.ToStdString(), ef, false);
                                }
                                mSequenceElements->get_undo_mgr().CaptureAddedEffect(el->GetParentElement()->GetModelName(), el->GetIndex(), ef->GetID());
                            }
                        }
                    }
                }
            }
            sendRenderDirtyEvent();
            mPartialCellSelected = false;
        }
        else
        {
            // we refer to all_efdata[1] below so make sure it is there
            if (all_efdata.size() < 2) return res;

            wxArrayString efdata = wxSplit(all_efdata[1], '\t');
            if (efdata.size() < 3) {
                return res;
            }
            if (efdata[0] == "Random")
            {
                Effect* ef = FillRandomEffects();
                if (res == nullptr) res = ef;
            }
            else
            {
                if (efdata.size() < 7) {
                    return res;
                }
                bool is_timing_effect = (efdata[7] == "TIMING_EFFECT");
                logger_base.info("mCellRangeSelected: %d   mPartialCellSelected: %d", mCellRangeSelected, mPartialCellSelected);

                if (mCellRangeSelected && !mPartialCellSelected)
                {
                    EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
                    mDropStartTimeMS = tel->GetEffect(mRangeStartCol)->GetStartTimeMS();
                    mDropEndTimeMS = tel->GetEffect(mRangeEndCol)->GetEndTimeMS();
                    int first_row = mSequenceElements->GetFirstVisibleModelRow();
                    mDropRow = mRangeStartRow - first_row;
                }
                if (number_of_timings == 0 && mDropRow < number_of_timing_rows)
                {
                    DisplayWarning("Cannot paste model effect into timing track.");
                    return res;
                }
                int effectIndex = xlights->GetEffectManager().GetEffectIndex(efdata[0].ToStdString());
                logger_base.info("mDropRow: %d   effectIndex: %d", mDropRow, effectIndex);
                if (effectIndex >= 0 || is_timing_effect) {
                    int end_time = mDropEndTimeMS;
                    if (((efdata.size() >= 7) && GetActiveTimingElement() == nullptr) || !paste_by_cell)  // use original effect length if no timing track is active
                    {
                        int drop_time_offset = wxAtoi(efdata[3]);
                        drop_time_offset = mDropStartTimeMS - drop_time_offset;
                        end_time = wxAtoi(efdata[4]);
                        end_time += drop_time_offset;
                    }
                    EffectLayer* el = mSequenceElements->GetVisibleEffectLayer(mDropRow);
                    if (el != nullptr && el->GetRangeIsClearMS(mDropStartTimeMS, end_time))
                    {
                        Effect* ef = el->AddEffect(0,
                            efdata[0].ToStdString(),
                            efdata[1].ToStdString(),
                            efdata[2].ToStdString(),
                            mDropStartTimeMS,
                            end_time,
                            EFFECT_SELECTED,
                            false);

                        if (res == nullptr) res = ef;

                        if (ef != nullptr)
                        {
                            ef->SetLocked(false);
                            logger_base.info("(2) Created effect %s  %s  %s  %d %d -->  %X",
                                (const char*)efdata[0].c_str(),
                                (const char*)efdata[1].Left(128).c_str(),
                                (const char*)efdata[2].c_str(),
                                mDropStartTimeMS,
                                end_time, ef);
                            if (!is_timing_effect && xlights->GetEffectManager().GetEffect(efdata[0].ToStdString())->needToAdjustSettings(pasteDataVersion.ToStdString())) {
                                xlights->GetEffectManager().GetEffect(efdata[0].ToStdString())->adjustSettings(pasteDataVersion.ToStdString(), ef, false);
                            }
                            mSequenceElements->get_undo_mgr().CreateUndoStep();
                            mSequenceElements->get_undo_mgr().CaptureAddedEffect(el->GetParentElement()->GetModelName(), el->GetIndex(), ef->GetID());
                            if (!is_timing_effect) {
                                RaiseSelectedEffectChanged(ef, true);
                                mSelectedEffect = ef;
                                mPartialCellSelected = false;
                                mSelectedRow = mDropRow;
                            }
                        }
                    }
                }
                sendRenderDirtyEvent();
            }
        }
    }
    else if (mCellRangeSelected) {
        if (number_of_timings == 0 && number_of_effects == 1) { // only single effect paste allowed for range
            // we refer to all_efdata[1] below so make sure it is there
            if (all_efdata.size() < 2) return res;

            wxArrayString efdata = wxSplit(all_efdata[1], '\t');
            if (efdata.size() < 3)  return res;
            if (efdata[0] == "Random") {
                Effect* ef = FillRandomEffects();
                if (res == nullptr) res = ef;
            }
            else
            {
                mSequenceElements->get_undo_mgr().CreateUndoStep();
                int row1 = mRangeStartRow;
                int row2 = mRangeEndRow;
                if (row1 > row2) {
                    std::swap(row1, row2);
                }
                int col1 = mRangeStartCol;
                int col2 = mRangeEndCol;
                if (col1 > col2) {
                    std::swap(col1, col2);
                }
                logger_base.info("row1: %d   row2: %d   col1: %d   col2: %d", row1, row2, col1, col2);

                for (int row = row1; row <= row2; row++) {
                    EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
                    if (tel->GetEffect(col1) == nullptr) {
                        logger_base.info("No start time");
                        break;
                    }
                    int start_time = tel->GetEffect(col1)->GetStartTimeMS();
                    if (tel->GetEffect(col2) == nullptr) {
                        logger_base.info("No end time");
                        break;
                    }
                    int end_time = tel->GetEffect(col2)->GetEndTimeMS();
                    if (!paste_by_cell) { // use original effect length if paste by time
                        if (efdata.size() < 5)  return res;
                        int drop_time_offset = wxAtoi(efdata[3]);
                        drop_time_offset = mDropStartTimeMS - drop_time_offset;
                        end_time = wxAtoi(efdata[4]);
                        end_time += drop_time_offset;
                    }
                    EffectLayer* el = mSequenceElements->GetEffectLayer(row);
                    if (el != nullptr && el->GetRangeIsClearMS(start_time, end_time)) {
                        int effectIndex = xlights->GetEffectManager().GetEffectIndex(efdata[0].ToStdString());
                        if (effectIndex >= 0) {
                            Effect* ef = el->AddEffect(0,
                                efdata[0].ToStdString(),
                                efdata[1].ToStdString(),
                                efdata[2].ToStdString(),
                                start_time,
                                end_time,
                                EFFECT_SELECTED,
                                false);

                            if (res == nullptr) res = ef;

                            if (ef != nullptr) {
                                ef->SetLocked(false);
                                logger_base.info("(3) Created effect %s  %s  %s  %d %d -->  %X",
                                    (const char*)efdata[0].c_str(),
                                    (const char*)efdata[1].Left(128).c_str(),
                                    (const char*)efdata[2].c_str(),
                                    mDropStartTimeMS,
                                    end_time, ef);
                                if (xlights->GetEffectManager().GetEffect(efdata[0].ToStdString())->needToAdjustSettings(pasteDataVersion.ToStdString())) {
                                    xlights->GetEffectManager().GetEffect(efdata[0].ToStdString())->adjustSettings(pasteDataVersion.ToStdString(), ef, false);
                                }
                                mSequenceElements->get_undo_mgr().CaptureAddedEffect(el->GetParentElement()->GetModelName(), el->GetIndex(), ef->GetID());
                                RaiseSelectedEffectChanged(ef, true);
                                mSelectedEffect = ef;
                            }
                        }
                    }
                }
            }
            sendRenderDirtyEvent();
            mCellRangeSelected = false;
        }
    }

    Refresh();
    return res;
}

void EffectsGrid::SelectEffect(Effect* ef)
{
    if (ef == nullptr) return;

    mSelectedEffect = ef;
    ef->SetSelected(EFFECT_SELECTED);
    RaiseSelectedEffectChanged(ef, false);

    int row = GetEffectRow(ef);
    if (row >= 0) {
        mSelectedRow = row;
        RaiseSelectedEffectChanged(mSelectedEffect, false);
        wxCommandEvent eventRowChanged(EVT_SELECTED_ROW_CHANGED);
        eventRowChanged.SetInt(mSelectedRow);
        eventRowChanged.SetString(ef->GetParentEffectLayer()->GetParentElement()->GetModelName());
        wxPostEvent(GetParent(), eventRowChanged);
    }
}

void EffectsGrid::ResizeMoveMultipleEffects(int position, bool offset)
{
    int time = mTimeline->GetAbsoluteTimeMSfromPosition(position);
    ResizeMoveMultipleEffectsMS(time, offset);
}

void EffectsGrid::ResizeMoveMultipleEffectsMS(int time, bool offset)
{
    int deltaTime = 0;
    int toLeft,toRight;
    GetRangeOfMovementForSelectedEffects(toLeft,toRight);
    if(mResizingMode==EFFECT_RESIZE_LEFT || mResizingMode==EFFECT_RESIZE_LEFT_EDGE) {
        deltaTime = time - mEffectLayer->GetEffect(mResizeEffectIndex)->GetStartTimeMS();
    }
    else if(mResizingMode==EFFECT_RESIZE_RIGHT || mResizingMode==EFFECT_RESIZE_RIGHT_EDGE)
    {
        deltaTime = time - mEffectLayer->GetEffect(mResizeEffectIndex)->GetEndTimeMS();
    }
    else if(mResizingMode == EFFECT_RESIZE_MOVE)
    {
        EFFECT_SCREEN_MODE mode;
        int x1,x2,x3,x4;
        Effect* eff = mEffectLayer->GetEffect(mResizeEffectIndex);
        mTimeline->GetPositionsFromTimeRange(eff->GetStartTimeMS(),
                                             eff->GetEndTimeMS(),mode,x1,x2,x3,x4);
        int midpoint = mTimeline->GetTimeMSfromPosition((x1+x2)/2) + mTimeline->GetStartTimeMS();
        deltaTime = time - midpoint;
        if (!MultipleEffectsSelected())
        {
            SetEffectStatusText(eff);
        }
    }
    deltaTime = mTimeline->RoundToMultipleOfPeriod(deltaTime, mSequenceElements->GetFrequency());
    if(deltaTime < 0.0)
    {
        deltaTime = std::max(deltaTime, -toLeft);
        if( deltaTime < 0.0 )
        {
             MoveAllSelectedEffects(deltaTime, offset);
        }
    }
    else
    {
        deltaTime = std::min(deltaTime, toRight);
        if (deltaTime > 0.0)
        {
            MoveAllSelectedEffects(deltaTime, offset);
        }
    }
    mCellRangeSelected = false;
    mRangeStartCol = mRangeEndCol = mRangeStartRow = mRangeEndRow = -1;
    SetRCToolTip();
}

void EffectsGrid::ResizeMoveMultipleEffectsByTime(int delta, bool force)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("EffectsGrid::ResizeMoveMultipleEffectsByTime.");

    int deltaTime = mTimeline->RoundToMultipleOfPeriod(std::abs(delta), mSequenceElements->GetFrequency());
    if( delta < 0 )
    {
        deltaTime *= -1;
    }
    int toLeft,toRight;
    if( force )
    {
        toLeft = -delta;
        toRight = delta;
    }
    else
    {
        GetRangeOfMovementForSelectedEffects(toLeft,toRight);
    }
    if(deltaTime < 0.0)
    {
        deltaTime = std::max(deltaTime, -toLeft);
        if( deltaTime < 0.0 )
        {
             MoveAllSelectedEffects(deltaTime, false);
        }
    }
    else
    {
        deltaTime = std::min(deltaTime, toRight);
        if (deltaTime > 0.0)
        {
            MoveAllSelectedEffects(deltaTime, false);
        }
    }
    mCellRangeSelected = false;
    mRangeStartCol = mRangeEndCol = mRangeStartRow = mRangeEndRow = -1;
    SetRCToolTip();
}

void EffectsGrid::ButtUpResizeMoveMultipleEffects(bool right)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("EffectsGrid::ButtUpResizeMoveMultipleEffects.");

    ((MainSequencer*)mParent)->TagAllSelectedEffects();

    for (int row = 0; row<mSequenceElements->GetRowInformationSize(); row++)
    {
        EffectLayer* el = mSequenceElements->GetEffectLayer(row);
        el->ButtUpMoveAllSelectedEffects(right, mTimeline->GetTimeLength(), mSequenceElements->get_undo_mgr());
    }

    mCellRangeSelected = false;
    mRangeStartCol = mRangeEndCol = mRangeStartRow = mRangeEndRow = -1;
    SetRCToolTip();
}

void EffectsGrid::StretchMultipleEffectsByTime(int delta)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("EffectsGrid::StretchMultipleEffectsByTime.");

    int deltaTime = mTimeline->RoundToMultipleOfPeriod(std::abs(delta), mSequenceElements->GetFrequency());
    if (delta < 0)
    {
        deltaTime *= -1;
    }
    int toLeft, toRight;
    GetRangeOfMovementForSelectedEffects(toLeft, toRight);
    if (deltaTime < 0.0)
    {
        deltaTime = std::max(deltaTime, -toLeft);
        if (deltaTime < 0.0)
        {
            StretchAllSelectedEffects(deltaTime, false);
        }
    }
    else
    {
        deltaTime = std::min(deltaTime, toRight);
        if (deltaTime > 0.0)
        {
            StretchAllSelectedEffects(deltaTime, false);
        }
    }
    mCellRangeSelected = false;
    mRangeStartCol = mRangeEndCol = mRangeStartRow = mRangeEndRow = -1;
    SetRCToolTip();
}

void EffectsGrid::ButtUpStretchMultipleEffects(bool right)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("EffectsGrid::ButtUpStretchMultipleEffects.");

    ((MainSequencer*)mParent)->TagAllSelectedEffects();

    for (int row = 0; row<mSequenceElements->GetRowInformationSize(); row++)
    {
        EffectLayer* el = mSequenceElements->GetEffectLayer(row);
        el->ButtUpStretchAllSelectedEffects(right, mTimeline->GetTimeLength(), mSequenceElements->get_undo_mgr());
    }
    mCellRangeSelected = false;
    mRangeStartCol = mRangeEndCol = mRangeStartRow = mRangeEndRow = -1;
    SetRCToolTip();
}

void EffectsGrid::ResizeSingleEffect(int position)
{
    int time = mTimeline->GetAbsoluteTimeMSfromPosition(position);
    ResizeSingleEffectMS(time);
}

void EffectsGrid::ResizeSingleEffectMS(int timems)
{
    if (mEffectLayer->GetEffect(mResizeEffectIndex)->IsLocked() || mEffectLayer->IsFixedTimingLayer()) return;

    int time = mTimeline->RoundToMultipleOfPeriod(timems, mSequenceElements->GetFrequency());
    if(mResizingMode==EFFECT_RESIZE_LEFT || mResizingMode==EFFECT_RESIZE_LEFT_EDGE)
    {
        int minimumTime = mEffectLayer->GetMinimumStartTimeMS(mResizeEffectIndex, mResizingMode==EFFECT_RESIZE_LEFT, mSequenceElements->GetMinPeriod());
        // User has dragged left side to the right side exit
        if (time >= mEffectLayer->GetEffect(mResizeEffectIndex)->GetEndTimeMS())
        {
            return;
        }
        else if (time >= minimumTime  || minimumTime == NO_MIN_MAX_TIME)
        {
            if (time < 0) {
                //cannot have a starting time less than 0 or we cannot preview the effect or update the effect
                time = 0;
            }
            if(mEffectLayer->IsStartTimeLinked(mResizeEffectIndex) && mResizingMode==EFFECT_RESIZE_LEFT)
            {
                Effect* eff = mEffectLayer->GetEffect(mResizeEffectIndex-1);
                if( mSequenceElements->get_undo_mgr().GetCaptureUndo() ) {
                    mSequenceElements->get_undo_mgr().CaptureEffectToBeMoved( mEffectLayer->GetParentElement()->GetModelName(), mEffectLayer->GetIndex(), eff->GetID(),
                                                                              eff->GetStartTimeMS(), eff->GetEndTimeMS() );
                }
                eff->SetEndTimeMS(time);
            }
            Effect* eff = mEffectLayer->GetEffect(mResizeEffectIndex);
            if( mSequenceElements->get_undo_mgr().GetCaptureUndo() ) {
                mSequenceElements->get_undo_mgr().CaptureEffectToBeMoved( mEffectLayer->GetParentElement()->GetModelName(), mEffectLayer->GetIndex(), eff->GetID(),
                                                                          eff->GetStartTimeMS(), eff->GetEndTimeMS() );
            }
            eff->SetStartTimeMS(time);
        }
        else
        {
            if(mResizeEffectIndex!=0)
            {
                Effect* eff = mEffectLayer->GetEffect(mResizeEffectIndex);
                if( mSequenceElements->get_undo_mgr().GetCaptureUndo() ) {
                    mSequenceElements->get_undo_mgr().CaptureEffectToBeMoved( mEffectLayer->GetParentElement()->GetModelName(), mEffectLayer->GetIndex(), eff->GetID(),
                                                                              eff->GetStartTimeMS(), eff->GetEndTimeMS() );
                }
                eff->SetStartTimeMS(mEffectLayer->GetEffect(mResizeEffectIndex-1)->GetEndTimeMS());
            }
        }
    }
    else if(mResizingMode==EFFECT_RESIZE_RIGHT || mResizingMode==EFFECT_RESIZE_RIGHT_EDGE)
    {
        int maximumTime = mEffectLayer->GetMaximumEndTimeMS(mResizeEffectIndex, mResizingMode==EFFECT_RESIZE_RIGHT, mSequenceElements->GetMinPeriod());
        // User has dragged right side to the left side exit
        if (time <= mEffectLayer->GetEffect(mResizeEffectIndex)->GetStartTimeMS())
        {
            return;
        }
        else if (time <= maximumTime  || maximumTime == NO_MIN_MAX_TIME)
        {
            if(mEffectLayer->IsEndTimeLinked(mResizeEffectIndex) && mResizingMode==EFFECT_RESIZE_RIGHT)
            {
                Effect* eff = mEffectLayer->GetEffect(mResizeEffectIndex+1);
                if( mSequenceElements->get_undo_mgr().GetCaptureUndo() ) {
                    mSequenceElements->get_undo_mgr().CaptureEffectToBeMoved( mEffectLayer->GetParentElement()->GetModelName(), mEffectLayer->GetIndex(), eff->GetID(),
                                                                              eff->GetStartTimeMS(), eff->GetEndTimeMS() );
                }
                eff->SetStartTimeMS(time);
            }
            Effect* eff = mEffectLayer->GetEffect(mResizeEffectIndex);
            if( mSequenceElements->get_undo_mgr().GetCaptureUndo() ) {
                mSequenceElements->get_undo_mgr().CaptureEffectToBeMoved( mEffectLayer->GetParentElement()->GetModelName(), mEffectLayer->GetIndex(), eff->GetID(),
                                                                          eff->GetStartTimeMS(), eff->GetEndTimeMS() );
            }
            eff->SetEndTimeMS(time);
        }
        else
        {
            if(mResizeEffectIndex < mEffectLayer->GetEffectCount()-1)
            {
                Effect* eff = mEffectLayer->GetEffect(mResizeEffectIndex);
                if( mSequenceElements->get_undo_mgr().GetCaptureUndo() ) {
                    mSequenceElements->get_undo_mgr().CaptureEffectToBeMoved( mEffectLayer->GetParentElement()->GetModelName(), mEffectLayer->GetIndex(), eff->GetID(),
                                                                              eff->GetStartTimeMS(), eff->GetEndTimeMS() );
                }
                eff->SetEndTimeMS(mEffectLayer->GetEffect(mResizeEffectIndex+1)->GetStartTimeMS());
            }
        }
    }

    Effect* eff = mEffectLayer->GetEffect(mResizeEffectIndex);
    if (eff != nullptr)
    {
        SetEffectStatusText(eff);
    }

    Refresh(false);
    // Move time line and waveform to new position
    UpdateZoomPosition(time);
}

void EffectsGrid::RunMouseOverHitTests(int rowIndex,int x,int y)
{
    int effectIndex;

    int time = mTimeline->GetRawTimeMSfromPosition(x);
    HitLocation selectionType = HitLocation::NONE;
    Effect * eff = GetEffectAtRowAndTime(rowIndex, time, effectIndex, selectionType);
    if (eff != nullptr) {
        mResizeEffectIndex = effectIndex;
        switch (selectionType) {
            case HitLocation::NONE:
                SetCursor(wxCURSOR_DEFAULT);
                mResizingMode = EFFECT_RESIZE_NO;
                break;
            case HitLocation::LEFT_EDGE_DISCONNECT:
                SetCursor(wxCURSOR_POINT_LEFT);
                mResizingMode = EFFECT_RESIZE_LEFT_EDGE;
                break;
            case HitLocation::LEFT_EDGE:
                SetCursor(wxCURSOR_SIZEWE);
                mResizingMode = EFFECT_RESIZE_LEFT;
                break;
            case HitLocation::RIGHT_EDGE_DISCONNECT:
                SetCursor(wxCURSOR_POINT_RIGHT);
                mResizingMode = EFFECT_RESIZE_RIGHT_EDGE;
                break;
            case HitLocation::RIGHT_EDGE:
                SetCursor(wxCURSOR_SIZEWE);
                mResizingMode = EFFECT_RESIZE_RIGHT;
                break;
            case HitLocation::LEFT:
            case HitLocation::RIGHT:
                SetCursor(wxCURSOR_DEFAULT);
                mResizingMode = EFFECT_RESIZE_NO;
                break;
            case HitLocation::CENTER:
                SetCursor(wxCURSOR_HAND);
                mResizingMode = EFFECT_RESIZE_MOVE;
                break;
            // update effect details
        }
        SetEffectStatusText(eff);
    } else {
        if (xlights->GetFilename() != "")
        {
            xlights->SetStatusText(xlights->GetFilename(), true);
        }
        else
        {
            xlights->SetStatusText(xlights->CurrentDir, true);
        }
        SetCursor(wxCURSOR_DEFAULT);
        mResizingMode = EFFECT_RESIZE_NO;
    }
}

void EffectsGrid::SetEffectStatusText(Effect* eff) const
{
    if (eff != nullptr)
    {
        wxString e = wxString::Format("start: %s end: %s duration: %s %s %s", FORMATTIME(eff->GetStartTimeMS()), FORMATTIME(eff->GetEndTimeMS()), FORMATTIME(eff->GetEndTimeMS() - eff->GetStartTimeMS()), eff->GetEffectName(), eff->GetDescription());
        xlights->SetStatusText(e, true);
    }
    else
    {
        if (xlights->GetFilename() != "")
        {
            xlights->SetStatusText(xlights->GetFilename(), true);
        }
        else
        {
            xlights->SetStatusText(xlights->CurrentDir, true);
        }
    }
}

void EffectsGrid::InsertEffectLayerAbove()
{
    int row = mSelectedRow;
    if (row == -1 && mRangeStartRow == mRangeEndRow) row = mRangeStartRow; // +mSequenceElements->GetNumberOfTimingRows();
    if (row == -1) return;

    EffectLayer* el = mSequenceElements->GetEffectLayer(row);
    if (el == nullptr) return;
    Element* element = el->GetParentElement();
    if (element == nullptr) return;
    element->InsertEffectLayer(el->GetLayerNumber()-1);
    wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
    wxPostEvent(this, eventRowHeaderChanged);
}

void EffectsGrid::InsertEffectLayerBelow()
{
    int row = mSelectedRow;
    if (row == -1 && mRangeStartRow == mRangeEndRow) row = mRangeStartRow; // +mSequenceElements->GetNumberOfTimingRows();
    if (row == -1) return;

    EffectLayer* el = mSequenceElements->GetEffectLayer(row);
    if (el == nullptr) return;

    Element* element = el->GetParentElement();
    if (element == nullptr) return;

    if (el->GetLayerNumber() - 1 < element->GetEffectLayerCount() - 1)
    {
        element->InsertEffectLayer(el->GetLayerNumber());
    }
    else
    {
        element->AddEffectLayer();
    }

    wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
    wxPostEvent(this, eventRowHeaderChanged);
}

void EffectsGrid::ToggleExpandElement(RowHeading* rowHeading)
{
    int row = mSelectedRow;
    if (row == -1 && mRangeStartRow == mRangeEndRow) row = mRangeStartRow; // +mSequenceElements->GetNumberOfTimingRows();
    if (row == -1) return;

    EffectLayer* el = mSequenceElements->GetEffectLayer(row);
    if (el == nullptr) return;

    Element* element = el->GetParentElement();
    if (element == nullptr) return;

    rowHeading->ToggleExpand(element);
}

void EffectsGrid::UpdateTimePosition(int time) const
{
    // Update time selection
    wxCommandEvent eventTimeSelected(EVT_TIME_SELECTED);
    eventTimeSelected.SetInt(time);
    wxPostEvent(mParent, eventTimeSelected);
}

void EffectsGrid::UpdateMousePosition(int time) const
{
    // Update time selection
    wxCommandEvent eventMousePos(EVT_MOUSE_POSITION);
    eventMousePos.SetInt(time);
    wxPostEvent(mParent, eventMousePos);
}

void EffectsGrid::UpdateZoomPosition(int time) const
{
    mTimeline->SetZoomMarkerMS(time);
}

void EffectsGrid::EstablishSelectionRectangle()
{
    mSequenceElements->UnSelectAllEffects();
    int first_row = mSequenceElements->GetFirstVisibleModelRow();
    int row1 = GetRow(mDragStartY);
    if (row1 >= mSequenceElements->GetNumberOfTimingRows())
    {
        row1 += first_row;
    }
    int row2 = GetRow(mDragEndY);
    if (row2 >= mSequenceElements->GetNumberOfTimingRows())
    {
        row2 += first_row;
    }
    if( row1 > row2 ) {
        std::swap(row1, row2);
    }
    mRangeStartRow = row1;
    mRangeCursorRow = row2;
    mRangeEndRow = row2;

    int start_x = mDragStartX;
    int end_x = mDragEndX;
    if( start_x > end_x ) {
        std::swap(start_x, end_x);
    }

    int startTime = mTimeline->GetAbsoluteTimeMSfromPosition(start_x);
    int endTime = mTimeline->GetAbsoluteTimeMSfromPosition(end_x);

    if (mSequenceElements->GetSelectedTimingRow() >= 0)
    {
        EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());

        if (tel != nullptr)
        {
            if( tel->GetEffectCount() > 0 )
            {
                int timingIndex1 = 0;
                int timingIndex2 = 0;
                if (tel->HitTestEffectByTime(startTime, timingIndex1) &&
                    tel->HitTestEffectByTime(endTime, timingIndex2))
                {
                    mCellRangeSelected = true;
                    mRangeStartCol = timingIndex1;
                    mRangeCursorCol = timingIndex2;
                    mRangeEndCol = timingIndex2;
                    mDropStartTimeMS = tel->GetEffect(mRangeStartCol)->GetStartTimeMS();  // set for paste
                }
            }
        }
    }

    if (mRangeStartCol >= 0 && mRangeStartRow >= 0)
    {
        UpdateSelectedEffects();
    }
    else
    {
        mSequenceElements->SelectEffectsInRowAndTimeRange(row1-first_row,row2-first_row,startTime,endTime);
    }
    SetRCToolTip();
}

void EffectsGrid::UpdateSelectionRectangle()
{
    // Unselect all effects and clear mSelectedEffect
    UnselectEffect();
    mSequenceElements->UnSelectAllEffects();

    if (GetRow(mDragEndY) < mSequenceElements->GetNumberOfTimingRows())
    {
        mRangeEndRow = GetRow(mDragEndY);
    }
    else
    {
        mRangeEndRow = GetRow(mDragEndY) + mSequenceElements->GetFirstVisibleModelRow();
    }

    if( mRangeEndRow >= mSequenceElements->GetRowInformationSize() )
    {
       mRangeEndRow = mSequenceElements->GetRowInformationSize() - 1;
    }

    if( mSequenceElements->GetNumberOfActiveTimingEffects() > 0 )
    {
        EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
        if( tel != nullptr )
        {
          int time = mTimeline->GetAbsoluteTimeMSfromPosition(mDragEndX);
          int timingIndex = 0;
          if(tel->HitTestEffectByTime(time,timingIndex))
          {
              mRangeEndCol = timingIndex;
          }
          UpdateSelectedEffects();
        }
    }
    else
    {
        int row1 = mRangeStartRow;
        int row2 = mRangeEndRow;
        if( row1 > row2 ) {
            std::swap(row1, row2);
        }

        int start_x = mDragStartX;
        int end_x = mDragEndX;
        if( start_x > end_x ) {
            std::swap(start_x, end_x);
        }

        int startTime = mTimeline->GetAbsoluteTimeMSfromPosition(start_x);
        int endTime = mTimeline->GetAbsoluteTimeMSfromPosition(end_x);
        if (mSequenceElements->SelectEffectsInRowAndTimeRange(row1, row2, startTime, endTime)) {
            SetFirstEffectSelected();
        }
    }
    SetRCToolTip();
}

void EffectsGrid::SetRCToolTip()
{
    int x = std::abs(mRangeEndCol - mRangeStartCol) + 1;
    int y = std::abs(mRangeEndRow - mRangeStartRow) + 1;

    if (!mCellRangeSelected ||
        mPartialCellSelected ||
        mRangeStartCol < 0 ||
        (x == 1 && y == 1) ||
        mSequenceElements->GetSelectedTimingRow() < 0)
    {
        UnsetToolTip();
    }
    else
    {
        SetToolTip(wxString::Format("%dC x %dR", x, y));
    }
}

void EffectsGrid::UpdateSelectedEffects()
{
    mSelectedEffect = nullptr;
    mSequenceElements->UnSelectAllEffects();
    if( mRangeStartCol >= 0 && mRangeStartRow >= 0 )
    {
        int start_row = mRangeStartRow;
        int end_row = mRangeEndRow;
        int start_col = mRangeStartCol;
        int end_col = mRangeEndCol;
        if( start_row > end_row ) {
            std::swap( start_row, end_row );
        }
        if( start_col > end_col ) {
            std::swap( start_col, end_col );
        }
        int adjusted_start_row = std::max(start_row, mSequenceElements->GetNumberOfTimingRows());
        if( end_row >= adjusted_start_row )
        {
            int num_selected = mSequenceElements->SelectEffectsInRowAndColumnRange(adjusted_start_row, end_row, start_col, end_col);
            if( num_selected != 1 )  // we don't know what to preview unless only 1 effect is selected
            {
                UnselectEffect(true);
            }
            else
            {
                SetFirstEffectSelected();
            }
        }
    }
}

int EffectsGrid::GetEffectRow(Effect* ef)
{
    for (int row = mSequenceElements->GetNumberOfTimingRows(); row < mSequenceElements->GetRowInformationSize(); row++)
    {
        EffectLayer* el = mSequenceElements->GetEffectLayer(row);
        for (int i = 0; i < el->GetEffectCount(); ++i)
        {
            if (el->GetEffect(i) == ef)
            {
                return row;
            }
        }
    }
    return -1;
}

void EffectsGrid::SetFirstEffectSelected()
{
    // set the selected effect
    bool found = false;
    int first_model_row = mSequenceElements->GetNumberOfTimingRows();
    for (int row = first_model_row; row < mSequenceElements->GetRowInformationSize() && !found; row++)
    {
        EffectLayer* el = mSequenceElements->GetEffectLayer(row);
        Element* element = el->GetParentElement();
        if (mSequenceElements->GetEffectLayer(row)->GetSelectedEffectCount() > 0)
        {
            int num_effects = mSequenceElements->GetEffectLayer(row)->GetEffectCount();
            for (int i = 0; i < num_effects && !found; ++i)
            {
                Effect* eff = el->GetEffect(i);
                if (eff->GetSelected())
                {
                    mSelectedRow = row;
                    mSelectedEffect = eff;
                    RaiseSelectedEffectChanged(mSelectedEffect, false);
                    RaisePlayModelEffect(element, mSelectedEffect, false);
                    wxCommandEvent eventRowChanged(EVT_SELECTED_ROW_CHANGED);
                    eventRowChanged.SetInt(mSelectedRow);
                    eventRowChanged.SetString(element->GetModelName());
                    wxPostEvent(GetParent(), eventRowChanged);
                    found = true;
                }
            }
        }
    }
}

void EffectsGrid::ForceRefresh()
{
    if( xlights->GetPlayStatus() != PLAY_TYPE_EFFECT && xlights->GetPlayStatus() != PLAY_TYPE_MODEL ) {
        if( mDragging ) {
            UpdateSelectionRectangle();
        }
    }
    Draw();
}

void EffectsGrid::SetTimeline(TimeLine* timeline)
{
    mTimeline = timeline;
}

void EffectsGrid::SetSequenceElements(SequenceElements* elements)
{
    mSequenceElements = elements;
}

void EffectsGrid::SetStartPixelOffset(int offset)
{
    mStartPixelOffset = offset;
}

void EffectsGrid::InitializeGLCanvas()
{
    if(xlights == nullptr) return;
    if(!IsShownOnScreen()) return;
    SetCurrentGLContext();
    CreateEffectIconTextures();
    mIsInitialized = true;
}
void EffectsGrid::InitializeGLContext()
{
    SetCurrentGLContext();
    LOG_GL_ERRORV(glClearColor(0.0f, 0.0f, 0.0f, 1.0f)); // Black Background
    LOG_GL_ERRORV(glClear(GL_COLOR_BUFFER_BIT));
    prepare2DViewport(0,0,mWindowWidth, mWindowHeight);
}


void EffectsGrid::DrawLines() const
{
    // Draw Horizontal lines
    int x1 = 1;
    int x2 = mWindowWidth - 1;
    int y;
    bool isEvenLayer=false;

    DrawGLUtils::xlVertexAccumulator va;
    va.PreAlloc(mSequenceElements->GetVisibleRowInformationSize() * 6);

    xlColor color(33, 33, 33);
    for(size_t row=0; row < mSequenceElements->GetVisibleRowInformationSize(); row++)
    {
        Row_Information_Struct* ri = mSequenceElements->GetVisibleRowInformation(row);
        Element* e = ri->element;
        y = row*DEFAULT_ROW_HEADING_HEIGHT;

        if(ri->layerIndex == 0 && ri->strandIndex == -1)
        {
            if (isEvenLayer)
            {
                //Element is collapsed only one row should be shaded
                int h = e->GetCollapsed()?DEFAULT_ROW_HEADING_HEIGHT:DEFAULT_ROW_HEADING_HEIGHT * e->GetEffectLayerCount();
                va.AddRect(x1, y, x2, y + h);
            }
            isEvenLayer = !isEvenLayer;
        } else if (ri->strandIndex != -1) {
            if (isEvenLayer)
            {
                va.AddRect(x1, y, x2, y + DEFAULT_ROW_HEADING_HEIGHT);
            }
            isEvenLayer = !isEvenLayer;
        }
    }
    DrawGLUtils::Draw(va, color, GL_TRIANGLES);
    va.Reset();

    DrawGLUtils::SetLineWidth(0.2f);
    for(size_t row=0;row < mSequenceElements->GetVisibleRowInformationSize();row++)
    {
        y = (row+1)*DEFAULT_ROW_HEADING_HEIGHT;
        va.AddVertex(x1, y);
        va.AddVertex(x2, y);
    }

    // Draw vertical lines
    int y1 = 0;
    int y2 = mWindowHeight-1;
    for(size_t x3=0;x3<mWindowWidth;x3++)
    {
        // Draw hash marks
        if ((x3+mStartPixelOffset)%(PIXELS_PER_MAJOR_HASH)==0)
        {
            va.AddVertex(x3, y1);
            va.AddVertex(x3, y2);
        }
    }

    DrawGLUtils::Draw(va, xlights->color_mgr.GetColor(ColorManager::COLOR_GRIDLINES), GL_LINES);
    DrawGLUtils::SetLineWidth(1.0f);
}


void EffectsGrid::DrawPlayMarker() const
{
    xlColor play_line = xlRED;
    int play_pos = mTimeline->GetPlayMarker();
    if( play_pos > 0 )
    {
        DrawGLUtils::DrawLine(play_line,255,play_pos,0,play_pos,GetSize().y,0.2f);
    }
}

int EffectsGrid::DrawEffectBackground(const Row_Information_Struct* ri, const Effect *e,
                                      int x1, int y1, int x2, int y2,
                                      DrawGLUtils::xlAccumulator &backgrounds) const
{
    if (e->GetPaletteSize() == 0) {
        //if there are no colors selected, none of the "backgrounds" make sense.  Don't draw
        //the background and instead make sure the icon is displayed to the user knows they
        //need to make some decisions about the colors to be used.
        return 1;
    }
    RenderableEffect *ef = xlights->GetEffectManager()[e->GetEffectIndex()];

    xlColor colorMask = xlColor::NilColor();
    Model* m = xlights->GetModel(ri->element->GetModelName());
    if (m != nullptr)
    {
        if (m->GetDisplayAs() == "Channel Block")
        {
            StrandElement* se = dynamic_cast<StrandElement*>(ri->element);
            if (se != nullptr)
            {
                colorMask = m->GetNodeMaskColor(se->GetStrand());
            }
            else
            {
                colorMask = xlWHITE;
            }
        }
        else
        {
            if (wxString(m->GetStringType()).StartsWith("Single Color") || m->GetStringType() == "Node Single Color")
            {
                colorMask = m->GetNodeMaskColor(0);
            }
        }
    }
    int result = ef == nullptr ? 1 : ef->DrawEffectBackground(e, x1, y1, x2, y2, backgrounds, (colorMask.IsNilColor() ? nullptr : &colorMask), xlights->IsDrawRamps());

    return result;
}

float ComputeFontSize(int &toffset, const float factor) {
    double fontSize = DEFAULT_ROW_HEADING_HEIGHT - 10;
    toffset = 0;
    if (fontSize < 10) {
        if (factor > 1.5) {
            fontSize = 9;
            toffset = 1;
        } else {
            fontSize = 10;
            toffset = 2;
        }
    }
    return fontSize;
}

void EffectsGrid::DrawEffects()
{
    int width = getWidth();
    for (int row=0; row < mSequenceElements->GetVisibleRowInformationSize(); row++)
    {
        Row_Information_Struct* ri = mSequenceElements->GetVisibleRowInformation(row);
        if(ri->element->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
            DrawTimingEffects(row);
        } else {
            wxString name = ri->element->GetName();
            EffectLayer* effectLayer = mSequenceElements->GetEffectLayer(ri);
            if (effectLayer == nullptr) {
                continue;
            }
            lines.PreAlloc(effectLayer->GetEffectCount() * 16);
            selectedLinesFixed.PreAlloc(effectLayer->GetEffectCount() * 16);
            selectedLinesLocked.PreAlloc(effectLayer->GetEffectCount() * 16);
            selectedLines.PreAlloc(effectLayer->GetEffectCount() * 16);
            selectFocusLines.PreAlloc(16);
            selectFocusLinesLocked.PreAlloc(16);

            DrawGLUtils::xlVertexAccumulator * linesRight;
            DrawGLUtils::xlVertexAccumulator * linesLeft;
            DrawGLUtils::xlVertexAccumulator * linesCenter;
            int y1 = (row*DEFAULT_ROW_HEADING_HEIGHT)+2;
            int y2 = ((row+1)*DEFAULT_ROW_HEADING_HEIGHT)-2;
            int y = (row*DEFAULT_ROW_HEADING_HEIGHT) + (DEFAULT_ROW_HEADING_HEIGHT/2);

            if (mGridNodeValues && ri->nodeIndex != -1) {
                std::vector<xlColor> colors;
                std::vector<double> xs;
                PixelBufferClass ncls(xlights);
                StrandElement *se = dynamic_cast<StrandElement*>(ri->element);
                Model* m = xlights->GetModel(ri->element->GetModelName());
                ncls.InitNodeBuffer(*m, se->GetStrand(), ri->nodeIndex, seqData->FrameTime());
                xlColor maskColor = m->GetNodeMaskColor(se->GetStrand());
                xlColor lastColor;
                for (size_t f = 0; f < seqData->NumFrames(); f++) {
                    ncls.SetNodeChannelValues(0, (*seqData)[f][ncls.NodeStartChannel(0)]);
                    xlColor c = ncls.GetNodeColor(0);
                    c.ApplyMask(&maskColor);
                    if (f == 0) {
                        colors.push_back(c);
                        lastColor = c;
                    } else if (c != lastColor) {
                        colors.push_back(c);
                        lastColor = c;
                        int timems = f * seqData->FrameTime();
                        xs.push_back(mTimeline->GetPositionFromTimeMS(timems));
                    }
                }
                int timems = seqData->NumFrames() * seqData->FrameTime();
                xs.push_back(mTimeline->GetPositionFromTimeMS(timems));

                backgrounds.PreAlloc(xs.size() * 6);
                float y1a = (row*DEFAULT_ROW_HEADING_HEIGHT)+3;
                float y2a = ((row+1)*DEFAULT_ROW_HEADING_HEIGHT)-3;
                float x =  mTimeline->GetPositionFromTimeMS(0);
                for (size_t n = 0; n < xs.size(); n++) {
                    int x2 = xs[n];
                    if (x2 >= 0) {
                        if (colors.size() < n)
                        {
                            // that wont work
                        }
                        else
                        {
                            backgrounds.AddRect(x, y1a, x2, y2a, colors[n]);
                        }
                    }
                    x = x2;
                    if (x > width) {
                        break;
                    }
                }
            }

            for(int effectIndex=0;effectIndex < effectLayer->GetEffectCount();effectIndex++)
            {
                Effect* e = effectLayer->GetEffect(effectIndex);
                EFFECT_SCREEN_MODE mode;

                int x1,x2,x3,x4;
                mTimeline->GetPositionsFromTimeRange(effectLayer->GetEffect(effectIndex)->GetStartTimeMS(),
                                                     effectLayer->GetEffect(effectIndex)->GetEndTimeMS(),mode,x1,x2,x3,x4);
                int x = x2-x1;

                if (x2 < 0) {
                    continue;
                }
                if (x1 > width) {
                    break;
                }
                // Draw Left line
                if( e == mSelectedEffect )
                {
                    if (effectLayer->GetEffect(effectIndex)->IsLocked())
                    {
                        linesLeft = effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_NOT_SELECTED ||
                            effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_RT_SELECTED ? &lines : &selectFocusLinesLocked;
                        linesRight = effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_NOT_SELECTED ||
                            effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_LT_SELECTED ? &lines : &selectFocusLinesLocked;
                        linesCenter = effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_SELECTED ? &selectFocusLinesLocked : &lines;
                    }
                    else
                    {
                        linesLeft = effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_NOT_SELECTED ||
                            effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_RT_SELECTED ? &lines : &selectFocusLines;
                        linesRight = effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_NOT_SELECTED ||
                            effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_LT_SELECTED ? &lines : &selectFocusLines;
                        linesCenter = effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_SELECTED ? &selectFocusLines : &lines;
                    }
                }
                else
                {
                    if (effectLayer->GetEffect(effectIndex)->IsLocked())
                    {
                        linesLeft = effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_NOT_SELECTED ||
                            effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_RT_SELECTED ? &lines : &selectedLinesLocked;
                        linesRight = effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_NOT_SELECTED ||
                            effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_LT_SELECTED ? &lines : &selectedLinesLocked;
                        linesCenter = effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_SELECTED ? &selectedLinesLocked : &lines;
                    }
                    else
                    {
                        linesLeft = effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_NOT_SELECTED ||
                            effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_RT_SELECTED ? &lines : &selectedLines;
                        linesRight = effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_NOT_SELECTED ||
                            effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_LT_SELECTED ? &lines : &selectedLines;
                        linesCenter = effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_SELECTED ? &selectedLines : &lines;
                    }
                }

                int drawIcon = 1;
                if(mGridIconBackgrounds && (ri->nodeIndex == -1 || !mGridNodeValues))
                {
                    drawIcon = DrawEffectBackground(ri, e, x3, y1, x4, y2, backgrounds);
                }
                if (mGridNodeValues && ri->nodeIndex != -1) {
                    drawIcon = 2;
                }

                if( drawIcon == 2 && xlights->IsACActive() ) {
                    drawIcon = 0;
                }

                if (mode != SCREEN_L_R_OFF)
                {
                    if(mode==SCREEN_L_R_ON || mode == SCREEN_L_ON)
                    {
                        if(effectIndex>0)
                        {
                            // Draw left line if effect has different start time then previous effect or
                            // previous effect was not selected, or only left was selected
                            if(effectLayer->GetEffect(effectIndex)->GetStartTimeMS() != effectLayer->GetEffect(effectIndex-1)->GetEndTimeMS() ||
                               effectLayer->GetEffect(effectIndex-1)->GetSelected() == EFFECT_NOT_SELECTED ||
                                effectLayer->GetEffect(effectIndex-1)->GetSelected() == EFFECT_LT_SELECTED)
                            {
                                linesLeft->AddVertex(x1, y1);
                                linesLeft->AddVertex(x1, y2);
                            }
                        }
                        else
                        {
                            linesLeft->AddVertex(x1, y1);
                            linesLeft->AddVertex(x1, y2);
                        }
                    }

                    // Draw Right line
                    if(mode==SCREEN_L_R_ON || mode == SCREEN_R_ON)
                    {
                        linesRight->AddVertex(x2, y1);
                        linesRight->AddVertex(x2, y2);
                    }

                    // Draw horizontal
                    if(mode!=SCREEN_L_R_OFF)
                    {
                        if (drawIcon) {
                            if(x > (DEFAULT_ROW_HEADING_HEIGHT + 4)) {
                                double sz = (DEFAULT_ROW_HEADING_HEIGHT - 6.0) / (2.0 * drawIcon) + 1.0;

                                double xl = (x1+x2)/2.0-sz;
                                double xr = (x1+x2)/2.0+sz;

                                textures[m_EffectTextures[e->GetEffectIndex()]].AddFullTexture(xl, y-sz, xr, y+sz);

                                linesLeft->AddVertex(x1, y);
                                linesLeft->AddVertex((x1+x2)/2.0-sz, y);

                                linesRight->AddVertex((x1+x2)/2.0+sz,y);
                                linesRight->AddVertex(x2,y);

                                lines.AddLinesRect(xl - 0.4, y - sz, xr + 0.4, y + sz);
                            }
                            else if (x > MINIMUM_EFFECT_WIDTH_FOR_SMALL_RECT)
                            {
                                linesLeft->AddVertex(x1, y);
                                linesLeft->AddVertex(x1+(x/2)-1,y);

                                linesRight->AddVertex(x1+(x/2)+1,y);
                                linesRight->AddVertex(x2,y);
                                float sz = 1;
                                float xl = x1+(x/2)-1;
                                float xr = x1+(x/2)+1;

                                textures[m_EffectTextures[e->GetEffectIndex()]].AddFullTexture(xl, y-sz, xr, y+sz);

                                lines.AddLinesRect(xl - 0.4, y - sz, xr + 0.4, y + sz);
                            }
                            else
                            {
                                linesCenter->AddVertex(x1,y);
                                linesCenter->AddVertex(x2,y);
                            }
                        }
                    }
                }
                DrawFadeHints(e, x3, y1, x4, y2, backgrounds);
            }
            if((mDragDropping || mPartialCellSelected) && mDropRow == row)
            {
                int y3 = row*DEFAULT_ROW_HEADING_HEIGHT;
                xlColor highlight_color;
                int selected_timing_row = mSequenceElements->GetSelectedTimingRow();
                if( selected_timing_row >= 0 )
                {
                    highlight_color = xlights->color_mgr.GetTimingColor(mSequenceElements->GetVisibleRowInformation(selected_timing_row)->colorIndex);
                }
                else
                {
                    highlight_color = xlights->color_mgr.GetTimingColor(0);
                }
                highlight_color.alpha = 128;
                selectedBoxes.AddRect(mDropStartX,y3,mDropStartX+mDropEndX-mDropStartX,y3+DEFAULT_ROW_HEADING_HEIGHT, highlight_color);
            }
        }
    }
    backgrounds.Finish(GL_TRIANGLES);
    DrawGLUtils::Draw(backgrounds);
    for (auto it = textures.begin(); it != textures.end(); ++it) {
        it->second.id = it->first;
        DrawGLUtils::Draw(it->second, GL_TRIANGLES);
        it->second.Reset();
    }
    DrawGLUtils::Draw(lines, xlights->color_mgr.GetColor(ColorManager::COLOR_EFFECT_DEFAULT), GL_LINES);
    DrawGLUtils::Draw(selectedLines, xlights->color_mgr.GetColor(ColorManager::COLOR_EFFECT_SELECTED), GL_LINES);
    DrawGLUtils::Draw(selectFocusLines, xlights->color_mgr.GetColor(ColorManager::COLOR_REFERENCE_EFFECT), GL_LINES);
    DrawGLUtils::Draw(selectedLinesLocked, xlights->color_mgr.GetColor(ColorManager::COLOR_EFFECT_SELECTED_LOCKED), GL_LINES);
    DrawGLUtils::Draw(selectedLinesFixed, xlights->color_mgr.GetColor(ColorManager::COLOR_EFFECT_SELECTED_FIXED), GL_LINES);
    DrawGLUtils::Draw(selectFocusLinesLocked, xlights->color_mgr.GetColor(ColorManager::COLOR_REFERENCE_EFFECT_LOCKED), GL_LINES);

    DrawGLUtils::SetLineWidth(2.0);
    DrawGLUtils::Draw(timingEffLines, xlights->color_mgr.GetColor(ColorManager::COLOR_TIMING_DEFAULT), GL_LINES);
    DrawGLUtils::Draw(textBackgrounds, GL_TRIANGLES);
    DrawGLUtils::SetLineWidth(1.0);
    DrawGLUtils::Draw(timingLines, GL_LINES, GL_BLEND);

    float factor = translateToBacking(1.0);
    int toffset;
    float fontSize = ComputeFontSize(toffset, factor);

    LOG_GL_ERRORV(glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA));
    DrawGLUtils::Draw(texts, fontSize, factor);
    DrawGLUtils::Draw(selectedBoxes, GL_TRIANGLES, GL_BLEND);

    textBackgrounds.Reset();
    timingLines.Reset();
    timingEffLines.Reset();
    selectedLinesFixed.Reset();
    selectedLinesLocked.Reset();
    texts.Reset();
    backgrounds.Reset();
    selectedBoxes.Reset();
    selectedLines.Reset();
    selectFocusLines.Reset();
    selectFocusLinesLocked.Reset();
    lines.Reset();
}

void EffectsGrid::DrawFadeHints(Effect* e, int x1, int y1, int x2, int y2, DrawGLUtils::xlAccumulator& backgrounds) const
{
    if (xlights->IsSuppressFadeHints()) return;
    const SettingsMap& sm(e->GetSettings());
    int inTransitionEnd = 0, outTransitionStart = 0;
    double fadeInTime = sm.GetDouble("T_TEXTCTRL_Fadein");
    if (fadeInTime != 0.)
    {
        double fadeInTimeMS = fadeInTime * 1000;
        int durationMS = e->GetEndTimeMS() - e->GetStartTimeMS();
        if (durationMS > 0)
        {
            double pct = std::min(fadeInTimeMS / durationMS, 1.);
            int width = int(pct * (x2 - x1));
            inTransitionEnd = x1 + width;
            backgrounds.AddRect(x1, y1, inTransitionEnd, y1 + 2, xlGREEN);
            backgrounds.AddRect(x1, y1 + 2, inTransitionEnd, y1 + 3, xlBLACK);
        }
    }
    double fadeOutTime = sm.GetDouble("T_TEXTCTRL_Fadeout");
    if (fadeOutTime != 0.)
    {
        double fadeOutTimeMS = fadeOutTime * 1000;
        int durationMS = e->GetEndTimeMS() - e->GetStartTimeMS();
        if (durationMS > 0)
        {
            double pct = std::min(fadeOutTimeMS / durationMS, 1.);
            int width = int(pct * (x2 - x1));
            outTransitionStart = x2 - width;
            backgrounds.AddRect(outTransitionStart, y1, x2, y1 + 2, xlRED);
            backgrounds.AddRect(outTransitionStart, y1 + 2, x2, y1 + 3, xlBLACK);
        }
    }

    if (fadeInTime != 0. && fadeOutTime != 0. && inTransitionEnd > outTransitionStart)
    {
        backgrounds.AddRect(outTransitionStart, y1, inTransitionEnd, y1 + 2, xlYELLOW);
        backgrounds.AddRect(outTransitionStart, y1 + 2, inTransitionEnd, y1 + 3, xlBLACK);
    }
}

void EffectsGrid::DrawTimingEffects(int row)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    Row_Information_Struct *ri = mSequenceElements->GetVisibleRowInformation(row);
    TimingElement* element = dynamic_cast<TimingElement*>(ri->element);
    EffectLayer* effectLayer = mSequenceElements->GetVisibleEffectLayer(row);

    if (effectLayer == nullptr) logger_base.debug("EffectsGrid::DrawTimingEffects null effectLayer.");

    bool fixed = element->IsFixedTiming();

    xlColor c(xlights->color_mgr.GetTimingColor(ri->colorIndex));
    c.alpha = 128;

    int toffset = 0;
    float factor = translateToBacking(1.0);
    float fontSize = ComputeFontSize(toffset, factor);

    for (int effectIndex = 0; effectIndex < effectLayer->GetEffectCount(); effectIndex++)
    {
        Effect *eff = effectLayer->GetEffect(effectIndex);

        EFFECT_SCREEN_MODE mode = SCREEN_L_R_OFF;

        int y1 = (row*DEFAULT_ROW_HEADING_HEIGHT) + 4;
        int y2 = ((row + 1)*DEFAULT_ROW_HEADING_HEIGHT) - 4;
        int y = (row*DEFAULT_ROW_HEADING_HEIGHT) + (DEFAULT_ROW_HEADING_HEIGHT / 2.0);
        int x1, x2, x3, x4;

        mTimeline->GetPositionsFromTimeRange(eff->GetStartTimeMS(),
            eff->GetEndTimeMS(), mode, x1, x2, x3, x4);

        DrawGLUtils::xlVertexAccumulator* linesLeft;
        DrawGLUtils::xlVertexAccumulator* linesRight;
        if (fixed)
        {
            linesLeft = &selectedLinesFixed;
            linesRight = &selectedLinesFixed;
            //DrawGLUtils::xlVertexAccumulator* linesCenter = &timingEffLines;
        }
        else if (eff->IsLocked())
        {
            linesLeft = eff->GetSelected() == EFFECT_NOT_SELECTED ||
                eff->GetSelected() == EFFECT_RT_SELECTED ? &timingEffLines : &selectedLinesLocked;
            linesRight = eff->GetSelected() == EFFECT_NOT_SELECTED ||
                eff->GetSelected() == EFFECT_LT_SELECTED ? &timingEffLines : &selectedLinesLocked;
            //DrawGLUtils::xlVertexAccumulator* linesCenter = eff->GetSelected() == EFFECT_SELECTED?&selectedLinesLocked:&timingEffLines;
        }
        else
        {
            linesLeft = eff->GetSelected() == EFFECT_NOT_SELECTED ||
                eff->GetSelected() == EFFECT_RT_SELECTED ? &timingEffLines : &selectedLines;
            linesRight = eff->GetSelected() == EFFECT_NOT_SELECTED ||
                eff->GetSelected() == EFFECT_LT_SELECTED ? &timingEffLines : &selectedLines;
            //DrawGLUtils::xlVertexAccumulator* linesCenter = eff->GetSelected() == EFFECT_SELECTED?&selectedLines:&timingEffLines;
        }

        if (mode != SCREEN_L_R_OFF) {
            // Draw Left line
            if (mode == SCREEN_L_R_ON || mode == SCREEN_L_ON)
            {
                if (effectIndex > 0)
                {
                    // Draw left line if effect has different start time then previous effect or
                    // previous effect was not selected, or only left was selected
                    if (eff->GetStartTimeMS() != effectLayer->GetEffect(effectIndex - 1)->GetEndTimeMS() ||
                        effectLayer->GetEffect(effectIndex - 1)->GetSelected() == EFFECT_NOT_SELECTED ||
                        effectLayer->GetEffect(effectIndex - 1)->GetSelected() == EFFECT_LT_SELECTED) {
                        linesLeft->AddVertex(x1, y1);
                        linesLeft->AddVertex(x1, y2);
                    }
                }
                else
                {
                    linesLeft->AddVertex(x1, y1);
                    linesLeft->AddVertex(x1, y2);
                }

                if (element->GetActive() && ri->layerIndex == 0)
                {
                    timingLines.AddVertex(x1, (row + 1)*DEFAULT_ROW_HEADING_HEIGHT, c);
                    timingLines.AddVertex(x1, GetSize().y, c);
                }
            }
            // Draw Right line
            if (mode == SCREEN_L_R_ON || mode == SCREEN_R_ON)
            {
                linesRight->AddVertex(x2, y1);
                linesRight->AddVertex(x2, y2);
                if (element->GetActive() && ri->layerIndex == 0)
                {
                    timingLines.AddVertex(x2, (row + 1)*DEFAULT_ROW_HEADING_HEIGHT, c);
                    timingLines.AddVertex(x2, GetSize().y, c);
                }
            }
            // Draw horizontal
            if (mode != SCREEN_L_R_OFF)
            {
                int half_width = (x2 - x1) / 2;
                linesLeft->AddVertex(x1, y);
                linesLeft->AddVertex(x1 + half_width, y);
                linesRight->AddVertex(x1 + half_width, y);
                linesRight->AddVertex(x2, y);
                if (eff->GetEffectName() != "" && (x2 - x1) > 20) {
                    int max_width = x2 - x1 - 18;
                    int text_width = DrawGLUtils::GetTextWidth(fontSize, eff->GetEffectName(), factor) + 8;
                    int width = std::min(text_width, max_width);
                    int center = x1 + (x2 - x1) / 2;
                    int label_start = center - width / 2;
                    xlColor label_color = xlights->color_mgr.GetColor(ColorManager::COLOR_LABELS);
                    if (ri->layerIndex == 0 && element->GetEffectLayerCount() > 1)
                    {
                        label_color = xlights->color_mgr.GetColor(ColorManager::COLOR_PHRASES);
                    }
                    else if (ri->layerIndex == 1)
                    {
                        label_color = xlights->color_mgr.GetColor(ColorManager::COLOR_WORDS);
                    }
                    else if (ri->layerIndex == 2)
                    {
                        label_color = xlights->color_mgr.GetColor(ColorManager::COLOR_PHONEMES);
                    }
                    textBackgrounds.AddRect(label_start, y1 - 2, label_start + width, y2 + 2, label_color);
                    timingLines.AddLinesRect(label_start - 0.4, y1 - 2 - 0.4, label_start + width + 0.4, y2 + 2 + 0.4, xlights->color_mgr.GetColor(ColorManager::COLOR_LABEL_OUTLINE));

                    // trim the text to fit
                    auto name = eff->GetEffectName();
                    while (name != "" && DrawGLUtils::GetTextWidth(fontSize, name, factor) > width)
                    {
                        name = name.substr(0, name.size() - 1);
                    }

                    texts.AddVertex(label_start + 4, y2 + toffset, name);
                }
            }
        }
    }
}

void EffectsGrid::render( wxPaintEvent& evt )
{
    wxPaintDC dc(this);
    Draw();
}

void EffectsGrid::Draw()
{
    if(!IsShownOnScreen()) return;
    if(!mIsInitialized) { InitializeGLCanvas(); }

    if (mWindowResized && mTimeline != nullptr) {
        mTimeline->RecalcEndTime();  // force a recalc of the Timeline end time so that timing effect positions will calculate correct during redraw
    }
    SetCurrentGLContext();

    LOG_GL_ERRORV(glClearColor(0.0f, 0.0f, 0.0f, 1.0f)); // Black Background
    LOG_GL_ERRORV(glClear(GL_COLOR_BUFFER_BIT));
    prepare2DViewport(0,0,mWindowWidth, mWindowHeight);

    if (mSequenceElements) {
        DrawLines();
        DrawEffects();
        DrawPlayMarker();

        bool has_timing_effects = (mSequenceElements->GetNumberOfActiveTimingEffects() > 0);
        if ((mDragging || mCellRangeSelected) && !mPartialCellSelected) {
            if (has_timing_effects && mRangeStartCol >= 0) {
                DrawSelectedCells();
            }
        }
        if (mDragging && !has_timing_effects) {
            int offset = (mDragStartRow - mSequenceElements->GetFirstVisibleModelRow()) * DEFAULT_ROW_HEADING_HEIGHT;
            DrawGLUtils::DrawRectangle(xlights->color_mgr.GetColor(ColorManager::COLOR_GRID_DASHES),true,mDragStartX,mDragStartY+offset,mDragEndX,mDragEndY);
        }
    }


    LOG_GL_ERRORV(SwapBuffers());
}

void EffectsGrid::DrawSelectedCells()
{
    EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
    int start_row = mRangeStartRow;
    int end_row = mRangeEndRow;
    int start_col = mRangeStartCol;
    int end_col = mRangeEndCol;
    if( start_row > end_row ) {
        std::swap( start_row, end_row );
    }
    if( start_col > end_col ) {
        std::swap( start_col, end_col );
    }
    if( tel->GetEffectCount() > 0 && end_col < tel->GetEffectCount() )
    {
        int first_row = mSequenceElements->GetFirstVisibleModelRow();
        int adjusted_start_row = start_row - first_row;
        adjusted_start_row = std::max(adjusted_start_row, mSequenceElements->GetNumberOfTimingRows());
        int last_row = end_row-first_row;
        if( last_row >= adjusted_start_row )
        {
            int start_x = mTimeline->GetPositionFromTimeMS(tel->GetEffect(start_col)->GetStartTimeMS())+1;
            int end_x = mTimeline->GetPositionFromTimeMS(tel->GetEffect(end_col)->GetEndTimeMS())-1;
            int start_y = adjusted_start_row*DEFAULT_ROW_HEADING_HEIGHT;
            int end_y = last_row*DEFAULT_ROW_HEADING_HEIGHT;

             if( !xlights->IsACActive() ) {
                xlColor highlight_color = xlights->color_mgr.GetTimingColor(mSequenceElements->GetVisibleRowInformation(mSequenceElements->GetSelectedTimingRow())->colorIndex);
                LOG_GL_ERRORV(glEnable(GL_BLEND));
                DrawGLUtils::DrawFillRectangle(highlight_color,80,start_x,start_y,end_x-start_x,end_y-start_y+DEFAULT_ROW_HEADING_HEIGHT);
                LOG_GL_ERRORV(glDisable(GL_BLEND));
            } else {
                DrawGLUtils::DrawRectangle(xlWHITE,false,start_x,start_y,end_x,end_y+DEFAULT_ROW_HEADING_HEIGHT);
                DrawGLUtils::DrawRectangle(xlWHITE,false,start_x+1,start_y+1,end_x-1,end_y-1+DEFAULT_ROW_HEADING_HEIGHT);
            }
        }
    }
}

void EffectsGrid::CreateEffectIconTextures()
{
    m_EffectTextures.resize(xlights->GetEffectManager().size());
    for (int x = 0; x < xlights->GetEffectManager().size(); x++) {
        RenderableEffect *eff = xlights->GetEffectManager()[x];
        DrawGLUtils::CreateOrUpdateTexture(eff->GetEffectIcon(64, true),
                                           eff->GetEffectIcon(32, true),
                                           eff->GetEffectIcon(16, true),
                                           &m_EffectTextures[eff->GetId()]);

    }
}

void EffectsGrid::DeleteEffectIconTextures()
{
    for (size_t x = 0; x < m_EffectTextures.size(); x++) {
        glDeleteTextures(1,&m_EffectTextures[x]);
    }
    m_EffectTextures.clear();
}

void EffectsGrid::magnify(wxMouseEvent& event) {
    magSinceLast += event.GetMagnification();
    if (magSinceLast > 0.05f)
    {
        wxCommandEvent eventZoom(EVT_ZOOM);
        eventZoom.SetInt(ZOOM_IN);
        wxPostEvent(mParent, eventZoom);
        magSinceLast = 0;
    }
    else if(magSinceLast < -0.05f)
    {
        wxCommandEvent eventZoom(EVT_ZOOM);
        eventZoom.SetInt(ZOOM_OUT);
        wxPostEvent(mParent, eventZoom);
        magSinceLast = 0;
    }
}

void EffectsGrid::mouseWheelMoved(wxMouseEvent& event)
{
    magSinceLast = 0;
    if (event.CmdDown())
    {
        int i = event.GetWheelRotation();
        if (i < 0)
        {
            wxCommandEvent eventZoom(EVT_ZOOM);
            eventZoom.SetInt(ZOOM_OUT);
            wxPostEvent(mParent, eventZoom);
        }
        else
        {
            wxCommandEvent eventZoom(EVT_ZOOM);
            eventZoom.SetInt(ZOOM_IN);
            wxPostEvent(mParent, eventZoom);
        }
    }
    else if(event.ShiftDown())
    {
        int i = event.GetWheelRotation();
        if (i < 0)
        {
            wxCommandEvent eventScroll(EVT_GSCROLL);
            eventScroll.SetInt(SCROLL_RIGHT);
            wxPostEvent(mParent, eventScroll);
        }
        else
        {
            wxCommandEvent eventScroll(EVT_GSCROLL);
            eventScroll.SetInt(SCROLL_LEFT);
            wxPostEvent(mParent, eventScroll);
        }
    }
    else
    {
        //cannot POST as we need the actual event to be processed inline
        //so we can tell the source of the event (trackpad vs mouse)
        GetParent()->GetEventHandler()->SafelyProcessEvent(event);
    }
}

int EffectsGrid::GetRow(int y) const
{
    return y/DEFAULT_ROW_HEADING_HEIGHT;
}

void EffectsGrid::RaiseSelectedEffectChanged(Effect* effect, bool isNew, bool updateUI, bool async) const
{
    if (effect == nullptr) return;

    // Place effect pointer in client data
    SelectedEffectChangedEvent eventEffectChanged(effect, isNew, updateUI);
    if (async)
    {
        wxPostEvent(GetParent(), eventEffectChanged);
    }
    else
    {
        // Pass it this way to prevent risk of effect being deleted before pointer is used
        GetParent()->GetEventHandler()->ProcessEvent(eventEffectChanged);
    }
}

void EffectsGrid::RaisePlayModelEffect(Element* element, Effect* effect, bool renderEffect) const
{
    // Place effect pointer in client data
    wxCommandEvent eventPlayModelEffect(EVT_PLAY_MODEL_EFFECT);
    playArgs->element = element;
    playArgs->effect = effect;
    playArgs->renderEffect = renderEffect;
    eventPlayModelEffect.SetClientData(playArgs);
    // Pass it this way to prevent risk of effect being deleted before pointer is used
    GetParent()->GetEventHandler()->ProcessEvent(eventPlayModelEffect);
    //wxPostEvent(GetParent(), eventPlayModelEffect);
}

void EffectsGrid::RaiseEffectDropped(int x, int y) const
{
    //Store Effect Layer to add effect to
    int row = GetRow(y);
    if (row >= mSequenceElements->GetVisibleRowInformationSize()) {
        return;
    }
    EffectLayer* effectLayer = mSequenceElements->GetVisibleEffectLayer(row);

    mSequenceElements->ClearSelectedRanges();
    EffectRange effectRange;
    effectRange.Layer = effectLayer;
    // Store start and end time. The effect text will be supplied by parent class
    effectRange.StartTime = mDropStartTimeMS;
    effectRange.EndTime = mDropEndTimeMS;
    mSequenceElements->AddSelectedRange(&effectRange);
    // Raise event
    wxCommandEvent eventDropped(EVT_EFFECT_DROPPED);
    wxPostEvent(GetParent(), eventDropped);
}

Element* EffectsGrid::GetActiveTimingElement() const
{
    Element* returnValue=nullptr;

    for (int row = 0; row<mSequenceElements->GetVisibleRowInformationSize(); row++)
    {
        Element* e = mSequenceElements->GetVisibleRowInformation(row)->element;
        if (e->GetType() == ElementType::ELEMENT_TYPE_TIMING && dynamic_cast<TimingElement*>(e)->GetActive())
        {
            returnValue = e;
            break;
        }
    }

    return returnValue;
}

void EffectsGrid::GetRangeOfMovementForSelectedEffects(int &toLeft, int &toRight) const
{
    int left;
    int right;
    toLeft = NO_MAX;
    toRight = NO_MAX;

    for (int row = 0; row<mSequenceElements->GetVisibleRowInformationSize(); row++)
    {
        EffectLayer* el = mSequenceElements->GetVisibleEffectLayer(row);
        el->GetMaximumRangeOfMovementForSelectedEffects(left, right);
        toLeft = toLeft < left ? toLeft : left;
        toRight = toRight < right ? toRight : right;
    }
}

void EffectsGrid::MoveAllSelectedEffects(int deltaMS, bool offset) const
{
    //static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // Tag all selected effects so we don't move them twice
    ((MainSequencer*)mParent)->TagAllSelectedEffects();

    if (!offset) {
        for(int row = 0; row < mSequenceElements->GetRowInformationSize(); row++)
        {
            EffectLayer* el = mSequenceElements->GetEffectLayer(row);
            //if (el == nullptr) logger_base.crit("MoveAllSelectedEffect EffectLayer A was NULL ... this is going to crash.");
            el->MoveAllSelectedEffects(deltaMS, mSequenceElements->get_undo_mgr());
        }
    } else {
        int start_row = -1;
        int end_row = -1;
        for(int row = 0; row < mSequenceElements->GetRowInformationSize(); row++)
        {
            EffectLayer* el = mSequenceElements->GetEffectLayer(row);
            //if (el == nullptr) logger_base.crit("MoveAllSelectedEffect EffectLayer B was NULL ... this is going to crash.");
            if (el->GetSelectedEffectCount() > 0) {
                if (start_row == -1) {
                    start_row = row;
                } else {
                    end_row = row;
                }
            }
        }
        int delta_step = 1000 / mSequenceElements->GetFrequency();
        if (start_row != end_row) {
            delta_step = deltaMS / (end_row - start_row);
        }
        delta_step = mTimeline->RoundToMultipleOfPeriod(delta_step, mSequenceElements->GetFrequency());
        for (int row = start_row; row <= end_row; row++)
        {
            EffectLayer* el = mSequenceElements->GetEffectLayer(row);
            //if (el == nullptr) logger_base.crit("MoveAllSelectedEffect EffectLayer C was NULL ... this is going to crash.");
            if( mResizingMode == EFFECT_RESIZE_RIGHT || mResizingMode == EFFECT_RESIZE_MOVE) {
                el->MoveAllSelectedEffects(delta_step*(row-start_row), mSequenceElements->get_undo_mgr());
            } else {
                el->MoveAllSelectedEffects(delta_step*(end_row-row), mSequenceElements->get_undo_mgr());
            }
        }
    }
}

void EffectsGrid::StretchAllSelectedEffects(int deltaMS, bool offset) const
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("EffectsGrid::StretchAllSelectedEffects.");

    // Tag all selected effects so we don't move them twice
    ((MainSequencer*)mParent)->TagAllSelectedEffects();

    if (!offset) {
        for (int row = 0; row<mSequenceElements->GetRowInformationSize(); row++)
        {
            EffectLayer* el = mSequenceElements->GetEffectLayer(row);
            el->StretchAllSelectedEffects(deltaMS, mSequenceElements->get_undo_mgr());
        }
    }
    else {
        int start_row = -1;
        int end_row = -1;
        for (int row = 0; row<mSequenceElements->GetRowInformationSize(); row++)
        {
            EffectLayer* el = mSequenceElements->GetEffectLayer(row);
            if (el->GetSelectedEffectCount() > 0) {
                if (start_row == -1) {
                    start_row = row;
                }
                else {
                    end_row = row;
                }
            }
        }
        int delta_step = deltaMS / (end_row - start_row);
        delta_step = mTimeline->RoundToMultipleOfPeriod(delta_step, mSequenceElements->GetFrequency());
        for (int row = start_row; row <= end_row; row++)
        {
            EffectLayer* el = mSequenceElements->GetEffectLayer(row);
            if (mResizingMode == EFFECT_RESIZE_RIGHT || mResizingMode == EFFECT_RESIZE_MOVE) {
                el->StretchAllSelectedEffects(delta_step*(row - start_row), mSequenceElements->get_undo_mgr());
            }
            else {
                el->StretchAllSelectedEffects(delta_step*(end_row - row), mSequenceElements->get_undo_mgr());
            }
        }
    }
}

void EffectsGrid::CopyModelEffects(int row_number, bool allLayers)
{
    if (!allLayers)
    {
        mSequenceElements->UnSelectAllEffects();
        EffectLayer* effectLayer = mSequenceElements->GetVisibleEffectLayer(row_number);
        Effect* effect = effectLayer->GetEffect(0);
        if (effect != nullptr)
        {
            mDropStartTimeMS = effect->GetStartTimeMS();
            mRangeCursorRow = mRangeStartRow;
            mRangeCursorCol = mRangeStartCol;
            mRangeStartCol = -1;
            mRangeEndCol = -1;
            mRangeStartRow = -1;
            mRangeEndRow = -1;
            mSequenceElements->SelectVisibleEffectsInRowAndTimeRange(row_number, row_number, mDropStartTimeMS, mSequenceElements->GetSequenceEnd());
            ((MainSequencer*)mParent)->CopySelectedEffects();
            effectLayer->UnSelectAllEffects();
            mPartialCellSelected = true;
            mCellRangeSelected = false;
            SetRCToolTip();
        }
        else
        {
            ((MainSequencer*)mParent)->PanelRowHeadings->SetCanPaste(false);
        }
    }
    else
    {
        Element* e = mSequenceElements->GetVisibleRowInformation(row_number)->element;
        mSequenceElements->UnSelectAllEffects();
        for (int i = 0; i < e->GetEffectLayerCount(); i++)
        {
            e->GetEffectLayer(i)->SelectAllEffects();
        }
        ((MainSequencer*)mParent)->CopySelectedEffects();
        mSequenceElements->UnSelectAllEffects();
        mPartialCellSelected = true;
        mCellRangeSelected = false;
        mRangeCursorRow = mRangeStartRow;
        mRangeCursorCol = mRangeStartCol;
        mRangeStartCol = -1;
        mRangeEndCol = -1;
        mRangeStartRow = -1;
        mRangeEndRow = -1;
        SetRCToolTip();
    }
}

void EffectsGrid::PasteModelEffects(int row_number, bool allLayers)
{
    if (allLayers)
    {
        mDropRow = row_number - mSequenceElements->GetVisibleRowInformation(row_number)->layerIndex;
    }
    else
    {
        mDropRow = row_number;
    }
    ((MainSequencer*)mParent)->Paste(true);
    mPartialCellSelected = true;
    ((MainSequencer*)mParent)->PanelRowHeadings->SetCanPaste(true);
}
