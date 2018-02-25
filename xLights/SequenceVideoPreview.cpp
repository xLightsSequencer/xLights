#include "SequenceVideoPreview.h"

#include <wx/dcclient.h>
#include <wx/panel.h>

BEGIN_EVENT_TABLE( SequenceVideoPreview, xlGLCanvas )
EVT_PAINT( SequenceVideoPreview::render )
END_EVENT_TABLE()


SequenceVideoPreview::SequenceVideoPreview(wxPanel *parent) : xlGLCanvas(parent, wxID_ANY)
{

}


SequenceVideoPreview::~SequenceVideoPreview()
{

}

void SequenceVideoPreview::InitializeGLCanvas()
{
   if ( !IsShownOnScreen() ) return;

   SetCurrentGLContext();
   LOG_GL_ERRORV( glClearColor( 0.0f, 0.0f, 0.0f, 1.0f ) );
   mIsInitialized = true;
}

void SequenceVideoPreview::render( wxPaintEvent& evt )
{
   if ( !mIsInitialized ) { InitializeGLCanvas(); }
   if ( !IsShownOnScreen() ) return;
   wxPaintDC( this );
   SetCurrentGLContext();

   LOG_GL_ERRORV( glClear( GL_COLOR_BUFFER_BIT ) );
}

void SequenceVideoPreview::Render( AVFrame *frame )
{
   if ( !mIsInitialized ) { InitializeGLCanvas(); }
   if ( !IsShownOnScreen() ) return;
   SetCurrentGLContext();

   LOG_GL_ERRORV( glClear( GL_COLOR_BUFFER_BIT ) );
   prepare2DViewport( 0, 0, mWindowWidth, mWindowHeight );
}