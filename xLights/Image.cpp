#include "wx/wx.h"
#include "Image.h"
#include "Image_Loader.h"

Image::Image()
{
}

Image::Image(wxString path)
{
    load(path);
}

Image::Image(wxImage &img)
{
    load(img);
}

void Image::load(wxString path)
{
    bool scaledW, scaledH;
    ID=loadImage(path, width, height, textureWidth, textureHeight, scaledW, scaledH, mAlpha);
    if (scaledW) {
        tex_coord_x = 1.0;
    } else {
        tex_coord_x = (float)width/(float)textureWidth;
    }
    if (scaledH) {
        tex_coord_y = 1.0;
    } else {
        tex_coord_y = (float)height/(float)textureHeight;
    }
}
void Image::load(wxImage &img)
{
    bool scaledW, scaledH;
    ID=loadImage(&img, width, height, textureWidth, textureHeight, scaledW, scaledH, mAlpha);
    if (scaledW) {
        tex_coord_x = 1.0;
    } else {
        tex_coord_x = (float)width/(float)textureWidth;
    }
    if (scaledH) {
        tex_coord_y = 1.0;
    } else {
        tex_coord_y = (float)height/(float)textureHeight;
    }
}
GLuint* Image::getID()
{
    return ID;
}

Image::~Image()
{
    glDeleteTextures (1, ID);
}
