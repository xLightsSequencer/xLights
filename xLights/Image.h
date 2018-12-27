#ifndef IMAGE_H
#define IMAGE_H



#include "wx/wx.h"

// include OpenGL
#ifdef __WXMAC__
#include "OpenGL/gl.h"
#else
//#ifdef _MSC_VER
//#include "GL/glut.h"
//#else
#include <GL/gl.h>
//#endif
#endif

class Image
{
    GLuint ID;

public:

	/*
	 * it is preferable to use textures that are a power of two. this loader will automatically
	 * resize texture to be a power of two, filling the remaining areas with black if the gl
     * context can support textures that size.   If not, we scale the image down to a supported size
     * and then fill the entire texture.
	 * width/height are the width of the actual loaded image.
	 * textureWidth/Height are the total width of the texture, including black filling.
	 * tex_coord_x/y are the texture coord parameter you must give OpenGL when rendering
	 * to get only the image, without the black filling.
	 */
    int width, height, textureWidth, textureHeight;
    bool mAlpha;
    float tex_coord_x, tex_coord_y;

    GLuint getID() const { return ID; }
    void setID(GLuint i) { ID = i; }
    bool hasAlpha() const { return mAlpha; }
    Image();
    Image(wxString path, bool whiteIsAlphaIfNoAlpha = false, bool useForcePowerOfTwo = false);
    Image(wxImage &img, bool whiteIsAlphaIfNoAlpha = false, bool useForcePowerOfTwo = false);

    ~Image();
    void load(wxImage &path, bool whiteIsAlphaIfNoAlpha, bool useForcePowerOfTwo);
    void load(wxString path, bool whiteIsAlphaIfNoAlpha, bool useForcePowerOfTwo);
};


#endif
