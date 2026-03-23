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

#include "ExternalHooks.h"
#include "UtilFunctions.h"

#include <pugixml.hpp>
#include <string>
#include <map>
#include <log.h>


std::string SpecialOptions::StashShowDir(const std::string& showDir)
{
    static std::string __showDir;

    if (showDir != "") __showDir = showDir;
    return __showDir;
}

std::string SpecialOptions::GetOption(const std::string& option, const std::string& defaultValue )
{
    static bool __loaded = false;
    static std::map<std::string, std::string> __cache;

    std::string file = StashShowDir() + GetPathSeparator() + "special.options";

    if (option == "")
    {
        __loaded = false;
        __cache.clear();
    }

    if (!FileExists(file))
    {
        if (__loaded)
        {
            __loaded = false;
        }
        else
        {
            spdlog::debug("Special options file not found at " + file);
            __loaded = true;
        }
        __cache.clear();
        return defaultValue;
    }

    if (!__loaded)
    {
        spdlog::debug("Loading special options from " + file);
        pugi::xml_document doc;
        auto result = doc.load_file(file.c_str());
        if (result && doc.document_element())
        {
            __loaded = true;
            for (pugi::xml_node n = doc.document_element().first_child(); n; n = n.next_sibling())
            {
                std::string nodeName = n.name();
                std::transform(nodeName.begin(), nodeName.end(), nodeName.begin(), ::tolower);
                if (nodeName == "option")
                {
                    std::string name = Trim(n.attribute("name").as_string());
                    std::string value = n.attribute("value").as_string();
                    if (name != "")
                    {
                        __cache[name] = value;
                        spdlog::debug("   Option '" + name + "' = '" + value + "'");
                    }
                }
            }
        }
        else
        {
            spdlog::error("Unable to load " + file + " invalid xml.");
            return defaultValue;
        }
    }

    if (option == "") return defaultValue;

    if (__cache.find(option) == __cache.end())
    {
        return defaultValue;
    }

    return __cache.at(option);
}

