#include "EffectsGrid.h"
#include "MainSequencer.h"
#include "TimeLine.h"

#include "wx/wx.h"
#include "wx/sizer.h"
#include "wx/glcanvas.h"
#ifdef __WXMAC__
 #include "OpenGL/gl.h"
#else
 #ifdef _MSC_VER
  #include "../GL/glut.h"
 #else
  #include <GL/gl.h>
 #endif
#endif

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


#define EFFECT_RESIZE_NO                    0
#define EFFECT_RESIZE_LEFT                  1
#define EFFECT_RESIZE_RIGHT                 2
#define EFFECT_RESIZE_MOVE                  3
#define EFFECT_RESIZE_LEFT_EDGE             4
#define EFFECT_RESIZE_RIGHT_EDGE            5


BEGIN_EVENT_TABLE(EffectsGrid, xlGLCanvas)
EVT_MOTION(EffectsGrid::mouseMoved)
EVT_MOUSEWHEEL(EffectsGrid::mouseWheelMoved)
EVT_LEFT_DOWN(EffectsGrid::mouseDown)
EVT_LEFT_UP(EffectsGrid::mouseReleased)
EVT_MOUSE_CAPTURE_LOST(EffectsGrid::OnLostMouseCapture)
EVT_RIGHT_DOWN(EffectsGrid::rightClick)
EVT_LEFT_DCLICK(EffectsGrid::mouseLeftDClick)
EVT_LEAVE_WINDOW(EffectsGrid::mouseLeftWindow)
//EVT_KEY_DOWN(EffectsGrid::keyPressed)
//EVT_KEY_UP(EffectsGrid::keyReleased)
EVT_PAINT(EffectsGrid::render)
END_EVENT_TABLE()

// Menu constants
const long EffectsGrid::ID_GRID_MNU_COPY = wxNewId();
const long EffectsGrid::ID_GRID_MNU_PASTE = wxNewId();
const long EffectsGrid::ID_GRID_MNU_DELETE = wxNewId();
const long EffectsGrid::ID_GRID_MNU_RANDOM_EFFECTS = wxNewId();
const long EffectsGrid::ID_GRID_MNU_UNDO = wxNewId();
const long EffectsGrid::ID_GRID_MNU_PRESETS = wxNewId();
const long EffectsGrid::ID_GRID_MNU_BREAKDOWN_PHRASE = wxNewId();
const long EffectsGrid::ID_GRID_MNU_BREAKDOWN_WORD = wxNewId();
const long EffectsGrid::ID_GRID_MNU_BREAKDOWN_WORDS = wxNewId();

EffectsGrid::EffectsGrid(MainSequencer* parent, wxWindowID id, const wxPoint &pos, const wxSize &size,
                       long style, const wxString &name)
    :xlGLCanvas(parent, id, pos, size, wxFULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN | wxCLIP_SIBLINGS, "", true)
{
    mParent = parent;
    mDragging = false;
    mResizing = false;
    mDragDropping = false;
    mDropStartX = 0;
    mDropEndX = 0;
    mCellRangeSelected = false;
    mPartialCellSelected = false;
    mDragStartX = -1;
    mDragStartY = -1;
    mCanPaste = false;
    mSelectedEffect = nullptr;
    mRangeStartRow = -1;
    mRangeEndRow = -1;
    mRangeStartCol = -1;
    mRangeEndCol = -1;
    mResizeEffectIndex = -1;
    mTimeline = nullptr;

    SetBackgroundStyle(wxBG_STYLE_CUSTOM);

    mEffectColor = new xlColor(192,192,192);
    mGridlineColor = new xlColor(40,40,40);

    mTimingColor = new xlColor(255,255,255);
    mTimingVerticalLine = new xlColor(130,178,207);
    mSelectionColor = new xlColor(255,0,255);

    mLabelColor = new xlColor(255,255,204);
    mLabelOutlineColor = new xlColor(103, 103, 103);
    mPhraseColor = new xlColor(153, 255, 153);
    mWordColor = new xlColor(255, 218, 145);
    mPhonemeColor = new xlColor(255, 181, 218);

    SetDropTarget(new EffectDropTarget((wxWindow*)this,true));
    playArgs = new EventPlayEffectArgs();
    mSequenceElements = NULL;
    xlights = nullptr;
}

EffectsGrid::~EffectsGrid()
{
	delete mEffectColor;
	delete mGridlineColor;
	delete mTimingColor;
	delete mTimingVerticalLine;
	delete mSelectionColor;
}

void EffectsGrid::mouseLeftDClick(wxMouseEvent& event)
{
    if (mSequenceElements == NULL) {
        return;
    }
    int selectedTimeMS = mTimeline->GetAbsoluteTimeMSfromPosition(event.GetX());
    UpdateTimePosition(selectedTimeMS);

    int row = GetRow(event.GetY());
    if(row>=mSequenceElements->GetVisibleRowInformationSize() || row < 0)
        return;
    int effectIndex;
    HitLocation selectionType = HitLocation::NONE;
    Effect* selectedEffect = GetEffectAtRowAndTime(row,selectedTimeMS,effectIndex,selectionType);
    if (selectedEffect != nullptr && selectedEffect->GetParentEffectLayer()->GetParentElement()->GetType() == "timing") {
        wxString label = selectedEffect->GetEffectName();

        wxTextEntryDialog dlg(this, "Edit Label", "Enter new label:", label);
        if (dlg.ShowModal()) {
            selectedEffect->SetEffectName(dlg.GetValue().ToStdString());
        }
        Refresh();
    }

}

void EffectsGrid::rightClick(wxMouseEvent& event)
{
    if (mSequenceElements == NULL) {
        return;
    }
    SetFocus();
    wxMenu *mnuLayer = nullptr;
    mSelectedRow = event.GetY()/DEFAULT_ROW_HEADING_HEIGHT;
    if (mSelectedRow >= mSequenceElements->GetVisibleRowInformationSize()) {
        return;
    }

    Row_Information_Struct *ri =  mSequenceElements->GetVisibleRowInformation(mSelectedRow);
    Element* element = ri->element;
    if (element->GetType()=="model")
    {
        mnuLayer = new wxMenu();
        wxMenuItem* menu_copy = mnuLayer->Append(ID_GRID_MNU_COPY,"Copy");
        wxMenuItem* menu_paste = mnuLayer->Append(ID_GRID_MNU_PASTE,"Paste");
        wxMenuItem* menu_delete = mnuLayer->Append(ID_GRID_MNU_DELETE,"Delete");
        if( mSelectedEffect == nullptr && !MultipleEffectsSelected() ) {
            menu_copy->Enable(false);
            menu_delete->Enable(false);
        }
        if( !mCanPaste || !(mCellRangeSelected || mPartialCellSelected) ) {
            menu_paste->Enable(false);
        }
        mnuLayer->AppendSeparator();
        wxString undo_string = mSequenceElements->get_undo_mgr().GetUndoString();
        wxMenuItem* menu_undo = mnuLayer->Append(ID_GRID_MNU_UNDO,undo_string);
        if( !mSequenceElements->get_undo_mgr().CanUndo() ) {
            menu_undo->Enable(false);
        }
        mnuLayer->AppendSeparator();
        mnuLayer->Append(ID_GRID_MNU_PRESETS,"Effect Presets");
        wxMenuItem* menu_random = mnuLayer->Append(ID_GRID_MNU_RANDOM_EFFECTS,"Create Random Effects");
        if( !(mCellRangeSelected || mPartialCellSelected) ) {
            menu_random->Enable(false);
        }
    }
    else if (element->GetType()=="timing")
    {
        mnuLayer = new wxMenu();
        int effectIndex;
        HitLocation selectionType;
        int startTime = mTimeline->GetAbsoluteTimeMSfromPosition(event.GetX());
        Effect* selectedEffect = GetEffectAtRowAndTime(mSelectedRow,startTime,effectIndex,selectionType);
        if (selectedEffect != nullptr && selectedEffect->GetParentEffectLayer()->GetParentElement()->GetType() == "timing") {
            if( ri->layerIndex == 0 )
            {
                mnuLayer->Append(ID_GRID_MNU_BREAKDOWN_PHRASE,"Breakdown Phrase");
            }
            else if( ri->layerIndex == 1 )
            {
                mnuLayer->Append(ID_GRID_MNU_BREAKDOWN_WORD,"Breakdown Word");
                if (selectedEffect->GetParentEffectLayer()->GetSelectedEffectCount() > 1) {
                    mnuLayer->Append(ID_GRID_MNU_BREAKDOWN_WORDS,"Breakdown Selected Words");
                }
            }
            mSelectedEffect = selectedEffect;
        }
        mnuLayer->AppendSeparator();
        wxMenuItem* menu_copy = mnuLayer->Append(ID_GRID_MNU_COPY,"Copy");
        wxMenuItem* menu_paste = mnuLayer->Append(ID_GRID_MNU_PASTE,"Paste");
        wxMenuItem* menu_delete = mnuLayer->Append(ID_GRID_MNU_DELETE,"Delete");
        if( mSelectedEffect == nullptr && !MultipleEffectsSelected() ) {
            menu_copy->Enable(false);
            menu_delete->Enable(false);
        }
        if( !mCanPaste || !(mCellRangeSelected || mPartialCellSelected) ) {
            menu_paste->Enable(false);
        }
    }

    if (mnuLayer != nullptr) {
        mnuLayer->Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EffectsGrid::OnGridPopup, NULL, this);
        Draw();
        PopupMenu(mnuLayer);
    }
}

void EffectsGrid::keyReleased(wxKeyEvent& event){}
void EffectsGrid::keyPressed(wxKeyEvent& event){}

void EffectsGrid::sendRenderEvent(const std::string &model, int start, int end, bool clear) {
    RenderCommandEvent event(model, start, end, clear, false);
    wxPostEvent(mParent, event);
}

