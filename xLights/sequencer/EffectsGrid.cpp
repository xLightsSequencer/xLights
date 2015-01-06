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


BEGIN_EVENT_TABLE(EffectsGrid, wxGLCanvas)
EVT_IDLE(EffectsGrid::OnIdle)
EVT_MOTION(EffectsGrid::mouseMoved)
EVT_MOUSEWHEEL(EffectsGrid::mouseWheelMoved)
EVT_LEFT_DOWN(EffectsGrid::mouseDown)
EVT_LEFT_UP(EffectsGrid::mouseReleased)
//EVT_RIGHT_DOWN(EffectsGrid::rightClick)
//EVT_LEAVE_WINDOW(EffectsGrid::mouseLeftWindow)
//EVT_SIZE(EffectsGrid::resized)
//EVT_KEY_DOWN(EffectsGrid::keyPressed)
//EVT_KEY_UP(EffectsGrid::keyReleased)
EVT_PAINT(EffectsGrid::render)
END_EVENT_TABLE()
// some useful events to use
void EffectsGrid::mouseMoved(wxMouseEvent& event)
{
    int rowIndex = GetRow(event.GetY());
    if(rowIndex >= mSequenceElements->GetRowInformationSize())
    {
        SetCursor(wxCURSOR_DEFAULT);
        Refresh(false);
        return;
    }
    if(mResizing)
    {
        Resize(event.GetX());
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
    }
    else
    {
        Element* element = mSequenceElements->GetRowInformation(rowIndex)->element;
        RunMouseOverHitTests(element,event.GetX(),event.GetY());
    }
    Refresh(false);
}

void EffectsGrid::Resize(int position)
{
    if(mResizingMode==EFFECT_RESIZE_LEFT)
    {
        double time = mTimeline->GetAbsoluteTimefromPosition(position);
        time = ElementEffects::RoundToMultipleOfPeriod(time,mTimeline->GetTimeFrequency());
        if(mElementEffects->IsStartTimeLinked(mResizeEffectIndex))
        {
            mElementEffects->GetEffect(mResizeEffectIndex-1)->EndTime = time;
        }
        mElementEffects->GetEffect(mResizeEffectIndex)->StartTime = time;
    }
    else if(mResizingMode==EFFECT_RESIZE_RIGHT)
    {
        double time = mTimeline->GetAbsoluteTimefromPosition(position);;
        time = ElementEffects::RoundToMultipleOfPeriod(time,mTimeline->GetTimeFrequency());
        if(mElementEffects->IsEndTimeLinked(mResizeEffectIndex))
        {
            mElementEffects->GetEffect(mResizeEffectIndex+1)->StartTime = time;
        }
        mElementEffects->GetEffect(mResizeEffectIndex)->EndTime = time;
    }
    Refresh();
    mPaintOnIdleCounter=0;
    // Move time line and waveform to new position
    UpdateTimePosition(position);
}

