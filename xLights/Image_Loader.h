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
