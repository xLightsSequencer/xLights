/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SpecialOptions.h"

#include "utils/ExternalHooks.h"
#include "UtilFunctions.h"

#include <pugixml.hpp>
#include <string>
#include <map>
#include <spdlog/spdlog.h>


std::string SpecialOptions::StashShowDir(const std::string& showDir)
{
    static std::string __showDir;
    if (showDir != "") __showDir = showDir;
    return __showDir;
}

std::string SpecialOptions::StashExeDir(const std::string& exeDir)
{
    static std::string __exeDir;
    if (exeDir != "") __exeDir = exeDir;
    return __exeDir;
}

std::string SpecialOptions::GetOption(const std::string& option, const std::string& defaultValue )
{
    static bool __loaded = false;
    static std::map<std::string, std::string> __cache;

    // Prefer show folder; fall back to the directory containing the executable.
    std::string showFile = StashShowDir() + GetPathSeparator() + "special.options";
    std::string exeFile  = StashExeDir()  + GetPathSeparator() + "special.options";
    std::string file;
    if (!StashShowDir().empty() && FileExists(showFile)) {
        file = showFile;
    } else if (!StashExeDir().empty() && FileExists(exeFile)) {
        file = exeFile;
    }

    if (option == "") {
        __loaded = false;
        __cache.clear();
    }

    if (file.empty()) {
        if (!__loaded) {
            spdlog::info("SpecialOptions: no special.options found (show='{}', exe='{}')",
                         showFile, exeFile);
            __loaded = true;
        }
        __cache.clear();
        return defaultValue;
    }

    if (!__loaded) {
        pugi::xml_document doc;
        auto result = doc.load_file(file.c_str());
        if (result && doc.document_element()) {
            __loaded = true;
            if (file == showFile) {
                spdlog::info("SpecialOptions: loaded from show folder '{}'", file);
            } else {
                spdlog::info("SpecialOptions: loaded from exe folder '{}'", file);
            }
            for (pugi::xml_node n = doc.document_element().first_child(); n; n = n.next_sibling()) {
                std::string nodeName = n.name();
                std::transform(nodeName.begin(), nodeName.end(), nodeName.begin(), ::tolower);
                if (nodeName == "option") {
                    std::string name = Trim(n.attribute("name").as_string());
                    std::string value = n.attribute("value").as_string();
                    if (name != "") {
                        __cache[name] = value;
                    }
                }
            }
        } else {
            return defaultValue;
        }
    }

    if (option == "") return defaultValue;

    if (__cache.find(option) == __cache.end()) {
        return defaultValue;
    }

    return __cache.at(option);
}

