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
#include "Image.h"
#include "Image_Loader.h"

Image::Image() : ID(0)
{
}

Image::Image(wxString path, bool whiteIsAlphaIfNoAlpha, bool useForcePowerOfTwo) : ID(0)
{
    load(path, whiteIsAlphaIfNoAlpha, useForcePowerOfTwo);
}

Image::Image(wxImage &img, bool whiteIsAlphaIfNoAlpha, bool useForcePowerOfTwo) : ID(0)
{
    load(img, whiteIsAlphaIfNoAlpha, useForcePowerOfTwo);
}

void Image::load(wxString path, bool whiteIsAlphaIfNoAlpha, bool useForcePowerOfTwo)
{
    wxImage img(path);
    load(img, whiteIsAlphaIfNoAlpha, useForcePowerOfTwo);
}

void Image::load(wxImage &img, bool whiteIsAlphaIfNoAlpha, bool useForcePowerOfTwo)
{
    mAlpha = img.HasAlpha();

    if (!mAlpha && whiteIsAlphaIfNoAlpha)
    {
        img.InitAlpha();
        for (int x = 0; x < img.GetWidth(); x++)
        {
            for (int y = 0; y < img.GetHeight(); y++)
            {
                int r = img.GetRed(x,y);
                if (r == img.GetGreen(x, y) && r == img.GetBlue(x, y))
                {
                    img.SetAlpha(x, y, r);
                }
            }
        }
    }

    bool scaledW, scaledH;
    ID = loadImage(&img, width, height, textureWidth, textureHeight, scaledW, scaledH, mAlpha, useForcePowerOfTwo);
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

Image::~Image()
{
    if (ID != 0) {
        glDeleteTextures(1, &ID);
    }
}
