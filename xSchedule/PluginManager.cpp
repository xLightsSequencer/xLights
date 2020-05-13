#include "PluginManager.h"
#include "xSchedulePlugin.h"
#include "xScheduleMain.h"
#include <wx/stdpaths.h>
#include <wx/dir.h>
#include <wx/dynlib.h>
#include <wx/filename.h>
#include "xScheduleApp.h"
#include "ScheduleManager.h"
#include <log4cpp/Category.hh>

uint32_t __nextId = 1;

PluginManager::PluginState::PluginState(wxDynamicLibrary* dl) : _dl(dl), _started(false), _id(__nextId++)
{
    if (_dl) {
        _loadFn = (p_xSchedule_Load)_dl->GetSymbol("xSchedule_Load");
        _unloadFn = (p_xSchedule_Unload)_dl->GetSymbol("xSchedule_Unload");
        _startFn = (p_xSchedule_Start)_dl->GetSymbol("xSchedule_Start");
        _stopFn = (p_xSchedule_Stop)_dl->GetSymbol("xSchedule_Stop");
        _handleWebFn = (p_xSchedule_HandleWeb)_dl->GetSymbol("xSchedule_HandleWeb");
        _wipeFn = (p_xSchedule_WipeSettings)_dl->GetSymbol("xSchedule_WipeSettings");
        _manipulateBufferFn = (p_xSchedule_ManipulateBuffer)_dl->GetSymbol("xSchedule_ManipulateBuffer");
        _notifyStatusFn = (p_xSchedule_NotifyStatus)_dl->GetSymbol("xSchedule_NotifyStatus");
        _getVirtualWebFolderFn = (p_xSchedule_GetVirtualWebFolder)_dl->GetSymbol("xSchedule_GetVirtualWebFolder");
        _getMenuLabelFn = (p_xSchedule_GetMenuLabel)_dl->GetSymbol("xSchedule_GetMenuLabel");
        _fireEventFn = (p_xSchedule_FireEvent)_dl->GetSymbol("xSchedule_FireEvent");
    } else {
        _loadFn = nullptr;
        _unloadFn = nullptr;
        _startFn = nullptr;
        _stopFn = nullptr;
        _handleWebFn = nullptr;
        _wipeFn = nullptr;
        _manipulateBufferFn = nullptr;
        _notifyStatusFn = nullptr;
        _getVirtualWebFolderFn = nullptr;
        _getMenuLabelFn = nullptr;
        _fireEventFn = nullptr;
    }
}
PluginManager::PluginState::~PluginState()
{
    if (_dl) {
        ///_dl->Detach(); ????
        delete _dl;
    }
}

PluginManager::PluginManager()
{
}

void PluginManager::ScanFolder(const std::string& folder)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("Scanning for plugins");

#ifdef __WXMSW__
    std::string filespec = "*.dll";
#elif defined __WXOSX__
    std::string filespec = "*.dylib";
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

        if (dl->IsLoaded()) {
            // look for our function
            p_xSchedule_Load fn = (p_xSchedule_Load)dl->GetSymbol("xSchedule_Load");

            // if found add to list
            if (fn != nullptr) {
                PluginState* pis = new PluginState(dl);
                pis->_filename = f;

                wxFileName fn(f);
                _plugins[fn.GetName()] = pis;

                logger_base.debug("Plugin found %s : %s", (const char*)fn.GetName().c_str(), (const char*)f.c_str());
            } else {
                delete dl;
            }
        } else {
            delete dl;
        }
    }
}

bool PluginManager::DoLoad(const std::string& plugin, char* showDir)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (_plugins.find(plugin) == _plugins.end()) return false;

    p_xSchedule_Load fn = _plugins.at(plugin)->_loadFn;
    if (fn != nullptr) {
        logger_base.debug("Loading plugin %s", (const char*)plugin.c_str());
        bool res = fn(showDir);
        logger_base.debug("Loaded plugin %s -> %d", (const char*)plugin.c_str(), res);
        return res;
    }
    return false;
}

