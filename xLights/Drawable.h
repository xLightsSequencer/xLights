#ifndef _drawable_
#define _drawable_

#include "Image.h"

class Drawable
{
public:
    int x,y, angle, hotspotX, hotspotY;
    float xscale, yscale;
    Image* image;
    bool xflip, yflip;

    Drawable(Image* image=(Image*)0);
    void setFlip(bool x, bool y);
    void move(int x, int y);
    void setHotspot(int x, int y);
    void scale(float x, float y);
    void scale(float k);
    void setImage(Image* image);
    void render();
    void rotate(int angle);

};


#endif
