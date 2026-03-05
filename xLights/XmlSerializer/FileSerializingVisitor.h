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

// FileSerializingVisitor serializes model/object data directly to a file
// (or any external std::ostream).  All stream-writing logic is inherited
// from StreamSerializingVisitor.

#include "StreamSerializingVisitor.h"

#include <fstream>
#include <string>

// Private base ensures _ofs is constructed before StreamSerializingVisitor
// (base classes initialize left-to-right before members).
struct FileStreamHolder {
    std::ofstream _ofs;
};

class FileSerializingVisitor : private FileStreamHolder, public StreamSerializingVisitor {
public:
    // Opens an internal ofstream to the given path.
    explicit FileSerializingVisitor(const std::string& path, bool exporting = false, bool prettyPrint = true);

    // Wraps an external ostream (caller owns the stream lifetime).
    explicit FileSerializingVisitor(std::ostream& os, bool exporting = false, bool prettyPrint = true);

    // For the file-path constructor: check whether the file was opened successfully.
    [[nodiscard]] bool IsOpen() const;
};
