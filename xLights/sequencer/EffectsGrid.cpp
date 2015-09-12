#include "EffectsGrid.h"
#include "MainSequencer.h"
#include "TimeLine.h"

#include "wx/wx.h"
#include "wx/sizer.h"
#include "wx/glcanvas.h"
#ifdef __WXMAC__
 #include "OpenGL/glu.h"
 #include "OpenGL/gl.h"
 #include <GLUT/glut.h>
#else
 #include <GL/glu.h>
 #include <GL/gl.h>
 #include <GL/glut.h>
#endif

#include  "RowHeading.h"
#include "../xLightsMain.h"
#include "SequenceElements.h"
#include "Effect.h"
#include "EffectLayer.h"
#include "EffectDropTarget.h"
#include "../DrawGLUtils.h"
#include "RenderCommandEvent.h"
#include "../BitmapCache.h"


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
//EVT_LEAVE_WINDOW(EffectsGrid::mouseLeftWindow)
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
    :xlGLCanvas(parent, id, pos, size, wxFULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN | wxCLIP_SIBLINGS)
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
            selectedEffect->SetEffectName(dlg.GetValue());
        }
        Refresh();
    }

}

void EffectsGrid::rightClick(wxMouseEvent& event)
{
    if (mSequenceElements == NULL) {
        return;
    }

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
    }

    if (mnuLayer != nullptr) {
        mnuLayer->Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EffectsGrid::OnGridPopup, NULL, this);
        Draw();
        PopupMenu(mnuLayer);
    }
}

void EffectsGrid::mouseLeftWindow(wxMouseEvent& event) {}
void EffectsGrid::keyReleased(wxKeyEvent& event){}
void EffectsGrid::keyPressed(wxKeyEvent& event){}

