#include "Image_Loader.h"
#include <cmath>

#include "wx/image.h"
#include "wx/wx.h"
#define GL_CLAMP_TO_EDGE 0x812F

#include "DrawGLUtils.h"
#include <log4cpp/Category.hh>

GLuint loadImage(wxString path, int &imageWidth, int &imageHeight, int &textureWidth, int &textureHeight,
                 bool &scaledW, bool &scaledH, bool &hasAlpha, bool useForcePowerOfTwo)
{
    // check the file exists
    if(!wxFileExists(path))
    {
        wxMessageBox( _("Failed to load resource image") );
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.crit("Failed to load resource image: " + path);
		exit(1);
    }
    
    wxImage img( path );
    return loadImage(&img, imageWidth, imageHeight, textureWidth, textureHeight,
                     scaledW, scaledH, hasAlpha, useForcePowerOfTwo);

}

GLuint loadImage(wxImage *img, int &imageWidth, int &imageHeight, int &textureWidth, int &textureHeight,
                  bool &scaledW, bool &scaledH, bool &hasAlpha, bool useForcePowerOfTwo)
{

	GLuint ID = 0;
    if (!DrawGLUtils::IsCoreProfile()) {
        LOG_GL_ERRORV(glEnable(GL_TEXTURE_2D));
    }
    int maxSize = 0;
    LOG_GL_ERRORV(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize));
	LOG_GL_ERRORV(glGenTextures( 1, &ID ));
    LOG_GL_ERRORV(glBindTexture( GL_TEXTURE_2D, ID ));



	imageWidth=img->GetWidth();
	imageHeight=img->GetHeight();

	LOG_GL_ERRORV(glPixelStorei(GL_UNPACK_ALIGNMENT,   1   ));
    /*
     * Many graphics card require that textures be power of two.
     * Below is a simple implementation, probably not optimal but working.
     * If your texture sizes are not restricted to power of 2s, you can
     * of course adapt the bit below as needed.
     */

	float power_of_two_that_gives_correct_width=std::log((float)imageWidth)/std::log(2.0);
	float power_of_two_that_gives_correct_height=std::log((float)imageHeight)/std::log(2.0);

    textureWidth  = imageWidth;
    textureHeight = imageHeight;
    scaledH = scaledW = false;
    if( (int)power_of_two_that_gives_correct_width != power_of_two_that_gives_correct_width ||
       (int)power_of_two_that_gives_correct_height != power_of_two_that_gives_correct_height)
    {
        textureWidth=(int)std::pow( 2.0, (int)(std::ceil(power_of_two_that_gives_correct_width)) );
        textureHeight=(int)std::pow( 2.0, (int)(std::ceil(power_of_two_that_gives_correct_height)) );
        while (textureWidth > maxSize) {
            //we have to scale down, we'll use the entire texture and have opengl scale it to the appropriate aspect ratio
            scaledW = true;
            textureWidth /= 2;
        }
        while (textureHeight > maxSize) {
            //we have to scale down, we'll use the entire texture and have opengl scale it to the appropriate aspect ratio
            scaledH = true;
            textureHeight /= 2;
        }
        if (useForcePowerOfTwo) {
            scaledW = true;
            scaledH = true;
            imageWidth = textureWidth;
            imageHeight = textureHeight;
        }
        if (scaledH || scaledW) {
            img->Rescale(scaledW ? textureWidth : imageWidth,
                         scaledH ? textureHeight : imageHeight,
                         wxIMAGE_QUALITY_HIGH);
        }
    }

    // note: must make a local copy before passing the data to OpenGL, as GetData() returns RGB
    // and we want the Alpha channel if it's present. Additionally OpenGL seems to interpret the
    // data upside-down so we need to compensate for that.
    GLubyte *bitmapData=img->GetData();
    GLubyte *alphaData=img->GetAlpha();

    hasAlpha = img->HasAlpha();
    int bytesPerPixel = img->HasAlpha() ?  4 : 3;

    int imageSize = textureWidth * textureHeight * bytesPerPixel;
    GLubyte *imageData=new GLubyte[imageSize];

    int rev_val= scaledH ? textureHeight-1 : imageHeight - 1;
    int bmpDataWid = scaledW ? textureWidth : imageWidth;

    for(int y=0; y< textureHeight; y++) {
            for(int x=0; x< textureWidth; x++) {
                if ((scaledW || x < imageWidth) && (scaledH || y < imageHeight)) {
                    //image and texture sizes match
                    imageData[(x+y*textureWidth)*bytesPerPixel+0]=
                            bitmapData[( x+(rev_val-y)*bmpDataWid)*3];

                    imageData[(x+y*(textureWidth))*bytesPerPixel+1]=
                            bitmapData[( x+(rev_val-y)*(bmpDataWid))*3 + 1];

                    imageData[(x+y*(textureWidth))*bytesPerPixel+2]=
                            bitmapData[( x+(rev_val-y)*(bmpDataWid))*3 + 2];

                    if(bytesPerPixel==4) imageData[(x+y*(textureWidth))*bytesPerPixel+3]=
                            alphaData[ x+(rev_val-y)*(bmpDataWid) ];
                } else {
                    imageData[(x+y*textureWidth)*bytesPerPixel+0] = 0;
                    imageData[(x+y*textureWidth)*bytesPerPixel+1] = 0;
                    imageData[(x+y*textureWidth)*bytesPerPixel+2] = 0;
                    if(bytesPerPixel==4) imageData[(x+y*(textureWidth))*bytesPerPixel+3] = 0;
                }
            }//next
    }//next

    // if yes, everything is fine
    LOG_GL_ERRORV(glTexImage2D(GL_TEXTURE_2D,
                 0,
                 img->HasAlpha() ?  GL_RGBA : GL_RGB,
                 textureWidth,
                 textureHeight,
                 0,
                 img->HasAlpha() ?  GL_RGBA : GL_RGB,
                 GL_UNSIGNED_BYTE,
                 imageData));

    delete [] imageData;
	// set texture parameters as you wish
	LOG_GL_ERRORV(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)); // GL_LINEAR
	LOG_GL_ERRORV(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)); // GL_LINEAR
	LOG_GL_ERRORV(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    LOG_GL_ERRORV(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    if (!DrawGLUtils::IsCoreProfile()) {
        LOG_GL_ERRORV(glDisable(GL_TEXTURE_2D));
    }
	return ID;

}