void EffectsGrid::OnGridPopup(wxCommandEvent& event)
{
    int id = event.GetId();
    if(id == ID_GRID_MNU_COPY)
    {
        ((MainSequencer*)mParent)->CopySelectedEffects();
        mCanPaste = true;
    }
    else if(id == ID_GRID_MNU_PASTE)
    {
        ((MainSequencer*)mParent)->Paste();
    }
    else if(id == ID_GRID_MNU_DELETE)
    {
        DeleteSelectedEffects();
    }
    else if(id == ID_GRID_MNU_RANDOM_EFFECTS)
    {
        FillRandomEffects();
    }
    else if(id == ID_GRID_MNU_DELETE)
    {
        DeleteSelectedEffects();
    }
    else if(id == ID_GRID_MNU_UNDO)
    {
        mSequenceElements->get_undo_mgr().UndoLastStep();
    }
    else if(id == ID_GRID_MNU_PRESETS)
    {
        if( xlights->EffectTreeDlg==NULL )
        {
            xlights->EffectTreeDlg = new EffectTreeDialog(xlights);
            xlights->EffectTreeDlg->InitItems(mSequenceElements->GetEffectsNode());
        }
        xlights->EffectTreeDlg->Show();
    }
    else if(id == ID_GRID_MNU_BREAKDOWN_PHRASE)
    {
        Effect* phrase_effect = mSelectedEffect;
        EffectLayer* word_layer;
        Element* element = phrase_effect->GetParentEffectLayer()->GetParentElement();
        element->SetFixedTiming(0);
        if( element->GetEffectLayerCount() == 1 )
        {
            word_layer = element->AddEffectLayer();
        }
        else
        {
            word_layer = element->GetEffectLayer(1);
        }
        mSequenceElements->get_undo_mgr().CreateUndoStep();
        word_layer->UnSelectAllEffects();
        word_layer->SelectEffectsInTimeRange(phrase_effect->GetStartTimeMS(), phrase_effect->GetEndTimeMS());
        word_layer->DeleteSelectedEffects(mSequenceElements->get_undo_mgr());
        mSequenceElements->BreakdownPhrase(word_layer, phrase_effect->GetStartTimeMS(), phrase_effect->GetEndTimeMS(), phrase_effect->GetEffectName());
        element->SetCollapsed(false);
        wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
        wxPostEvent(mParent, eventRowHeaderChanged);
    }
    else if(id == ID_GRID_MNU_BREAKDOWN_WORD)
    {
        Effect* word_effect = mSelectedEffect;
        EffectLayer* phoneme_layer;
        Element* element = word_effect->GetParentEffectLayer()->GetParentElement();
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
        phoneme_layer->DeleteSelectedEffects(mSequenceElements->get_undo_mgr());
        mSequenceElements->BreakdownWord(phoneme_layer, word_effect->GetStartTimeMS(), word_effect->GetEndTimeMS(), word_effect->GetEffectName());
        element->SetCollapsed(false);
        wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
        wxPostEvent(mParent, eventRowHeaderChanged);
    }
    else if(id == ID_GRID_MNU_BREAKDOWN_WORDS)
    {
        Effect* word_effect = mSelectedEffect;
        EffectLayer* phoneme_layer;
        Element* element = word_effect->GetParentEffectLayer()->GetParentElement();
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
                phoneme_layer->SelectEffectsInTimeRange(word_effect->GetStartTimeMS(), word_effect->GetEndTimeMS());
            }
        }
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
    Refresh();
}

void EffectsGrid::FillRandomEffects()
{
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
        if (timingIndex1 != -1 && timingIndex2 != -1) {
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
                        mSequenceElements->get_undo_mgr().CaptureAddedEffect( effectLayer->GetParentElement()->GetName(), effectLayer->GetIndex(), ef->GetID() );
                        RaiseSelectedEffectChanged(ef, true);
                        mSelectedEffect = ef;
                    }
                }
            }
            mCellRangeSelected = false;
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
            mSequenceElements->get_undo_mgr().CreateUndoStep();
            mSequenceElements->get_undo_mgr().CaptureAddedEffect( el->GetParentElement()->GetName(), el->GetIndex(), ef->GetID() );
            RaiseSelectedEffectChanged(ef, true);
            mSelectedEffect = ef;
            if (!ef->GetPaletteMap().empty()) {
                sendRenderEvent(el->GetParentElement()->GetName(),
                                mDropStartTimeMS,
                                mDropEndTimeMS, true);
            }
            RaiseSelectedEffectChanged(ef, true);
            mPartialCellSelected = false;
        }
    }
}

void EffectsGrid::ProcessDroppedEffect(Effect* effect)
{
    mSelectedEffect = effect;
    mSelectedRow = mDropRow;
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
            if(selectedTimingIndex >= 0)
            {
                EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(selectedTimingIndex);
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
            }
            else
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
    if( mDragDropping )
    {
        RaiseEffectDropped(x,y);
        mDragDropping = false;
    }
    Refresh(false);
}

void EffectsGrid::mouseMoved(wxMouseEvent& event)
{
    if (!mIsInitialized || mSequenceElements == NULL) {
        return;
    }

    int rowIndex = GetRow(event.GetY());
    bool out_of_bounds =  rowIndex < 0 || (rowIndex >= mSequenceElements->GetVisibleRowInformationSize());

    if(mResizing)
    {
        Resize(event.GetX(), event.AltDown());
        Refresh(false);
        Update();
    }
    else if (mDragging)
    {
        mDragEndX = event.GetX();
        mDragEndY = event.GetY();
        if( mSequenceElements->GetSelectedTimingRow() < 0 ) {
            EstablishSelectionRectangle();
        } else {
            UpdateSelectionRectangle();
        }
        Refresh(false);
        Update();
    }
    else
    {
        if(!out_of_bounds)
        {
            Element* element = mSequenceElements->GetVisibleRowInformation(rowIndex)->element;
            if( element != nullptr )
            {
                RunMouseOverHitTests(rowIndex,event.GetX(),event.GetY());
            }
        }
    }

    int mouseTimeMS = mTimeline->GetAbsoluteTimeMSfromPosition(event.GetX());
    UpdateMousePosition(mouseTimeMS);
}

void EffectsGrid::mouseLeftWindow(wxMouseEvent& event)
{
    UpdateMousePosition(-1);
}

int EffectsGrid::GetClippedPositionFromTimeMS(int ms) {
    int i = mTimeline->GetPositionFromTimeMS(ms);
    if (i < -10) {
        i = -10;
    }
    if (i > GetSize().x) {
        i = GetSize().x;
    }
    return i;
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
    EffectLayer* effectLayer = mSequenceElements->GetVisibleEffectLayer(row);

    Effect *eff = nullptr;
    selectionType = HitLocation::NONE;
    if (effectLayer->HitTestEffectByTime(ms, index)) {
        eff = effectLayer->GetEffect(index);
        int startPos = GetClippedPositionFromTimeMS(eff->GetStartTimeMS());
        int endPos = GetClippedPositionFromTimeMS(eff->GetEndTimeMS());
        int position = GetClippedPositionFromTimeMS(ms);
        int mid = (startPos + endPos) / 2;

        if ((endPos - startPos) < 8) {
            //too small to really differentiate, just
            //provide ability to make the effect larger
            if (position < mid) {
                selectionType = HitLocation::LEFT_EDGE;
            } else {
                selectionType = HitLocation::RIGHT_EDGE;
            }
        } else if(position > endPos - 6) {
            selectionType = HitLocation::RIGHT_EDGE;
        } else if(position > endPos - 12) {
            selectionType = HitLocation::RIGHT_EDGE_DISCONNECT;
        } else if(position > mid + 8) {
            selectionType = HitLocation::RIGHT;
        } else if(position < startPos + 6) {
            selectionType = HitLocation::LEFT_EDGE;
        } else if(position < startPos + 12) {
            selectionType = HitLocation::LEFT_EDGE_DISCONNECT;
        } else if(position < mid - 8) {
            selectionType = HitLocation::LEFT;
        } else {
            selectionType = HitLocation::CENTER;
        }
    }
    return eff;
}


