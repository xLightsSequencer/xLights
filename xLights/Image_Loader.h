#ifndef _image_loader
#define _image_loader
#ifdef __WXMAC__
#include "OpenGL/gl.h"
#else
#include <GL/gl.h>
#endif
#include "wx/wx.h"

GLuint* loadImage(wxImage *img, int &imageWidth, int &imageHeight, int &textureWidth, int &textureHeight, bool &scaledW, bool &scaledH, bool &hasAlpha);

GLuint* loadImage(wxString path, int &imageWidth, int &imageHeight, int &textureWidth, int &textureHeight, bool &scaledW, bool &scaledH, bool &hasAlpha);

#endif
