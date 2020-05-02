/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "SequenceVideoPreview.h"

//#include "DrawGLUtils.h"

#include <wx/dcclient.h>
#include <wx/panel.h>

#include <libavutil/frame.h>

BEGIN_EVENT_TABLE( SequenceVideoPreview, xlGLCanvas )
EVT_PAINT( SequenceVideoPreview::paint )
END_EVENT_TABLE()


SequenceVideoPreview::SequenceVideoPreview(wxPanel *parent)
    : xlGLCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, "ViewPreview"), _texId(0), _texWidth(0), _texHeight(0)
{

}


SequenceVideoPreview::~SequenceVideoPreview()
{
   deleteTexture();
}

void SequenceVideoPreview::InitializeGLContext()
{
   wxColour bg = GetBackgroundColour();
   SetCurrentGLContext();
   LOG_GL_ERRORV( glClearColor( bg.Red() / 255.f, bg.Green() / 255.f, bg.Blue() / 255.f, 0.f ) );
}

void SequenceVideoPreview::paint( wxPaintEvent& evt )
{
   if(!IsShownOnScreen()) return;
   if ( !mIsInitialized ) { InitializeGLCanvas(); }

   wxPaintDC( this );
   SetCurrentGLContext();
   wxColour bg = GetBackgroundColour();
   LOG_GL_ERRORV( glClearColor( bg.Red() / 255.f, bg.Green() / 255.f, bg.Blue() / 255.f, 0.f ) );
   LOG_GL_ERRORV( glClear( GL_COLOR_BUFFER_BIT ) );

   SwapBuffers();
}

void SequenceVideoPreview::Render( AVFrame *frame )
{
   if ( !mIsInitialized ) { InitializeGLCanvas(); }
   if ( !IsShownOnScreen() ) return;
   SetCurrentGLContext();

   if ( _texId == 0 || frame->width != _texWidth || frame->height != _texHeight )
      reinitTexture( frame->width, frame->height );

   // Upload video frame to texture
   LOG_GL_ERRORV( glBindTexture( GL_TEXTURE_2D, _texId ) );
   LOG_GL_ERRORV( glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, _texWidth, _texHeight, GL_RGB, GL_UNSIGNED_BYTE, frame->data[0] ) );
   LOG_GL_ERRORV( glBindTexture( GL_TEXTURE_2D, 0 ) );

   // Draw video frame and swap-buffers
   prepare2DViewport( 0, 0, mWindowWidth, mWindowHeight );
   cache->Ortho( 0, 0, mWindowWidth, mWindowHeight );
   cache->DrawTexture( _texId, 0, 0, mWindowWidth, mWindowHeight );

   SwapBuffers();
}

void SequenceVideoPreview::Clear()
{
    if (cache == nullptr) {
        // nothing has been displayed yet, delay allocating resources and such
        return;
    }
    SetCurrentGLContext();
    deleteTexture();


    LOG_GL_ERRORV( glClear( GL_COLOR_BUFFER_BIT ) );

    SwapBuffers();
   
    Refresh();
}

#define GL_CLAMP_TO_EDGE 0x812F

void SequenceVideoPreview::reinitTexture( int width, int height )
{
   deleteTexture();

   LOG_GL_ERRORV( glGenTextures( 1, &_texId ) );
   LOG_GL_ERRORV( glBindTexture( GL_TEXTURE_2D, _texId ) );

   LOG_GL_ERRORV( glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr ) );
   LOG_GL_ERRORV( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ) );
   LOG_GL_ERRORV( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) );
   LOG_GL_ERRORV( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ) );
   LOG_GL_ERRORV( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE ) );

   LOG_GL_ERRORV( ::glBindTexture( GL_TEXTURE_2D, 0 ) );

   _texWidth = width;
   _texHeight = height;
}

void SequenceVideoPreview::deleteTexture()
{
   if ( _texId )
   {
      LOG_GL_ERRORV( glDeleteTextures( 1, &_texId ) );
      _texId = 0;
   }
}