void PluginManager::DoUnload(const std::string& plugin)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (_plugins.find(plugin) == _plugins.end()) return;

    p_xSchedule_Unload fn = _plugins.at(plugin)->_unloadFn;
    if (fn != nullptr) {
        logger_base.debug("Unloading plugin %s", (const char*)plugin.c_str());
        fn();
    }
}

bool PluginManager::DoStart(const std::string& plugin, char* showDir, char* xScheduleURL)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (_plugins.find(plugin) == _plugins.end()) return false;
    if (_plugins.at(plugin)->_started) return true;

    p_xSchedule_Start fn = _plugins.at(plugin)->_startFn;
    if (fn != nullptr) {
        logger_base.debug("Starting plugin %s", (const char*)plugin.c_str());
        _plugins.at(plugin)->_started = fn(showDir, xScheduleURL, Action);
        logger_base.debug("Started plugin %s -> %d", (const char*)plugin.c_str(), _plugins.at(plugin)->_started);
    }
    return _plugins.at(plugin)->_started;
}

bool PluginManager::HandleWeb(const std::string& plugin, const std::string& action, const std::wstring& parms, const std::wstring& data, const std::wstring& reference, std::wstring& response)
{
    if (_plugins.find(plugin) == _plugins.end()) return false;
    if (!_plugins.at(plugin)->_started) return false;

    p_xSchedule_HandleWeb fn = _plugins.at(plugin)->_handleWebFn;
    if (fn != nullptr) {
        wchar_t resp[4096];
        bool res = fn((const char*)action.c_str(), (const wchar_t*)parms.c_str(), (const wchar_t*)data.c_str(), (const wchar_t*)reference.c_str(), resp, sizeof(resp));
        response = std::wstring(resp);
        return res;
    }

    return false;
}

void PluginManager::ManipulateBuffer(uint8_t* buffer, size_t bufferSize)
{
    for (auto it : _plugins) {
        DoManipulateBuffer(it.first, buffer, bufferSize);
    }
}

void PluginManager::NotifyStatus(const std::string& statusJSON)
{
    const char* s = (const char*)statusJSON.c_str();
    for (auto it : _plugins) {
        DoNotifyStatus(it.first, s);
    }
}

void PluginManager::FirePluginEvent(const std::string& plugin, const std::string& eventType, const std::string& eventParam)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (_plugins.find(plugin) == _plugins.end()) return;
    if (!_plugins.at(plugin)->_started) return;

    p_xSchedule_FireEvent fn = _plugins.at(plugin)->_fireEventFn;
    if (fn != nullptr) {
        fn((const char*)eventType.c_str(), (const char*)eventParam.c_str());
        logger_base.debug("Fired event to plugin %s %s:%s", (const char*)plugin.c_str(), (const char*)eventType.c_str(), (const char*)eventParam.c_str());
    }
}

void PluginManager::FireEvent(const std::string& eventType, const std::string& eventParam)
{
    for (auto it : _plugins) {
        FirePluginEvent(it.first, eventType, eventParam);
    }
}

void PluginManager::DoStop(const std::string& plugin)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_plugins.find(plugin) == _plugins.end()) return;
    if (!_plugins.at(plugin)->_started) return;

    p_xSchedule_Stop fn = _plugins.at(plugin)->_stopFn;
    if (fn != nullptr) {
        fn();
        logger_base.debug("Stopped plugin %s", (const char*)plugin.c_str());
        _plugins.at(plugin)->_started = false;
    }
}

void PluginManager::DoWipeSettings(const std::string& plugin)
{
    if (_plugins.find(plugin) == _plugins.end()) return;

    p_xSchedule_WipeSettings fn = _plugins.at(plugin)->_wipeFn;
    if (fn != nullptr) {
        fn();
    }
}

bool PluginManager::DoManipulateBuffer(const std::string& plugin, uint8_t* buffer, size_t bufferSize)
{
    if (_plugins.find(plugin) == _plugins.end()) return false;
    if (!_plugins.at(plugin)->_started) return false;

    p_xSchedule_ManipulateBuffer fn = _plugins.at(plugin)->_manipulateBufferFn;
    if (fn != nullptr) {
        fn(buffer, bufferSize);
        return true;
    }
    return false;
}

