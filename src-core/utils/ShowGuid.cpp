/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ShowGuid.h"

#include "UtilFunctions.h"
#include "globals.h"

#include <pugixml.hpp>

namespace ShowGuid {

std::string ReadFromShowFolder(std::string const& showDir)
{
    if (showDir.empty()) {
        return {};
    }

    pugi::xml_document doc;
    std::string const path = showDir + GetPathSeparator() + XLIGHTS_RGBEFFECTS_FILE;
    if (!doc.load_file(path.c_str())) {
        return {};
    }

    pugi::xml_node root = doc.child("xrgb");
    if (!root) {
        root = doc.child("xlights");
    }
    if (!root) {
        return {};
    }
    for (pugi::xml_node s = root.child("settings").first_child(); s; s = s.next_sibling()) {
        if (std::string(s.name()) == "ShowGUID") {
            return s.attribute("value").as_string("");
        }
    }
    return {};
}

} // namespace ShowGuid
