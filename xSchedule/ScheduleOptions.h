#ifndef SCHEDULEOPTIONS_H
#define SCHEDULEOPTIONS_H
#include <list>
#include <string>
#include <map>
#include <vector>

class wxXmlNode;
class UserButton;

class ScheduleOptions
{
	bool _sync;
    int _changeCount;
    int _lastSavedChangeCount;
    bool _sendOffWhenNotRunning;
    bool _webAPIOnly;
    int _port;
    std::string _wwwRoot;
    std::map<std::string, std::string> _projectorIPs;
    std::map<std::string, std::string> _projectorPasswords;
    std::vector<UserButton*> _buttons;

    public:

        bool IsDirty() const;
        void ClearDirty();
        ScheduleOptions(wxXmlNode* node);
        ScheduleOptions();
        virtual ~ScheduleOptions();
        wxXmlNode* Save();
        UserButton* GetButton(const std::string& label) const;
		bool IsSync() const { return _sync; }
        void SetSync(bool sync) { if (_sync != sync) { _sync = sync; _changeCount++; } }
        void SetSendOffWhenNotRunning(bool send) { if (_sendOffWhenNotRunning != send) { _sendOffWhenNotRunning = send; _changeCount++; } }
        bool IsSendOffWhenNotRunning() const { return _sendOffWhenNotRunning; }
        std::list<std::string> GetProjectors() const;
        std::vector<UserButton*> GetButtons() const;
        std::string GetProjectorIpAddress(const std::string& projector);
        std::string GetProjectorPassword(const std::string& projector);
        void ClearProjectors();
        void ClearButtons();
        void SetProjectorIPAddress(const std::string& projector, const std::string& ip);
        void SetProjectorPassword(const std::string& projector, const std::string& ip);
        std::string GetButtonsJSON() const;
        int GetWebServerPort() const { return _port; }
        void SetWebServerPort(int port) { if (_port != port) { _port = port; _changeCount++; } }
        std::string GetWWWRoot() const { return _wwwRoot; }
        void SetWWWRoot(const std::string& wwwRoot) { if (_wwwRoot != wwwRoot) { _wwwRoot = wwwRoot; _changeCount++; } }
        void AddButton(const std::string& label, const std::string& command, const std::string& parms, char hotkey);
        bool GetAPIOnly() const { return _webAPIOnly; }
        void SetAPIOnly(bool apiOnly) { if (_webAPIOnly != apiOnly) { _webAPIOnly = apiOnly; _changeCount++; } }
};

#endif
