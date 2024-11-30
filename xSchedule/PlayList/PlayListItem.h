#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/notebook.h>
#include <wx/wx.h>
#include <list>
#include <string>

class wxXmlNode;
class AudioManager;
class ScheduleOptions;

class PlayListItem {
protected:
#pragma region Member Variables
    wxUint32 _id;
    std::string _type;
    int _lastSavedChangeCount;
    int _changeCount;
    std::string _name;
    size_t _frames;
    int _msPerFrame;
    size_t _delay;
    size_t _priority;
    int _volume;
    int _currentFrame;
    long _stepLengthMS;
    bool _restOfStep; // while not used in every item it could be common
#pragma endregion Member Variables

    void Save(wxXmlNode* node);
    void Copy(PlayListItem* to, const bool isClone) const;
    bool IsInSlaveMode() const;
    bool IsSuppressAudioOnSlaves() const;
    std::string ReplaceTags(const std::string s) const;

public:
    static std::string GetTagHint();

#pragma region Constructors and Destructors
    PlayListItem(wxXmlNode* node);
    PlayListItem();
    virtual ~PlayListItem(){};
    PlayListItem* Clone() const;
    virtual PlayListItem* Copy(const bool isClone) const = 0;
#pragma endregion Constructors and Destructors

#pragma region Getters and Setters
    std::string GetType() const {
        return _type;
    }
    bool GetRestOfStep() const {
        return _restOfStep;
    }
    void SetRestOfStep(bool restOfStep) {
        if (_restOfStep != restOfStep) {
            _restOfStep = restOfStep;
            _changeCount++;
        }
    }
    virtual bool HasIP() const {
        return false;
    }
    wxUint32 GetId() const {
        return _id;
    }
    virtual size_t GetDurationMS() const {
        if (_restOfStep)
            return _stepLengthMS - _delay;
        else
            return _delay;
    }
    virtual size_t GetDurationMS(size_t frameMS) const {
        return GetDurationMS();
    }
    bool IsDirty() const {
        return _lastSavedChangeCount != _changeCount;
    }
    void ClearDirty() {
        _lastSavedChangeCount = _changeCount;
    }
    void SetDirty() {
        _changeCount++;
    }
    std::string GetName() const;
    std::string GetRawName() const {
        return _name;
    }
    virtual std::string GetNameNoTime() const;
    void SetName(const std::string& name) {
        if (_name != name) {
            _name = name;
            _changeCount++;
        }
    }
    virtual long GetLength() {
        return _frames;
    }
    long GetDelay() const {
        return _delay;
    }
    int GetCurrentFrame() const {
        return _currentFrame;
    }
    void SetDelay(long delay) {
        if (_delay != delay) {
            _delay = delay;
            _changeCount++;
        }
    }
    int GetVolume() const {
        return _volume;
    }
    void SetVolume(int volume) {
        if (_volume != volume) {
            _volume = volume;
            _changeCount++;
        }
    }
    virtual bool ControlsTiming() const {
        return false;
    }
    virtual size_t GetPositionMS() const {
        return 0;
    }
    virtual size_t GetFrameMS() const {
        return 50;
    }
    size_t GetPriority() const {
        return _priority;
    }
    void SetPriority(size_t priority) {
        if (_priority != priority) {
            _priority = priority;
            _changeCount++;
        }
    }
    virtual bool Done() const {
        return false;
    }
    virtual void Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe) = 0;
    virtual std::string GetSyncItemFSEQ() const {
        return "";
    }
    virtual std::string GetSyncItemMedia() {
        return "";
    }
    virtual std::string GetTitle() const = 0;
    virtual std::list<std::string> GetMissingFiles() {
        return std::list<std::string>();
    }
    virtual long GetFSEQChannels() const {
        return 0;
    }
    void SetStepLength(long stepLengthMS) {
        _stepLengthMS = stepLengthMS;
    }
    virtual bool SetPosition(size_t frame, size_t ms);
    ScheduleOptions* GetOptions() const;
    std::string GetLocalIP() const;
#pragma endregion Getters and Setters

    virtual wxXmlNode* Save() = 0;
    virtual void Load(wxXmlNode* node);

#pragma region Playing
    virtual void Start(long stepLengthMS) {
        _stepLengthMS = stepLengthMS;
    }
    virtual void Stop() {
    }
    virtual void Restart() {
    }
    virtual void Pause(bool pause) {
    }
    virtual void Suspend(bool suspend) {
    }
    virtual bool Advance(int seconds) {
        return false;
    }
#pragma endregion Playing

#pragma region UI
    // returns nullptr if cancelled
    virtual void Configure(wxNotebook* notebook) = 0;
#pragma endregion UI
};
