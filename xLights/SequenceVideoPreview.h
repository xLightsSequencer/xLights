#ifndef SEQUENCEVIDEOPREVIEW_H
#define SEQUENCEVIDEOPREVIEW_H

#include "xlGLCanvas.h"

struct AVFrame;
class wxPanel;

class SequenceVideoPreview : public xlGLCanvas
{
public:
   SequenceVideoPreview(wxPanel *parent);

   virtual ~SequenceVideoPreview();

   void Render( AVFrame * frame );

protected:
   void InitializeGLCanvas() override;

   void reinitTexture( int width, int height );

private:
   void paint( wxPaintEvent& evt );

   unsigned _TexId;
   int      _TexWidth;
   int      _TexHeight;

   DECLARE_EVENT_TABLE()
};

#endif