void EffectsGrid::mouseDown(wxMouseEvent& event)
{
    mPartialCellSelected = false;
    // if no shift key clear any cell range selections
    if (!event.ShiftDown()) {
        mCellRangeSelected = false;
        mRangeStartCol = mRangeEndCol = mRangeStartRow = mRangeEndRow = -1;
        Refresh();
    }
    if (mSequenceElements == NULL) {
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

            mSelectedEffect = selectedEffect;
            if(element->GetType()=="model")
            {
                mSelectedRow = row;
                mSelectedEffect = selectedEffect;
                RaiseSelectedEffectChanged(mSelectedEffect, false);
                RaisePlayModelEffect(element,mSelectedEffect,false);
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
                if(selectedEffect == nullptr)
                {
                    mSelectedEffect = nullptr;
                    mSelectedRow = -1;
                }
            }
            if( mSequenceElements->GetSelectedTimingRow() >= 0 ) {
                mCellRangeSelected = true;
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

void adjustMS(int timeMS, int &min, int &max) {
    if (timeMS < min) {
        min = timeMS;
    }
    if (timeMS > max) {
        max = timeMS;
    }
}

void EffectsGrid::mouseReleased(wxMouseEvent& event)
{
    if (mSequenceElements == NULL) {
        return;
    }
    bool checkForEmptyCell = false;
    if(mResizing)
    {
        if(mEffectLayer->GetParentElement()->GetType()=="model")
        {
            int stime = mStartResizeTimeMS;
            int timeMS = mTimeline->GetAbsoluteTimeMSfromPosition(event.GetX());
            int min = stime;
            int max = stime;
            adjustMS(timeMS, min, max);

            Effect* effect = mEffectLayer->GetEffect(mResizeEffectIndex);
            if(effect)
            {
                adjustMS(mEffectLayer->GetEffect(mResizeEffectIndex)->GetStartTimeMS(), min, max);
                adjustMS(mEffectLayer->GetEffect(mResizeEffectIndex)->GetEndTimeMS(), min, max);
                if( mSelectedEffect->GetSelected() == EFFECT_LT_SELECTED && mResizeEffectIndex > 0) {
                    //also have to re-render the effect to the left
                    adjustMS(mEffectLayer->GetEffect(mResizeEffectIndex - 1)->GetStartTimeMS(), min, max);
                    adjustMS(mEffectLayer->GetEffect(mResizeEffectIndex - 1)->GetEndTimeMS(), min, max);
                } else if (mSelectedEffect->GetSelected() == EFFECT_RT_SELECTED
                           && mResizeEffectIndex < (mEffectLayer->GetEffectCount() - 1)) {
                    adjustMS(mEffectLayer->GetEffect(mResizeEffectIndex + 1)->GetStartTimeMS(), min, max);
                    adjustMS(mEffectLayer->GetEffect(mResizeEffectIndex + 1)->GetEndTimeMS(), min, max);
                }

                sendRenderEvent(mEffectLayer->GetParentElement()->GetName(), min, max);
                RaisePlayModelEffect(mEffectLayer->GetParentElement(), effect, false);
            }
        }

        // if dragging an effect endpoint move the selection point with it so it will
        // focus on that spot if you zoom afterwards.
        if( mSelectedEffect->GetSelected() == EFFECT_LT_SELECTED )
        {
            int selected_time = (int)(mSelectedEffect->GetStartTimeMS());
            UpdateZoomPosition(selected_time);
        }
        else if( mSelectedEffect->GetSelected() == EFFECT_RT_SELECTED )
        {
            int selected_time = (int)(mSelectedEffect->GetEndTimeMS());
            UpdateZoomPosition(selected_time);
        }
    } else if (mDragging) {
        ReleaseMouse();
        mDragging = false;

        if((mDragStartX == event.GetX() && mDragStartY == event.GetY()) || mSequenceElements->GetSelectedTimingRow() >= 0) {
            checkForEmptyCell = true;
        }
    } else if( !event.ControlDown() ) {
        checkForEmptyCell = true;
    }

    if (checkForEmptyCell) {
        int row = GetRow(event.GetY());
        if (mSequenceElements->GetSelectedTimingRow() >= 0 && row < mSequenceElements->GetVisibleRowInformationSize())
        {
            CheckForPartialCell(event.GetX());
            if( mPartialCellSelected )
            {
                mSelectedRow = -1;
                mSelectedEffect = nullptr;
                mSequenceElements->UnSelectAllEffects();
                wxCommandEvent eventUnSelected(EVT_UNSELECTED_EFFECT);
                wxPostEvent(mParent, eventUnSelected);
            }
        }
        else if (row >= 0 && row < mSequenceElements->GetVisibleRowInformationSize()) {
            EffectLayer* el = mSequenceElements->GetVisibleEffectLayer(row);
            int startTime = mTimeline->GetAbsoluteTimeMSfromPosition(event.GetX());
            int effectIndex = 0;
            if (el != nullptr) {
                if (el->HitTestEffectByTime(startTime, effectIndex)) {
                    el->GetEffect(effectIndex)->SetSelected(EFFECT_SELECTED);
                } else {
                    mDropStartTimeMS = mTimeline->GetAbsoluteTimeMSfromPosition(event.GetX());
                    mDropEndTimeMS = mDropStartTimeMS + 1000;
                    mDropStartX = mTimeline->GetPositionFromTimeMS(mDropStartTimeMS);
                    mDropEndX = mTimeline->GetPositionFromTimeMS(mDropEndTimeMS);
                    AdjustDropLocations(event.GetX(), el);
                    mPartialCellSelected = true;
                    mDropRow = row;
                    mSequenceElements->UnSelectAllEffects();
                    wxCommandEvent eventUnSelected(EVT_UNSELECTED_EFFECT);
                    wxPostEvent(mParent, eventUnSelected);
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

void EffectsGrid::CheckForPartialCell(int x_pos)
{
    mPartialCellSelected = false;
    // make sure a valid row and column is selected
    if( mRangeStartCol >= 0 && mRangeStartRow >= 0 )
    {
        // check for only single cell selection
        if( mRangeStartRow == mRangeEndRow && mRangeStartCol == mRangeEndCol )
        {
            EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
            EffectLayer* el = mSequenceElements->GetVisibleEffectLayer(mRangeStartRow - mSequenceElements->GetFirstVisibleModelRow());

            int startTime = mTimeline->GetAbsoluteTimeMSfromPosition(x_pos);
            int effectIndex = 0;
            if( !el->HitTestEffectByTime(startTime, effectIndex)) {
                Effect* eff = tel->GetEffect(mRangeStartCol);

                mDropStartX = mTimeline->GetPositionFromTimeMS(eff->GetStartTimeMS());
                mDropEndX = mTimeline->GetPositionFromTimeMS(eff->GetEndTimeMS());
                mDropStartTimeMS = eff->GetStartTimeMS();
                mDropEndTimeMS = eff->GetEndTimeMS();
                mDropRow = mRangeStartRow - mSequenceElements->GetFirstVisibleModelRow();
                if( AdjustDropLocations(x_pos, el) ) {
                    mPartialCellSelected = true;
                }
            }
        }
    }
}

void EffectsGrid::Resize(int position, bool offset)
{
    if (mSequenceElements == nullptr) {
        return;
    }
    if(MultipleEffectsSelected() || mResizingMode == EFFECT_RESIZE_MOVE)
    {
        ResizeMoveMultipleEffects(position, offset);
    }
    else
    {
        ResizeSingleEffect(position);
    }
    if( mSequenceElements->get_undo_mgr().ChangeCaptured() )
    {
        mSequenceElements->get_undo_mgr().SetCaptureUndo(false);
    }
}

void EffectsGrid::MoveSelectedEffectUp(bool shift)
{
    if (mSequenceElements == nullptr) {
        return;
    }
    if( mCellRangeSelected )
    {
        if( shift )
        {
            if( mRangeEndRow > mRangeStartRow )
            {
                mRangeEndRow--;
            }
        }
        else
        {
            if( mRangeStartRow > mSequenceElements->GetNumberOfTimingRows()+ mSequenceElements->GetFirstVisibleModelRow()) {
                mRangeStartRow--;
                mRangeEndRow--;
            }
        }
        UpdateSelectedEffects();
        Refresh(false);
    }
    else if( !MultipleEffectsSelected() && mSelectedEffect != nullptr && mSelectedRow > 0)
    {
        Row_Information_Struct *ri = mSequenceElements->GetVisibleRowInformation(mSelectedRow);
        int row = mSelectedRow-1;
        EffectLayer* el = mSelectedEffect->GetParentEffectLayer();
        while( row >= mSequenceElements->GetNumberOfTimingRows() )
        {
            EffectLayer* new_el = mSequenceElements->GetVisibleEffectLayer(row);
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
                mSelectedRow = row;
                mSelectedEffect = ef;
                el->DeleteSelectedEffects(mSequenceElements->get_undo_mgr());
                mSequenceElements->get_undo_mgr().CaptureAddedEffect( new_el->GetParentElement()->GetName(), new_el->GetIndex(), ef->GetID() );
                Refresh(false);
                return;
            }
            row--;
        }
    }
}

void EffectsGrid::MoveSelectedEffectDown(bool shift)
{
    if (mSequenceElements == nullptr) {
        return;
    }
    int first_row = mSequenceElements->GetFirstVisibleModelRow();
    if( mCellRangeSelected )
    {
        if( shift )
        {
            if( mRangeEndRow < mSequenceElements->GetVisibleRowInformationSize()+first_row-1 )
            {
                mRangeEndRow++;
            }
        }
        else
        {
            if( mRangeStartRow < mSequenceElements->GetVisibleRowInformationSize()+first_row-1 ) {
                mRangeStartRow++;
                mRangeEndRow++;
            }
        }
        UpdateSelectedEffects();
        Refresh(false);
    }
    else if( !MultipleEffectsSelected() && mSelectedEffect != nullptr && mSelectedRow >= 0)
    {
        Row_Information_Struct *ri = mSequenceElements->GetVisibleRowInformation(mSelectedRow);
        int row = mSelectedRow+1;
        EffectLayer* el = mSelectedEffect->GetParentEffectLayer();
        while( row < mSequenceElements->GetVisibleRowInformationSize() )
        {
            EffectLayer* new_el = mSequenceElements->GetVisibleEffectLayer(row);
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
                mSelectedRow = row;
                mSelectedEffect = ef;
                el->DeleteSelectedEffects(mSequenceElements->get_undo_mgr());
                mSequenceElements->get_undo_mgr().CaptureAddedEffect( new_el->GetParentElement()->GetName(), new_el->GetIndex(), ef->GetID() );
                Refresh(false);
                return;
            }
            row++;
        }
    }
}

void EffectsGrid::MoveSelectedEffectRight(bool shift)
{
    if (mSequenceElements == nullptr) {
        return;
    }
    if( mCellRangeSelected )
    {
        EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
        Effect* eff1 = tel->GetEffect(mRangeStartCol+1);
        Effect* eff2 = tel->GetEffect(mRangeEndCol+1);
        if( eff1 != nullptr && eff2 != nullptr )
        {
            mRangeEndCol++;
            if( !shift )
            {
                mRangeStartCol++;
            }
            UpdateSelectedEffects();
            Refresh(false);
        }
    }
    else if( !MultipleEffectsSelected() && mSelectedEffect != nullptr && mSelectedRow >= 0 )
    {
        int effect_length_ms = mSelectedEffect->GetEndTimeMS() - mSelectedEffect->GetStartTimeMS();
        Row_Information_Struct *ri = mSequenceElements->GetVisibleRowInformation(mSelectedRow);
        EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
        if( tel != nullptr )
        {
            int col = -1;
            for( int index = 0; index < tel->GetEffectCount(); index++ )
            {
                Effect* tim_ef = tel->GetEffect(index);
                if( mSelectedEffect->GetStartTimeMS() >= tim_ef->GetStartTimeMS() && mSelectedEffect->GetStartTimeMS() < tim_ef->GetEndTimeMS() )
                {
                    col = index+1;
                    break;
                }
            }
            if( col >= 0 )
            {
                EffectLayer* el = mSelectedEffect->GetParentEffectLayer();
                Effect* timing_effect = tel->GetEffect(col);
                while( timing_effect != nullptr && col < tel->GetEffectCount() )
                {
                    int new_end_time_ms = timing_effect->GetStartTimeMS() + effect_length_ms;
                    int start_time_for_check = timing_effect->GetStartTimeMS();
                    if( start_time_for_check < mSelectedEffect->GetEndTimeMS() )
                    {
                        start_time_for_check = mSelectedEffect->GetEndTimeMS();
                    }
                    if( el->GetRangeIsClearMS( start_time_for_check, new_end_time_ms ) )
                    {
                        mSequenceElements->get_undo_mgr().CreateUndoStep();
                        mSequenceElements->get_undo_mgr().CaptureEffectToBeMoved( el->GetParentElement()->GetName(), el->GetIndex(), mSelectedEffect->GetID(),
                                                                                  mSelectedEffect->GetStartTimeMS(), mSelectedEffect->GetEndTimeMS() );
                        mSelectedEffect->SetStartTimeMS(timing_effect->GetStartTimeMS());
                        mSelectedEffect->SetEndTimeMS(new_end_time_ms);
                        Refresh(false);
                        return;
                    }
                    col++;
                    if( col < tel->GetEffectCount() )
                    {
                        timing_effect = tel->GetEffect(col);
                    }
                }
            }
        }
        else // no timing selected...move 1 frame
        {
            EffectLayer* el = mSelectedEffect->GetParentEffectLayer();
            if( el->GetRangeIsClearMS( mSelectedEffect->GetEndTimeMS(), mSelectedEffect->GetEndTimeMS() + mSequenceElements->GetMinPeriod() ) )
            {
                mSequenceElements->get_undo_mgr().CreateUndoStep();
                mSequenceElements->get_undo_mgr().CaptureEffectToBeMoved( el->GetParentElement()->GetName(), el->GetIndex(), mSelectedEffect->GetID(),
                                                                          mSelectedEffect->GetStartTimeMS(), mSelectedEffect->GetEndTimeMS() );
                mSelectedEffect->SetStartTimeMS(mSelectedEffect->GetStartTimeMS() + mSequenceElements->GetMinPeriod());
                mSelectedEffect->SetEndTimeMS(mSelectedEffect->GetEndTimeMS() + mSequenceElements->GetMinPeriod());
                Refresh(false);
            }
        }
    }
}

void EffectsGrid::MoveSelectedEffectLeft(bool shift)
{
    if (mSequenceElements == nullptr) {
        return;
    }
    if( mCellRangeSelected )
    {
        EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
        Effect* eff = tel->GetEffect(mRangeEndCol-1);
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
            UpdateSelectedEffects();
            Refresh(false);
        }
    }
    else if( !MultipleEffectsSelected() && mSelectedEffect != nullptr && mSelectedRow >= 0 )
    {
        int effect_length_ms = mSelectedEffect->GetEndTimeMS() - mSelectedEffect->GetStartTimeMS();
        Row_Information_Struct *ri = mSequenceElements->GetVisibleRowInformation(mSelectedRow);
        EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
        if( tel != nullptr )
        {
            int col = -1;
            for( int index = 0; index < tel->GetEffectCount(); index++ )
            {
                Effect* tim_ef = tel->GetEffect(index);
                if( mSelectedEffect->GetStartTimeMS() >= tim_ef->GetStartTimeMS() && mSelectedEffect->GetStartTimeMS() < tim_ef->GetEndTimeMS() )
                {
                    col = index-1;
                    break;
                }
            }
            if( col >= 0 )
            {
                EffectLayer* el = mSelectedEffect->GetParentEffectLayer();
                Effect* timing_effect = tel->GetEffect(col);
                while( timing_effect != nullptr && col >= 0 )
                {
                    int new_end_time_ms = timing_effect->GetStartTimeMS() + effect_length_ms;
                    int end_time_for_check = new_end_time_ms;
                    if( end_time_for_check > mSelectedEffect->GetStartTimeMS() )
                    {
                        end_time_for_check = mSelectedEffect->GetStartTimeMS();
                    }
                    if( el->GetRangeIsClearMS( timing_effect->GetStartTimeMS(), end_time_for_check))
                    {
                        mSequenceElements->get_undo_mgr().CreateUndoStep();
                        mSequenceElements->get_undo_mgr().CaptureEffectToBeMoved( el->GetParentElement()->GetName(), el->GetIndex(), mSelectedEffect->GetID(),
                                                                                  mSelectedEffect->GetStartTimeMS(), mSelectedEffect->GetEndTimeMS() );
                        mSelectedEffect->SetStartTimeMS(timing_effect->GetStartTimeMS());
                        mSelectedEffect->SetEndTimeMS(new_end_time_ms);
                        Refresh(false);
                        return;
                    }
                    col--;
                    if( col >= 0 )
                    {
                        timing_effect = tel->GetEffect(col);
                    }
                }
            }
        }
        else // no timing selected...move 1 frame
        {
            EffectLayer* el = mSelectedEffect->GetParentEffectLayer();
            if(  mSelectedEffect->GetStartTimeMS() > 0 )
            {
                if( el->GetRangeIsClearMS( mSelectedEffect->GetStartTimeMS() - mSequenceElements->GetMinPeriod(), mSelectedEffect->GetStartTimeMS() ) )
                {
                    mSequenceElements->get_undo_mgr().CreateUndoStep();
                    mSequenceElements->get_undo_mgr().CaptureEffectToBeMoved( el->GetParentElement()->GetName(), el->GetIndex(), mSelectedEffect->GetID(),
                                                                              mSelectedEffect->GetStartTimeMS(), mSelectedEffect->GetEndTimeMS() );
                    mSelectedEffect->SetStartTimeMS(mSelectedEffect->GetStartTimeMS() - mSequenceElements->GetMinPeriod());
                    mSelectedEffect->SetEndTimeMS(mSelectedEffect->GetEndTimeMS() - mSequenceElements->GetMinPeriod());
                    Refresh(false);
                }
            }
        }
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
            if (ef->GetSelected() != EFFECT_NOT_SELECTED) {
                if (ef->GetStartTimeMS() < start) {
                    start = ef->GetStartTimeMS();
                }
                if (ef->GetEndTimeMS() > end) {
                    end = ef->GetEndTimeMS();
                }
            }
        }
        if (end > 0) {
            RenderCommandEvent event(element->GetName(), start, end, true, true);
            wxPostEvent(mParent, event);
            el->DeleteSelectedEffects(mSequenceElements->get_undo_mgr());
        }
    }
    mSelectedEffect = nullptr;
    mSelectedRow = -1;
    mResizing = false;
    mDragging = false;
    ForceRefresh();
}

bool EffectsGrid::PapagayoEffectsSelected()
{
    for(int i=0;i<mSequenceElements->GetVisibleRowInformationSize();i++)
    {
        EffectLayer* el = mSequenceElements->GetVisibleEffectLayer(i);
        if( el->GetParentElement()->GetType() == "timing" )
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

bool EffectsGrid::MultipleEffectsSelected()
{
    int count=0;
    for(int i=0;i<mSequenceElements->GetVisibleRowInformationSize();i++)
    {
        EffectLayer* el = mSequenceElements->GetVisibleEffectLayer(i);
        count+= el->GetSelectedEffectCount();
        if(count > 1)
        {
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

void EffectsGrid::OldPaste(const wxString &data) {
    if (mSequenceElements == nullptr) {
        return;
    }

    ((MainSequencer*)mParent)->PanelRowHeadings->SetCanPaste(false);
    if (mPartialCellSelected || OneCellSelected()) {
        wxArrayString all_efdata = wxSplit(data, '\n');
        if (all_efdata.size() == 0) {
            return;
        }
        if( all_efdata.size() > 2 )  // multi-effect paste
        {
            wxArrayString eff1data = wxSplit(all_efdata[0], '\t');
			if (eff1data.size() < 7)
			{
				// this code assumes at least 7 so dont go there
			}
			else
			{
				int drop_time_offset, new_start_time, new_end_time, column_start_time;
				column_start_time = wxAtoi(eff1data[6]);
				drop_time_offset = wxAtoi(eff1data[3]);
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
				for (int i = 0; i < all_efdata.size() - 1; i++)
				{
					wxArrayString efdata = wxSplit(all_efdata[i], '\t');
					if (efdata.size() < 7) {
						break;
					}
					new_start_time = wxAtoi(efdata[3]);
					new_end_time = wxAtoi(efdata[4]);
					new_start_time += drop_time_offset;
					new_end_time += drop_time_offset;
					int eff_row = wxAtoi(efdata[5]);
					drop_row = eff_row + drop_row_offset;
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
							mSequenceElements->get_undo_mgr().CaptureAddedEffect(el->GetParentElement()->GetName(), el->GetIndex(), ef->GetID());
							if (!ef->GetPaletteMap().empty()) {
								sendRenderEvent(el->GetParentElement()->GetName(),
									new_start_time,
									new_end_time, true);
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
                return;
            }
            if( efdata[0] == "Random" )
            {
                FillRandomEffects();
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
                EffectLayer* el = mSequenceElements->GetVisibleEffectLayer(mDropRow);
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
                    if( el->GetRangeIsClearMS(mDropStartTimeMS, end_time) )
                    {
                        Effect* ef = el->AddEffect(0,
                                      efdata[0].ToStdString(),
                                      efdata[1].ToStdString(),
                                      efdata[2].ToStdString(),
                                      mDropStartTimeMS,
                                      end_time,
                                      EFFECT_SELECTED,
                                      false);
                        mSequenceElements->get_undo_mgr().CreateUndoStep();
                        mSequenceElements->get_undo_mgr().CaptureAddedEffect( el->GetParentElement()->GetName(), el->GetIndex(), ef->GetID() );
                        if (!ef->GetPaletteMap().empty()) {
                            sendRenderEvent(el->GetParentElement()->GetName(),
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
    else if (mCellRangeSelected) {
        wxArrayString all_efdata = wxSplit(data, '\n');
        if (all_efdata.size() == 0) {
            return;
        }
        if( all_efdata.size() <= 2 )  // only single effect paste allowed for range
        {
            wxArrayString efdata = wxSplit(all_efdata[0], '\t');
            if (efdata.size() < 3) {
                return;
            }
            if( efdata[0] == "Random" )
            {
                FillRandomEffects();
            }
            else
            {
                int start_time, end_time;
                mSequenceElements->get_undo_mgr().CreateUndoStep();
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
                    start_time = tel->GetEffect(col1)->GetStartTimeMS();
                    end_time = tel->GetEffect(col2)->GetEndTimeMS();
                    EffectLayer* el = mSequenceElements->GetEffectLayer(row);
                    if( el->GetRangeIsClearMS(start_time, end_time) )
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
                            mSequenceElements->get_undo_mgr().CaptureAddedEffect( el->GetParentElement()->GetName(), el->GetIndex(), ef->GetID() );
                            if (!ef->GetPaletteMap().empty()) {
                                sendRenderEvent(el->GetParentElement()->GetName(),
                                                start_time,
                                                end_time, true);
                            }
                            RaiseSelectedEffectChanged(ef, true);
                            mSelectedEffect = ef;
                         }
                    }
                }
            }
            mCellRangeSelected = false;
        }
    }

    Refresh();
}

void EffectsGrid::Paste(const wxString &data) {
    if (mSequenceElements == nullptr) {
        return;
    }

    wxArrayString all_efdata = wxSplit(data, '\n');
    if (all_efdata.size() == 0) {
        return;
    }
    wxArrayString banner_data = wxSplit(all_efdata[0], '\t');
    if( banner_data[0] != "CopyFormat1" )
    {
        OldPaste(data);
        return;
    }

    bool paste_by_cell = ((MainSequencer*)mParent)->PasteByCellActive();
    if( paste_by_cell )
    {
        if( banner_data[6] == "NO_PASTE_BY_CELL" )
        {
            wxMessageBox("Paste By Cell information missing.\nYou can only Paste By Time with this data.", "Paste Warning!", wxICON_WARNING | wxOK );
            return;
        }
        if( mSequenceElements->GetSelectedTimingRow() < 0 )
        {
            wxMessageBox("Paste By Cell requires an active timing track.", "Paste Warning!", wxICON_WARNING | wxOK );
            return;
        }
    }

	int number_of_timings = wxAtoi(banner_data[1]);
	int number_of_effects = wxAtoi(banner_data[2]);
	int number_of_original_timing_rows = wxAtoi(banner_data[3]);
	int last_timing_row = wxAtoi(banner_data[4]);
	int start_column = wxAtoi(banner_data[5]);
	int selected_start_column = 0;
	int number_of_timing_rows = mSequenceElements->GetNumberOfTimingRows();

    if( number_of_timings > 0 && number_of_effects > 0 )
    {
        if( number_of_original_timing_rows != number_of_timing_rows )
        {
            wxMessageBox("Number of timing rows does not match how many existed when copied.", "Paste Warning!", wxICON_WARNING | wxOK );
            return;
        }
    }

    ((MainSequencer*)mParent)->PanelRowHeadings->SetCanPaste(false);
    if (mPartialCellSelected || OneCellSelected()) {
        if( (number_of_timings + number_of_effects) > 1 || paste_by_cell)  // multi-effect paste or pasting by cell
        {
            wxArrayString eff1data = wxSplit(all_efdata[1], '\t');
            int drop_time_offset, new_start_time, new_end_time, column_start_time;
            column_start_time = wxAtoi(eff1data[6]);
            drop_time_offset = wxAtoi(eff1data[3]);
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
            if( number_of_timings > 0 && number_of_effects > 0 )  // only allow timing and model effects to be pasted on same rows
            {
                drop_row = 0;
                drop_row_offset = 0;
            }
            else if ( number_of_timings > 0 && number_of_effects == 0 ) // if only timing effects make sure enough rows remain to receive effects
            {
                if( last_timing_row + mDropRow >= number_of_timing_rows )
                {
                    wxMessageBox("Not enough timing rows to paste timing effects starting on this row.", "Paste Warning!", wxICON_WARNING | wxOK );
                    return;
                }
            }
            else if ( number_of_timings == 0 && number_of_effects > 0 ) // if only model effects make sure target row isn't in timing tracks
            {
                if( mDropRow < number_of_timing_rows )
                {
                    wxMessageBox("Cannot paste model effects into timing tracks.", "Paste Warning!", wxICON_WARNING | wxOK );
                    return;
                }
            }
            if( paste_by_cell )
            {
                bool found_selected_start_column = tel->HitTestEffectByTime(mDropStartTimeMS+1, selected_start_column);
                if( !found_selected_start_column )
                {
                    wxMessageBox("Unable to find a selected timing start location for Paste By Cell.", "Paste Warning!", wxICON_WARNING | wxOK );
                    return;
                }
            }

            mSequenceElements->get_undo_mgr().CreateUndoStep();
            for (int i = 1; i < all_efdata.size() - 1; i++)
            {
                wxArrayString efdata = wxSplit(all_efdata[i], '\t');
                if (efdata.size() < 7) {
                    break;
                }
                bool is_timing_effect = (efdata[7] == "TIMING_EFFECT");
                new_start_time = wxAtoi(efdata[3]);
                new_end_time = wxAtoi(efdata[4]);
                if( paste_by_cell && !is_timing_effect )
                {
                    int eff_start_column = wxAtoi(efdata[7]);
                    int eff_end_column = wxAtoi(efdata[8]);
                    int eff_start_pct = wxAtoi(efdata[9]);
                    int eff_end_pct = wxAtoi(efdata[10]);
                    int column_offset = selected_start_column - start_column;
                    eff_start_column += column_offset;
                    eff_end_column += column_offset;
                    Effect* te_start = tel->GetEffect(eff_start_column);
                    Effect* te_end = tel->GetEffect(eff_end_column);
                    if( te_start == nullptr || te_end == nullptr )
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
                drop_row = eff_row + drop_row_offset;
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
                        mSequenceElements->get_undo_mgr().CaptureAddedEffect(el->GetParentElement()->GetName(), el->GetIndex(), ef->GetID());
                        if (!is_timing_effect && !ef->GetPaletteMap().empty() ) {
                            sendRenderEvent(el->GetParentElement()->GetName(),
                                new_start_time,
                                new_end_time, true);
                        }
                    }
                }
            }
            mPartialCellSelected = false;
        }
        else
        {
            wxArrayString efdata = wxSplit(all_efdata[1], '\t');
            if (efdata.size() < 3) {
                return;
            }
            if( efdata[0] == "Random" )
            {
                FillRandomEffects();
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
                if( number_of_timings == 0 && mDropRow < number_of_timing_rows )
                {
                    wxMessageBox("Cannot paste model effect into timing track.", "Paste Warning!", wxICON_WARNING | wxOK );
                    return;
                }
                EffectLayer* el = mSequenceElements->GetVisibleEffectLayer(mDropRow);
                int effectIndex = xlights->GetEffectManager().GetEffectIndex(efdata[0].ToStdString());
                if (effectIndex >= 0) {
                    int end_time = mDropEndTimeMS;
                    if( (efdata.size() >= 7) && GetActiveTimingElement() == nullptr )  // use original effect length if no timing track is active
                    {
                        int drop_time_offset = wxAtoi(efdata[3]);
                        drop_time_offset = mDropStartTimeMS - drop_time_offset;
                        end_time = wxAtoi(efdata[4]);
                        end_time += drop_time_offset;
                    }
                    if( el->GetRangeIsClearMS(mDropStartTimeMS, end_time) )
                    {
                        Effect* ef = el->AddEffect(0,
                                      efdata[0].ToStdString(),
                                      efdata[1].ToStdString(),
                                      efdata[2].ToStdString(),
                                      mDropStartTimeMS,
                                      end_time,
                                      EFFECT_SELECTED,
                                      false);
                        mSequenceElements->get_undo_mgr().CreateUndoStep();
                        mSequenceElements->get_undo_mgr().CaptureAddedEffect( el->GetParentElement()->GetName(), el->GetIndex(), ef->GetID() );
                        if (!ef->GetPaletteMap().empty()) {
                            sendRenderEvent(el->GetParentElement()->GetName(),
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
    else if (mCellRangeSelected) {
        if ( number_of_timings == 0 && number_of_effects == 1 )  // only single effect paste allowed for range
        {
            wxArrayString efdata = wxSplit(all_efdata[1], '\t');
            if (efdata.size() < 3) {
                return;
            }
            if( efdata[0] == "Random" )
            {
                FillRandomEffects();
            }
            else
            {
                int start_time, end_time;
                mSequenceElements->get_undo_mgr().CreateUndoStep();
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
                    start_time = tel->GetEffect(col1)->GetStartTimeMS();
                    end_time = tel->GetEffect(col2)->GetEndTimeMS();
                    EffectLayer* el = mSequenceElements->GetEffectLayer(row);
                    if( el->GetRangeIsClearMS(start_time, end_time) )
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
                            mSequenceElements->get_undo_mgr().CaptureAddedEffect( el->GetParentElement()->GetName(), el->GetIndex(), ef->GetID() );
                            if (!ef->GetPaletteMap().empty()) {
                                sendRenderEvent(el->GetParentElement()->GetName(),
                                                start_time,
                                                end_time, true);
                            }
                            RaiseSelectedEffectChanged(ef, true);
                            mSelectedEffect = ef;
                         }
                    }
                }
            }
            mCellRangeSelected = false;
        }
    }

    Refresh();
}

void EffectsGrid::ResizeMoveMultipleEffects(int position, bool offset)
{
    int deltaTime = 0;
    int toLeft,toRight;
    int time = mTimeline->GetAbsoluteTimeMSfromPosition(position);
    GetRangeOfMovementForSelectedEffects(toLeft,toRight);
    if(mResizingMode==EFFECT_RESIZE_LEFT || mResizingMode==EFFECT_RESIZE_LEFT_EDGE)
    {
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
        mTimeline->GetPositionsFromTimeRange(mEffectLayer->GetEffect(mResizeEffectIndex)->GetStartTimeMS(),
                                             mEffectLayer->GetEffect(mResizeEffectIndex)->GetEndTimeMS(),mode,x1,x2,x3,x4);
        int midpoint = mTimeline->GetTimeMSfromPosition((x1+x2)/2) + mTimeline->GetStartTimeMS();
        deltaTime = time - midpoint;
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
}

void EffectsGrid::ResizeSingleEffect(int position)
{
    int time = mTimeline->GetAbsoluteTimeMSfromPosition(position);
    time = mTimeline->RoundToMultipleOfPeriod(time, mSequenceElements->GetFrequency());
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
                    mSequenceElements->get_undo_mgr().CaptureEffectToBeMoved( mEffectLayer->GetParentElement()->GetName(), mEffectLayer->GetIndex(), eff->GetID(),
                                                                              eff->GetStartTimeMS(), eff->GetEndTimeMS() );
                }
                eff->SetEndTimeMS(time);
            }
            Effect* eff = mEffectLayer->GetEffect(mResizeEffectIndex);
            if( mSequenceElements->get_undo_mgr().GetCaptureUndo() ) {
                mSequenceElements->get_undo_mgr().CaptureEffectToBeMoved( mEffectLayer->GetParentElement()->GetName(), mEffectLayer->GetIndex(), eff->GetID(),
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
                    mSequenceElements->get_undo_mgr().CaptureEffectToBeMoved( mEffectLayer->GetParentElement()->GetName(), mEffectLayer->GetIndex(), eff->GetID(),
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
                    mSequenceElements->get_undo_mgr().CaptureEffectToBeMoved( mEffectLayer->GetParentElement()->GetName(), mEffectLayer->GetIndex(), eff->GetID(),
                                                                              eff->GetStartTimeMS(), eff->GetEndTimeMS() );
                }
                eff->SetStartTimeMS(time);
            }
            Effect* eff = mEffectLayer->GetEffect(mResizeEffectIndex);
            if( mSequenceElements->get_undo_mgr().GetCaptureUndo() ) {
                mSequenceElements->get_undo_mgr().CaptureEffectToBeMoved( mEffectLayer->GetParentElement()->GetName(), mEffectLayer->GetIndex(), eff->GetID(),
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
                    mSequenceElements->get_undo_mgr().CaptureEffectToBeMoved( mEffectLayer->GetParentElement()->GetName(), mEffectLayer->GetIndex(), eff->GetID(),
                                                                              eff->GetStartTimeMS(), eff->GetEndTimeMS() );
                }
                eff->SetEndTimeMS(mEffectLayer->GetEffect(mResizeEffectIndex+1)->GetStartTimeMS());
            }
        }
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
        }
    } else {
        SetCursor(wxCURSOR_DEFAULT);
        mResizingMode = EFFECT_RESIZE_NO;
    }
}

void EffectsGrid::UpdateTimePosition(int time)
{
    // Update time selection
    wxCommandEvent eventTimeSelected(EVT_TIME_SELECTED);
    eventTimeSelected.SetInt(time);
    wxPostEvent(mParent, eventTimeSelected);
}

void EffectsGrid::UpdateMousePosition(int time)
{
    // Update time selection
    wxCommandEvent eventMousePos(EVT_MOUSE_POSITION);
    eventMousePos.SetInt(time);
    wxPostEvent(mParent, eventMousePos);
}

void EffectsGrid::UpdateZoomPosition(int time)
{
    mTimeline->SetZoomMarkerMS(time);
}

void EffectsGrid::EstablishSelectionRectangle()
{
    mSequenceElements->UnSelectAllEffects();
    int first_row = mSequenceElements->GetFirstVisibleModelRow();
    int row1 =  GetRow(mDragStartY) + first_row;
    int row2 =  GetRow(mDragEndY) + first_row;
    if( row1 > row2 ) {
        std::swap(row1, row2);
    }
    mRangeStartRow = row1;
    mRangeEndRow = row2;

    int start_x = mDragStartX;
    int end_x = mDragEndX;
    if( start_x > end_x ) {
        std::swap(start_x, end_x);
    }

    int startTime = mTimeline->GetAbsoluteTimeMSfromPosition(start_x);
    int endTime = mTimeline->GetAbsoluteTimeMSfromPosition(end_x);

    if( mSequenceElements->GetSelectedTimingRow() >= 0 )
    {
        EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
        int timingIndex1 = 0;
        int timingIndex2 = 0;
        if(tel->HitTestEffectByTime(startTime, timingIndex1)
           && tel->HitTestEffectByTime(endTime, timingIndex2))
        {
            mRangeStartCol = timingIndex1;
            mRangeEndCol = timingIndex2;
            mDropStartTimeMS = tel->GetEffect(mRangeStartCol)->GetStartTimeMS();  // set for paste
        }
    }

    if( mRangeStartCol >= 0 && mRangeStartRow >= 0 )
    {
        UpdateSelectedEffects();
    }
    else
    {
        mSequenceElements->UnSelectAllEffects();
        mSequenceElements->SelectEffectsInRowAndTimeRange(row1-first_row,row2-first_row,startTime,endTime);
    }
}

void EffectsGrid::UpdateSelectionRectangle()
{
    mSequenceElements->UnSelectAllEffects();
    mRangeEndRow = GetRow(mDragEndY) + mSequenceElements->GetFirstVisibleModelRow();
    if( mRangeEndRow >= mSequenceElements->GetRowInformationSize() )
    {
       mRangeEndRow = mSequenceElements->GetRowInformationSize() - 1;
    }

    if( mSequenceElements->GetSelectedTimingRow() >= 0 )
    {
        EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
        int time = mTimeline->GetAbsoluteTimeMSfromPosition(mDragEndX);
        int timingIndex = 0;
        if(tel->HitTestEffectByTime(time,timingIndex))
        {
            mRangeEndCol = timingIndex;
        }
    }
    UpdateSelectedEffects();
}

void EffectsGrid::UpdateSelectedEffects()
{
    mSelectedEffect = nullptr;
    mSequenceElements->UnSelectAllEffects();
    int first_row = mSequenceElements->GetFirstVisibleModelRow();
    if( mRangeStartCol >= 0 && mRangeStartRow >= 0 )
    {
        int start_row = mRangeStartRow-first_row;
        int end_row = mRangeEndRow-first_row;
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
                wxCommandEvent eventUnSelected(EVT_UNSELECTED_EFFECT);
                wxPostEvent(mParent, eventUnSelected);
            }
        }
    }
}

void EffectsGrid::ForceRefresh()
{
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
    if(!IsShownOnScreen() || xlights == nullptr) return;
    SetCurrentGLContext();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    prepare2DViewport(0,0,mWindowWidth, mWindowHeight);
    CreateEffectIconTextures();
    mIsInitialized = true;
}

void EffectsGrid::DrawLines()
{
    // Draw Horizontal lines
    int x1=1;
    int x2 = mWindowWidth-1;
    int y;
    bool isEvenLayer=false;

    xlColor color(33, 33, 33);
    for(int row=0;row < mSequenceElements->GetVisibleRowInformationSize();row++)
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
                DrawGLUtils::AddRectAsTriangles(x1, y, x2, y + h, color);
            }
            isEvenLayer = !isEvenLayer;
        } else if (ri->strandIndex != -1) {
            if (isEvenLayer)
            {
                DrawGLUtils::AddRectAsTriangles(x1, y, x2, y + DEFAULT_ROW_HEADING_HEIGHT, color);
            }
            isEvenLayer = !isEvenLayer;
        }
    }
    DrawGLUtils::End(GL_TRIANGLES);

    glLineWidth(0.2);
    for(int row=0;row < mSequenceElements->GetVisibleRowInformationSize();row++)
    {
        y = (row+1)*DEFAULT_ROW_HEADING_HEIGHT;
        DrawGLUtils::AddVertex(x1, y, *mGridlineColor);
        DrawGLUtils::AddVertex(x2, y, *mGridlineColor);
    }

    // Draw vertical lines
    int y1 = 0;
    int y2 = mWindowHeight-1;
    for(int x1=0;x1<mWindowWidth;x1++)
    {
        // Draw hash marks
        if ((x1+mStartPixelOffset)%(PIXELS_PER_MAJOR_HASH)==0)
        {
            DrawGLUtils::AddVertex(x1, y1, *mGridlineColor);
            DrawGLUtils::AddVertex(x1, y2, *mGridlineColor);
        }
    }

    DrawGLUtils::End(GL_LINES);
    glLineWidth(1);
}


void EffectsGrid::DrawPlayMarker()
{
    xlColor play_line = xlRED;
    int play_pos = mTimeline->GetPlayMarker();
    if( play_pos > 0 )
    {
        DrawGLUtils::DrawLine(play_line,255,play_pos,0,play_pos,GetSize().y,.2);
    }
}

int EffectsGrid::DrawEffectBackground(const Row_Information_Struct* ri, const Effect *e,
                                      int x1, int y1, int x2, int y2,
                                      DrawGLUtils::xlVertexColorAccumulator &backgrounds) {
    if (e->GetPalette().size() == 0) {
        //if there are no colors selected, none of the "backgrounds" make sense.  Don't draw
        //the background and instead make sure the icon is displayed to the user knows they
        //need to make some decisions about the colors to be used.
        return 1;
    }
    RenderableEffect *ef = xlights->GetEffectManager()[e->GetEffectIndex()];
    return ef == nullptr ? 1 : ef->DrawEffectBackground(e, x1, y1, x2, y2, backgrounds);
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
    for(int row=0;row<mSequenceElements->GetVisibleRowInformationSize();row++)
    {
        wxString type = mSequenceElements->GetVisibleRowInformation(row)->element->GetType();
        wxString name = mSequenceElements->GetVisibleRowInformation(row)->element->GetName();
        if(type!="view" && type != "model") {
            DrawTimingEffects(row);
        } else {
            Row_Information_Struct *ri = mSequenceElements->GetVisibleRowInformation(row);
            EffectLayer* effectLayer = mSequenceElements->GetEffectLayer(ri);
            lines.PreAlloc(effectLayer->GetEffectCount() * 16);
            selectedLines.PreAlloc(effectLayer->GetEffectCount() * 16);
            
            DrawGLUtils::xlVertexAccumulator * linesRight;
            DrawGLUtils::xlVertexAccumulator * linesLeft;
            DrawGLUtils::xlVertexAccumulator * linesCenter;
            int y1 = (row*DEFAULT_ROW_HEADING_HEIGHT)+2;
            int y2 = ((row+1)*DEFAULT_ROW_HEADING_HEIGHT)-2;
            int y = (row*DEFAULT_ROW_HEADING_HEIGHT) + (DEFAULT_ROW_HEADING_HEIGHT/2);

            if (mGridNodeValues && ri->nodeIndex != -1) {
                std::vector<xlColor> colors;
                std::vector<double> xs;
                PixelBufferClass ncls(xlights, true);
                ncls.InitNodeBuffer(*xlights->GetModel(ri->element->GetName()), ri->strandIndex, ri->nodeIndex, seqData->FrameTime());
                xlColor lastColor;
                for (int f = 0; f < seqData->NumFrames(); f++) {
                    ncls.SetNodeChannelValues(0, (*seqData)[f][ncls.NodeStartChannel(0)]);
                    xlColor c = ncls.GetNodeColor(0);
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
                int timems = (seqData->NumFrames() + 1) * seqData->FrameTime();
                xs.push_back(mTimeline->GetPositionFromTimeMS(timems));
                
                backgrounds.PreAlloc(xs.size() * 6);
                float y1 = (row*DEFAULT_ROW_HEADING_HEIGHT)+3;
                float y2 = ((row+1)*DEFAULT_ROW_HEADING_HEIGHT)-3;
                float x =  mTimeline->GetPositionFromTimeMS(0);
                for (int n = 0; n < xs.size(); n++) {
                    int x2 = xs[n];
                    if (x2 >= 0) {
                        backgrounds.AddRect(x, y1, x2, y2, colors[n]);
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
                linesLeft = effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_NOT_SELECTED ||
                                   effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_RT_SELECTED?&lines:&selectedLines;
                linesRight = effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_NOT_SELECTED ||
                                   effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_LT_SELECTED?&lines:&selectedLines;
                linesCenter = effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_SELECTED?&selectedLines:&lines;

                int drawIcon = 1;
                if(mGridIconBackgrounds && (ri->nodeIndex == -1 || !mGridNodeValues))
                {
                    drawIcon = DrawEffectBackground(ri, e, x3, y1, x4, y2, backgrounds);
                }
                if (mGridNodeValues && ri->nodeIndex != -1) {
                    drawIcon = 2;
                }

                if (mode != SCREEN_L_R_OFF)
                {
                    if(mode==SCREEN_L_R_ON || mode == SCREEN_L_ON)
                    {
                        if(effectIndex>0)
                        {
                            // Draw left line if effect has different start time then previous effect or
                            // previous effect was not selected, or onlwidthy left was selected
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
                                
                                lines.AddVertex(xl-0.4,y-sz);
                                lines.AddVertex(xr+0.4,y-sz);
                                lines.AddVertex(xl-0.4,y+sz);
                                lines.AddVertex(xr+0.4,y+sz);
                                lines.AddVertex(xl-0.4,y+sz+0.25);
                                lines.AddVertex(xl-0.4,y-sz);
                                lines.AddVertex(xr+0.4,y+sz+0.25);
                                lines.AddVertex(xr+0.4,y-sz);
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
                                lines.AddVertex(xl-0.4,y-sz);
                                lines.AddVertex(xr+0.4,y-sz);
                                lines.AddVertex(xl-0.4,y+sz);
                                lines.AddVertex(xr+0.4,y+sz);
                                lines.AddVertex(xl-0.4,y+sz+0.25);
                                lines.AddVertex(xl-0.4,y-sz);
                                lines.AddVertex(xr+0.4,y+sz+0.25);
                                lines.AddVertex(xr+0.4,y-sz);
                            }
                            else
                            {
                                linesCenter->AddVertex(x1,y);
                                linesCenter->AddVertex(x2,y);
                            }
                        }
                    }

                }
            }
            if((mDragDropping || mPartialCellSelected) && mDropRow == row)
            {
                int y3 = row*DEFAULT_ROW_HEADING_HEIGHT;
                xlColor highlight_color;
                int selected_timing_row = mSequenceElements->GetSelectedTimingRow();
                if( selected_timing_row >= 0 )
                {
                    highlight_color = *RowHeading::GetTimingColor(mSequenceElements->GetVisibleRowInformation(selected_timing_row)->colorIndex);
                }
                else
                {
                    highlight_color = *RowHeading::GetTimingColor(0);
                }
                highlight_color.alpha = 128;
                selectedBoxes.AddRect(mDropStartX,y3,mDropStartX+mDropEndX-mDropStartX,y3+DEFAULT_ROW_HEADING_HEIGHT, highlight_color);
            }
        }
    }
    for (auto it = textures.begin(); it != textures.end(); it++) {
        it->second.id = it->first;
        DrawGLUtils::Draw(it->second, GL_TRIANGLES);
        it->second.Reset();
    }
    DrawGLUtils::Draw(backgrounds, GL_TRIANGLES);
    DrawGLUtils::Draw(lines, *mEffectColor, GL_LINES);
    DrawGLUtils::Draw(selectedLines, *mSelectionColor, GL_LINES);

    glLineWidth(2.0);
    DrawGLUtils::Draw(timingEffLines, xlWHITE, GL_LINES);
    DrawGLUtils::Draw(textBackgrounds, GL_TRIANGLES);
    glLineWidth(1.0);
    DrawGLUtils::Draw(timingLines, GL_LINES, GL_BLEND);
    
    float factor = translateToBacking(1.0);
    int toffset;
    float fontSize = ComputeFontSize(toffset, factor);

    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    DrawGLUtils::Draw(texts, fontSize, factor, GL_BLEND);
    DrawGLUtils::Draw(selectedBoxes, GL_TRIANGLES, GL_BLEND);
    
    textBackgrounds.Reset();
    timingLines.Reset();
    timingEffLines.Reset();
    texts.Reset();
    backgrounds.Reset();
    selectedBoxes.Reset();
    selectedLines.Reset();
    lines.Reset();
}

void EffectsGrid::DrawTimingEffects(int row)
{
    Row_Information_Struct *ri = mSequenceElements->GetVisibleRowInformation(row);
    Element* element =ri->element;
    EffectLayer* effectLayer=mSequenceElements->GetVisibleEffectLayer(row);
    
    DrawGLUtils::xlVertexAccumulator * linesRight;
    DrawGLUtils::xlVertexAccumulator * linesLeft;
    DrawGLUtils::xlVertexAccumulator * linesCenter;
    xlColor c(*RowHeading::GetTimingColor(ri->colorIndex));
    c.alpha = 128;

    int toffset = 0;
    float factor = translateToBacking(1.0);
    float fontSize = ComputeFontSize(toffset, factor);

    for(int effectIndex=0;effectIndex < effectLayer->GetEffectCount();effectIndex++)
    {
        EFFECT_SCREEN_MODE mode = SCREEN_L_R_OFF;

        int y1 = (row*DEFAULT_ROW_HEADING_HEIGHT)+4;
        int y2 = ((row+1)*DEFAULT_ROW_HEADING_HEIGHT)-4;
        int y = (row*DEFAULT_ROW_HEADING_HEIGHT) + (DEFAULT_ROW_HEADING_HEIGHT / 2.0);
        int x1,x2,x3,x4;

        mTimeline->GetPositionsFromTimeRange(effectLayer->GetEffect(effectIndex)->GetStartTimeMS(),
                                             effectLayer->GetEffect(effectIndex)->GetEndTimeMS(),mode,x1,x2,x3,x4);

        linesLeft = effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_NOT_SELECTED ||
            effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_RT_SELECTED?&timingEffLines:&selectedLines;
        linesRight = effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_NOT_SELECTED ||
            effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_LT_SELECTED?&timingEffLines:&selectedLines;
        linesCenter = effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_SELECTED?&selectedLines:&timingEffLines;
        

        
        if(mode!=SCREEN_L_R_OFF) {
            // Draw Left line
            if(mode==SCREEN_L_R_ON || mode == SCREEN_L_ON)
            {
                if(effectIndex>0)
                {
                    // Draw left line if effect has different start time then previous effect or
                    // previous effect was not selected, or only left was selected
                    if(effectLayer->GetEffect(effectIndex)->GetStartTimeMS() != effectLayer->GetEffect(effectIndex-1)->GetEndTimeMS() ||
                       effectLayer->GetEffect(effectIndex-1)->GetSelected() == EFFECT_NOT_SELECTED ||
                        effectLayer->GetEffect(effectIndex-1)->GetSelected() == EFFECT_LT_SELECTED) {
                        linesLeft->AddVertex(x1, y1);
                        linesLeft->AddVertex(x1, y2);
                    }
                }
                else
                {
                    linesLeft->AddVertex(x1, y1);
                    linesLeft->AddVertex(x1, y2);
                }

                if(element->GetActive() && ri->layerIndex == 0)
                {
                    timingLines.AddVertex(x1,(row+1)*DEFAULT_ROW_HEADING_HEIGHT, c);
                    timingLines.AddVertex(x1,GetSize().y, c);
                }
            }
            // Draw Right line
            if(mode==SCREEN_L_R_ON || mode == SCREEN_R_ON)
            {
                linesRight->AddVertex(x2, y1);
                linesRight->AddVertex(x2, y2);
                if(element->GetActive() && ri->layerIndex == 0)
                {
                    timingLines.AddVertex(x2,(row+1)*DEFAULT_ROW_HEADING_HEIGHT, c);
                    timingLines.AddVertex(x2,GetSize().y, c);
                }
            }
            // Draw horizontal
            if(mode!=SCREEN_L_R_OFF)
            {
                int half_width = (x2-x1)/2;
                linesLeft->AddVertex(x1, y);
                linesLeft->AddVertex(x1+half_width,y);
                linesRight->AddVertex(x1+half_width,y);
                linesRight->AddVertex(x2,y);
                if (effectLayer->GetEffect(effectIndex)->GetEffectName() != "" && (x2-x1) > 20 ) {
                    int max_width = x2-x1-18;
                    int text_width = DrawGLUtils::GetTextWidth(fontSize, effectLayer->GetEffect(effectIndex)->GetEffectName(), factor) + 8;
                    int width = std::min(text_width, max_width);
                    int center = x1 + (x2-x1)/2;
                    int label_start = center - width/2;
                    xlColor* label_color = mLabelColor;
                    if( ri->layerIndex == 0 && element->GetEffectLayerCount() > 1)
                    {
                        label_color = mPhraseColor;
                    }
                    else if( ri->layerIndex == 1 )
                    {
                        label_color = mWordColor;
                    }
                    else if( ri->layerIndex == 2 )
                    {
                        label_color = mPhonemeColor;
                    }
                    textBackgrounds.AddRect(label_start,y1-2,label_start+width,y2+2, *label_color);
                    timingLines.AddLinesRect(label_start-0.4,y1-2-0.4,label_start+width+0.4,y2+2+0.4, *mLabelOutlineColor);
                    texts.AddVertex(label_start + 4, y2 + toffset, effectLayer->GetEffect(effectIndex)->GetEffectName());
                }
            }
        }
    }
}

void EffectsGrid::render( wxPaintEvent& evt )
{
    wxClientDC dc(this);
    Draw();
}

void EffectsGrid::Draw()
{
    if(!mIsInitialized) { InitializeGLCanvas(); }
    if(!IsShownOnScreen()) return;

    SetCurrentGLContext();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if( mWindowResized )
    {
        prepare2DViewport(0,0,mWindowWidth, mWindowHeight);
        if (mTimeline != nullptr) {
            mTimeline->RecalcEndTime();  // force a recalc of the Timeline end time so that timing effect positions will calculate correct during redraw
        }
    }
    if( mSequenceElements )
    {
        DrawLines();
        DrawEffects();
        DrawPlayMarker();
    }

    if((mDragging || mCellRangeSelected) && !mPartialCellSelected)
    {
        if (mSequenceElements->GetSelectedTimingRow() >= 0 && mRangeStartCol >= 0) {
            DrawSelectedCells();
        }
    }
    if(mDragging && (mSequenceElements->GetSelectedTimingRow() == -1))
    {
        DrawGLUtils::DrawRectangle(xlYELLOW,true,mDragStartX,mDragStartY,mDragEndX,mDragEndY);
    }

    SwapBuffers();
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
            xlColor highlight_color;
            highlight_color = *RowHeading::GetTimingColor(mSequenceElements->GetVisibleRowInformation(mSequenceElements->GetSelectedTimingRow())->colorIndex);
            glEnable(GL_BLEND);
            DrawGLUtils::DrawFillRectangle(highlight_color,80,start_x,start_y,end_x-start_x,end_y-start_y+DEFAULT_ROW_HEADING_HEIGHT);
            glDisable(GL_BLEND);
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
    for (int x = 0; x < m_EffectTextures.size(); x++) {
        glDeleteTextures(1,&m_EffectTextures[x]);
    }
    m_EffectTextures.clear();
}

void EffectsGrid::mouseWheelMoved(wxMouseEvent& event)
{
    if(event.CmdDown())
    {
        int i = event.GetWheelRotation();
        if(i<0)
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
    else
    {
        wxPostEvent(GetParent()->GetEventHandler(), event);
        event.Skip();
    }
}

int EffectsGrid::GetRow(int y)
{
    return y/DEFAULT_ROW_HEADING_HEIGHT;
}

void EffectsGrid::RaiseSelectedEffectChanged(Effect* effect, bool isNew)
{
    // Place effect pointer in client data
    wxCommandEvent eventEffectChanged(EVT_SELECTED_EFFECT_CHANGED);
    eventEffectChanged.SetClientData(effect);
    eventEffectChanged.SetInt(isNew);
    wxPostEvent(GetParent(), eventEffectChanged);
}

void EffectsGrid::RaisePlayModelEffect(Element* element, Effect* effect,bool renderEffect)
{
    // Place effect pointer in client data
    wxCommandEvent eventPlayModelEffect(EVT_PLAY_MODEL_EFFECT);
    playArgs->element = element;
    playArgs->effect = effect;
    playArgs->renderEffect = renderEffect;
    eventPlayModelEffect.SetClientData(playArgs);
    wxPostEvent(GetParent(), eventPlayModelEffect);
}

void EffectsGrid::RaiseEffectDropped(int x, int y)
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

Element* EffectsGrid::GetActiveTimingElement()
{
    Element* returnValue=nullptr;
    for(int row=0;row<mSequenceElements->GetVisibleRowInformationSize();row++)
    {
        Element* e = mSequenceElements->GetVisibleRowInformation(row)->element;
        if(e->GetType()== "timing" && e->GetActive())
        {
            returnValue = e;
            break;
        }
    }
    return returnValue;
}

void EffectsGrid::GetRangeOfMovementForSelectedEffects(int &toLeft, int &toRight)
{
    int left,right;
    toLeft = NO_MAX;
    toRight = NO_MAX;
    for(int row=0;row<mSequenceElements->GetVisibleRowInformationSize();row++)
    {
        EffectLayer* el = mSequenceElements->GetVisibleEffectLayer(row);
        el->GetMaximumRangeOfMovementForSelectedEffects(left,right);
        toLeft = toLeft<left?toLeft:left;
        toRight = toRight<right?toRight:right;
    }
}

void EffectsGrid::MoveAllSelectedEffects(int deltaMS, bool offset)
{
    // Tag all selected effects so we don't move them twice
    for(int row=0;row<mSequenceElements->GetVisibleRowInformationSize();row++)
    {
        EffectLayer* el = mSequenceElements->GetVisibleEffectLayer(row);
        el->TagAllSelectedEffects();
    }

    if( !offset ) {
        for(int row=0;row<mSequenceElements->GetVisibleRowInformationSize();row++)
        {
            EffectLayer* el = mSequenceElements->GetVisibleEffectLayer(row);
            el->MoveAllSelectedEffects(deltaMS, mSequenceElements->get_undo_mgr());
        }
    } else {
        int start_row = -1;
        int end_row = -1;
        for(int row=0;row<mSequenceElements->GetVisibleRowInformationSize();row++)
        {
            EffectLayer* el = mSequenceElements->GetVisibleEffectLayer(row);
            if( el->GetSelectedEffectCount() > 0 ) {
                if( start_row == -1 ) {
                    start_row = row;
                } else {
                    end_row = row;
                }
            }
        }
        int delta_step = deltaMS / (end_row-start_row);
        delta_step = mTimeline->RoundToMultipleOfPeriod(delta_step, mSequenceElements->GetFrequency());
        for(int row=start_row;row<=end_row;row++)
        {
            EffectLayer* el = mSequenceElements->GetVisibleEffectLayer(row);
            if( mResizingMode == EFFECT_RESIZE_RIGHT || mResizingMode == EFFECT_RESIZE_MOVE) {
                el->MoveAllSelectedEffects(delta_step*(row-start_row), mSequenceElements->get_undo_mgr());
            } else {
                el->MoveAllSelectedEffects(delta_step*(end_row-row), mSequenceElements->get_undo_mgr());
            }
        }
    }
}

void EffectsGrid::CopyModelEffects(int row_number)
{
    mSequenceElements->UnSelectAllEffects();
    EffectLayer* effectLayer = mSequenceElements->GetVisibleEffectLayer(row_number);
    Effect* effect = effectLayer->GetEffect(0);
    if( effect != NULL )
    {
        mDropStartTimeMS = effect->GetStartTimeMS();
        mRangeStartCol = -1;
        mRangeEndCol = -1;
        mRangeStartRow = -1;
        mRangeEndRow = -1;
        int first_row = mSequenceElements->GetFirstVisibleModelRow();
        mSequenceElements->SelectEffectsInRowAndTimeRange(row_number-first_row,row_number-first_row,mDropStartTimeMS,mSequenceElements->GetSequenceEnd());
        ((MainSequencer*)mParent)->CopySelectedEffects();
        mCanPaste = true;
        effectLayer->UnSelectAllEffects();
        mPartialCellSelected = true;
        mCellRangeSelected = false;
    }
    else
    {
        ((MainSequencer*)mParent)->PanelRowHeadings->SetCanPaste(false);
    }
}

void EffectsGrid::PasteModelEffects(int row_number)
{
    mDropRow = row_number;
    ((MainSequencer*)mParent)->Paste();
    mPartialCellSelected = true;
    ((MainSequencer*)mParent)->PanelRowHeadings->SetCanPaste(true);
}

