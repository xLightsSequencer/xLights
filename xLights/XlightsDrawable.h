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

#include "Image.h"

class xLightsDrawable
{
public:
    int x,y, angle;
    float hotspotX, hotspotY;
    float xscale, yscale;
    Image* image;
    bool xflip, yflip;

    xLightsDrawable(Image* image=(Image*)0);
    void setFlip(bool x, bool y);
    void move(int x, int y);
    void setHotspot(float x, float y);
    void scale(float x, float y);
    void scale(float k);
    void setImage(Image* image);
    void render();
    void rotate(int angle);

};
