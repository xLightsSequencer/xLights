#ifndef SCHEDULEOPTIONS_H
#define SCHEDULEOPTIONS_H
#include <list>
#include <string>
#include <map>
#include <vector>
#include "MatrixMapper.h"
#include "VirtualMatrix.h"

class CommandManager;
class wxXmlNode;
class UserButton;
class Projector;

class ScheduleOptions
{
    bool _advancedMode;
	bool _sync;
    int _changeCount;
    int _lastSavedChangeCount;
    bool _sendOffWhenNotRunning;
    bool _sendBackgroundWhenNotRunning;
    bool _webAPIOnly;
    int _port;
    std::string _wwwRoot;
    std::string _password;
    int _passwordTimeout;
    std::list<Projector*> _projectors;
    std::vector<UserButton*> _buttons;
    std::list<MatrixMapper*> _matrices;
    std::list<VirtualMatrix*> _virtualMatrices;
    std::list<std::string> _fppRemotes;

    public:

        bool IsDirty() const;
        void ClearDirty();
        ScheduleOptions(wxXmlNode* node);
        ScheduleOptions();
        virtual ~ScheduleOptions();
        wxXmlNode* Save();
        UserButton* GetButton(const std::string& label) const;
        UserButton* GetButton(wxUint32 id) const;
        bool IsSync() const { return _sync; }
        bool IsAdvancedMode() const { return _advancedMode; }
        std::list<MatrixMapper*>* GetMatrices() { return &_matrices; }
        std::list<VirtualMatrix*>* GetVirtualMatrices() { return &_virtualMatrices; }
        void SetAdvancedMode(bool advancedMode) { if (_advancedMode != advancedMode) { _advancedMode = advancedMode; _changeCount++; } }
        void SetSync(bool sync) { if (_sync != sync) { _sync = sync; _changeCount++; } }
        void SetSendOffWhenNotRunning(bool send) { if (_sendOffWhenNotRunning != send) { _sendOffWhenNotRunning = send; _changeCount++; } }
        bool IsSendOffWhenNotRunning() const { return _sendOffWhenNotRunning; }
        void SetSendBackgroundWhenNotRunning(bool send) { if (_sendBackgroundWhenNotRunning != send) { _sendBackgroundWhenNotRunning = send; _changeCount++; } }
        bool IsSendBackgroundWhenNotRunning() const { return _sendBackgroundWhenNotRunning; }
        std::list<Projector*> GetProjectors() const;
        std::vector<UserButton*> GetButtons() const;
        Projector* GetProjector(const std::string& projector);
        void ClearProjectors();
        void ClearButtons();
        std::string GetButtonsJSON(const CommandManager &cmdMgr, const std::string& reference) const;
        int GetWebServerPort() const { return _port; }
        std::list<std::string> GetFPPRemotes() const { return _fppRemotes; }
        void SetWebServerPort(int port) { if (_port != port) { _port = port; _changeCount++; } }
        void SetFPPRemotes(std::list<std::string> remotes) { _fppRemotes = remotes; _changeCount++; } 
        std::string GetWWWRoot() const { return _wwwRoot; }
        std::string GetDefaultRoot() const;
        void SetWWWRoot(const std::string& wwwRoot) { if (_wwwRoot != wwwRoot) { _wwwRoot = wwwRoot; _changeCount++; } }
        void AddButton(const std::string& label, const std::string& command, const std::string& parms, char hotkey, const std::string& color);
        void AddProjector(const std::string& name, const std::string& ip, const std::string& password);
        bool GetAPIOnly() const { return _webAPIOnly; }
        std::string GetPassword() const { return _password; }
        int GetPasswordTimeout() const { return _passwordTimeout; }
        void SetAPIOnly(bool apiOnly) { if (_webAPIOnly != apiOnly) { _webAPIOnly = apiOnly; _changeCount++; } }
        void SetPasswordTimeout(int passwordTimeout) { if (_passwordTimeout != passwordTimeout) { _passwordTimeout = passwordTimeout; _changeCount++; } }
        void SetPassword(const std::string& password) { if (_password != password) { _password = password; _changeCount++; } }
};

#endif
