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

#include <string>

class OpenGLShaders
{
public:
   static bool HasShaderSupport();
   static bool HasFramebufferObjects();

   static unsigned compile( const std::string& vertexSource, const std::string& fragmentSource );
};