void EffectsGrid::sendRenderEvent(const wxString &model, int start, int end, bool clear) {
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
        ((MainSequencer*)mParent)->DeleteAllSelectedEffects();
    }
    else if(id == ID_GRID_MNU_RANDOM_EFFECTS)
    {
        FillRandomEffects();
    }
    else if(id == ID_GRID_MNU_DELETE)
    {
        ((MainSequencer*)mParent)->DeleteAllSelectedEffects();
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
        if( phrase_effect->GetParentEffectLayer()->GetParentElement()->GetEffectLayerCount() == 1 )
        {
            word_layer = phrase_effect->GetParentEffectLayer()->GetParentElement()->AddEffectLayer();
        }
        else
        {
            word_layer = phrase_effect->GetParentEffectLayer()->GetParentElement()->GetEffectLayer(1);
        }
        mSequenceElements->get_undo_mgr().CreateUndoStep();
        word_layer->UnSelectAllEffects();
        word_layer->SelectEffectsInTimeRange(phrase_effect->GetStartTimeMS(), phrase_effect->GetEndTimeMS());
        word_layer->DeleteSelectedEffects(mSequenceElements->get_undo_mgr());
        mSequenceElements->BreakdownPhrase(word_layer, phrase_effect->GetStartTimeMS(), phrase_effect->GetEndTimeMS(), phrase_effect->GetEffectName());
        phrase_effect->GetParentEffectLayer()->GetParentElement()->SetCollapsed(false);
        wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
        wxPostEvent(mParent, eventRowHeaderChanged);
    }
    else if(id == ID_GRID_MNU_BREAKDOWN_WORD)
    {
        Effect* word_effect = mSelectedEffect;
        EffectLayer* phoneme_layer;
        if( word_effect->GetParentEffectLayer()->GetParentElement()->GetEffectLayerCount() < 3 )
        {
            phoneme_layer = word_effect->GetParentEffectLayer()->GetParentElement()->AddEffectLayer();
        }
        else
        {
            phoneme_layer = word_effect->GetParentEffectLayer()->GetParentElement()->GetEffectLayer(2);
        }
        mSequenceElements->get_undo_mgr().CreateUndoStep();
        phoneme_layer->UnSelectAllEffects();
        phoneme_layer->SelectEffectsInTimeRange(word_effect->GetStartTimeMS(), word_effect->GetEndTimeMS());
        phoneme_layer->DeleteSelectedEffects(mSequenceElements->get_undo_mgr());
        mSequenceElements->BreakdownWord(phoneme_layer, word_effect->GetStartTimeMS(), word_effect->GetEndTimeMS(), word_effect->GetEffectName());
        word_effect->GetParentEffectLayer()->GetParentElement()->SetCollapsed(false);
        wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
        wxPostEvent(mParent, eventRowHeaderChanged);
    }
    else if(id == ID_GRID_MNU_BREAKDOWN_WORDS)
    {
        Effect* word_effect = mSelectedEffect;
        EffectLayer* phoneme_layer;
        if( word_effect->GetParentEffectLayer()->GetParentElement()->GetEffectLayerCount() < 3 )
        {
            phoneme_layer = word_effect->GetParentEffectLayer()->GetParentElement()->AddEffectLayer();
        }
        else
        {
            phoneme_layer = word_effect->GetParentEffectLayer()->GetParentElement()->GetEffectLayer(2);
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
        word_effect->GetParentEffectLayer()->GetParentElement()->SetCollapsed(false);
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
                                                  0,
                                                  "Random",
                                                  "",
                                                  "",
                                                  eff->GetStartTimeMS(),
                                                  eff->GetEndTimeMS(),
                                                  EFFECT_SELECTED,
                                                  false);
                        mSequenceElements->get_undo_mgr().CaptureAddedEffect( effectLayer->GetParentElement()->GetName(), effectLayer->GetIndex(), ef->GetID() );
                        RaiseSelectedEffectChanged(ef);
                        mSelectedEffect = ef;
                    }
                }
            }
            mCellRangeSelected = false;
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

        if( selectedEffect != nullptr )
        {
            mEffectLayer = mSequenceElements->GetVisibleEffectLayer(row);
            Element* element = mEffectLayer->GetParentElement();

            mSelectedEffect = selectedEffect;
            if(element->GetType()=="model")
            {
                mSelectedRow = row;
                mSelectedEffect = selectedEffect;
                RaiseSelectedEffectChanged(mSelectedEffect);
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
            EffectLayer* el = mSequenceElements->GetVisibleRowInformation(row)->element->GetEffectLayer(mSequenceElements->GetVisibleRowInformation(row)->layerIndex);
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
    if( !MultipleEffectsSelected() && mSelectedEffect != nullptr )
    {
        Row_Information_Struct *ri = mSequenceElements->GetVisibleRowInformation(mSelectedRow);
        int row = mSelectedRow-1;
        EffectLayer* el = mSelectedEffect->GetParentEffectLayer();
        int layer_index = ri->layerIndex;
        while( layer_index > 0 )
        {
            EffectLayer* new_el = mSequenceElements->GetVisibleEffectLayer(row);
            if( new_el->GetRangeIsClearMS( mSelectedEffect->GetStartTimeMS(), mSelectedEffect->GetEndTimeMS()))
            {
                mSequenceElements->get_undo_mgr().CreateUndoStep();
                Effect* ef = new_el->AddEffect(0,
                                               mSelectedEffect->GetEffectIndex(),
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
            layer_index--;
        }
    }
    else if( mCellRangeSelected )
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
}

void EffectsGrid::MoveSelectedEffectDown(bool shift)
{
    int first_row = mSequenceElements->GetFirstVisibleModelRow();
    if( !MultipleEffectsSelected() && mSelectedEffect != nullptr )
    {
        Row_Information_Struct *ri = mSequenceElements->GetVisibleRowInformation(mSelectedRow);
        int row = mSelectedRow+1;
        EffectLayer* el = mSelectedEffect->GetParentEffectLayer();
        Element* element = el->GetParentElement();
        int layer_index = ri->layerIndex;
        while( layer_index < element->GetEffectLayerCount()-1 )
        {
            EffectLayer* new_el = mSequenceElements->GetVisibleEffectLayer(row);
            if( new_el->GetRangeIsClearMS( mSelectedEffect->GetStartTimeMS(), mSelectedEffect->GetEndTimeMS()))
            {
                mSequenceElements->get_undo_mgr().CreateUndoStep();
                Effect* ef = new_el->AddEffect(0,
                                               mSelectedEffect->GetEffectIndex(),
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
            layer_index++;
        }
    }
    else if( mCellRangeSelected )
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
}

void EffectsGrid::MoveSelectedEffectRight(bool shift)
{
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
}

void EffectsGrid::MoveSelectedEffectLeft(bool shift)
{
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

void EffectsGrid::Paste(const wxString &data) {
    if (mPartialCellSelected || OneCellSelected()) {
        wxArrayString all_efdata = wxSplit(data, '\n');
        if (all_efdata.size() == 0) {
            return;
        }
        if( all_efdata.size() > 2 )  // multi-effect paste
        {
            wxArrayString eff1data = wxSplit(all_efdata[0], '\t');
            int drop_time_offset, new_start_time, new_end_time, column_start_time;
            column_start_time = wxAtoi(eff1data[6]);
            drop_time_offset = wxAtoi(eff1data[3]);
            EffectLayer* tel = mSequenceElements->GetVisibleEffectLayer(mSequenceElements->GetSelectedTimingRow());
            if( column_start_time < 0 || tel == nullptr)
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
            for( int i = 0; i < all_efdata.size()-1; i++ )
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
                if( row_info == nullptr ) break;
                Element* elem = row_info->element;
                if( elem == nullptr ) break;
                EffectLayer* el = mSequenceElements->GetEffectLayer(row_info);
                if( el == nullptr ) break;
                if( el->GetRangeIsClearMS(new_start_time, new_end_time) )
                {
                    int effectIndex = Effect::GetEffectIndex(efdata[0]);
                    if (effectIndex >= 0) {
                        Effect* ef = el->AddEffect(0,
                                      effectIndex,
                                      efdata[0],
                                      efdata[1],
                                      efdata[2],
                                      new_start_time,
                                      new_end_time,
                                      EFFECT_NOT_SELECTED,
                                      false);
                        mSequenceElements->get_undo_mgr().CaptureAddedEffect( el->GetParentElement()->GetName(), el->GetIndex(), ef->GetID() );
                        if (!ef->GetPaletteMap().empty()) {
                            sendRenderEvent(el->GetParentElement()->GetName(),
                                            new_start_time ,
                                            new_end_time, true);
                        }
                    }
                }
            }
            mPartialCellSelected = false;
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
                EffectLayer* el = mSequenceElements->GetVisibleEffectLayer(mDropRow);
                int effectIndex = Effect::GetEffectIndex(efdata[0]);
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
                                      effectIndex,
                                      efdata[0],
                                      efdata[1],
                                      efdata[2],
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
                        RaiseSelectedEffectChanged(ef);
                        mSelectedEffect = ef;
                        mPartialCellSelected = false;
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
                        int effectIndex = Effect::GetEffectIndex(efdata[0]);
                        if (effectIndex >= 0) {
                            Effect* ef = el->AddEffect(0,
                                      effectIndex,
                                      efdata[0],
                                      efdata[1],
                                      efdata[2],
                                      start_time,
                                      end_time,
                                      EFFECT_SELECTED,
                                      false);
                            mSequenceElements->get_undo_mgr().CaptureAddedEffect( el->GetParentElement()->GetName(), el->GetIndex(), ef->GetID() );
                            RaiseSelectedEffectChanged(ef);
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
        int midpoint = mEffectLayer->GetEffect(mResizeEffectIndex)->GetStartTimeMS() +
                            (mEffectLayer->GetEffect(mResizeEffectIndex)->GetEndTimeMS() - mEffectLayer->GetEffect(mResizeEffectIndex)->GetStartTimeMS()) / 2;
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
        int minimumTime = mEffectLayer->GetMinimumStartTimeMS(mResizeEffectIndex, mResizingMode==EFFECT_RESIZE_LEFT);
        // User has dragged left side to the right side exit
        if (time >= mEffectLayer->GetEffect(mResizeEffectIndex)->GetEndTimeMS())
        {
            return;
        }
        else if (time >= minimumTime  || minimumTime == NO_MIN_MAX_TIME)
        {
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
        int maximumTime = mEffectLayer->GetMaximumEndTimeMS(mResizeEffectIndex, mResizingMode==EFFECT_RESIZE_RIGHT);
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
        int num_selected = mSequenceElements->SelectEffectsInRowAndColumnRange(start_row, end_row, start_col, end_col);
        if( num_selected != 1 )  // we don't know what to preview unless only 1 effect is selected
        {
            wxCommandEvent eventUnSelected(EVT_UNSELECTED_EFFECT);
            wxPostEvent(mParent, eventUnSelected);
        }
    }
}

void EffectsGrid::ForceRefresh()
{
    Refresh();
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
    if(!IsShownOnScreen()) return;
    SetCurrentGLContext();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Black Background
    glDisable(GL_TEXTURE_2D);   // textures
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    prepare2DViewport(0,0,mWindowWidth, mWindowHeight);
    glLoadIdentity();
    CreateEffectIconTextures();
    mIsInitialized = true;
}

void EffectsGrid::DrawHorizontalLines()
{
    // Draw Horizontal lines
    int x1=1;
    int x2 = mWindowWidth-1;
    int y;
    bool isEvenLayer=false;

    glEnable(GL_BLEND);
    glColor4ub(100,100,100,5);
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
                DrawGLUtils::DrawFillRectangle(xlLIGHT_GREY,40,x1,y,x2,h);
            }
            isEvenLayer = !isEvenLayer;
        } else if (ri->strandIndex != -1) {
            if (isEvenLayer)
            {
                DrawGLUtils::DrawFillRectangle(xlLIGHT_GREY,40,x1,y,x2,DEFAULT_ROW_HEADING_HEIGHT);
            }
            isEvenLayer = !isEvenLayer;
        }
    }
    glDisable(GL_BLEND);

    for(int row=0;row < mSequenceElements->GetVisibleRowInformationSize();row++)
    {
        y = (row+1)*DEFAULT_ROW_HEADING_HEIGHT;
        DrawGLUtils::DrawLine(*mGridlineColor,255,x1,y,x2,y,.2);
    }

}

void EffectsGrid::DrawVerticalLines()
{
    // Draw vertical lines
    int y1 = 0;
    int y2 = mWindowHeight-1;
    for(int x1=0;x1<mWindowWidth;x1++)
    {
        // Draw hash marks
        if ((x1+mStartPixelOffset)%(PIXELS_PER_MAJOR_HASH)==0)
        {
            DrawGLUtils::DrawLine(*mGridlineColor,255,x1,y1,x1,y2,.2);
        }
    }
}

void EffectsGrid::DrawEffects()
{
    for(int row=0;row<mSequenceElements->GetVisibleRowInformationSize();row++)
    {
        wxString type = mSequenceElements->GetVisibleRowInformation(row)->element->GetType();
        wxString name = mSequenceElements->GetVisibleRowInformation(row)->element->GetName();
        if(type=="view" || type == "model")
        {
            DrawModelOrViewEffects(row);
        }
    }
}

void EffectsGrid::DrawTimings()
{
    for(int row=0;row<mSequenceElements->GetVisibleRowInformationSize();row++)
    {
        wxString type = mSequenceElements->GetVisibleRowInformation(row)->element->GetType();
        if(type!="view" && type != "model")
        {
            DrawTimingEffects(row);
        }
    }
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

void GetOnEffectColors(const Effect *e, xlColor &start, xlColor &end) {
    int starti = wxAtoi(e->GetSettings().Get("E_TEXTCTRL_Eff_On_Start", "100"));
    int endi = wxAtoi(e->GetSettings().Get("E_TEXTCTRL_Eff_On_End", "100"));
    xlColor newcolor;
    newcolor = e->GetPalette()[0];
    if (starti == 100 && endi == 100) {
        start = end = newcolor;
    } else {
        wxImage::HSVValue hsv = newcolor.asHSV();
        hsv.value = (hsv.value * starti) / 100;
        start = hsv;
        hsv = newcolor.asHSV();
        hsv.value = (hsv.value * endi) / 100;
        end = hsv;
    }
}

void GetMorphEffectColors(const Effect *e, xlColor &start_h, xlColor &end_h, xlColor &start_t, xlColor &end_t) {
    int useHeadStart = wxAtoi(e->GetSettings().Get("E_CHECKBOX_MorphUseHeadStartColor", "0"));
    int useTailStart = wxAtoi(e->GetSettings().Get("E_CHECKBOX_MorphUseHeadEndColor", "0"));

    int hcols = 0, hcole = 1;
    int tcols = 2, tcole = 3;
    switch (e->GetPalette().size()) {
        case 1:  //one color selected, use it for all
            hcols = hcole = tcols = tcole = 0;
            break;
        case 2: //two colors, head/tail
            hcols = hcole = 0;
            tcols = tcole = 1;
            break;
        case 3: //three colors, head /tail start/end
            hcols = hcole = 0;
            tcols = 1;
            tcole = 2;
            break;
    }

    if( useHeadStart > 0 )
    {
        tcols = hcols;
    }

    if( useTailStart > 0 )
    {
        tcole = hcole;
    }

    start_h = e->GetPalette()[hcols];
    end_h = e->GetPalette()[hcole];
    start_t = e->GetPalette()[tcols];
    end_t = e->GetPalette()[tcole];
}

int EffectsGrid::DrawEffectBackground(const Row_Information_Struct* ri, const Effect *e, int x1, int y1, int x2, int y2) {
    if (e->GetPalette().size() == 0) {
        //if there are no colors selected, none of the "backgrounds" make sense.  Don't draw
        //the background and instead make sure the icon is displayed to the user knows they
        //need to make some decisions about the colors to be used.
        return 1;
    }

    //some effects might have pre-rendered display lists.  Use those before dropping to the generic routines
    switch (e->GetEffectIndex()) {
        case BitmapCache::eff_ON:
        case BitmapCache::eff_COLORWASH:
        case BitmapCache::eff_SINGLESTRAND: {
            if (e->HasBackgroundDisplayList()) {
                DrawGLUtils::DrawDisplayList(x1, y1, x2-x1, y2-y1, e->GetBackgroundDisplayList());
                return e->GetBackgroundDisplayList().iconSize;
            }
        }
    }

    //haven't rendered an effect background yet, use a default redering mechanism
    switch (e->GetEffectIndex()) {
        case BitmapCache::eff_ON: {
            xlColor start;
            xlColor end;
            GetOnEffectColors(e, start, end);
            DrawGLUtils::DrawHBlendedRectangle(start, end, x1, y1, x2, y2);
            return 2;
        }
        break;
        case BitmapCache::eff_COLORWASH:
        case BitmapCache::eff_SHOCKWAVE: {
            DrawGLUtils::DrawHBlendedRectangle(e->GetPalette(), x1, y1, x2, y2);
            return 2;
        }
        break;
        case BitmapCache::eff_MORPH: {
            int head_duration = wxAtoi(e->GetSettings().Get("E_SLIDER_MorphDuration", "20"));
            xlColor start_h;
            xlColor end_h;
            xlColor start_t;
            xlColor end_t;
            GetMorphEffectColors(e, start_h, end_h, start_t, end_t);
            int x_mid = (int)((float)(x2-x1) * (float)head_duration / 100.0) + x1;
            DrawGLUtils::DrawHBlendedRectangle(start_h, end_h, x1, y1+1, x_mid, y2-1);
            if(e->GetPalette().size() <= 4) {
                DrawGLUtils::DrawHBlendedRectangle(start_t, end_t, x_mid, y1+4, x2, y2-4);
            }
            else {
                DrawGLUtils::DrawHBlendedRectangle(e->GetPalette(), x_mid, y1+4, x2, y2-4, 2);
            }
            return 0;
        }
        break;
        case BitmapCache::eff_GALAXY: {
            int head_duration = wxAtoi(e->GetSettings().Get("E_SLIDER_Galaxy_Duration", "20"));
            int num_colors = e->GetPalette().size();
            xlColor head_color = e->GetPalette()[0];
            int x_mid = (int)((float)(x2-x1) * (float)head_duration / 100.0) + x1;
            if( x_mid > x1 )
            {
                DrawGLUtils::DrawHBlendedRectangle(head_color, head_color, x1, y1+1, x_mid, y2-1);
            }
            int color_length = (x2 - x_mid) / num_colors;
            for(int i = 0; i < num_colors; i++ )
            {
                int cx1 = x_mid + (i*color_length);
                if( i == (num_colors-1) ) // fix any roundoff error for last color
                {
                    DrawGLUtils::DrawHBlendedRectangle(e->GetPalette()[i], e->GetPalette()[i], cx1, y1+4, x2, y2-4);
                }
                else
                {
                    DrawGLUtils::DrawHBlendedRectangle(e->GetPalette()[i], e->GetPalette()[i+1], cx1, y1+4, cx1+color_length, y2-4);
                }
            }
            return 0;
        }
        case BitmapCache::eff_FAN: {
            int head_duration = wxAtoi(e->GetSettings().Get("E_SLIDER_Fan_Duration", "50"));
            int num_colors = e->GetPalette().size();
            xlColor head_color = e->GetPalette()[0];
            int x_mid = (int)((float)(x2-x1) * (float)head_duration / 100.0) + x1;
            int head_length;
            int color_length;
            if( num_colors > 1 )
            {
                head_length = (x_mid - x1) / (num_colors-1);
                color_length = (x2 - x_mid) / (num_colors-1);
            }
            else
            {
                head_length = (x_mid - x1);
                color_length = (x2 - x_mid);
            }
            for(int i = 0; i < num_colors; i++ )
            {
                int cx = x1 + (i*head_length);
                int cx1 = x_mid + (i*color_length);
                if( i == (num_colors-1) ) // fix any roundoff error for last color
                {
                    DrawGLUtils::DrawHBlendedRectangle(e->GetPalette()[i], e->GetPalette()[i], cx, y1+1, x_mid, y2-1);
                    DrawGLUtils::DrawHBlendedRectangle(e->GetPalette()[i], e->GetPalette()[i], cx1, y1+4, x2, y2-4);
                }
                else
                {
                    DrawGLUtils::DrawHBlendedRectangle(e->GetPalette()[i], e->GetPalette()[i+1], cx, y1+1, cx+head_length, y2-1);
                    DrawGLUtils::DrawHBlendedRectangle(e->GetPalette()[i], e->GetPalette()[i+1], cx1, y1+4, cx1+color_length, y2-4);
                }
            }
            return 0;
        }
        break;
        default: {}
    }
    return 1;
}

void EffectsGrid::DrawModelOrViewEffects(int row)
{
    Row_Information_Struct *ri = mSequenceElements->GetVisibleRowInformation(row);
    EffectLayer* effectLayer = mSequenceElements->GetEffectLayer(ri);
    xlColor* mEffectColorRight;
    xlColor* mEffectColorLeft;
    xlColor* mEffectColorCenter;
    int y1 = (row*DEFAULT_ROW_HEADING_HEIGHT)+2;
    int y2 = ((row+1)*DEFAULT_ROW_HEADING_HEIGHT)-2;
    int y = (row*DEFAULT_ROW_HEADING_HEIGHT) + (DEFAULT_ROW_HEADING_HEIGHT/2);

    if (mGridNodeValues && ri->nodeIndex != -1) {
        std::vector<xlColor> colors;
        std::vector<double> xs;
        PixelBufferClass ncls;
        ncls.InitNodeBuffer(*xlights->GetModelClass(ri->element->GetName()), ri->strandIndex, ri->nodeIndex, seqData->FrameTime());
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
        DrawGLUtils::DrawRectangleArray((row*DEFAULT_ROW_HEADING_HEIGHT)+3,
                                        ((row+1)*DEFAULT_ROW_HEADING_HEIGHT)-3,
                                        mTimeline->GetPositionFromTimeMS(0), xs, colors);
    }

    for(int effectIndex=0;effectIndex < effectLayer->GetEffectCount();effectIndex++)
    {
        Effect* e = effectLayer->GetEffect(effectIndex);
        EFFECT_SCREEN_MODE mode;

        int x1,x2,x3,x4;
        mTimeline->GetPositionsFromTimeRange(effectLayer->GetEffect(effectIndex)->GetStartTimeMS(),
                                             effectLayer->GetEffect(effectIndex)->GetEndTimeMS(),mode,x1,x2,x3,x4);
        int x = x2-x1;
        // Draw Left line
        mEffectColorLeft = effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_NOT_SELECTED ||
                           effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_RT_SELECTED?mEffectColor:mSelectionColor;
        mEffectColorRight = effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_NOT_SELECTED ||
                           effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_LT_SELECTED?mEffectColor:mSelectionColor;
        mEffectColorCenter = effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_SELECTED?mSelectionColor:mEffectColor;

        int drawIcon = 1;
        if(mGridIconBackgrounds && (ri->nodeIndex == -1 || !mGridNodeValues))
        {
            drawIcon = DrawEffectBackground(ri, e, x3, y1, x4, y2);
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
                        DrawGLUtils::DrawLine(*mEffectColorLeft,255,x1,y1,x1,y2,2);
                    }
                }
                else
                {
                    DrawGLUtils::DrawLine(*mEffectColorLeft,255,x1,y1,x1,y2,2);
                }
            }

            // Draw Right line
            if(mode==SCREEN_L_R_ON || mode == SCREEN_R_ON)
            {
                DrawGLUtils::DrawLine(*mEffectColorRight,255,x2,y1,x2,y2,2);
            }

            // Draw horizontal
            if(mode!=SCREEN_L_R_OFF)
            {
                if (drawIcon) {
                    if(x > (DEFAULT_ROW_HEADING_HEIGHT + 4)) {
                        double sz = (DEFAULT_ROW_HEADING_HEIGHT - 6.0) / (2.0 * drawIcon) + 1.0;

                        double xl = (x1+x2)/2.0-sz;
                        double xr = (x1+x2)/2.0+sz;
                        DrawGLUtils::DrawTexture(&m_EffectTextures[e->GetEffectIndex()],
                                                 xl,y-sz,
                                                 xr,y+sz);
                        DrawGLUtils::DrawLine(*mEffectColorLeft,255,x1,y,(x1+x2)/2.0-sz,y,1);
                        DrawGLUtils::DrawLine(*mEffectColorRight,255,(x1+x2)/2.0+sz,y,x2,y,1);
                        DrawGLUtils::DrawRectangle(*mEffectColor,false,
                                                   xl,y-sz,
                                                   xr,y+sz);
                    }
                    else if (x > MINIMUM_EFFECT_WIDTH_FOR_SMALL_RECT)
                    {
                        DrawGLUtils::DrawLine(*mEffectColorLeft,255,x1,y,x1+(x/2)-1,y,1);
                        DrawGLUtils::DrawLine(*mEffectColorRight,255,x1+(x/2)+1,y,x2,y,1);
                        DrawGLUtils::DrawRectangle(*mEffectColor,false,x1+(x/2)-1,y-1,x1+(x/2)+1,y+1);
                    }
                    else
                    {
                        DrawGLUtils::DrawLine(*mEffectColorCenter,255,x1,y,x2,y,1);
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
        glEnable(GL_BLEND);
        DrawGLUtils::DrawFillRectangle(highlight_color,80,mDropStartX,y3,mDropEndX-mDropStartX,DEFAULT_ROW_HEADING_HEIGHT);
        glDisable(GL_BLEND);
    }
}

void EffectsGrid::DrawTimingEffects(int row)
{
    Row_Information_Struct *ri = mSequenceElements->GetVisibleRowInformation(row);
    Element* element =ri->element;
    EffectLayer* effectLayer=mSequenceElements->GetVisibleEffectLayer(row);
    xlColor* mEffectColorRight;
    xlColor* mEffectColorLeft;
    xlColor* mEffectColorCenter;
    //if(effectLayer==nullptr)
    //    return;
    for(int effectIndex=0;effectIndex < effectLayer->GetEffectCount();effectIndex++)
    {
        EFFECT_SCREEN_MODE mode = SCREEN_L_R_OFF;

        int y1 = (row*DEFAULT_ROW_HEADING_HEIGHT)+4;
        int y2 = ((row+1)*DEFAULT_ROW_HEADING_HEIGHT)-4;
        int y = (row*DEFAULT_ROW_HEADING_HEIGHT) + (DEFAULT_ROW_HEADING_HEIGHT / 2.0);
        int x1,x2,x3,x4;

        mTimeline->GetPositionsFromTimeRange(effectLayer->GetEffect(effectIndex)->GetStartTimeMS(),
                                             effectLayer->GetEffect(effectIndex)->GetEndTimeMS(),mode,x1,x2,x3,x4);
        mEffectColorLeft = effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_NOT_SELECTED ||
                           effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_RT_SELECTED?mTimingColor:mSelectionColor;
        mEffectColorRight = effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_NOT_SELECTED ||
                           effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_LT_SELECTED?mTimingColor:mSelectionColor;
        mEffectColorCenter = effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_SELECTED?mSelectionColor:mTimingColor;

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
                        effectLayer->GetEffect(effectIndex-1)->GetSelected() == EFFECT_LT_SELECTED)
                    {
                        DrawGLUtils::DrawLine(*mEffectColorLeft,255,x1,y1,x1,y2,2);
                    }
                }
                else
                {
                    DrawGLUtils::DrawLine(*mEffectColorLeft,255,x1,y1,x1,y2,2);
                }

                if(element->GetActive() && ri->layerIndex == 0)
                {
                    glEnable(GL_BLEND);
                    DrawGLUtils::DrawLine(*RowHeading::GetTimingColor(ri->colorIndex),128,x1,(row+1)*DEFAULT_ROW_HEADING_HEIGHT,x1,GetSize().y,1);
                    glDisable(GL_BLEND);
                }
            }
            // Draw Right line
            if(mode==SCREEN_L_R_ON || mode == SCREEN_R_ON)
            {
                DrawGLUtils::DrawLine(*mEffectColorRight,255,x2,y1,x2,y2,2);
                if(element->GetActive() && ri->layerIndex == 0)
                {
                    glEnable(GL_BLEND);
                    DrawGLUtils::DrawLine(*RowHeading::GetTimingColor(ri->colorIndex),128,x2,(row+1)*DEFAULT_ROW_HEADING_HEIGHT,x2,GetSize().y,1);
                    glDisable(GL_BLEND);
                }
            }
            // Draw horizontal
            if(mode!=SCREEN_L_R_OFF)
            {
                int half_width = (x2-x1)/2;
                DrawGLUtils::DrawLine(*mEffectColorLeft,255,x1,y,x1+half_width,y,2);
                DrawGLUtils::DrawLine(*mEffectColorRight,255,x1+half_width,y,x2,y,2);
                if (effectLayer->GetEffect(effectIndex)->GetEffectName() != "" && (x2-x1) > 20 ) {
                    double fontSize = DEFAULT_ROW_HEADING_HEIGHT - 10;
                    double factor = translateToBacking(1.0);
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
                    DrawGLUtils::DrawFillRectangle(*label_color,80,label_start,y1-2,width,y2-y1+4);
                    DrawGLUtils::DrawRectangle(*mLabelOutlineColor,false,label_start,y1-2,label_start + width,y2+2);
                    DrawGLUtils::DrawText(label_start + 4, y2-3, fontSize, effectLayer->GetEffect(effectIndex)->GetEffectName(), factor);
                }
            }
        }
    }
}

void EffectsGrid::render( wxPaintEvent& evt )
{
    Draw();
}

void EffectsGrid::Draw()
{
    if(!mIsInitialized) { InitializeGLCanvas(); }
    if(!IsShownOnScreen()) return;

    SetCurrentGLContext();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    wxClientDC(this);
    if( mWindowResized )
    {
        prepare2DViewport(0,0,mWindowWidth, mWindowHeight);
    }
    if( mSequenceElements )
    {
        DrawHorizontalLines();
        DrawVerticalLines();
        DrawEffects();
        DrawTimings();
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

    glFlush();
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
        int start_x = mTimeline->GetPositionFromTimeMS(tel->GetEffect(start_col)->GetStartTimeMS())+1;
        int end_x = mTimeline->GetPositionFromTimeMS(tel->GetEffect(end_col)->GetEndTimeMS())-1;
        int start_y = (start_row-first_row)*DEFAULT_ROW_HEADING_HEIGHT;
        int end_y = (end_row-first_row)*DEFAULT_ROW_HEADING_HEIGHT;
        xlColor highlight_color;
        highlight_color = *RowHeading::GetTimingColor(mSequenceElements->GetVisibleRowInformation(mSequenceElements->GetSelectedTimingRow())->colorIndex);
        glEnable(GL_BLEND);
        DrawGLUtils::DrawFillRectangle(highlight_color,80,start_x,start_y,end_x-start_x,end_y-start_y+DEFAULT_ROW_HEADING_HEIGHT);
        glDisable(GL_BLEND);
    }
}

void EffectsGrid::CreateEffectIconTextures()
{
    for(int effectID=0;effectID<BitmapCache::eff_LASTEFFECT;effectID++)
    {
        wxString tooltip;
        DrawGLUtils::CreateOrUpdateTexture(BitmapCache::GetEffectIcon(effectID, tooltip, 64, true),
                                           BitmapCache::GetEffectIcon(effectID, tooltip, 32, true),
                                           BitmapCache::GetEffectIcon(effectID, tooltip, 16, true),
                                           &m_EffectTextures[effectID]);
    }
}

void EffectsGrid::DeleteEffectIconTextures()
{
    for(int effectID=0;effectID<BitmapCache::eff_LASTEFFECT;effectID++)
    {
        glDeleteTextures(1,&m_EffectTextures[effectID]);
    }
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

void EffectsGrid::RaiseSelectedEffectChanged(Effect* effect)
{
    // Place effect pointer in client data
    wxCommandEvent eventEffectChanged(EVT_SELECTED_EFFECT_CHANGED);
    eventEffectChanged.SetClientData(effect);
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
    if( !offset ) {
        for(int row=0;row<mSequenceElements->GetVisibleRowInformationSize();row++)
        {
            EffectLayer* el = mSequenceElements->GetVisibleEffectLayer(row);
            el->MoveAllSelectedEffects(deltaMS, mSequenceElements->get_undo_mgr());
        }
    } else {
        int num_rows_with_selections = 0;
        int start_row = -1;
        int end_row = -1;
        for(int row=0;row<mSequenceElements->GetVisibleRowInformationSize();row++)
        {
            EffectLayer* el = mSequenceElements->GetVisibleEffectLayer(row);
            if( el->GetSelectedEffectCount() > 0 ) {
                num_rows_with_selections++;
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


