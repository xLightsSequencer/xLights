#include "SequenceVideoPreview.h"

//#include "DrawGLUtils.h"

#include <wx/dcclient.h>
#include <wx/panel.h>

#include <libavutil/frame.h>

BEGIN_EVENT_TABLE( SequenceVideoPreview, xlGLCanvas )
EVT_PAINT( SequenceVideoPreview::paint )
END_EVENT_TABLE()


SequenceVideoPreview::SequenceVideoPreview(wxPanel *parent) : xlGLCanvas(parent, wxID_ANY), _TexId(0), _TexWidth(0), _TexHeight(0)
{

}


SequenceVideoPreview::~SequenceVideoPreview()
{
   deleteTexture();
}

void SequenceVideoPreview::InitializeGLCanvas()
{
   if ( !IsShownOnScreen() ) return;

   wxColour bg = GetBackgroundColour();
   SetCurrentGLContext();
   LOG_GL_ERRORV( glClearColor( bg.Red() / 255.f, bg.Green() / 255.f, bg.Blue() / 255.f, 0.f ) );
   mIsInitialized = true;
}

void SequenceVideoPreview::paint( wxPaintEvent& evt )
{
   if ( !mIsInitialized ) { InitializeGLCanvas(); }
   if ( !IsShownOnScreen() ) return;
   wxPaintDC( this );
   SetCurrentGLContext();

   LOG_GL_ERRORV( glClear( GL_COLOR_BUFFER_BIT ) );

   SwapBuffers();
}

void SequenceVideoPreview::Render( AVFrame *frame )
{
   if ( !mIsInitialized ) { InitializeGLCanvas(); }
   if ( !IsShownOnScreen() ) return;
   SetCurrentGLContext();

   if ( _TexId == 0 || frame->width != _TexWidth || frame->height != _TexHeight )
      reinitTexture( frame->width, frame->height );

   // Upload video frame to texture
   LOG_GL_ERRORV( glBindTexture( GL_TEXTURE_2D, _TexId ) );
   LOG_GL_ERRORV( glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, _TexWidth, _TexHeight, GL_RGB, GL_UNSIGNED_BYTE, frame->data[0] ) );
   LOG_GL_ERRORV( glBindTexture( GL_TEXTURE_2D, 0 ) );

   // Draw video frame and swap-buffers
   prepare2DViewport( 0, 0, mWindowWidth, mWindowHeight );
   cache->Ortho( 0, 0, mWindowWidth, mWindowHeight );
   cache->DrawTexture( _TexId, 0, 0, mWindowWidth, mWindowHeight );

   SwapBuffers();
}

void SequenceVideoPreview::Clear()
{
   deleteTexture();

   SetCurrentGLContext();

   LOG_GL_ERRORV( glClear( GL_COLOR_BUFFER_BIT ) );

   SwapBuffers();
   
   Refresh();
}

#define GL_CLAMP_TO_EDGE 0x812F

void SequenceVideoPreview::reinitTexture( int width, int height )
{
   deleteTexture();

   LOG_GL_ERRORV( glGenTextures( 1, &_TexId ) );
   LOG_GL_ERRORV( glBindTexture( GL_TEXTURE_2D, _TexId ) );

   LOG_GL_ERRORV( glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr ) );
   LOG_GL_ERRORV( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ) );
   LOG_GL_ERRORV( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) );
   LOG_GL_ERRORV( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ) );
   LOG_GL_ERRORV( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE ) );

   LOG_GL_ERRORV( ::glBindTexture( GL_TEXTURE_2D, 0 ) );

   _TexWidth = width;
   _TexHeight = height;
}

void SequenceVideoPreview::deleteTexture()
{
   if ( _TexId )
   {
      LOG_GL_ERRORV( glDeleteTextures( 1, &_TexId ) );
      _TexId = 0;
   }
}