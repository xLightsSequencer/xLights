#ifndef SCHEDULEOPTIONS_H
#define SCHEDULEOPTIONS_H
#include <list>
#include <string>
#include <map>

class wxXmlNode;

class ScheduleOptions
{
	bool _sync;
    int _changeCount;
    int _lastSavedChangeCount;
    bool _sendOffWhenNotRunning;
    int _port;
    std::string _wwwRoot;
    std::map<std::string, std::string> _projectorIPs;
    std::map<std::string, std::string> _projectorPasswords;
    std::map<std::string, std::string> _buttonCommands;
    std::map<std::string, std::string> _buttonParameters;

    public:

        bool IsDirty() const { return _lastSavedChangeCount != _changeCount; };
        void ClearDirty() { _lastSavedChangeCount = _changeCount; };
        ScheduleOptions(wxXmlNode* node);
        ScheduleOptions();
        virtual ~ScheduleOptions();
        wxXmlNode* Save();
		bool IsSync() const { return _sync; }
        void SetSync(bool sync) { if (_sync != sync) { _sync = sync; _changeCount++; } }
        void SetSendOffWhenNotRunning(bool send) { if (_sendOffWhenNotRunning != send) { _sendOffWhenNotRunning = send; _changeCount++; } }
        bool IsSendOffWhenNotRunning() const { return _sendOffWhenNotRunning; }
        std::list<std::string> GetProjectors() const;
        std::list<std::string> GetButtons() const;
        std::string GetProjectorIpAddress(const std::string& projector);
        std::string GetProjectorPassword(const std::string& projector);
        void ClearProjectors();
        void ClearButtons();
        void SetProjectorIPAddress(const std::string& projector, const std::string& ip);
        void SetProjectorPassword(const std::string& projector, const std::string& ip);
        void SetButtonCommand(const std::string& button, const std::string& command);
        void SetButtonParameter(const std::string& button, const std::string& parameter);
        std::string GetButtonCommand(const std::string& button);
        std::string GetButtonParameter(const std::string& button);
        std::string GetButtonsJSON() const;
        int GetWebServerPort() const { return _port; }
        void SetWebServerPort(int port) { if (_port != port) { _port = port; _changeCount++; } }
        std::string GetWWWRoot() const { return _wwwRoot; }
        void SetWWWRoot(const std::string& wwwRoot) { if (_wwwRoot != wwwRoot) { _wwwRoot = wwwRoot; _changeCount++; } }
};

#endif
