#include "PluginManager.h"
#include "xSchedulePlugin.h"
#include "xScheduleMain.h"
#include <wx/stdpaths.h>
#include <wx/dir.h>
#include <wx/dynlib.h>
#include <wx/filename.h>

uint32_t __nextId = 1;

PluginManager::PluginManager()
{
}

void PluginManager::ScanFolder(const std::string& folder)
{
#ifdef __WXMSW__
    std::string filespec = "*.dll";
#else
    std::string filespec = "*.so";
#endif
    wxArrayString files;
    wxDir::GetAllFiles(folder, &files, filespec);

    for (auto f : files)
    {
        // load the dll
        wxDynamicLibrary* dl = new wxDynamicLibrary();
        dl->Load(f);

        if (dl->IsLoaded())
        {
            // look for our function
            p_xSchedule_Load fn = (p_xSchedule_Load)dl->GetSymbol("xSchedule_Load");

            // if found add to list
            if (fn != nullptr)
            {
                PluginState* pis = new PluginState();
                pis->_dl = dl;
                pis->_filename = f;
                pis->_started = false;
                pis->_id = __nextId++;

                wxFileName fn(f);
                _plugins[fn.GetName()] = pis;
                continue;
            }
        }
        delete dl;
    }
}

bool PluginManager::DoLoad(const std::string& plugin, char* showDir)
{
    if (_plugins.find(plugin) == _plugins.end()) return false;

    p_xSchedule_Load fn = (p_xSchedule_Load)(_plugins.at(plugin)->_dl->GetSymbol("xSchedule_Load"));
    if (fn != nullptr)
    {
        return fn(showDir);
    }
    return false;
}

void PluginManager::DoUnload(const std::string& plugin)
{
    if (_plugins.find(plugin) == _plugins.end()) return;

    p_xSchedule_Unload fn = (p_xSchedule_Unload)(_plugins.at(plugin)->_dl->GetSymbol("xSchedule_Unload"));
    if (fn != nullptr)
    {
        fn();
    }
}

bool PluginManager::DoStart(const std::string& plugin, char* showDir)
{
    if (_plugins.find(plugin) == _plugins.end()) return false;
    if (_plugins.at(plugin)->_started) return true;

    p_xSchedule_Start fn = (p_xSchedule_Start)(_plugins.at(plugin)->_dl->GetSymbol("xSchedule_Start"));
    if (fn != nullptr)
    {
        _plugins.at(plugin)->_started = fn(showDir);
    }
    return _plugins.at(plugin)->_started;
}

bool PluginManager::HandleWeb(const std::string& plugin, const char* action, const char* parms)
{
    if (_plugins.find(plugin) == _plugins.end()) return false;

    p_xSchedule_HandleWeb fn = (p_xSchedule_HandleWeb)(_plugins.at(plugin)->_dl->GetSymbol("xSchedule_HandleWeb"));
    if (fn != nullptr)
    {
        return fn(action, parms);
    }

    return false;
}

void PluginManager::DoStop(const std::string& plugin)
{
    if (_plugins.find(plugin) == _plugins.end()) return;
    if (!_plugins.at(plugin)->_started) return;

    p_xSchedule_Stop fn = (p_xSchedule_Stop)(_plugins.at(plugin)->_dl->GetSymbol("xSchedule_Stop"));
    if (fn != nullptr)
    {
        fn();
        _plugins.at(plugin)->_started = false;
    }
}

void PluginManager::DoWipeSettings(const std::string& plugin)
{
    if (_plugins.find(plugin) == _plugins.end()) return;

    p_xSchedule_WipeSettings fn = (p_xSchedule_WipeSettings)(_plugins.at(plugin)->_dl->GetSymbol("xSchedule_WipeSettings"));
    if (fn != nullptr)
    {
        fn();
    }
}

std::string PluginManager::GetVirtualWebFolder(const std::string& plugin)
{
    if (_plugins.find(plugin) == _plugins.end()) return "";

    char buffer[200];
    memset(buffer, 0x00, sizeof(buffer));

    p_xSchedule_GetVirtualWebFolder fn = (p_xSchedule_GetVirtualWebFolder)(_plugins.at(plugin)->_dl->GetSymbol("xSchedule_GetVirtualWebFolder"));
    if (fn != nullptr)
    {
        fn(buffer, sizeof(buffer));
    }
    return std::string(buffer);
}

std::string PluginManager::GetMenuLabel(const std::string& plugin)
{
    if (_plugins.find(plugin) == _plugins.end()) return "";

    char buffer[200];
    memset(buffer, 0x00, sizeof(buffer));

    p_xSchedule_GetMenuLabel fn = (p_xSchedule_GetMenuLabel)(_plugins.at(plugin)->_dl->GetSymbol("xSchedule_GetMenuLabel"));
    if (fn != nullptr)
    {
        fn(buffer, sizeof(buffer));
    }
    return std::string(buffer);
}

void PluginManager::Initialise(const std::string& showDir)
{
    // scan xSchedule folder first

    wxFileName fi(wxStandardPaths::Get().GetExecutablePath().ToStdString());

    ScanFolder(fi.GetPath());

    // then scan show folder
    ScanFolder(showDir);

    for (auto it : _plugins)
    {
        DoLoad(it.first, (char*)showDir.c_str());
    }
}

bool PluginManager::StartPlugin(const std::string& plugin, const std::string& showDir)
{
    return DoStart(plugin, (char *)showDir.c_str());
}

void PluginManager::StopPlugin(const std::string& plugin)
{
    DoStop(plugin);
}

uint32_t PluginManager::GetId(const std::string& plugin) const
{
    if (_plugins.find(plugin) == _plugins.end()) return -1;

    return _plugins.at(plugin)->_id;
}

std::string PluginManager::GetPluginFromId(uint32_t id) const
{
    for (auto it : _plugins)
    {
        if (it.second->_id == id)
        {
            return it.first;
        }
    }
    return "";
}

void PluginManager::StopPlugins()
{
    for (auto it : _plugins)
    {
        DoStop(it.first);
    }
}

void PluginManager::WipeSettings()
{
    for (auto it : _plugins)
    {
        DoWipeSettings(it.first);
    }
}

void PluginManager::Uninitialise()
{
    for (auto it : _plugins)
    {
        DoUnload(it.first);
        delete it.second->_dl;
        delete it.second;
    }
    _plugins.clear();
}

std::vector<std::string> PluginManager::GetPlugins() const
{
    std::vector<std::string> res;

    for (auto it : _plugins)
    {
        res.push_back(it.first);
    }
    return res;
}

PluginManager::~PluginManager()
{
    Uninitialise();
}