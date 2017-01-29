#ifndef SCHEDULEOPTIONS_H
#define SCHEDULEOPTIONS_H
#include <list>
#include <string>
#include <map>
#include <vector>
#include "MatrixMapper.h"

class wxXmlNode;
class UserButton;

class ScheduleOptions
{
    bool _advancedMode;
	bool _sync;
    int _changeCount;
    int _lastSavedChangeCount;
    bool _sendOffWhenNotRunning;
    bool _webAPIOnly;
    int _port;
    std::string _wwwRoot;
    std::string _password;
    int _passwordTimeout;
    std::map<std::string, std::string> _projectorIPs;
    std::map<std::string, std::string> _projectorPasswords;
    std::vector<UserButton*> _buttons;
    std::list<MatrixMapper*> _matrices;

    public:

        bool IsDirty() const;
        void ClearDirty();
        ScheduleOptions(wxXmlNode* node);
        ScheduleOptions();
        virtual ~ScheduleOptions();
        wxXmlNode* Save();
        UserButton* GetButton(const std::string& label) const;
        bool IsSync() const { return _sync; }
        bool IsAdvancedMode() const { return _advancedMode; }
        std::list<MatrixMapper*>* GetMatrices() { return &_matrices; }
        void SetAdvancedMode(bool advancedMode) { if (_advancedMode != advancedMode) { _advancedMode = advancedMode; _changeCount++; } }
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
        std::string GetPassword() const { return _password; }
        int GetPasswordTimeout() const { return _passwordTimeout; }
        void SetAPIOnly(bool apiOnly) { if (_webAPIOnly != apiOnly) { _webAPIOnly = apiOnly; _changeCount++; } }
        void SetPasswordTimeout(int passwordTimeout) { if (_passwordTimeout != passwordTimeout) { _passwordTimeout = passwordTimeout; _changeCount++; } }
        void SetPassword(const std::string& password) { if (_password != password) { _password = password; _changeCount++; } }
};

#endif
