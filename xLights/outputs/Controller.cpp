#include "Controller.h"

#include <wx/xml/xml.h>
#include <wx/stdpaths.h>
#include <wx/dir.h>
#include <wx/filename.h>

#include <log4cpp/Category.hh>

std::list<Controller> Controller::__controllers;
 
std::list<std::string> Controller::GetControllerTypes()
{
    if (__controllers.size() == 0) LoadControllers();

    std::list<std::string> res;

    for (auto it = __controllers.begin(); it != __controllers.end(); ++it)
    {
        res.push_back(it->GetId());
    }

    return res;
}
 
Controller* Controller::GetController(const std::string& id)
{
    if (__controllers.size() == 0) LoadControllers();

    for (auto it = __controllers.begin(); it != __controllers.end(); ++it)
    {
        if (it->GetId() == id)
        {
            return &(*it);
        }
    }

    return nullptr;
}
 
void Controller::LoadControllers()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxStandardPaths stdp = wxStandardPaths::Get();

#ifndef __WXMSW__
    wxString d = wxStandardPaths::Get().GetResourcesDir() + "/controllers";
#else
    wxString d = wxFileName(stdp.GetExecutablePath()).GetPath() + "/controllers";
#endif
    if (wxDir::Exists(d))
    {
        wxDir dir(d);

        wxString filename;
        bool cont = dir.GetFirst(&filename, "*.xcontroller", wxDIR_FILES);

        while (cont)
        {
            wxFileName fn(dir.GetNameWithSep() + filename);

            wxXmlDocument doc;
            doc.Load(fn.GetFullPath());
            if (doc.IsOk())
            {
                __controllers.push_back(doc.GetRoot()->GetChildren());
            }
            else
            {
                logger_base.warn("Error loading controller specification: %s.", (const char *)fn.GetFullPath().c_str());
            }

            cont = dir.GetNext(&filename);
        }
    }
    else
    {
        logger_base.info("Directory for controller files not found: %s.", (const char *)d.c_str());
    }
}
 
Controller::Controller(wxXmlNode* node)
{
    _maxChannelsPerUniverse = 512;
    _maxUniverses = -1;

    for (auto e = node->GetChildren(); e != nullptr; e = e->GetNext())
    {
        if (e->GetName() == "Brand")
        {
            _brand = e->GetNodeContent().ToStdString();
        }
        else if (e->GetName() == "Model")
        {
            _model = e->GetNodeContent().ToStdString();
        }
        else if (e->GetName() == "MaxInputUniverses")
        {
            _maxUniverses = wxAtoi(e->GetNodeContent());
        }
        else if (e->GetName() == "MaxChannelsPerUniverse")
        {
            _maxChannelsPerUniverse = wxAtoi(e->GetNodeContent());
        }
        else if (e->GetName() == "SupportedProtocols")
        {
            for (auto e2 = e->GetChildren(); e2 != nullptr; e2 = e2->GetNext())
            {
                if (e2->GetName() == "Protocol")
                {
                    _supportedProtocols.push_back(e->GetNodeContent().Trim(false).Trim(true).ToStdString());
                }
            }
        }
        else if (e->GetName() == "Outputs")
        {
            for (auto e2 = e->GetChildren(); e2 != nullptr; e2 = e2->GetNext())
            {
                std::string type = e2->GetName().ToStdString();
                for (auto e3 = e2->GetChildren(); e3 != nullptr; e3 = e3->GetNext())
                {
                    if (e3->GetName() == "Count")
                    {
                        _outputs[type] = wxAtoi(e->GetNodeContent());
                    }
                    else if (e3->GetName() == "MaxChannels")
                    {
                        _channelsPerOutput[type] = wxAtoi(e->GetNodeContent());
                    }
                }
            }
        }
    }
}
 
int Controller::GetOutputCount(const std::string& type) const
{
    if (_outputs.find(type) == _outputs.end()) return -1;

    return _outputs.at(type);
}
 
int Controller::GetChannelsPerOutput(const std::string& type) const
{
    if (_channelsPerOutput.find(type) == _channelsPerOutput.end()) return -1;

    return _channelsPerOutput.at(type);
}

bool Controller::SupportsProtocol(const std::string& protocol)
{
    return std::find(_supportedProtocols.begin(), _supportedProtocols.end(), protocol) != _supportedProtocols.end();
}
