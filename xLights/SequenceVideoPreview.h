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

#include "graphics/xlGraphicsBase.h"
#include "graphics/xlGraphicsAccumulators.h"

struct AVFrame;
class wxPanel;
class xlGraphicsContext;

class SequenceVideoPreview : public GRAPHICS_BASE_CLASS
{
public:
   SequenceVideoPreview(wxPanel *parent);

   virtual ~SequenceVideoPreview();

   void Render( AVFrame * frame );
   void Clear();

protected:

   void reinitTexture( xlGraphicsContext *ctx, int width, int height );

   void deleteTexture();

private:
   void paint( wxPaintEvent& evt );

   xlTexture *_texture;
   int      _texWidth;
   int      _texHeight;

   DECLARE_EVENT_TABLE()
};
