#include "ControllerCaps.h"

#include <wx/xml/xml.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/dir.h>
#include <log4cpp/Category.hh>

#include "../UtilFunctions.h"
#include "../outputs/Controller.h"

std::map<std::string, std::map<std::string, std::map<std::string, ControllerCaps*>>> ControllerCaps::__controllers;

void ControllerCaps::UnloadControllers()
{
    // delete all the wxXmlNodes
    for (const auto& it : __controllers)
    {
        for (const auto& it2 : it.second)
        {
            for (const auto& it3 : it2.second)
            {
                delete it3.second;
            }
        }
    }
    __controllers.clear();
}

void ControllerCaps::LoadControllers()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (__controllers.size() != 0) return;

    wxString d;
    wxStandardPaths stdp = wxStandardPaths::Get();

#ifndef __WXMSW__
    d = wxStandardPaths::Get().GetResourcesDir() + "/controllers";
#else
    d = wxFileName(stdp.GetExecutablePath()).GetPath() + "/controllers";
#endif

#ifdef _DEBUG
    // in debug look in the master folder
    if (!wxDir::Exists(d))
    {
#ifdef __WXMSW__
        d = wxFileName(stdp.GetExecutablePath()).GetPath() + "/../../../controllers";
#endif
    }
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
                for (wxXmlNode* n = doc.GetRoot(); n != nullptr; n = n->GetNext())
                {
                    if (n->GetName() == "Vendor")
                    {
                        auto vendor = n->GetAttribute("Name");

                        if (__controllers.find(vendor) == end(__controllers))
                        {
                            __controllers[vendor] = std::map<std::string, std::map<std::string, ControllerCaps*>>();
                        }

                        auto& v = __controllers[vendor];

                        for (wxXmlNode* nn = n->GetChildren(); nn != nullptr; nn = nn->GetNext())
                        {
                            if (nn->GetName() == "Controller")
                            {
                                auto controller = nn->GetAttribute("Name");
                                if (v.find(controller) == v.end())
                                {
                                    v[controller] = std::map<std::string, ControllerCaps*>();
                                }

                                auto& c = v[controller];
                                for (wxXmlNode* nnn = nn->GetChildren(); nnn != nullptr; nnn = nnn->GetNext())
                                {
                                    if (nnn->GetName() == "FirmwareVersion")
                                    {
                                        auto fwv = nnn->GetAttribute("Name");
                                        if (c.find(fwv) == c.end())
                                        {
                                            c[fwv] = nullptr;
                                        }
                                        c[fwv] = new ControllerCaps(nnn);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                logger_base.error("Problem loading " + fn.GetFullPath());
            }

            cont = dir.GetNext(&filename);
        }
    }
    else
    {
        logger_base.error("Controllers folder not found " + d);
    }
}

std::list<std::string> ControllerCaps::GetVendors()
{
    LoadControllers();
    std::list<std::string> vendors;
    vendors.push_back("");
    for (const auto& it : __controllers)
    {
        vendors.push_back(it.first);
    }
    return vendors;
}

std::list<std::string> ControllerCaps::GetModels(const std::string& vendor)
{
    LoadControllers();
    std::list<std::string> models;
    models.push_back("");

    auto v = __controllers.find(vendor);

    if (v != __controllers.end())
    {
        for (const auto& it : v->second)
        {
            models.push_back(it.first);
        }
    }

    return models;
}

std::list<std::string> ControllerCaps::GetFirmwareVersions(const std::string& vendor, const std::string& model)
{
    LoadControllers();
    std::list<std::string> versions;
    versions.push_back("");

    auto v = __controllers.find(vendor);
    if (v != __controllers.end())
    {
        auto m = v->second.find(model);
        if (m != v->second.end())
        {
            for (const auto& it : m->second)
            {
                if (it.first != "") versions.push_back(it.first);
            }
        }
    }

    return versions;
}

ControllerCaps* ControllerCaps::GetControllerConfig(const Controller* const controller)
{
    if (controller == nullptr) return nullptr;
    return GetControllerConfig(controller->GetVendor(), controller->GetModel(), controller->GetFirmwareVersion());
}

ControllerCaps* ControllerCaps::GetControllerConfig(const std::string& vendor, const std::string& model, const std::string& firmwareVersion)
{
    LoadControllers();
    std::list<std::string> versions;

    auto v = __controllers.find(vendor);
    if (v != __controllers.end())
    {
        auto m = v->second.find(model);
        if (m != v->second.end())
        {
            auto f = m->second.find(firmwareVersion);
            if (f != m->second.end()) return f->second;
        }
    }

    return nullptr;
}

bool ControllerCaps::SupportsUpload() const
{
    return DoesXmlNodeExist(_config, "SupportsUpload") || 
           DoesXmlNodeExist(_config, "SupportsInputOnlyUpload");
}

bool ControllerCaps::SupportsInputOnlyUpload() const
{
    return DoesXmlNodeExist(_config, "SupportsInputOnlyUpload");
}

int ControllerCaps::GetMaxInputE131Universes() const
{
    return wxAtoi(GetXmlNodeContent(_config, "MaxInputUniverses"));
}

int ControllerCaps::GetMaxPixelPort() const
{
    return wxAtoi(GetXmlNodeContent(_config, "MaxPixelPort"));
}

int ControllerCaps::GetMaxSerialPort() const
{
    return wxAtoi(GetXmlNodeContent(_config, "MaxSerialPort"));
}

bool ControllerCaps::SupportsLEDPanelMatrix() const
{
    return DoesXmlNodeExist(_config, "SupportsLEDPanelMatix");
}

int ControllerCaps::GetMaxSerialPortChannels() const
{
    return wxAtoi(GetXmlNodeContent(_config, "MaxSerialPortChannels"));
}

int ControllerCaps::GetMaxInputUniverseChannels() const
{
    return wxAtoi(GetXmlNodeContent(_config, "MaxInputUniverseChannels", "512"));
}

bool ControllerCaps::SupportsVirtualStrings() const
{
    return DoesXmlNodeExist(_config, "SupportsVirtualStrings");
}

bool ControllerCaps::SupportsSmartRemotes() const
{
    return DoesXmlNodeExist(_config, "SupportsSmartRemotes");
}

bool ControllerCaps::IsValidPixelProtocol(const std::string& protocol) const
{
    auto pp = GetPixelProtocols();
    auto p = Lower(protocol);

    for (const auto& it : pp)
    {
        if (Lower(it) == p) return true;
    }
    return false;
}

bool ControllerCaps::IsValidSerialProtocol(const std::string& protocol) const
{
    auto pp = GetSerialProtocols();
    auto p = Lower(protocol);

    for (const auto& it : pp)
    {
        if (Lower(it) == p) return true;
    }
    return false;
}

bool ControllerCaps::SupportsMultipleSimultaneousOutputProtocols() const
{
    return DoesXmlNodeExist(_config, "SupportsMultipleSimultaneousOutputProtocols");
}

bool ControllerCaps::SupportsMultipleSimultaneousInputProtocols() const
{
    return DoesXmlNodeExist(_config, "SupportsMultipleSimultaneousInputProtocols");
}

bool ControllerCaps::MergeConsecutiveVirtualStrings() const
{
    return DoesXmlNodeExist(_config, "MergeConsecutiveVirtualStrings");
}

bool ControllerCaps::AllInputUniversesMustBeSameSize() const
{
    return DoesXmlNodeExist(_config, "AllInputUniversesMustBeSameSize");
}

bool ControllerCaps::IsValidInputProtocol(const std::string& protocol) const
{
    auto pp = GetInputProtocols();
    auto p = Lower(protocol);

    for (const auto& it : pp)
    {
        if (Lower(it) == p) return true;
    }
    return false;
}

int ControllerCaps::GetMaxPixelPortChannels() const
{
    return wxAtoi(GetXmlNodeContent(_config, "MaxPixelPortChannels"));
}

std::list<std::string> ControllerCaps::GetInputProtocols() const
{
    return GetXmlNodeListContent(_config, "InputProtocols", "Protocol");
}

std::list<std::string> ControllerCaps::GetPixelProtocols() const
{
    return GetXmlNodeListContent(_config, "PixelProtocols", "Protocol");
}

std::list<std::string> ControllerCaps::GetSerialProtocols() const
{
    return GetXmlNodeListContent(_config, "SerialProtocols", "Protocol");
}

bool ControllerCaps::UniversesMustBeInNumericalOrder() const
{
    return DoesXmlNodeExist(_config, "UniversesMustBeInNumericalOrder");
}

bool ControllerCaps::UniversesMustBeSequential() const
{
    return DoesXmlNodeExist(_config, "UniversesMustBeSequential");
}

std::string ControllerCaps::GetCustomPropertyByPath(const std::string name, const std::string& def) const
{
    return GetXmlNodeContent(_config, name, def);
}

void ControllerCaps::Dump() const
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Controller Capabilities " + _vendor + ":" + _model + ":" + _firmware);

    if (SupportsUpload()) logger_base.debug("   Supports upload.");
    if (SupportsInputOnlyUpload()) logger_base.debug("   Supports input only upload.");
    if (SupportsLEDPanelMatrix()) logger_base.debug("   Supports LED panel matrices.");
    if (SupportsVirtualStrings()) logger_base.debug("   Supports sirtual strings.");
    if (SupportsSmartRemotes()) logger_base.debug("   Supports smart remotes.");
    if (SupportsMultipleSimultaneousOutputProtocols()) logger_base.debug("   Supports multiple simultaneous output protocols.");
    if (AllInputUniversesMustBeSameSize()) logger_base.debug("   All input universes must be the same size.");
    if (UniversesMustBeInNumericalOrder()) logger_base.debug("   All input universes must be in numerical order.");
    logger_base.debug("   Inputs: maximum of %d universes.", GetMaxInputE131Universes());
    logger_base.debug("   Input protocols supported:");
    for (const auto& it : GetInputProtocols())
    {
        logger_base.debug("      " + it);
    }
    logger_base.debug("   Pixel ports: %d ports with a maximum of %d channels per port.", GetMaxPixelPort(), GetMaxPixelPortChannels());
    logger_base.debug("   Pixel protocols supported:");
    for (const auto& it : GetPixelProtocols())
    {
        logger_base.debug("      " + it);
    }
    logger_base.debug("   Serial ports: %d ports with a maximum of %d channels per port.", GetMaxSerialPort(), GetMaxSerialPortChannels());
    logger_base.debug("   Serial protocols supported:");
    for (const auto& it : GetSerialProtocols())
    {
        logger_base.debug("      " + it);
    }
}
