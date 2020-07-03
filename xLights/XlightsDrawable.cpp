/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "XlightsDrawable.h"
#include <iostream>

#ifdef __WXMAC__
#include "OpenGL/gl.h"
#else
//#ifdef _MSC_VER
//#include "GL/glut.h"
//#else
#include <GL/gl.h>
//#endif
#endif

#include "DrawGLUtils.h"

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

    if(angle!=0)
	{
        DrawGLUtils::Rotate(angle, 0,0,1);
    }
    
    DrawGLUtils::DrawTexture(image->getID(), -hotspotX*xscale, -hotspotY*yscale, (image->width-hotspotX)*xscale, (image->height-hotspotY)*yscale,
                             xflip? image->tex_coord_x : 0, yflip? 0 : image->tex_coord_y,
                             xflip? 0 : image->tex_coord_x, yflip? image->tex_coord_y : 0);

}