void EffectsGrid::RunMouseOverHitTests(Element* element,int x,int y)
{
    int effectIndex;
    int result;

    ElementEffects* effects = element->GetElementEffects();
    bool isHit = effects->HitTestEffect(x,effectIndex,result);
    if(isHit)
    {
        mElementEffects = effects;
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

void EffectsGrid::mouseDown(wxMouseEvent& event)
{
    int FirstSelected;
    if(!(event.ShiftDown() || event.ControlDown()))
    {
        mSequenceElements->UnSelectAllEffects();
    }

    int row = GetRow(event.GetY());
    if(row>=mSequenceElements->GetRowInformationSize())
        return;
    mSequenceElements->SelectEffectsInRowAndPositionRange(row,row,event.GetX(),event.GetX(),FirstSelected);
    if(mSelectedRow!=row || mSelectedEffectIndex!=FirstSelected && FirstSelected>=0)
    {
        Element* element = mSequenceElements->GetRowInformation(row)->element;
        RaiseSelectedEffectChanged(element,FirstSelected);
    }
    mSelectedRow = row;
    mSelectedEffectIndex = FirstSelected;
    mPaintOnIdleCounter = 0;

    if(mResizingMode!=EFFECT_RESIZE_NO)
    {
        mResizing = true;
    }
    else
    {
        mDragging = true;
        mDragStartX = event.GetX();
        mDragStartY = event.GetY();
        mDragEndX = event.GetX();
        mDragEndY = event.GetY();
   }

   UpdateTimePosition(event.GetX());
   event.Skip(true);
}

void EffectsGrid::UpdateTimePosition(int position)
{
    // Update time selection
    wxCommandEvent eventTimeSelected(EVT_TIME_SELECTED);
    eventTimeSelected.SetInt(position);
    wxPostEvent(mParent, eventTimeSelected);
}


void EffectsGrid::mouseReleased(wxMouseEvent& event)
{
    mResizing = false;
    mDragging = false;
    mPaintOnIdleCounter = 0;
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

void EffectsGrid::rightClick(wxMouseEvent& event) {}
void EffectsGrid::mouseLeftWindow(wxMouseEvent& event) {}
void EffectsGrid::keyPressed(wxKeyEvent& event) {}
void EffectsGrid::keyReleased(wxKeyEvent& event) {}

//EffectsGrid::EffectsGrid(wxWindow* parent, int* args) :
EffectsGrid::EffectsGrid(wxScrolledWindow* parent, wxWindowID id, const wxPoint &pos, const wxSize &size,
                       long style, const wxString &name):wxGLCanvas(parent, wxID_ANY, nullptr, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
{
    mIsInitialized = false;
    mParent = parent;
    mDragging = false;
	m_context = new wxGLContext(this);
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
    mEffectColor = new wxColour(192,192,192);
    mGridlineColor = new wxColour(40,40,40);
    mTimingColor = new wxColour(255,255,255);
    mTimingVerticalLine = new wxColour(130,178,207);
    mSelectionColor = new wxColour(255,255,0);

    mPaintOnIdleCounter=0;

}

EffectsGrid::~EffectsGrid()
{
	delete m_context;
}

void EffectsGrid::OnIdle(wxIdleEvent &event)
{
    // This is a hack to get the grid to repaint after row header
    // information has changed. The counter prevents grid from
    // continuously repainting during idle causing excessive
    // cpu usage. It will only repaint on idle for 25 times
    // mPaintOnIdleCounter is reset to "0".
    if(mPaintOnIdleCounter < 25)
    {
        Refresh(false);
        mPaintOnIdleCounter++;
    }
};

void EffectsGrid::SetTimeline(TimeLine* timeline)
{
        mTimeline = timeline;
}

void EffectsGrid::ClearBackground()
{
    wxGLCanvas::SetCurrent(*m_context);
    wxClientDC dc(this); // only to be used in paint events. use wxClientDC to paint outside the paint event
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    SwapBuffers();
    return;
}


void EffectsGrid::resized(wxSizeEvent& evt)
{
}


/** Inits the OpenGL viewport for drawing in 2D. */
void EffectsGrid::prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
    glDisable(GL_TEXTURE_2D);   // textures
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glViewport(topleft_x, topleft_y, bottomrigth_x-topleft_x, bottomrigth_y-topleft_y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(topleft_x, bottomrigth_x, bottomrigth_y, topleft_y, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int EffectsGrid::getWidth()
{
    return GetSize().x;
}

int EffectsGrid::getHeight()
{
    return GetSize().y;
}

void EffectsGrid::SetCanvasSize(int w, int h)
{
    SetSize(wxSize(w,h));
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
    mIsInitialized = true;
    wxGLCanvas::SetCurrent(*m_context);
    wxClientDC dc(this);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    prepare2DViewport(0,0,getWidth(), getHeight());
    glLoadIdentity();
    CreateEffectIconTextures();
}

void EffectsGrid::StartDrawing(wxDouble pointSize)
{
    mIsDrawing = true;
    wxGLCanvas::SetCurrent(*m_context);
    wxClientDC dc(this);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    prepare2DViewport(0,0,getWidth(), getHeight());
    glPointSize( pointSize );
}

void EffectsGrid::DrawPoint(const wxColour &color, wxDouble x, wxDouble y)
{
    glColor3ub(color.Red(), color.Green(),color.Blue());
    glVertex2f(x, y);
}

void EffectsGrid::EndDrawing()
{
    glEnd();
    glFlush();
    SwapBuffers();
    mIsDrawing = false;
}

void EffectsGrid::DrawHorizontalLines()
{
    // Draw Horizontal lines
    int x1=0;
    int x2 = getWidth()-1;
    int y1,y2;
    for(int row=0;(row*22)< getHeight();row++)
    {
        y1 = row*DEFAULT_ROW_HEADING_HEIGHT;
        DrawLine(*mGridlineColor,255,x1,y1,x2,y1,.2);
    }
}

void EffectsGrid::DrawVerticalLines()
{
    int x1=0;
    int x2 = getWidth()-1;
    // Draw vertical lines
    int y1 = 0;
    int y2 = getHeight()-1;
    for(int x1=0;x1<getWidth();x1++)
    {
        // Draw hash marks
        if ((x1+mStartPixelOffset)%(PIXELS_PER_MAJOR_HASH)==0)
        {
            DrawLine(*mGridlineColor,255,x1,y1,x1,y2,.2);
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
    ElementEffects* effects =mSequenceElements->GetRowInformation(row)->element->GetElementEffects();
    wxColour* mEffectColorRight;
    wxColour* mEffectColorLeft;
    wxColour* mEffectColorCenter;
    for(int effectIndex=0;effectIndex < effects->GetEffectCount();effectIndex++)
    {
        Effect_Struct* e = effects->GetEffect(effectIndex);
        EFFECT_SCREEN_MODE mode;

        int y1 = (row*DEFAULT_ROW_HEADING_HEIGHT)+2;
        int y2 = ((row+1)*DEFAULT_ROW_HEADING_HEIGHT)-2;
        int y = (row*DEFAULT_ROW_HEADING_HEIGHT) + (DEFAULT_ROW_HEADING_HEIGHT/2);
        int x1,x2;
        mTimeline->GetPositionFromTime(effects->GetEffect(effectIndex)->StartTime,
                                       effects->GetEffect(effectIndex)->EndTime,mode,x1,x2);
        int x = x2-x1;
        // Draw Left line
        mEffectColorLeft = effects->GetEffect(effectIndex)->Selected == EFFECT_NOT_SELECTED ||
                           effects->GetEffect(effectIndex)->Selected == EFFECT_RT_SELECTED?mEffectColor:mSelectionColor;
        mEffectColorRight = effects->GetEffect(effectIndex)->Selected == EFFECT_NOT_SELECTED ||
                           effects->GetEffect(effectIndex)->Selected == EFFECT_LT_SELECTED?mEffectColor:mSelectionColor;
        mEffectColorCenter = effects->GetEffect(effectIndex)->Selected == EFFECT_SELECTED?mSelectionColor:mEffectColor;

        if (mode==SCREEN_L_R_OFF)
        {
            effects->GetEffect(effectIndex)->StartPosition = -10;
            effects->GetEffect(effectIndex)->EndPosition = -10;
        }
        else
        {
            if(mode==SCREEN_L_R_ON || mode == SCREEN_L_ON)
            {
                if(effectIndex>0)
                {
                    // Draw left line if effect has different start time then previous effect or
                    // previous effect was not selected, or only left was selected
                    if(effects->GetEffect(effectIndex)->StartTime != effects->GetEffect(effectIndex-1)->EndTime ||
                       effects->GetEffect(effectIndex-1)->Selected == EFFECT_NOT_SELECTED ||
                        effects->GetEffect(effectIndex-1)->Selected == EFFECT_LT_SELECTED)
                    {
                        DrawLine(*mEffectColorLeft,255,x1,y1,x1,y2,2);
                    }
                }
                else
                {
                    DrawLine(*mEffectColorLeft,255,x1,y1,x1,y2,2);
                }
                effects->GetEffect(effectIndex)->StartPosition = x1;
            }
            else
            {
                effects->GetEffect(effectIndex)->StartPosition = -10;
            }

            // Draw Right line
            if(mode==SCREEN_L_R_ON || mode == SCREEN_R_ON)
            {
                DrawLine(*mEffectColorRight,255,x2,y1,x2,y2,2);
                effects->GetEffect(effectIndex)->EndPosition = x2;
            }
            else
            {
                effects->GetEffect(effectIndex)->EndPosition = getWidth()+10;
            }

            // Draw horizontal
            if(mode!=SCREEN_L_R_OFF)
            {
                if(x > MINIMUM_EFFECT_WIDTH_FOR_ICON)
                {
                    DrawLine(*mEffectColorLeft,255,x1,y,x1+(x/2)-9,y,1);
                    DrawLine(*mEffectColorRight,255,x1+(x/2)+9,y,x2,y,1);
                    DrawRectangle(*mEffectColor,false,x1+(x/2)-9,y1,x1+(x/2)+9,y2);
                    glEnable(GL_TEXTURE_2D);
                    DrawEffectIcon(&m_EffectTextures[e->EffectIndex],x1+(x/2)-11,row*DEFAULT_ROW_HEADING_HEIGHT);
                    glDisable(GL_TEXTURE_2D);

                }
                else if (x > MINIMUM_EFFECT_WIDTH_FOR_SMALL_RECT)
                {
                    DrawLine(*mEffectColorLeft,255,x1,y,x1+(x/2)-1,y,1);
                    DrawLine(*mEffectColorRight,255,x1+(x/2)+1,y,x2,y,1);
                    DrawRectangle(*mEffectColor,false,x1+(x/2)-1,y-1,x1+(x/2)+1,y+1);
                }
                else
                {
                    DrawLine(*mEffectColorCenter,255,x1,y,x2,y,1);
                }
            }

        }
    }
}


//
//    glEnable(GL_BLEND);
//    DrawFillRectangle(wxColor(255,255,255),128,400,33,100,100);
//    glDisable(GL_BLEND);


void EffectsGrid::DrawTimingEffects(int row)
{
    Element* element =mSequenceElements->GetRowInformation(row)->element;
    ElementEffects* effects =mSequenceElements->GetRowInformation(row)->element->GetElementEffects();
    wxColour* mEffectColorRight;
    wxColour* mEffectColorLeft;
    wxColour* mEffectColorCenter;

    for(int effectIndex=0;effectIndex < effects->GetEffectCount();effectIndex++)
    {
        Effect_Struct* e = effects->GetEffect(effectIndex);
        EFFECT_SCREEN_MODE mode;

        int y1 = (row*DEFAULT_ROW_HEADING_HEIGHT)+4;
        int y2 = ((row+1)*DEFAULT_ROW_HEADING_HEIGHT)-4;
        int y = (row*DEFAULT_ROW_HEADING_HEIGHT) + (DEFAULT_ROW_HEADING_HEIGHT/2);
        int x1,x2;

        mTimeline->GetPositionFromTime(effects->GetEffect(effectIndex)->StartTime,
                                       effects->GetEffect(effectIndex)->EndTime,mode,x1,x2);
        mEffectColorLeft = effects->GetEffect(effectIndex)->Selected == EFFECT_NOT_SELECTED ||
                           effects->GetEffect(effectIndex)->Selected == EFFECT_RT_SELECTED?mTimingColor:mSelectionColor;
        mEffectColorRight = effects->GetEffect(effectIndex)->Selected == EFFECT_NOT_SELECTED ||
                           effects->GetEffect(effectIndex)->Selected == EFFECT_LT_SELECTED?mTimingColor:mSelectionColor;
        mEffectColorCenter = effects->GetEffect(effectIndex)->Selected == EFFECT_SELECTED?mSelectionColor:mTimingColor;

        if(mode==SCREEN_L_R_OFF)
        {
            effects->GetEffect(effectIndex)->StartPosition = -10;
            effects->GetEffect(effectIndex)->EndPosition = -10;
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
                    if(effects->GetEffect(effectIndex)->StartTime != effects->GetEffect(effectIndex-1)->EndTime ||
                       effects->GetEffect(effectIndex-1)->Selected == EFFECT_NOT_SELECTED ||
                        effects->GetEffect(effectIndex-1)->Selected == EFFECT_LT_SELECTED)
                    {
                        DrawLine(*mEffectColorLeft,255,x1,y1,x1,y2,2);
                    }
                }
                else
                {
                    DrawLine(*mEffectColorLeft,255,x1,y1,x1,y2,2);
                }

                effects->GetEffect(effectIndex)->StartPosition = x1;
                if(element->GetActive())
                {
                    glEnable(GL_BLEND);
                    DrawLine(*RowHeading::GetTimingColor(mSequenceElements->GetRowInformation(row)->colorIndex),128,x1,(row+1)*DEFAULT_ROW_HEADING_HEIGHT,x1,GetSize().y,1);
                    glDisable(GL_BLEND);
                }
            }
            else
            {
                effects->GetEffect(effectIndex)->StartPosition = -10;
            }
            // Draw Right line
            if(mode==SCREEN_L_R_ON || mode == SCREEN_R_ON)
            {
                DrawLine(*mEffectColorRight,255,x2,y1,x2,y2,2);
                effects->GetEffect(effectIndex)->EndPosition = x2;
                if(element->GetActive())
                {
                    glEnable(GL_BLEND);
                    DrawLine(*RowHeading::GetTimingColor(mSequenceElements->GetRowInformation(row)->colorIndex),128,x2,(row+1)*DEFAULT_ROW_HEADING_HEIGHT,x2,GetSize().y,1);
                    glDisable(GL_BLEND);
                }
            }
            else
            {
                effects->GetEffect(effectIndex)->EndPosition = getWidth()+10;
            }
            // Draw horizontal
            if(mode!=SCREEN_L_R_OFF)
            {
                DrawLine(*mTimingColor,255,x1,y,x2,y,2);
            }

        }
    }
}

void EffectsGrid::render( wxPaintEvent& evt )
{
    if(!mIsInitialized) return;
    wxGLCanvas::SetCurrent(*m_context);
    wxPaintDC(this); // only to be used in paint events. use wxClientDC to paint outside the paint event
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    prepare2DViewport(0,0,getWidth(), getHeight());
    DrawHorizontalLines();
    DrawVerticalLines();
    DrawEffects();
    if(mDragging)
    {
        DrawRectangle(*wxYELLOW,true,mDragStartX,mDragStartY,mDragEndX,mDragEndY);
    }
    //glEnable(GL_BLEND);
    glFlush();
    SwapBuffers();
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

void EffectsGrid::DrawLine(const wxColour &color, wxByte alpha,int x1, int y1,int x2, int y2,float width)
{
    glLineWidth(width);
    glColor4ub(color.Red(), color.Green(),color.Blue(),alpha);
    glBegin(GL_LINES);
    glVertex2i(x1, y1);
    glVertex2i(x2, y2);
    glEnd();
}

void EffectsGrid::CreateEffectIconTextures()
{
    const char** p_XPM;
    for(int effectID=0;effectID<NUMBER_OF_EFFECTS;effectID++)
    {
        switch(effectID)
        {
            case EFFECT_BARS:
                p_XPM = bars;
                break;
            case EFFECT_BUTTERFLY:
                p_XPM = butterfly;
                break;
            case EFFECT_CIRCLES:
                p_XPM = circles;
                break;
            case EFFECT_COLORWASH:
                p_XPM = ColorWash;
                break;
            case EFFECT_CURTAIN:
                p_XPM = curtain;
                break;
            case EFFECT_FIRE:
                p_XPM = fire;
                break;
            case EFFECT_FIREWORKS:
                p_XPM = fireworks;
                break;
            case EFFECT_GARLANDS:
                p_XPM = garlands;
                break;
            case EFFECT_GLEDIATOR:
                p_XPM = glediator;
                break;
            case EFFECT_LIFE:
                p_XPM = life;
                break;
            case EFFECT_METEORS:
                p_XPM = meteors;
                break;
            case EFFECT_PINWHEEL:
                p_XPM = pinwheel;
                break;
            case EFFECT_RIPPLE:
                p_XPM = ripple;
                break;
            case EFFECT_SINGLESTRAND:
                p_XPM = singleStrand;
                break;
            case EFFECT_SNOWFLAKES:
                p_XPM = snowflakes;
                break;
            case EFFECT_SNOWSTORM:
                p_XPM = snowstorm;
                break;
            case EFFECT_SPIRALS:
                p_XPM = spirals;
                break;
            case EFFECT_SPIROGRAPH:
                p_XPM = spirograph;
                break;
            case EFFECT_TREE:
                p_XPM = tree;
                break;
            case EFFECT_TWINKLE:
                p_XPM = twinkle;
                break;
            case EFFECT_WAVE:
                p_XPM = wave;
                break;
            default:
                break;
        }
        CreateOrUpdateTexture((char**)p_XPM,&m_EffectTextures[effectID]);
    }
}

void EffectsGrid::DeleteEffectIconTextures()
{
    for(int effectID=0;effectID<NUMBER_OF_EFFECTS;effectID++)
    {
        glDeleteTextures(1,&m_EffectTextures[effectID]);
    }
}

void EffectsGrid::DrawRectangle(const wxColour &color, bool dashed, int x1, int y1,int x2, int y2)
{
    glColor3ub(color.Red(), color.Green(),color.Blue());
    if (!dashed)
    {
        glBegin(GL_LINES);
        glVertex2f(x1, y1);
        glVertex2f(x2, y1);

        glVertex2f(x2, y1);
        glVertex2f(x2, y2);

        glVertex2f(x2, y2);
        glVertex2f(x1, y2);

        glVertex2f(x1+1, y2);
        glVertex2f(x1+1, y1);
        glEnd();
    }
    else
    {
        glBegin(GL_POINTS);
        // Line 1
        int xs = x1<x2?x1:x2;
        int xf = x1>x2?x1:x2;
        for(int x=xs;x<=xf;x++)
        {
            if(x%8<4)
            {
                glVertex2f(x, y1);
            }
        }
        // Line 2
        int ys = y1<y2?y1:y2;
        int yf = y1>y2?y1:y2;
        for(int y=ys;y<=yf;y++)
        {
            if(y%8<4)
            {
                glVertex2f(x2,y);
            }
        }
        // Line 3
        xs = x1<x2?x1:x2;
        xf = x1>x2?x1:x2;
        for(int x=xs;x<=xf;x++)
        {
            if(x%8<4)
            {
                glVertex2f(x, y2);
            }
        }
        // Line 4
        ys = y1<y2?y1:y2;
        yf = y1>y2?y1:y2;
        for(int y=ys;y<=yf;y++)
        {
            if(y%8<4)
            {
                glVertex2f(x1,y);
            }
        }
        glEnd();
    }
}

void EffectsGrid::DrawFillRectangle(const wxColour &color, wxByte alpha, int x, int y,int width, int height)
{
    glColor4ub(color.Red(), color.Green(),color.Blue(),alpha);
    //glColor3ub(color.Red(), color.Green(),color.Blue());
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x+width, y);
    glVertex2f(x+width, y+height);
    glVertex2f(x, y+height);
    glEnd();
}


void EffectsGrid::CreateOrUpdateTexture(char** p_XPM, GLuint* texture)
{
    if (p_XPM != NULL)
    {
        wxBitmap l_Bitmap(p_XPM);
        wxImage l_Image(l_Bitmap.ConvertToImage());

        if (l_Image.IsOk() == true)
        {
            //if(*texture==0)
            //{
                glGenTextures(1,texture);
                GLuint k = *texture;
                if (*texture != 0)
                {
                    glBindTexture(GL_TEXTURE_2D, *texture);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)l_Image.GetWidth(), (GLsizei)l_Image.GetHeight(),
                             0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)l_Image.GetData());
                }
            // Comment out because we will never replace image, only create new
            //}
            //else
            //{
            //    glBindTexture(GL_TEXTURE_2D, *texture);
            //    glTexSubImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)l_Image.GetWidth(), (GLsizei)l_Image.GetHeight(),
            //             0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)l_Image.GetData());
            //}
        }
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
        wxPostEvent(GetGrandParent()->GetEventHandler(), event);
    }
}

int EffectsGrid::GetRow(int y)
{
    return y/DEFAULT_ROW_HEADING_HEIGHT;
}

void EffectsGrid::RaiseSelectedEffectChanged(Element* element,int effectIndex)
{
    // Place effect pointer in client data
    // and the effect index in the event integer
    wxCommandEvent eventEffectChanged(EVT_SELECTED_EFFECT_CHANGED);
    eventEffectChanged.SetClientData(element);
    eventEffectChanged.SetInt(effectIndex);
    wxPostEvent(GetParent(), eventEffectChanged);
}



