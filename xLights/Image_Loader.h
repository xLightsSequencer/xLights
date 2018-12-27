#ifndef _image_loader
#define _image_loader


#include "wx/wx.h"

#ifdef __WXMAC__
#include "OpenGL/gl.h"
#else
//#ifdef _MSC_VER
//#include "GL/glut.h"
//#else
#include <GL/gl.h>
//#endif
#endif

GLuint loadImage(wxImage *img, int &imageWidth, int &imageHeight, int &textureWidth, int &textureHeight, bool &scaledW, bool &scaledH, bool &hasAlpha, bool useForcePowerOfTwo);

GLuint loadImage(wxString path, int &imageWidth, int &imageHeight, int &textureWidth, int &textureHeight, bool &scaledW, bool &scaledH, bool &hasAlpha, bool useForcePowerOfTwo);

#endif
