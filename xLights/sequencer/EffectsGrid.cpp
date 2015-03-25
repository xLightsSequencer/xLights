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

BEGIN_EVENT_TABLE(EffectsGrid, xlGLCanvas)
EVT_IDLE(EffectsGrid::OnIdle)
EVT_MOTION(EffectsGrid::mouseMoved)
EVT_MOUSEWHEEL(EffectsGrid::mouseWheelMoved)
EVT_LEFT_DOWN(EffectsGrid::mouseDown)
EVT_LEFT_UP(EffectsGrid::mouseReleased)
EVT_MOUSE_CAPTURE_LOST(EffectsGrid::OnLostMouseCapture)
//EVT_RIGHT_DOWN(EffectsGrid::rightClick)
//EVT_LEAVE_WINDOW(EffectsGrid::mouseLeftWindow)
EVT_SIZE(EffectsGrid::resized)
//EVT_KEY_DOWN(EffectsGrid::keyPressed)
//EVT_KEY_UP(EffectsGrid::keyReleased)
EVT_PAINT(EffectsGrid::render)
END_EVENT_TABLE()
// some useful events to use


EffectsGrid::EffectsGrid(MainSequencer* parent, wxWindowID id, const wxPoint &pos, const wxSize &size,
                       long style, const wxString &name)
    :xlGLCanvas(parent, id, pos, size, wxFULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN | wxCLIP_SIBLINGS)
{
    mIsInitialized = false;
    mParent = parent;
    mDragging = false;
    mResizing = false;
    mDragDropping = false;
    mDropStartX = 0;
    mDropEndX = 0;
    mWindowWidth = 0;
    mWindowHeight = 0;
    mWindowResized = false;

    SetBackgroundStyle(wxBG_STYLE_CUSTOM);

    mEffectColor = new xlColor(192,192,192);
    mGridlineColor = new xlColor(40,40,40);

    mTimingColor = new xlColor(255,255,255);
    mTimingVerticalLine = new xlColor(130,178,207);
    mSelectionColor = new xlColor(255,0,255);

    mPaintOnIdleCounter=0;
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

void EffectsGrid::rightClick(wxMouseEvent& event) {}
void EffectsGrid::mouseLeftWindow(wxMouseEvent& event) {}
void EffectsGrid::keyReleased(wxKeyEvent& event){}
void EffectsGrid::keyPressed(wxKeyEvent& event){}

void EffectsGrid::OnLostMouseCapture(wxMouseCaptureLostEvent& event)
{
    mDragging = false;
    mResizing = false;
    mDragDropping = false;
    mResizingMode = EFFECT_RESIZE_NO;
}

void EffectsGrid::DragOver(int x, int y)
{
    int row = GetRow(y);
    int selectedTimingIndex = mSequenceElements->GetSelectedTimingRow();
    if(selectedTimingIndex >= 0)
    {
        Element* e = mSequenceElements->GetRowInformation(selectedTimingIndex)->element;
        EffectLayer* el = e->GetEffectLayer(mSequenceElements->GetRowInformation(selectedTimingIndex)->layerIndex);
        int selectionType;
        int timingIndex = el->GetEffectIndexThatContainsPosition(x,selectionType);
        if(timingIndex >=0)
        {
            mDragDropping = true;
            mDropStartX = el->GetEffect(timingIndex)->GetStartPosition();
            mDropEndX = el->GetEffect(timingIndex)->GetEndPosition();
            mDropStartTime = el->GetEffect(timingIndex)->GetStartTime();
            mDropEndTime = el->GetEffect(timingIndex)->GetEndTime();
            mDropRow = row;
        }
        else
        {
            mDragDropping = false;
        }
    }
    else
    {
        mDragDropping = false;
    }
    Refresh(false);
    mPaintOnIdleCounter=0;
}

void EffectsGrid::OnDrop(int x, int y)
{
    mDragDropping = false;
    RaiseEffectDropped(x,y);
    Refresh(false);
}

void EffectsGrid::mouseMoved(wxMouseEvent& event)
{
    if (!mIsInitialized || mSequenceElements == NULL) {
        return;
    }
    int rowIndex = GetRow(event.GetY());
    if(rowIndex >= mSequenceElements->GetRowInformationSize())
    {
        SetCursor(wxCURSOR_DEFAULT);
        //Refresh(false);
        return;
    }
    if(mResizing)
    {
        Resize(event.GetX());
        Refresh(false);
    }
    else if (mDragging)
    {
        mDragEndX = event.GetX();
        mDragEndY = event.GetY();
        if (!(event.ControlDown() || event.ShiftDown()))
        {
            mSequenceElements->UnSelectAllEffects();
        }
        CheckForSelectionRectangle();
        Refresh(false);
    }
    else
    {
        Element* element = mSequenceElements->GetRowInformation(rowIndex)->element;
        RunMouseOverHitTests(element,mSequenceElements->GetRowInformation(rowIndex)->layerIndex,event.GetX(),event.GetY());
    }
}

void EffectsGrid::mouseDown(wxMouseEvent& event)
{
    if (mSequenceElements == NULL) {
        return;
    }
    if(!(event.ShiftDown() || event.ControlDown()) && mResizingMode == EFFECT_RESIZE_NO)
    {
        mSequenceElements->UnSelectAllEffects();
    }
    int selected_time = mTimeline->GetAbsoluteTimeMSfromPosition(event.GetX());
    int row = GetRow(event.GetY());
    if(row>=mSequenceElements->GetRowInformationSize())
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
        if(selectedEffect->GetSelected() != selectionType && !(event.ShiftDown() || event.ControlDown()))
        {
            mSequenceElements->UnSelectAllEffects();
            selectedEffect->SetSelected(selectionType);
        }
        mEffectLayer = mSequenceElements->GetRowInformation(row)->element->GetEffectLayer(mSequenceElements->GetRowInformation(row)->layerIndex);
        Element* element = mSequenceElements->GetRowInformation(row)->element;
        mSelectedRow = row;
        mPaintOnIdleCounter = 0;

        if(mSelectedRow!=row || selectedEffect!=mSelectedEffect)
        {
            mSelectedEffect = selectedEffect;
            if(element->GetType()=="model")
            {
                RaiseSelectedEffectChanged(mSelectedEffect);
                RaisePlayModelEffect(element,mSelectedEffect,false);
            }
        }
    }

    if(mResizingMode!=EFFECT_RESIZE_NO)
    {
        mResizing = true;
        mResizeEffectIndex = effectIndex;
    }
    else
    {
        if( !mDragging )
        {
            mDragging = true;
            mDragStartX = event.GetX();
            mDragStartY = event.GetY();
            mDragEndX = event.GetX();
            mDragEndY = event.GetY();
            CaptureMouse();
        }
    }
    UpdateTimePosition(selected_time);
    event.Skip(true);
}

