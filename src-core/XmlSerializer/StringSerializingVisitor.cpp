/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// StringSerializingVisitor — owns an ostringstream, delegates to
// StreamSerializingVisitor for all XML writing.

#include "StringSerializingVisitor.h"

StringSerializingVisitor::StringSerializingVisitor(bool exporting, bool prettyPrint)
    : StreamSerializingVisitor(_oss, exporting, prettyPrint) {
    WriteXmlDeclaration();
}

std::string StringSerializingVisitor::GetResult() const {
    return _oss.str();
}
