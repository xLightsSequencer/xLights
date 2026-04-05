/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "XsqFileScanner.h"
#include "ExternalHooks.h"

#include <fstream>

// Decode the five XML predefined entities in a short string.
static std::string DecodeXmlEntities(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '&') {
            if (s.compare(i, 4, "&lt;") == 0) { out += '<';  i += 3; }
            else if (s.compare(i, 4, "&gt;") == 0) { out += '>';  i += 3; }
            else if (s.compare(i, 5, "&amp;") == 0) { out += '&';  i += 4; }
            else if (s.compare(i, 6, "&apos;") == 0) { out += '\''; i += 5; }
            else if (s.compare(i, 6, "&quot;") == 0) { out += '"';  i += 5; }
            else { out += s[i]; }
        } else {
            out += s[i];
        }
    }
    return out;
}

XsqFileInfo ScanXsqFile(const std::string& filename) {
    XsqFileInfo info;

    // Read the first 48 KB — metadata is always near the top of xLights files.
    static constexpr size_t SCAN_SIZE = 48 * 1024;
    std::string buf(SCAN_SIZE, '\0');

    ObtainAccessToURL(filename);

    std::ifstream file(filename, std::ios::binary);
    if (!file) return info;

    file.read(&buf[0], SCAN_SIZE);
    buf.resize(static_cast<size_t>(file.gcount()));
    if (buf.empty()) return info;

    // Look for <xsequence (with possible attributes or just >)
    if (buf.find("<xsequence") != std::string::npos) {
        info.isSequence = true;
    }

    // Look for <mediaFile>...</mediaFile>
    const std::string openTag = "<mediaFile>";
    const std::string closeTag = "</mediaFile>";
    auto start = buf.find(openTag);
    if (start != std::string::npos) {
        start += openTag.size();
        auto end = buf.find(closeTag, start);
        if (end != std::string::npos) {
            info.mediaFile = DecodeXmlEntities(buf.substr(start, end - start));
        }
    }

    return info;
}
