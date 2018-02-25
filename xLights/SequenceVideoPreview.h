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

private:
   void render( wxPaintEvent& evt );

   DECLARE_EVENT_TABLE()
};

#endif