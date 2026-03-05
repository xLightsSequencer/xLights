#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// StringSerializingVisitor serializes model/object data to a std::string
// via an internal ostringstream.  All stream-writing logic is inherited
// from StreamSerializingVisitor; this class only adds GetResult().

#include "StreamSerializingVisitor.h"

#include <sstream>
#include <string>

class StringSerializingVisitor : public StreamSerializingVisitor {
public:
    explicit StringSerializingVisitor(bool exporting = false, bool prettyPrint = true);

    // Retrieve the accumulated XML string (includes the leading declaration).
    [[nodiscard]] std::string GetResult() const;

private:
    std::ostringstream _oss;
};
