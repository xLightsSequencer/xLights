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

void Image::load(wxString path)
{
    ID=loadImage(path, &width, &height, &textureWidth, &textureHeight);

    tex_coord_x = (float)width/(float)textureWidth;
    tex_coord_y = (float)height/(float)textureHeight;
}

GLuint* Image::getID()
{
    return ID;
}

Image::~Image()
{
    glDeleteTextures (1, ID);
}
