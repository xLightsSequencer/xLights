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

// Lightweight scanner that peeks at the first few KB of an .xsq / .xml
// file to detect whether it is an xLights sequence and to extract the
// media filename.  Does NOT parse the entire file — safe to call on
// arbitrarily large files.

#include <string>

struct XsqFileInfo {
    bool isSequence = false;   // true if <xsequence was found
    std::string mediaFile;     // content of <mediaFile>...</mediaFile>, empty if absent
};

// Read at most the first ~48 KB looking for <xsequence and <mediaFile>.
// Returns immediately once both are resolved or the buffer is exhausted.
XsqFileInfo ScanXsqFile(const std::string& filename);
