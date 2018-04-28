#ifndef OPENGLSHADERS_H
#define OPENGLSHADERS_H

#include <string>

class OpenGLShaders
{
public:
   static bool HasShaderSupport();

protected:
   static unsigned compile( const std::string& vertexSource, const std::string& fragmentSource );
};

#endif // OPENGLSHADERS_H
