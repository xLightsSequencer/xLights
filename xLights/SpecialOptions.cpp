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

#include <wx/xml/xml.h>
#include <string>
#include <map>
#include "spdlog/spdlog.h"


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
        wxXmlDocument doc;
        doc.Load(file);
        if (doc.IsOk() && doc.GetRoot() != nullptr)
        {
            __loaded = true;
            for (wxXmlNode* n = doc.GetRoot()->GetChildren(); n != nullptr; n = n->GetNext())
            {
                if (n->GetName().Lower() == "option")
                {
                    std::string name = n->GetAttribute("name").Trim(false).Trim(true);
                    std::string value = n->GetAttribute("value");
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

