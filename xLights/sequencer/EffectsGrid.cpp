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
//EVT_MOTION(EffectsGrid::mouseMoved)
EVT_MOUSEWHEEL(EffectsGrid::mouseWheelMoved)
EVT_LEFT_DOWN(EffectsGrid::mouseDown)
//EVT_LEFT_UP(EffectsGrid::mouseReleased)
//EVT_RIGHT_DOWN(EffectsGrid::rightClick)
//EVT_LEAVE_WINDOW(EffectsGrid::mouseLeftWindow)
//EVT_SIZE(EffectsGrid::resized)
//EVT_KEY_DOWN(EffectsGrid::keyPressed)
//EVT_KEY_UP(EffectsGrid::keyReleased)
EVT_PAINT(EffectsGrid::render)
END_EVENT_TABLE()
// some useful events to use
void EffectsGrid::mouseMoved(wxMouseEvent& event) {}
void EffectsGrid::mouseDown(wxMouseEvent& event)
{
    wxCommandEvent eventTimeSelected(EVT_TIME_SELECTED);
    eventTimeSelected.SetInt(event.GetX());
    wxPostEvent(mParent, eventTimeSelected);
    event.Skip(true);
}
void EffectsGrid::mouseReleased(wxMouseEvent& event) {
}
void EffectsGrid::rightClick(wxMouseEvent& event) {}
void EffectsGrid::mouseLeftWindow(wxMouseEvent& event) {}
void EffectsGrid::keyPressed(wxKeyEvent& event) {}
void EffectsGrid::keyReleased(wxKeyEvent& event) {}

EffectsGrid::EffectsGrid(wxWindow* parent, int* args) :
    wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
{
    mParent = parent;
	m_context = new wxGLContext(this);
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
}

EffectsGrid::~EffectsGrid()
{
	delete m_context;
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

    gluOrtho2D(topleft_x, bottomrigth_x, bottomrigth_y, topleft_y);
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
    glBegin(GL_POINTS);
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

void EffectsGrid::render( wxPaintEvent& evt )
{
    if(!mIsInitialized) return;
    wxGLCanvas::SetCurrent(*m_context);
    wxPaintDC(this); // only to be used in paint events. use wxClientDC to paint outside the paint event
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int x1 = 0;
    int x2 = getWidth()-1;
    int y1,y2;
    for(int row=0;(row*22)< getHeight();row++)
    {
        y1 = row*DEFAULT_ROW_HEADING_HEIGHT;
        DrawLine(wxColor(24,24,24),x1,y1,x2,y1);
    }

    y1 = 0;
    y2 = getHeight()-1;
    for(int col=0;(col*100)< getWidth();col++)
    {
        x1 = col*100;
        DrawLine(wxColor(40,40,40),x1,y1,x1,y2);
    }

    DrawFillRectangle(wxColor(192,192,192),100,33,100,2);
    DrawRectangle(wxColor(192,192,192),false,140,24,160,42);
    DrawFillRectangle(wxColor(192,192,192),100,28,2,14);
    DrawFillRectangle(wxColor(192,192,192),200,28,2,14);
    //DrawRectangle(wxColor(0,0,0),false,100,22,200,44);

    DrawFillRectangle(wxColor(192,192,192),200,33,100,2);
    DrawRectangle(wxColor(192,192,192),false,240,24,260,42);
    DrawFillRectangle(wxColor(192,192,192),200,24,2,18);
    DrawFillRectangle(wxColor(192,192,192),300,24,2,18);
    //DrawRectangle(wxColor(0,0,0),false,100,22,300,44);

    DrawFillRectangle(wxColor(192,192,192),300,33,400,2);
    DrawRectangle(wxColor(192,192,192),false,490,24,510,42);
    DrawFillRectangle(wxColor(192,192,192),300,24,2,18);
    DrawFillRectangle(wxColor(192,192,192),700,24,2,18);
    //DrawRectangle(wxColor(0,0,0),false,300,22,300,44);






    DrawFillRectangle(wxColor(192,192,192),100,55,100,2);
    DrawRectangle(wxColor(192,192,192),false,140,46,160,64);
    DrawFillRectangle(wxColor(192,192,192),100,50,2,14);
    DrawFillRectangle(wxColor(192,192,192),200,50,2,14);
    //DrawRectangle(wxColor(0,0,0),false,100,22,200,44);



    glEnable(GL_TEXTURE_2D);
    DrawEffectIcon(&m_EffectTextures[2],139,22);
    DrawEffectIcon(&m_EffectTextures[3],239,22);
    DrawEffectIcon(&m_EffectTextures[1],489,22);

    DrawEffectIcon(&m_EffectTextures[7],139,44);

    //for(int i=0;i<20;i++)
    //{
    //    DrawEffectIcon(&m_EffectTextures[i],51,i*22);
    //}
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
    //glEnable(GL_BLEND);
    //glEnable(GL_BLEND);
    glFlush();
    SwapBuffers();
}

void EffectsGrid::DrawEffectIcon(GLuint* texture,int x, int y)
{
    glColor3f(1.0f, 1.0f, 1.0f);
    glBindTexture(GL_TEXTURE_2D,*texture);
    //glDisable(GL_BLEND);
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

}

void EffectsGrid::DrawLine(const wxColour &color, wxDouble x1, wxDouble y1,wxDouble x2, wxDouble y2)
{
    glLineWidth(.2);
    glColor3ub(color.Red(), color.Green(),color.Blue());
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
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

void EffectsGrid::DrawFillRectangle(const wxColour &color, int x, int y,int width, int height)
{
    glColor3ub(color.Red(), color.Green(),color.Blue());
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
//            ZoomOut();
        }
        else
        {
            wxCommandEvent eventZoom(EVT_ZOOM);
            eventZoom.SetInt(ZOOM_IN);
            wxPostEvent(mParent, eventZoom);
//            ZoomIn();
        }
    }
    else
    {
        wxPostEvent(GetGrandParent()->GetEventHandler(), event);
    }
}



