#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/file.h>
#include <wx/filename.h>
#include <wx/xml/xml.h>
#include <string>
#include <map>
#include <log4cpp/Category.hh>

// Special options are read from an xml file in the show folder
// its use is for options we want the user to be able to set when required but not common enough to put in the UI

class SpecialOptions
{
public:
    static std::string StashShowDir(const std::string& showDir = "")
    {
        static std::string __showDir;

        if (showDir != "") __showDir = showDir;
        return __showDir;
    }
    static std::string GetOption(const std::string& option, const std::string& defaultValue = "")
    {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        static bool __loaded = false;
        static std::map<std::string, std::string> __cache;

        std::string file = StashShowDir() + wxFileName::GetPathSeparator() + "special.options";

        if (option == "")
        {
            __loaded = false;
            __cache.clear();
        }

        if (!wxFile::Exists(file))
        {
            if (__loaded)
            {
                __loaded = false;
            }
            else
            {
                logger_base.debug("Special options file not found at " + file);
                __loaded = true;
            }
            __cache.clear();
            return defaultValue;
        }

        if (!__loaded)
        {
            logger_base.debug("Loading special options from " + file);
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
                            logger_base.debug("   Option '" + name + "' = '" + value + "'");
                        }
                    }
                }
            }
            else
            {
                logger_base.error("Unable to load " + file + " invalid xml.");
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
};