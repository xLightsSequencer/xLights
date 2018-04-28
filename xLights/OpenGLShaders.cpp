#ifdef __WXMAC__
 #include "OpenGL/gl.h"
#else
#include <GL/gl.h>
#endif

#include "OpenGLShaders.h"

#include "DrawGLUtils.h"

unsigned OpenGLShaders::compile( const std::string& vertexSource, const std::string& fragmentSource )
{
   return 0;
}