void EffectsGrid::mouseReleased(wxMouseEvent& event)
{
    if(mResizing)
    {
        if(mEffectLayer->GetParentElement()->GetType()=="model")
        {
            Effect* effect = mEffectLayer->GetEffect(mResizeEffectIndex);
            if(effect)
            {
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
    }

    if(mDragging)
    {
        ReleaseMouse();
        mDragging = false;
    }

    mResizing = false;
    mDragDropping = false;
    mPaintOnIdleCounter = 0;
}

void EffectsGrid::Resize(int position)
{
    if(MultipleEffectsSelected())
    {
        ResizeMoveMultipleEffects(position);
    }
    else
    {
        ResizeSingleEffect(position);
    }
}

bool EffectsGrid::MultipleEffectsSelected()
{
    int count=0;
    for(int i=0;i<mSequenceElements->GetRowInformationSize();i++)
    {
        EffectLayer* el = mSequenceElements->GetRowInformation(i)->element->GetEffectLayer(mSequenceElements->GetRowInformation(i)->layerIndex);
        count+= el->GetSelectedEffectCount();
        if(count > 1)
        {
             return true;
        }
    }
    return false;
}

void EffectsGrid::ResizeMoveMultipleEffects(int position)
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
    if(deltaTime<0)
    {
        if (abs(deltaTime)< toLeft)
        {
            MoveAllSelectedEffects(deltaTime);
        }
    }
    else
    {
        if (deltaTime< toRight)
        {
            MoveAllSelectedEffects(deltaTime);
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
    mPaintOnIdleCounter=0;
    // Move time line and waveform to new position
    int selected_time = mTimeline->GetAbsoluteTimeMSfromPosition(position);
    UpdateTimePosition(selected_time);
}


void EffectsGrid::RunMouseOverHitTests(Element* element,int effectLayerIndex,int x,int y)
{
    int effectIndex;
    int result;

    EffectLayer* layer = element->GetEffectLayer(effectLayerIndex);
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
    if(mDragStartX != mDragEndX || mDragStartY != mDragEndY)
    {
        int startRow = GetRow(mDragStartY);
        int endRow = GetRow(mDragEndY);
        int row1 =  startRow<=endRow?startRow:endRow;
        int row2 =  startRow>endRow?startRow:endRow;
        int x1 = mDragStartX<=mDragEndX?mDragStartX:mDragEndX;
        int x2 = mDragStartX>mDragEndX?mDragStartX:mDragEndX;
        int firstSelected;
        mSequenceElements->SelectEffectsInRowAndPositionRange(row1,row2,x1,x2,firstSelected);
    }
}

void EffectsGrid::OnIdle(wxIdleEvent &event)
{
    // This is a hack to get the grid to repaint after row header
    // information has changed. The counter prevents grid from
    // continuously repainting during idle causing excessive
    // cpu usage. It will only repaint on idle for 25 times
    // mPaintOnIdleCounter is reset to "0".
    if(mPaintOnIdleCounter <5)
    {
        Refresh(false);
        mPaintOnIdleCounter++;
}
}

void EffectsGrid::ForceRefresh()
{
    mPaintOnIdleCounter=0;
}


void EffectsGrid::SetTimeline(TimeLine* timeline)
{
        mTimeline = timeline;
}

void EffectsGrid::resized(wxSizeEvent& evt)
{
    mWindowWidth = evt.GetSize().GetWidth();
    mWindowHeight = evt.GetSize().GetHeight();
    mWindowResized = true;
}


/** Inits the OpenGL viewport for drawing in 2D. */
void EffectsGrid::prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y)
{
    glViewport(topleft_x, topleft_y, bottomrigth_x-topleft_x, bottomrigth_y-topleft_y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(topleft_x, bottomrigth_x, bottomrigth_y, topleft_y, 0, 1);
    glMatrixMode(GL_MODELVIEW);
}

void EffectsGrid::SetCanvasSize(int w, int h)
{
    SetSize(wxSize(w,h));
    SetMaxSize(wxSize(w,h));
    SetMinSize(wxSize(w,h));
    mPaintOnIdleCounter= 0;
}

void EffectsGrid::SetSequenceElements(SequenceElements* elements)
{
    mSequenceElements = elements;
}

void EffectsGrid::SetStartPixelOffset(int offset)
{
    mStartPixelOffset = offset;
}

void EffectsGrid::InitializeGrid()
{
    if(!IsShownOnScreen()) return;
    SetCurrentGLContext();
//    wxClientDC dc(this);
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
    for(int row=0;(row*DEFAULT_ROW_HEADING_HEIGHT)< mWindowHeight, row < mSequenceElements->GetRowInformationSize();row++)
    {
        Row_Information_Struct* ri = mSequenceElements->GetRowInformation(row);
        Element* e = ri->element;
        y = row*DEFAULT_ROW_HEADING_HEIGHT;

        if(ri->layerIndex == 0)
        {
            if (isEvenLayer)
            {
                //Element is collapsed only one row should be shaded
                int h = e->GetCollapsed()?DEFAULT_ROW_HEADING_HEIGHT:DEFAULT_ROW_HEADING_HEIGHT * e->GetEffectLayerCount();
                DrawGLUtils::DrawFillRectangle(xlLIGHT_GREY,40,x1,y,x2,h);
            }
            isEvenLayer = !isEvenLayer;
        }
    }
    glDisable(GL_BLEND);

    for(int row=0;(row*DEFAULT_ROW_HEADING_HEIGHT)< mWindowHeight, row < mSequenceElements->GetRowInformationSize();row++)
    {
        y = (row+1)*DEFAULT_ROW_HEADING_HEIGHT;
        DrawGLUtils::DrawLine(*mGridlineColor,255,x1,y,x2,y,.2);
    }

}

void EffectsGrid::DrawVerticalLines()
{
    int x1=0;
    int x2 = mWindowWidth-1;
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
        else
        {
            DrawTimingEffects(row);
        }
    }
}

void EffectsGrid::DrawModelOrViewEffects(int row)
{
    EffectLayer* effectLayer =mSequenceElements->GetRowInformation(row)->
                              element->GetEffectLayer(mSequenceElements->GetRowInformation(row)->layerIndex);
    xlColor* mEffectColorRight;
    xlColor* mEffectColorLeft;
    xlColor* mEffectColorCenter;
    int y1 = (row*DEFAULT_ROW_HEADING_HEIGHT)+2;
    int y2 = ((row+1)*DEFAULT_ROW_HEADING_HEIGHT)-2;
    int y = (row*DEFAULT_ROW_HEADING_HEIGHT) + (DEFAULT_ROW_HEADING_HEIGHT/2);

    for(int effectIndex=0;effectIndex < effectLayer->GetEffectCount();effectIndex++)
    {
        Effect* e = effectLayer->GetEffect(effectIndex);
        EFFECT_SCREEN_MODE mode;

        int x1,x2;
        mTimeline->GetPositionsFromTimeRange(effectLayer->GetEffect(effectIndex)->GetStartTime(),
                                       effectLayer->GetEffect(effectIndex)->GetEndTime(),mode,x1,x2);
        int x = x2-x1;
        // Draw Left line
        mEffectColorLeft = effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_NOT_SELECTED ||
                           effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_RT_SELECTED?mEffectColor:mSelectionColor;
        mEffectColorRight = effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_NOT_SELECTED ||
                           effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_LT_SELECTED?mEffectColor:mSelectionColor;
        mEffectColorCenter = effectLayer->GetEffect(effectIndex)->GetSelected() == EFFECT_SELECTED?mSelectionColor:mEffectColor;

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
                if(x > MINIMUM_EFFECT_WIDTH_FOR_ICON)
                {
                    DrawGLUtils::DrawLine(*mEffectColorLeft,255,x1,y,x1+(x/2)-9,y,1);
                    DrawGLUtils::DrawLine(*mEffectColorRight,255,x1+(x/2)+9,y,x2,y,1);
                    DrawGLUtils::DrawRectangle(*mEffectColor,false,x1+(x/2)-9,y1,x1+(x/2)+9,y2);
                    glEnable(GL_TEXTURE_2D);
                    DrawEffectIcon(&m_EffectTextures[e->GetEffectIndex()],x1+(x/2)-11,row*DEFAULT_ROW_HEADING_HEIGHT);
                    glDisable(GL_TEXTURE_2D);

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
    if(mDragDropping && mDropRow == row)
    {
        int y3 = row*DEFAULT_ROW_HEADING_HEIGHT;
        const xlColor c = *RowHeading::GetTimingColor(mSequenceElements->GetRowInformation(mSequenceElements->GetSelectedTimingRow())->colorIndex);

        glEnable(GL_BLEND);
        DrawGLUtils::DrawFillRectangle(c,80,mDropStartX,y3,mDropEndX-mDropStartX,DEFAULT_ROW_HEADING_HEIGHT);
        glDisable(GL_BLEND);
    }
}

void EffectsGrid::DrawTimingEffects(int row)
{
    Element* element =mSequenceElements->GetRowInformation(row)->element;
    int lIndex = mSequenceElements->GetRowInformation(row)->layerIndex;
    EffectLayer* effectLayer=element->GetEffectLayer(mSequenceElements->GetRowInformation(row)->layerIndex);
    xlColor* mEffectColorRight;
    xlColor* mEffectColorLeft;
    xlColor* mEffectColorCenter;
    //if(effectLayer==nullptr)
    //    return;
    for(int effectIndex=0;effectIndex < effectLayer->GetEffectCount();effectIndex++)
    {
        Effect* e = effectLayer->GetEffect(effectIndex);
        EFFECT_SCREEN_MODE mode;

        int y1 = (row*DEFAULT_ROW_HEADING_HEIGHT)+4;
        int y2 = ((row+1)*DEFAULT_ROW_HEADING_HEIGHT)-4;
        int y = (row*DEFAULT_ROW_HEADING_HEIGHT) + (DEFAULT_ROW_HEADING_HEIGHT/2);
        int x1,x2;

        mTimeline->GetPositionsFromTimeRange(effectLayer->GetEffect(effectIndex)->GetStartTime(),
                                       effectLayer->GetEffect(effectIndex)->GetEndTime(),mode,x1,x2);
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
    if(!mIsInitialized) { InitializeGrid(); }
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
    }
    if(mDragging)
    {
        DrawGLUtils::DrawRectangle(xlYELLOW,true,mDragStartX,mDragStartY,mDragEndX,mDragEndY);
    }
    //DrawGLUtils::DrawRectangle(xlYELLOW,false,5,5, GetSize().x - 10, GetSize().y - 10);

    glFlush();
    SwapBuffers();
    //wxLogDebug("EffectsGrid::Draw");
}



void EffectsGrid::DrawEffectIcon(GLuint* texture,int x, int y)
{
    glColor3f(1.0f, 1.0f, 1.0f);
    glBindTexture(GL_TEXTURE_2D,*texture);
    glPushMatrix();
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2f( x+4, y+4 );

    glTexCoord2f(1,0);
    glVertex2f(x+18,y+4);

    glTexCoord2f(1,1);
    glVertex2f(x+18,y+18);

    glTexCoord2f(0,1);
    glVertex2f(x+4,y+18);
    glEnd();
    glPopMatrix();
}

void EffectsGrid::CreateEffectIconTextures()
{
    const char** p_XPM;
    for(int effectID=0;effectID<xLightsFrame::RGB_EFFECTS_e::eff_LASTEFFECT;effectID++)
    {
        wxString tooltip;
        p_XPM = xLightsFrame::GetIconBuffer(effectID, tooltip);
        DrawGLUtils::CreateOrUpdateTexture((char**)p_XPM,&m_EffectTextures[effectID]);
    }
}

void EffectsGrid::DeleteEffectIconTextures()
{
    for(int effectID=0;effectID<NUMBER_OF_EFFECTS;effectID++)
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
    Element* e = mSequenceElements->GetRowInformation(row)->element;
    EffectLayer* effectLayer = e->GetEffectLayer(mSequenceElements->GetRowInformation(row)->layerIndex);

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
        if(e->GetType()== "Timing" && e->GetActive())
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
        Element* element = mSequenceElements->GetRowInformation(row)->element;
        EffectLayer* el =  element->GetEffectLayer( mSequenceElements->GetRowInformation(row)->layerIndex);
        el->GetMaximumRangeOfMovementForSelectedEffects(left,right);
        toLeft = toLeft<left?toLeft:left;
        toRight = toRight<right?toRight:right;
    }
}

void EffectsGrid::MoveAllSelectedEffects(double delta)
{
    for(int row=0;row<mSequenceElements->GetRowInformationSize();row++)
    {
        Element* element = mSequenceElements->GetRowInformation(row)->element;
        EffectLayer* el =  element->GetEffectLayer( mSequenceElements->GetRowInformation(row)->layerIndex);
        el->MoveAllSelectedEffects(delta);
    }
}


