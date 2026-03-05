/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// FileSerializingVisitor — writes XML to a file via an ofstream, or wraps
// an externally-owned ostream.

#include "FileSerializingVisitor.h"

FileSerializingVisitor::FileSerializingVisitor(const std::string& path, bool exporting, bool prettyPrint)
    : FileStreamHolder{std::ofstream(path, std::ios::out | std::ios::trunc)},
      StreamSerializingVisitor(_ofs, exporting, prettyPrint) {
}

FileSerializingVisitor::FileSerializingVisitor(std::ostream& os, bool exporting, bool prettyPrint)
    : StreamSerializingVisitor(os, exporting, prettyPrint) {
}

bool FileSerializingVisitor::IsOpen() const {
    return _ofs.is_open();
}
