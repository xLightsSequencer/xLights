#include "EffectsGrid.h"
#include "MainSequencer.h"
#include "TimeLine.h"

#include "wx/wx.h"
#include "wx/sizer.h"
#include "wx/glcanvas.h"
#ifdef __WXMAC__
 #include "OpenGL/glu.h"
 #include "OpenGL/gl.h"
#else
 #include <GL/glu.h>
 #include <GL/gl.h>
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


BEGIN_EVENT_TABLE(EffectsGrid, xlGLCanvas)
EVT_MOTION(EffectsGrid::mouseMoved)
EVT_MOUSEWHEEL(EffectsGrid::mouseWheelMoved)
EVT_LEFT_DOWN(EffectsGrid::mouseDown)
EVT_LEFT_UP(EffectsGrid::mouseReleased)
EVT_MOUSE_CAPTURE_LOST(EffectsGrid::OnLostMouseCapture)
EVT_RIGHT_DOWN(EffectsGrid::rightClick)
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

    SetBackgroundStyle(wxBG_STYLE_CUSTOM);

    mEffectColor = new xlColor(192,192,192);
    mGridlineColor = new xlColor(40,40,40);

    mTimingColor = new xlColor(255,255,255);
    mTimingVerticalLine = new xlColor(130,178,207);
    mSelectionColor = new xlColor(255,0,255);

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

void EffectsGrid::rightClick(wxMouseEvent& event)
{
    wxMenu *mnuLayer;
    mSelectedRow = event.GetY()/DEFAULT_ROW_HEADING_HEIGHT;
    if (mSelectedRow >= mSequenceElements->GetRowInformationSize()) {
        return;
    }

    Row_Information_Struct *ri =  mSequenceElements->GetRowInformation(mSelectedRow);
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
        wxMenuItem* menu_random = mnuLayer->Append(ID_GRID_MNU_RANDOM_EFFECTS,"Create Random Effects");
        if( !(mCellRangeSelected || mPartialCellSelected) ) {
            menu_random->Enable(false);
        }
    }
    else if (element->GetType()=="timing")
    {
        mnuLayer = new wxMenu();
    }

    mnuLayer->Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EffectsGrid::OnGridPopup, NULL, this);
    Draw();
    PopupMenu(mnuLayer);
}

void EffectsGrid::mouseLeftWindow(wxMouseEvent& event) {}
void EffectsGrid::keyReleased(wxKeyEvent& event){}
void EffectsGrid::keyPressed(wxKeyEvent& event){}

void EffectsGrid::sendRenderEvent(const wxString &model, double start, double end, bool clear) {
    RenderCommandEvent event(model, start, end, clear, false);
    wxPostEvent(mParent, event);
}

void EffectsGrid::OnGridPopup(wxCommandEvent& event)
{
    Row_Information_Struct *ri = mSequenceElements->GetRowInformation(mSelectedRow);
    Element* element = ri->element;
    int layer_index = ri->layerIndex;
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

    Refresh();
}

