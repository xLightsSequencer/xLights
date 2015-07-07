#include "XlightsDrawable.h"
#include <iostream>

#ifdef __WXMAC__
#include "OpenGL/gl.h"
#else
#include <GL/gl.h>
#endif

//#include "wx/wx.h"

/*
 * This is a simple class built on top of OpenGL that manages drawing images in a higher-level and quicker way.
 */

xLightsDrawable::xLightsDrawable(Image* image_arg)
{

    x=0;
    y=0;
    hotspotX=0.0;
    hotspotY=0.0;
    angle=0;

    xscale=1;
    yscale=1;

    xflip=false;
    yflip=true;

    if(image_arg!=NULL) setImage(image_arg);
    else image=NULL;
}

void xLightsDrawable::setFlip(bool x, bool y)
{
    xflip=x;
    yflip=y;
}

void xLightsDrawable::setHotspot(float x, float y)
{
    hotspotX=x;
    hotspotY=y;
}

void xLightsDrawable::move(int x, int y)
{
    xLightsDrawable::x=x;
    xLightsDrawable::y=y;
}

void xLightsDrawable::scale(float x, float y)
{
    xLightsDrawable::xscale=x;
    xLightsDrawable::yscale=y;
}

void xLightsDrawable::scale(float k)
{
    xLightsDrawable::xscale=k;
    xLightsDrawable::yscale=k;
}

void xLightsDrawable::setImage(Image* image)
{
    xLightsDrawable::image=image;
}

void xLightsDrawable::rotate(int angle)
{
    xLightsDrawable::angle=angle;
}

void xLightsDrawable::render()
{
    assert(image!=NULL);

    //glLoadIdentity();

    //glTranslatef(x,y,0);

    if(xscale!=1 || yscale!=1)
	{
        glScalef(xscale, yscale, 1);
    }

    if(angle!=0)
	{
        glRotatef(angle, 0,0,1);
    }

    glBindTexture(GL_TEXTURE_2D, image->getID()[0] );

    glBegin(GL_QUADS);

    glTexCoord2f(xflip? image->tex_coord_x : 0, yflip? 0 : image->tex_coord_y);
    glVertex2f( -hotspotX, -hotspotY );

    glTexCoord2f(xflip? 0 : image->tex_coord_x, yflip? 0 : image->tex_coord_y);
    glVertex2f( image->width-hotspotX, -hotspotY );

    glTexCoord2f(xflip? 0 : image->tex_coord_x, yflip? image->tex_coord_y : 0);
    glVertex2f( image->width-hotspotX, image->height-hotspotY );

    glTexCoord2f(xflip? image->tex_coord_x : 0, yflip? image->tex_coord_y : 0);
    glVertex2f( -hotspotX, image->height-hotspotY );

    glEnd();

}



