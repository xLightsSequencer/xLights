#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

class wxDynamicLibrary;

#include <map>
#include <string>
#include <vector>

#include "xSchedulePlugin.h"

extern "C"
{
    // callback function from plugins that want xSchedule to do something
    // can be any valid command or query
    bool Action(const char* command, const wchar_t* parameters, const char* data, char* buffer, size_t bufferSize);
}

class PluginManager
{
public:
    class PluginState
    {
    public:
        PluginState(wxDynamicLibrary* dl, const std::string& filename);
        virtual ~PluginState();
        
        std::string _filename;
        wxDynamicLibrary* _dl;
        bool _started;
        uint32_t _id;
        
        p_xSchedule_Load _loadFn;
        p_xSchedule_Unload _unloadFn;
        p_xSchedule_Start _startFn;
        p_xSchedule_Stop _stopFn;
        p_xSchedule_HandleWeb _handleWebFn;
        p_xSchedule_WipeSettings _wipeFn;
        p_xSchedule_ManipulateBuffer _manipulateBufferFn;
        p_xSchedule_NotifyStatus _notifyStatusFn;
        p_xSchedule_GetVirtualWebFolder _getVirtualWebFolderFn;
        p_xSchedule_GetMenuLabel _getMenuLabelFn;
        p_xSchedule_FireEvent _fireEventFn;
    };
private:
    std::map<std::string, PluginState*> _plugins;

    void ScanFolder(const std::string& folder);
    void RegisterStaticPlugins();

    bool DoLoad(const std::string& plugin, char* showDir);
    bool DoStart(const std::string& plugin, char* showDir, char* xScheduleURL);
    void DoStop(const std::string& plugin);
    void DoUnload(const std::string& plugin);
    void DoWipeSettings(const std::string& plugin);
    bool DoManipulateBuffer(const std::string& plugin, uint8_t* buffer, size_t bufferSize);
    void DoNotifyStatus(const std::string& plugin, const char* statusJSON);

    public:
	
		PluginManager();
        void Initialise(const std::string& showDir);
        bool StartPlugin(const std::string& plugin, const std::string& showDir, const std::string& xScheduleURL);
        void StopPlugin(const std::string& plugin);
        uint32_t GetId(const std::string& plugin) const;
        std::string GetPluginFromId(uint32_t id) const;
        void StopPlugins();
        void WipeSettings();
        void Uninitialise();
        std::vector<std::string> GetPlugins() const;
        virtual ~PluginManager();

        std::string GetVirtualWebFolder(const std::string& plugin);
        std::string GetMenuLabel(const std::string& plugin);
        bool HandleWeb(const std::string& plugin, const std::string& command, const std::wstring& parameters, const std::wstring& data, const std::wstring& reference, std::wstring& response);
        void ManipulateBuffer(uint8_t* buffer, size_t bufferSize);
        void NotifyStatus(const std::string& statusJSON);
        void FirePluginEvent(const std::string& plugin, const std::string& eventType, const std::string& eventParam);
        void FireEvent(const std::string& eventType, const std::string& eventParam);
};
#endif