void EffectsGrid::FillRandomEffects()
{
    int row1 = GetRow(mDragStartY);
    int row2 = GetRow(mDragEndY);
    if( row1 > row2 ) {
        std::swap(row1, row2);
    }
    int selected_timing_row = mSequenceElements->GetSelectedTimingRow();
    if (selected_timing_row >= 0 &&
        row1 < mSequenceElements->GetRowInformationSize() &&
        row2 < mSequenceElements->GetRowInformationSize() ) {
        EffectLayer* tel = mSequenceElements->GetEffectLayer(selected_timing_row);
        int selectionType;
        int start_x = mDragStartX;
        int end_x = mDragEndX;
        if( start_x > end_x ) {
            std::swap(start_x, end_x);
        }
        int timingIndex1 = tel->GetEffectIndexThatContainsPosition(start_x,selectionType);
        int timingIndex2 = tel->GetEffectIndexThatContainsPosition(end_x,selectionType);
        if (timingIndex1 != -1 && timingIndex2 != -1) {
            for( int row = row1; row <= row2; row++)
            {
                EffectLayer* effectLayer = mSequenceElements->GetEffectLayer(row);
                for(int i = timingIndex1; i <= timingIndex2; i++)
                {
                    Effect* eff = tel->GetEffect(i);
                    if( effectLayer->GetRangeIsClear(eff->GetStartTime(), eff->GetEndTime()) )
                    {
                        Effect* ef = effectLayer->AddEffect(0,
                                                  0,
                                                  "Random",
                                                  "",
                                                  "",
                                                  eff->GetStartTime(),
                                                  eff->GetEndTime(),
                                                  EFFECT_SELECTED,
                                                  false);
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
    if( !el->GetRangeIsClear(mDropStartX, mDropEndX) )
    {
        Effect* before_eff = el->GetEffectBeforeEmptySpace(x);
        if( before_eff != nullptr )
        {
            if( before_eff->GetEndPosition() > mDropStartX ) {
                mDropStartX = before_eff->GetEndPosition();
                mDropStartTime = before_eff->GetEndTime();
                return_val = true;
            }
        }
        Effect* after_eff = el->GetEffectAfterEmptySpace(x);
        if( after_eff != nullptr )
        {
            if( after_eff->GetStartPosition() < mDropEndX ) {
                mDropEndX = after_eff->GetStartPosition();
                mDropEndTime = after_eff->GetStartTime();
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
    if( row < mSequenceElements->GetRowInformationSize() )
    {
        int effectIndex;
        int selectionType;

        Effect* selectedEffect = mSequenceElements->GetSelectedEffectAtRowAndPosition(row,x,effectIndex,selectionType);
        if(selectedEffect != nullptr)
        {
            mDragDropping = true;
            mDropStartX = selectedEffect->GetStartPosition();
            mDropEndX = selectedEffect->GetEndPosition();
            mDropStartTime = selectedEffect->GetStartTime();
            mDropEndTime = selectedEffect->GetEndTime();
            mDropRow = row;
        }
        else
        {
            int selectedTimingIndex = mSequenceElements->GetSelectedTimingRow();
            if(selectedTimingIndex >= 0)
            {
                EffectLayer* tel = mSequenceElements->GetEffectLayer(selectedTimingIndex);
                int selectionType;
                int timingIndex = tel->GetEffectIndexThatContainsPosition(x,selectionType);
                if(timingIndex >=0)
                {
                    EffectLayer* el = mSequenceElements->GetEffectLayer(row);
                    if( el != nullptr )
                    {
                        mDropStartX = tel->GetEffect(timingIndex)->GetStartPosition();
                        mDropEndX = tel->GetEffect(timingIndex)->GetEndPosition();
                        mDropStartTime = tel->GetEffect(timingIndex)->GetStartTime();
                        mDropEndTime = tel->GetEffect(timingIndex)->GetEndTime();
                        AdjustDropLocations(x, el);
                        mDragDropping = true;
                        mDropRow = row;
                    }
                }
            }
            else
            {
                EffectLayer* el = mSequenceElements->GetEffectLayer(row);
                if( el != nullptr )
                {
                    mDropStartTime = mTimeline->GetAbsoluteTimefromPosition(x);
                    mDropStartTime -= 0.5;
                    if( mDropStartTime < 0.0 ) mDropStartTime = 0.0;
                    mDropEndTime = mDropStartTime + 1.0;
                    mDropStartX = mTimeline->GetPositionFromTime(mDropStartTime);
                    mDropEndX = mTimeline->GetPositionFromTime(mDropEndTime);
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
    bool out_of_bounds =  rowIndex < 0 || (rowIndex >= mSequenceElements->GetRowInformationSize());

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
        if (!(event.ControlDown() || event.ShiftDown() || event.AltDown()))
        {
            mSequenceElements->UnSelectAllEffects();
        }
        CheckForSelectionRectangle();
        Refresh(false);
    }
    else
    {
        if(!out_of_bounds)
        {
            Element* element = mSequenceElements->GetRowInformation(rowIndex)->element;
            if( element != nullptr )
            {
                RunMouseOverHitTests(rowIndex,event.GetX(),event.GetY());
            }
        }
    }
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
    int selected_time = mTimeline->GetAbsoluteTimeMSfromPosition(event.GetX());
    mStartResizeTime = selected_time;
    int row = GetRow(event.GetY());
    if(row>=mSequenceElements->GetRowInformationSize() || row < 0)
        return;
    int effectIndex;
    int selectionType;
    Effect* selectedEffect = mSequenceElements->GetSelectedEffectAtRowAndPosition(row,event.GetX(),effectIndex,selectionType);
    if(selectedEffect!= nullptr)
    {
        // If clicked on effect edge set the selection to that position
        if( selectionType == EFFECT_LT_SELECTED )
        {
            selected_time = (int)(selectedEffect->GetStartTime()*1000.0);
        }
        else if( selectionType == EFFECT_RT_SELECTED )
        {
            selected_time = (int)(selectedEffect->GetEndTime()*1000.0);
        }
        else if( mResizingMode == EFFECT_RESIZE_MOVE )
        {
            int midpoint = selectedEffect->GetStartPosition() + (selectedEffect->GetEndPosition()-selectedEffect->GetStartPosition())/2;
            selected_time = (int)(midpoint*1000.0);
        }
        //if(selectedEffect->GetSelected() != selectionType && !(event.ShiftDown() || event.ControlDown() || event.AltDown()))
        if(!(event.ShiftDown() || event.ControlDown() || event.AltDown()))
        {
            mSequenceElements->UnSelectAllEffects();
            if(( selectionType != EFFECT_SELECTED ) || ((selectionType == EFFECT_SELECTED) && ( mResizingMode == EFFECT_RESIZE_MOVE )))
            {
                selectedEffect->SetSelected(selectionType);
            }
        }

        mEffectLayer = mSequenceElements->GetEffectLayer(row);
        Element* element = mEffectLayer->GetParentElement();

        mSelectedEffect = selectedEffect;
        if(element->GetType()=="model")
        {
            mSelectedRow = row;
            mSelectedEffect = selectedEffect;
            RaiseSelectedEffectChanged(mSelectedEffect);
            RaisePlayModelEffect(element,mSelectedEffect,false);
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
            }
            if( mSequenceElements->GetSelectedTimingRow() >= 0 ) {
                mCellRangeSelected = true;
            }
            mDragging = true;
            mDragEndX = event.GetX();
            mDragEndY = event.GetY();
            CheckForSelectionRectangle();
            CaptureMouse();
            Refresh(false);
        }
    }
    UpdateTimePosition(selected_time);
}

void adjust(double time, double &min, double &max) {
    if (time < min) {
        min = time;
    }
    if (time > max) {
        max = time;
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
            double stime = mStartResizeTime / 1000;
            double time = mTimeline->GetAbsoluteTimefromPosition(event.GetX());
            double min = stime;
            double max = stime;
            adjust(time, min, max);

            Effect* effect = mEffectLayer->GetEffect(mResizeEffectIndex);
            if(effect)
            {
                adjust(mEffectLayer->GetEffect(mResizeEffectIndex)->GetStartTime(), min, max);
                adjust(mEffectLayer->GetEffect(mResizeEffectIndex)->GetEndTime(), min, max);
                if( mSelectedEffect->GetSelected() == EFFECT_LT_SELECTED && mResizeEffectIndex > 0) {
                    //also have to re-render the effect to the left
                    adjust(mEffectLayer->GetEffect(mResizeEffectIndex - 1)->GetStartTime(), min, max);
                    adjust(mEffectLayer->GetEffect(mResizeEffectIndex - 1)->GetEndTime(), min, max);
                } else if (mSelectedEffect->GetSelected() == EFFECT_RT_SELECTED
                           && mResizeEffectIndex < (mEffectLayer->GetEffectCount() - 1)) {
                    adjust(mEffectLayer->GetEffect(mResizeEffectIndex + 1)->GetStartTime(), min, max);
                    adjust(mEffectLayer->GetEffect(mResizeEffectIndex + 1)->GetEndTime(), min, max);
                }

                sendRenderEvent(mEffectLayer->GetParentElement()->GetName(), min, max);
                RaisePlayModelEffect(mEffectLayer->GetParentElement(),effect,true);
            }
        }

        // if dragging an effect endpoint move the selection point with it so it will
        // focus on that spot if you zoom afterwards.
        if( mSelectedEffect->GetSelected() == EFFECT_LT_SELECTED )
        {
            int selected_time = (int)(mSelectedEffect->GetStartTime()*1000.0);
            UpdateTimePosition(selected_time);
        }
        else if( mSelectedEffect->GetSelected() == EFFECT_RT_SELECTED )
        {
            int selected_time = (int)(mSelectedEffect->GetEndTime()*1000.0);
            UpdateTimePosition(selected_time);
        }
    } else if (mDragging) {
        ReleaseMouse();
        mDragging = false;

        if(mDragStartX == event.GetX()
            && mDragStartY == event.GetY()) {
            checkForEmptyCell = true;
        }
    } else {
        checkForEmptyCell = true;
    }

    if (checkForEmptyCell) {
        int row = GetRow(event.GetY());
        if (mSequenceElements->GetSelectedTimingRow() >= 0 && row < mSequenceElements->GetRowInformationSize())
        {
            CheckForPartialCell(event.GetX());
            mSelectedRow = -1;
            mSelectedEffect = nullptr;
            mSequenceElements->UnSelectAllEffects();
            wxCommandEvent eventUnSelected(EVT_UNSELECTED_EFFECT);
            wxPostEvent(mParent, eventUnSelected);
        }
        else if (row < mSequenceElements->GetRowInformationSize()) {
            EffectLayer* el = mSequenceElements->GetRowInformation(row)->element->GetEffectLayer(mSequenceElements->GetRowInformation(row)->layerIndex);
            int selectionType;
            int effectIndex = el->GetEffectIndexThatContainsPosition(event.GetX(),selectionType);
            if( effectIndex == -1 && el != nullptr )
            {
                mDropStartTime = mTimeline->GetAbsoluteTimefromPosition(event.GetX());
                mDropEndTime = mDropStartTime + 1.0;
                mDropStartX = mTimeline->GetPositionFromTime(mDropStartTime);
                mDropEndX = mTimeline->GetPositionFromTime(mDropEndTime);
                AdjustDropLocations(event.GetX(), el);
                mPartialCellSelected = true;
                mDropRow = row;
                mSequenceElements->UnSelectAllEffects();
                wxCommandEvent eventUnSelected(EVT_UNSELECTED_EFFECT);
                wxPostEvent(mParent, eventUnSelected);
            }
        }
    }

    mResizing = false;
    mDragDropping = false;
    Refresh(false);
}

void EffectsGrid::CheckForPartialCell(int x_pos)
{
    mPartialCellSelected = false;
    // make sure a valid row and column is selected
    if( mRangeStartCol > 0 && mRangeStartRow > 0 )
    {
        // check for only single cell selection
        if( mRangeStartRow == mRangeEndRow && mRangeStartCol == mRangeEndCol )
        {
            EffectLayer* tel = mSequenceElements->GetEffectLayer(mSequenceElements->GetSelectedTimingRow());
            EffectLayer* el = mSequenceElements->GetEffectLayer(mRangeStartRow);

            int selectionType;
            int effectIndex = el->GetEffectIndexThatContainsPosition(x_pos,selectionType);
            if( effectIndex == -1 )
            {
                Effect* eff = tel->GetEffect(mRangeStartCol);
                mDropStartX = eff->GetStartPosition();
                mDropEndX = eff->GetEndPosition();
                mDropStartTime = eff->GetStartTime();
                mDropEndTime = eff->GetEndTime();
                mDropRow = mRangeStartRow;
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
}

void EffectsGrid::MoveSelectedEffectUp(bool shift)
{
    if( !MultipleEffectsSelected() )
    {
        if( mSelectedEffect != nullptr )
        {
            Row_Information_Struct *ri = mSequenceElements->GetRowInformation(mSelectedRow);
            int row = mSelectedRow-1;
            EffectLayer* el = mSelectedEffect->GetParentEffectLayer();
            int layer_index = ri->layerIndex;
            while( layer_index > 0 )
            {
                EffectLayer* new_el = mSequenceElements->GetEffectLayer(row);
                if( new_el->GetRangeIsClear( mSelectedEffect->GetStartTime(), mSelectedEffect->GetEndTime()))
                {
                    Effect* ef = new_el->AddEffect(0,
                                                   mSelectedEffect->GetEffectIndex(),
                                                   mSelectedEffect->GetEffectName(),
                                                   mSelectedEffect->GetSettingsAsString(),
                                                   mSelectedEffect->GetPaletteAsString(),
                                                   mSelectedEffect->GetStartTime(),
                                                   mSelectedEffect->GetEndTime(),
                                                   EFFECT_SELECTED,
                                                   false);
                    mSelectedRow = row;
                    mSelectedEffect = ef;
                    el->DeleteSelectedEffects();
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
                if( mRangeStartRow > mSequenceElements->GetNumberOfTimingRows() ) {
                    mRangeStartRow--;
                    mRangeEndRow--;
                    mDragStartY -= DEFAULT_ROW_HEADING_HEIGHT;
                    mDragEndY -= DEFAULT_ROW_HEADING_HEIGHT;
                }
            }
            Refresh(false);
        }
    }
}

void EffectsGrid::MoveSelectedEffectDown(bool shift)
{
    if( !MultipleEffectsSelected() )
    {
        if( mSelectedEffect != nullptr )
        {
            Row_Information_Struct *ri = mSequenceElements->GetRowInformation(mSelectedRow);
            int row = mSelectedRow+1;
            EffectLayer* el = mSelectedEffect->GetParentEffectLayer();
            Element* element = el->GetParentElement();
            int layer_index = ri->layerIndex;
            while( layer_index < element->GetEffectLayerCount()-1 )
            {
                EffectLayer* new_el = mSequenceElements->GetEffectLayer(row);
                if( new_el->GetRangeIsClear( mSelectedEffect->GetStartTime(), mSelectedEffect->GetEndTime()))
                {
                    Effect* ef = new_el->AddEffect(0,
                                                   mSelectedEffect->GetEffectIndex(),
                                                   mSelectedEffect->GetEffectName(),
                                                   mSelectedEffect->GetSettingsAsString(),
                                                   mSelectedEffect->GetPaletteAsString(),
                                                   mSelectedEffect->GetStartTime(),
                                                   mSelectedEffect->GetEndTime(),
                                                   EFFECT_SELECTED,
                                                   false);
                    mSelectedRow = row;
                    mSelectedEffect = ef;
                    el->DeleteSelectedEffects();
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
                if( mRangeEndRow < mSequenceElements->GetRowInformationSize()-1 )
                {
                    mRangeEndRow++;
                }
            }
            else
            {
                if( mRangeStartRow < mSequenceElements->GetRowInformationSize() ) {
                    mRangeStartRow++;
                    mRangeEndRow++;
                    mDragStartY += DEFAULT_ROW_HEADING_HEIGHT;
                    mDragEndY += DEFAULT_ROW_HEADING_HEIGHT;
                }
            }
            Refresh(false);
        }
    }
}

void EffectsGrid::MoveSelectedEffectRight(bool shift)
{
    if( !MultipleEffectsSelected() )
    {
        if( mSelectedEffect == nullptr )
        {
            if( mCellRangeSelected )
            {
                EffectLayer* tel = mSequenceElements->GetEffectLayer(mSequenceElements->GetSelectedTimingRow());
                Effect* eff1 = tel->GetEffect(mRangeStartCol+1);
                Effect* eff2 = tel->GetEffect(mRangeEndCol+1);
                if( eff1 != nullptr && eff2 != nullptr )
                {
                    mRangeEndCol++;
                    if( !shift )
                    {
                        mRangeStartCol++;
                        mDragStartX = eff1->GetStartPosition()+1;
                        mDragEndX = eff2->GetEndPosition()-1;
                    }
                    Refresh(false);
                }
            }
        }
    }
}

void EffectsGrid::MoveSelectedEffectLeft(bool shift)
{
    if( !MultipleEffectsSelected() )
    {
        if( mSelectedEffect == nullptr )
        {
            if( mCellRangeSelected )
            {
                EffectLayer* tel = mSequenceElements->GetEffectLayer(mSequenceElements->GetSelectedTimingRow());
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
                        Effect* eff2 = tel->GetEffect(mRangeStartCol);
                        mDragStartX = eff2->GetStartPosition()+1;
                        mDragEndX = eff->GetEndPosition()-1;
                    }
                    Refresh(false);
                }
            }
        }
    }
}

bool EffectsGrid::MultipleEffectsSelected()
{
    int count=0;
    for(int i=0;i<mSequenceElements->GetRowInformationSize();i++)
    {
        EffectLayer* el = mSequenceElements->GetEffectLayer(i);
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
            double drop_time_offset, new_start_time, new_end_time, column_start_time;
            eff1data[6].ToDouble(&column_start_time);
            eff1data[3].ToDouble(&drop_time_offset);
            EffectLayer* tel = mSequenceElements->GetEffectLayer(mSequenceElements->GetSelectedTimingRow());
            if( column_start_time < 0 || tel == nullptr)
            {
                drop_time_offset = mDropStartTime - drop_time_offset;
            }
            else
            {
                drop_time_offset = mDropStartTime - column_start_time;
            }
            int drop_row = mSequenceElements->GetRowInformation(mDropRow)->RowNumber;
            int start_row = wxAtoi(eff1data[5]);
            int drop_row_offset = drop_row - start_row;
            for( int i = 0; i < all_efdata.size()-1; i++ )
            {
                wxArrayString efdata = wxSplit(all_efdata[i], '\t');
                if (efdata.size() < 7) {
                    break;
                }
                efdata[3].ToDouble(&new_start_time);
                efdata[4].ToDouble(&new_end_time);
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
                if( el->GetRangeIsClear(new_start_time, new_end_time) )
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
        else
        {
            wxArrayString efdata = wxSplit(all_efdata[0], '\t');
            if (efdata.size() < 3) {
                return;
            }
            EffectLayer* el = mSequenceElements->GetEffectLayer(mDropRow);
            int effectIndex = Effect::GetEffectIndex(efdata[0]);
            if (effectIndex >= 0) {
                double end_time = mDropEndTime;
                if( (efdata.size() == 7) && GetActiveTimingElement() == nullptr )  // use original effect length if no timing track is active
                {
                    double drop_time_offset;
                    efdata[3].ToDouble(&drop_time_offset);
                    drop_time_offset = mDropStartTime - drop_time_offset;
                    efdata[4].ToDouble(&end_time);
                    end_time += drop_time_offset;
                }
                if( el->GetRangeIsClear(mDropStartTime, end_time) )
                {
                    Effect* ef = el->AddEffect(0,
                                  effectIndex,
                                  efdata[0],
                                  efdata[1],
                                  efdata[2],
                                  mDropStartTime,
                                  end_time,
                                  EFFECT_SELECTED,
                                  false);
                    if (!ef->GetPaletteMap().empty()) {
                        sendRenderEvent(el->GetParentElement()->GetName(),
                                        mDropStartTime,
                                        mDropEndTime, true);
                    }
                    RaiseSelectedEffectChanged(ef);
                    mSelectedEffect = ef;
                    mPartialCellSelected = false;
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
                double start_time, end_time;
                for( int row = mRangeStartRow; row <= mRangeEndRow; row++ )
                {
                    EffectLayer* tel = mSequenceElements->GetEffectLayer(mSequenceElements->GetSelectedTimingRow());
                    start_time = tel->GetEffect(mRangeStartCol)->GetStartTime();
                    end_time = tel->GetEffect(mRangeEndCol)->GetEndTime();
                    EffectLayer* el = mSequenceElements->GetEffectLayer(row);
                    if( el->GetRangeIsClear(start_time, end_time) )
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
    double deltaTime = 0;
    double toLeft,toRight;
    double time = mTimeline->GetAbsoluteTimefromPosition(position);
    GetRangeOfMovementForSelectedEffects(toLeft,toRight);
    if(mResizingMode==EFFECT_RESIZE_LEFT)
    {
        deltaTime = time - mEffectLayer->GetEffect(mResizeEffectIndex)->GetStartTime();
    }
    else if(mResizingMode==EFFECT_RESIZE_RIGHT)
    {
        deltaTime = time - mEffectLayer->GetEffect(mResizeEffectIndex)->GetEndTime();
    }
    else if(mResizingMode == EFFECT_RESIZE_MOVE)
    {
        double midpoint = mEffectLayer->GetEffect(mResizeEffectIndex)->GetStartTime() +
                            (mEffectLayer->GetEffect(mResizeEffectIndex)->GetEndTime() - mEffectLayer->GetEffect(mResizeEffectIndex)->GetStartTime())/2.0;
        deltaTime = time - midpoint;
    }
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
    double time = mTimeline->GetAbsoluteTimefromPosition(position);
    if(mResizingMode==EFFECT_RESIZE_LEFT)
    {
        double minimumTime = mEffectLayer->GetMinimumStartTime(mResizeEffectIndex);
        // User has dragged left side to the right side exit
        if (time >= mEffectLayer->GetEffect(mResizeEffectIndex)->GetEndTime())
        {
            return;
        }
        else if (time >= minimumTime  || minimumTime == NO_MIN_MAX_TIME)
        {
            if(mEffectLayer->IsStartTimeLinked(mResizeEffectIndex))
            {
                mEffectLayer->GetEffect(mResizeEffectIndex-1)->SetEndTime(time);
            }
            mEffectLayer->GetEffect(mResizeEffectIndex)->SetStartTime(time);
        }
        else
        {
            if(mResizeEffectIndex!=0)
            {
                mEffectLayer->GetEffect(mResizeEffectIndex)->SetStartTime(mEffectLayer->GetEffect(mResizeEffectIndex-1)->GetEndTime());
            }
        }
    }
    else if(mResizingMode==EFFECT_RESIZE_RIGHT)
    {
        double maximumTime = mEffectLayer->GetMaximumEndTime(mResizeEffectIndex);
        // User has dragged right side to the left side exit
        if (time <= mEffectLayer->GetEffect(mResizeEffectIndex)->GetStartTime())
        {
            return;
        }
        else if (time <= maximumTime  || maximumTime == NO_MIN_MAX_TIME)
        {
            if(mEffectLayer->IsEndTimeLinked(mResizeEffectIndex))
            {
                mEffectLayer->GetEffect(mResizeEffectIndex+1)->SetStartTime(time);
            }
            mEffectLayer->GetEffect(mResizeEffectIndex)->SetEndTime(time);
        }
        else
        {
            if(mResizeEffectIndex < mEffectLayer->GetEffectCount()-1)
            {
                mEffectLayer->GetEffect(mResizeEffectIndex)->SetEndTime(mEffectLayer->GetEffect(mResizeEffectIndex+1)->GetStartTime());
            }
        }
    }
    Refresh(false);
    // Move time line and waveform to new position
    int selected_time = mTimeline->GetAbsoluteTimeMSfromPosition(position);
    UpdateTimePosition(selected_time);
}


void EffectsGrid::RunMouseOverHitTests(int rowIndex,int x,int y)
{
    int effectIndex;
    int result;

    wxLogDebug("EffectsGrid::RunMouseOverHitTests");
    EffectLayer* layer = mSequenceElements->GetEffectLayer(rowIndex);
    bool isHit = layer->HitTestEffect(x,effectIndex,result);
    if(isHit)
    {
        //mElementEffects = effects;
        mResizeEffectIndex = effectIndex;
        if (result == HIT_TEST_EFFECT_LT)
        {
            SetCursor(wxCURSOR_SIZEWE);
            mResizingMode = EFFECT_RESIZE_LEFT;
        }
        else if (result == HIT_TEST_EFFECT_RT)
        {
            SetCursor(wxCURSOR_SIZEWE);
            mResizingMode = EFFECT_RESIZE_RIGHT;
        }
        else if (result == HIT_TEST_EFFECT_CTR)
        {
            SetCursor(wxCURSOR_HAND);
            mResizingMode = EFFECT_RESIZE_MOVE;
        }
        else
        {
            SetCursor(wxCURSOR_DEFAULT);
            mResizingMode = EFFECT_RESIZE_NO;
        }
    }
    else
    {
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

void EffectsGrid::CheckForSelectionRectangle()
{
    int row1 =  GetRow(mDragStartY);
    int row2 =  GetRow(mDragEndY);
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

    if( mSequenceElements->GetSelectedTimingRow() >= 0 &&
        row1 < mSequenceElements->GetRowInformationSize() &&
        row2 < mSequenceElements->GetRowInformationSize() )
    {
        EffectLayer* tel = mSequenceElements->GetEffectLayer(mSequenceElements->GetSelectedTimingRow());
        int selectionType;
        int timingIndex1 = tel->GetEffectIndexThatContainsPosition(start_x,selectionType);
        int timingIndex2 = tel->GetEffectIndexThatContainsPosition(end_x,selectionType);
        if(timingIndex1 != -1 && timingIndex2 != -1)
        {
            mRangeStartCol = timingIndex1;
            mRangeEndCol = timingIndex2;
        }
    }

    if(mDragStartX != mDragEndX || mDragStartY != mDragEndY)
    {
        int firstSelected;
        mSequenceElements->SelectEffectsInRowAndPositionRange(row1,row2,start_x,end_x,firstSelected);
    }
}

void EffectsGrid::HighlightSelectionRectangle()
{
    EffectLayer* tel = mSequenceElements->GetEffectLayer(mSequenceElements->GetSelectedTimingRow());
    Effect* eff1 = tel->GetEffect(mRangeStartCol);
    Effect* eff2 = tel->GetEffect(mRangeEndCol);
    if( eff1 != nullptr && eff2 != nullptr )
    {
        int start_x = eff1->GetStartPosition();
        int end_x = eff1->GetEndPosition();
        int firstSelected;
        mSequenceElements->SelectEffectsInRowAndPositionRange(mRangeStartRow,mRangeEndRow,start_x,end_x,firstSelected);
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
    for(int row=0;row < mSequenceElements->GetRowInformationSize();row++)
    {
        Row_Information_Struct* ri = mSequenceElements->GetRowInformation(row);
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

    for(int row=0;row < mSequenceElements->GetRowInformationSize();row++)
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
    for(int row=0;row<mSequenceElements->GetRowInformationSize();row++)
    {
        wxString type = mSequenceElements->GetRowInformation(row)->element->GetType();
        wxString name = mSequenceElements->GetRowInformation(row)->element->GetName();
        if(type=="view" || type == "model")
        {
            DrawModelOrViewEffects(row);
        }
    }
}

void EffectsGrid::DrawTimings()
{
    for(int row=0;row<mSequenceElements->GetRowInformationSize();row++)
    {
        wxString type = mSequenceElements->GetRowInformation(row)->element->GetType();
        if(type!="view" && type != "model")
        {
            DrawTimingEffects(row);
        }
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
        case BitmapCache::RGB_EFFECTS_e::eff_ON:
        case BitmapCache::RGB_EFFECTS_e::eff_COLORWASH:
        case BitmapCache::RGB_EFFECTS_e::eff_SINGLESTRAND: {
            if (e->HasBackgroundDisplayList()) {
                DrawGLUtils::DrawDisplayList(x1, y1, x2-x1, y2-y1, e->GetBackgroundDisplayList());
                return e->GetBackgroundDisplayList().iconSize;
            }
        }
    }

    //haven't rendered an effect background yet, use a default redering mechanism
    switch (e->GetEffectIndex()) {
        case BitmapCache::RGB_EFFECTS_e::eff_ON: {
            xlColor start;
            xlColor end;
            GetOnEffectColors(e, start, end);
            DrawGLUtils::DrawHBlendedRectangle(start, end, x1, y1, x2, y2);
            return 2;
        }
        break;
        case BitmapCache::RGB_EFFECTS_e::eff_COLORWASH:
        case BitmapCache::RGB_EFFECTS_e::eff_SHOCKWAVE: {
            DrawGLUtils::DrawHBlendedRectangle(e->GetPalette(), x1, y1, x2, y2);
            return 2;
        }
        break;
        case BitmapCache::RGB_EFFECTS_e::eff_MORPH: {
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
        case BitmapCache::RGB_EFFECTS_e::eff_GALAXY: {
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
        case BitmapCache::RGB_EFFECTS_e::eff_FAN: {
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
    Row_Information_Struct *ri = mSequenceElements->GetRowInformation(row);
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
        ncls.InitNodeBuffer(xlights->GetModelClass(ri->element->GetName()), ri->strandIndex, ri->nodeIndex, seqData->FrameTime());
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
        mTimeline->GetPositionsFromTimeRange(effectLayer->GetEffect(effectIndex)->GetStartTime(),
                                       effectLayer->GetEffect(effectIndex)->GetEndTime(),mode,x1,x2,x3,x4);
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

        if (mode==SCREEN_L_R_OFF)
        {
            effectLayer->GetEffect(effectIndex)->SetStartPosition(-10);
            effectLayer->GetEffect(effectIndex)->SetEndPosition(-10);
        }
        else
        {
            if(mode==SCREEN_L_R_ON || mode == SCREEN_L_ON)
            {
                if(effectIndex>0)
                {
                    // Draw left line if effect has different start time then previous effect or
                    // previous effect was not selected, or onlwidthy left was selected
                    if(effectLayer->GetEffect(effectIndex)->GetStartTime() != effectLayer->GetEffect(effectIndex-1)->GetEndTime() ||
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
                effectLayer->GetEffect(effectIndex)->SetStartPosition(x1);
            }
            else
            {
                effectLayer->GetEffect(effectIndex)->SetStartPosition(-10);
            }

            // Draw Right line
            if(mode==SCREEN_L_R_ON || mode == SCREEN_R_ON)
            {
                DrawGLUtils::DrawLine(*mEffectColorRight,255,x2,y1,x2,y2,2);
                effectLayer->GetEffect(effectIndex)->SetEndPosition(x2);
            }
            else
            {
                effectLayer->GetEffect(effectIndex)->SetEndPosition(mWindowWidth+10);
            }

            // Draw horizontal
            if(mode!=SCREEN_L_R_OFF)
            {
                if (drawIcon) {
                    if(x > (DEFAULT_ROW_HEADING_HEIGHT + 4)) {
                        double sz = (DEFAULT_ROW_HEADING_HEIGHT - 6.0) / (2.0 * drawIcon) + 1.0;

                        double xl = (x1+x2)/2.0-sz;
                        double xr = (x1+x2)/2.0+sz;
                        DrawEffectIcon(&m_EffectTextures[e->GetEffectIndex()],
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
            highlight_color = *RowHeading::GetTimingColor(mSequenceElements->GetRowInformation(selected_timing_row)->colorIndex);
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
    Element* element =mSequenceElements->GetRowInformation(row)->element;
    EffectLayer* effectLayer=mSequenceElements->GetEffectLayer(row);
    xlColor* mEffectColorRight;
    xlColor* mEffectColorLeft;
    xlColor* mEffectColorCenter;
    //if(effectLayer==nullptr)
    //    return;
    for(int effectIndex=0;effectIndex < effectLayer->GetEffectCount();effectIndex++)
    {
        EFFECT_SCREEN_MODE mode;

        int y1 = (row*DEFAULT_ROW_HEADING_HEIGHT)+4;
        int y2 = ((row+1)*DEFAULT_ROW_HEADING_HEIGHT)-4;
        int y = (row*DEFAULT_ROW_HEADING_HEIGHT) + (DEFAULT_ROW_HEADING_HEIGHT/2);
        int x1,x2,x3,x4;

        mTimeline->GetPositionsFromTimeRange(effectLayer->GetEffect(effectIndex)->GetStartTime(),
                                       effectLayer->GetEffect(effectIndex)->GetEndTime(),mode,x1,x2,x3,x4);
        mEffectColorLeft = effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_NOT_SELECTED ||
                           effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_RT_SELECTED?mTimingColor:mSelectionColor;
        mEffectColorRight = effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_NOT_SELECTED ||
                           effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_LT_SELECTED?mTimingColor:mSelectionColor;
        mEffectColorCenter = effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_SELECTED?mSelectionColor:mTimingColor;

        if(mode==SCREEN_L_R_OFF)
        {
            effectLayer->GetEffect(effectIndex)->SetStartPosition(-10);
            effectLayer->GetEffect(effectIndex)->SetEndPosition(-10);
        }
        else
        {
            // Draw Left line
            if(mode==SCREEN_L_R_ON || mode == SCREEN_L_ON)
            {
                if(effectIndex>0)
                {
                    // Draw left line if effect has different start time then previous effect or
                    // previous effect was not selected, or only left was selected
                    if(effectLayer->GetEffect(effectIndex)->GetStartTime() != effectLayer->GetEffect(effectIndex-1)->GetEndTime() ||
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

                effectLayer->GetEffect(effectIndex)->SetStartPosition(x1);
                if(element->GetActive())
                {
                    glEnable(GL_BLEND);
                    DrawGLUtils::DrawLine(*RowHeading::GetTimingColor(mSequenceElements->GetRowInformation(row)->colorIndex),128,x1,(row+1)*DEFAULT_ROW_HEADING_HEIGHT,x1,GetSize().y,1);
                    glDisable(GL_BLEND);
                }
            }
            else
            {
                effectLayer->GetEffect(effectIndex)->SetStartPosition(-10);
            }
            // Draw Right line
            if(mode==SCREEN_L_R_ON || mode == SCREEN_R_ON)
            {
                DrawGLUtils::DrawLine(*mEffectColorRight,255,x2,y1,x2,y2,2);
                effectLayer->GetEffect(effectIndex)->SetEndPosition(x2);
                if(element->GetActive())
                {
                    glEnable(GL_BLEND);
                    DrawGLUtils::DrawLine(*RowHeading::GetTimingColor(mSequenceElements->GetRowInformation(row)->colorIndex),128,x2,(row+1)*DEFAULT_ROW_HEADING_HEIGHT,x2,GetSize().y,1);
                    glDisable(GL_BLEND);
                }
            }
            else
            {
                effectLayer->GetEffect(effectIndex)->SetEndPosition(mWindowWidth+10);
            }
            // Draw horizontal
            if(mode!=SCREEN_L_R_OFF)
            {
                DrawGLUtils::DrawLine(*mTimingColor,255,x1,y,x2,y,2);
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
    }

    if((mDragging || mCellRangeSelected) && !mPartialCellSelected)
    {
        if (mSequenceElements->GetSelectedTimingRow() >= 0 && mRangeStartCol > 0) {
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
    EffectLayer* tel = mSequenceElements->GetEffectLayer(mSequenceElements->GetSelectedTimingRow());
    if( tel->GetEffectCount() > 0 )
    {
        int start_x = tel->GetEffect(mRangeStartCol)->GetStartPosition()+1;
        int end_x = tel->GetEffect(mRangeEndCol)->GetEndPosition()-1;
        int start_y = mRangeStartRow*DEFAULT_ROW_HEADING_HEIGHT;
        int end_y = mRangeEndRow*DEFAULT_ROW_HEADING_HEIGHT;
        xlColor highlight_color;
        highlight_color = *RowHeading::GetTimingColor(mSequenceElements->GetRowInformation(mSequenceElements->GetSelectedTimingRow())->colorIndex);
        glEnable(GL_BLEND);
        DrawGLUtils::DrawFillRectangle(highlight_color,80,start_x,start_y,end_x-start_x,end_y-start_y+DEFAULT_ROW_HEADING_HEIGHT);
        glDisable(GL_BLEND);
    }
}

void EffectsGrid::DrawEffectIcon(GLuint* texture,double x, double y, double x2, double y2)
{
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBindTexture(GL_TEXTURE_2D,*texture);
    glPushMatrix();
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);

    glVertex2f(x-0.4, y);

    glTexCoord2f(1,0);
    glVertex2f(x2-0.4,y);

    glTexCoord2f(1,1);
    glVertex2f(x2-0.4,y2);

    glTexCoord2f(0,1);
    glVertex2f(x-0.4,y2);
    glEnd();
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void EffectsGrid::CreateEffectIconTextures()
{
    for(int effectID=0;effectID<BitmapCache::RGB_EFFECTS_e::eff_LASTEFFECT;effectID++)
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
    for(int effectID=0;effectID<BitmapCache::RGB_EFFECTS_e::eff_LASTEFFECT;effectID++)
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
    if (row >= mSequenceElements->GetRowInformationSize()) {
        return;
    }
    EffectLayer* effectLayer = mSequenceElements->GetEffectLayer(row);

    mSequenceElements->ClearSelectedRanges();
    EffectRange effectRange;
    effectRange.Layer = effectLayer;
    // Store start and end time. The effect text will be supplied by parent class
    effectRange.StartTime = mDropStartTime;
    effectRange.EndTime = mDropEndTime;
    mSequenceElements->AddSelectedRange(&effectRange);
    // Raise event
    wxCommandEvent eventDropped(EVT_EFFECT_DROPPED);
    wxPostEvent(GetParent(), eventDropped);
}

Element* EffectsGrid::GetActiveTimingElement()
{
    Element* returnValue=nullptr;
    for(int row=0;row<mSequenceElements->GetRowInformationSize();row++)
    {
        Element* e = mSequenceElements->GetRowInformation(row)->element;
        if(e->GetType()== "timing" && e->GetActive())
        {
            returnValue = e;
            break;
        }
    }
    return returnValue;
}

void EffectsGrid::GetRangeOfMovementForSelectedEffects(double &toLeft, double &toRight)
{
    double left,right;
    toLeft = 1000;
    toRight = 1000;
    for(int row=0;row<mSequenceElements->GetRowInformationSize();row++)
    {
        EffectLayer* el = mSequenceElements->GetEffectLayer(row);
        el->GetMaximumRangeOfMovementForSelectedEffects(left,right);
        toLeft = toLeft<left?toLeft:left;
        toRight = toRight<right?toRight:right;
    }
}

void EffectsGrid::MoveAllSelectedEffects(double delta, bool offset)
{
    if( !offset ) {
        for(int row=0;row<mSequenceElements->GetRowInformationSize();row++)
        {
            EffectLayer* el = mSequenceElements->GetEffectLayer(row);
            el->MoveAllSelectedEffects(delta);
        }
    } else {
        int num_rows_with_selections = 0;
        int start_row = -1;
        int end_row = -1;
        for(int row=0;row<mSequenceElements->GetRowInformationSize();row++)
        {
            EffectLayer* el = mSequenceElements->GetEffectLayer(row);
            if( el->GetSelectedEffectCount() > 0 ) {
                num_rows_with_selections++;
                if( start_row == -1 ) {
                    start_row = row;
                } else {
                    end_row = row;
                }
            }
        }
        double delta_step = delta / double(end_row-start_row);
        for(int row=start_row;row<=end_row;row++)
        {
            EffectLayer* el = mSequenceElements->GetEffectLayer(row);
            if( mResizingMode == EFFECT_RESIZE_RIGHT || mResizingMode == EFFECT_RESIZE_MOVE) {
                el->MoveAllSelectedEffects(delta_step*(double)(row-start_row));
            } else {
                el->MoveAllSelectedEffects(delta_step*(double)(end_row-row));
            }
        }
    }
}