void PluginManager::DoNotifyStatus(const std::string& plugin, const char* statusJSON)
{
    if (_plugins.find(plugin) == _plugins.end()) return;
    if (!_plugins.at(plugin)->_started) return;

    p_xSchedule_NotifyStatus fn = _plugins.at(plugin)->_notifyStatusFn;
    if (fn != nullptr) {
        fn(statusJSON);
    }
}

std::string PluginManager::GetVirtualWebFolder(const std::string& plugin)
{
    if (_plugins.find(plugin) == _plugins.end()) return "";
    if (!_plugins.at(plugin)->_started) return "";

    char buffer[200];
    memset(buffer, 0x00, sizeof(buffer));

    p_xSchedule_GetVirtualWebFolder fn = _plugins.at(plugin)->_getVirtualWebFolderFn;
    if (fn != nullptr) {
        fn(buffer, sizeof(buffer));
    }
    return std::string(buffer);
}

std::string PluginManager::GetMenuLabel(const std::string& plugin)
{
    if (_plugins.find(plugin) == _plugins.end()) return "";

    char buffer[200];
    memset(buffer, 0x00, sizeof(buffer));

    p_xSchedule_GetMenuLabel fn = _plugins.at(plugin)->_getMenuLabelFn;
    if (fn != nullptr) {
        fn(buffer, sizeof(buffer));
    }
    return std::string(buffer);
}


#ifdef __WXOSX__
PluginManager::PluginState *CreateSMSPluginState();
#endif

void PluginManager::RegisterStaticPlugins() {
#ifdef __WXOSX__
    PluginManager::PluginState *p = CreateSMSPluginState();
    if (p) _plugins[p->_filename] = p;
#endif
}


void PluginManager::Initialise(const std::string& showDir)
{
    // scan xSchedule folder first

    wxFileName fi(wxStandardPaths::Get().GetExecutablePath().ToStdString());

    ScanFolder(fi.GetPath());

    // then scan show folder
    ScanFolder(showDir);

    // then add anything that is statically compiled in
    RegisterStaticPlugins();
    
    for (auto it : _plugins) {
        DoLoad(it.first, (char*)showDir.c_str());
    }
}

bool PluginManager::StartPlugin(const std::string& plugin, const std::string& showDir, const std::string& xScheduleURL)
{
    return DoStart(plugin, (char *)showDir.c_str(), (char*)xScheduleURL.c_str());
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
    for (auto it : _plugins) {
        DoWipeSettings(it.first);
    }
}

void PluginManager::Uninitialise()
{
    for (auto it : _plugins)
    {
        DoUnload(it.first);
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

bool Action(const char* command, const wchar_t* parameters, const char* data, char* buffer, size_t bufferSize)
{
    memset(buffer, 0x00, bufferSize);
    std::string c(command);
    std::wstring p(parameters);
    std::string d(data);
    wxString dd(data);
    size_t rate = 0;
    wxString msg;
    if (((xScheduleFrame*)wxTheApp->GetTopWindow())->GetScheduleManager()->IsQuery(c))
    {
        wxString ip;
        wxString reference;
        bool result = ((xScheduleFrame*)wxTheApp->GetTopWindow())->GetScheduleManager()->Query(c, p, dd, msg, ip, reference);
        if (result || msg == "")
        {
            msg = dd;
        }
        else
        {
            msg = "{\"result\":\"failed\",\"command\":\"" +
                c + "\",\"message\":\"" +
                msg + "\"}";
        }
        strncpy(buffer, (const char*)msg.c_str(), bufferSize - 1);
        return result;
    }
    else
    {
        bool result = ((xScheduleFrame*)wxTheApp->GetTopWindow())->GetScheduleManager()->Action(c, p, d, nullptr, nullptr, nullptr, rate, msg);
        if (result || msg == "")
        {
            msg = "{result\":\"ok\"}";
        }
        else
        {
            msg = "{\"result\":\"failed\",\"command\":\"" +
                c + "\",\"message\":\"" +
                msg + "\"}";
        }
        strncpy(buffer, (const char*)msg.c_str(), bufferSize - 1);
        return result;
    }
}
