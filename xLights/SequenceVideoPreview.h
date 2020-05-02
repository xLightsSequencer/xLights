#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "xlGLCanvas.h"

struct AVFrame;
class wxPanel;

class SequenceVideoPreview : public xlGLCanvas
{
public:
   SequenceVideoPreview(wxPanel *parent);

   virtual ~SequenceVideoPreview();

   void Render( AVFrame * frame );
   void Clear();

protected:
   void InitializeGLContext() override;

   void reinitTexture( int width, int height );

   void deleteTexture();

private:
   void paint( wxPaintEvent& evt );

   unsigned _texId;
   int      _texWidth;
   int      _texHeight;

   DECLARE_EVENT_TABLE()
};
