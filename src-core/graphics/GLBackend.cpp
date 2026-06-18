/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "GLBackend.h"

namespace xlGLBackend {

Type CompiledBackend() {
#ifdef USE_GLES
    return Type::ANGLE;
#else
    return Type::Native;
#endif
}

const char* ToString(Type t) {
    switch (t) {
    case Type::Native: return "Native";
    case Type::ANGLE:  return "ANGLE";
    }
    return "?";
}

} // namespace xlGLBackend
