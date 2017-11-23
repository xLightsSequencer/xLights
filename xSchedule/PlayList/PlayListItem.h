#ifndef PLAYLISTITEM_H
#define PLAYLISTITEM_H

#include <string>
#include <list>
#include <wx/wx.h>
#include <wx/notebook.h>

class wxXmlNode;

class PlayListItem
{
protected:

    #pragma region Member Variables
    wxUint32 _id;
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
    #pragma endregion Member Variables

    void Save(wxXmlNode* node);
    void Copy(PlayListItem* to) const;
    bool IsInSlaveMode() const;

public:

    #pragma region Constructors and Destructors
    PlayListItem(wxXmlNode* node);
    PlayListItem();
    virtual ~PlayListItem() {};
    virtual PlayListItem* Copy() const = 0;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    wxUint32 GetId() const { return _id; }
    virtual size_t GetDurationMS() const { return _delay; }
    virtual size_t GetDurationMS(size_t frameMS) const { return GetDurationMS(); }
    bool IsDirty() const { return _lastSavedChangeCount != _changeCount; }
    void ClearDirty() { _lastSavedChangeCount = _changeCount; }
    std::string GetName() const;
    virtual std::string GetNameNoTime() const;
    void SetName(const std::string& name) { if (_name != name) { _name = name; _changeCount++; } }
    virtual long GetLength() { return _frames; }
    long GetDelay() const { return _delay; }
    int GetCurrentFrame() const { return _currentFrame; }
    void SetDelay(long delay) { if (_delay != delay) { _delay = delay; _changeCount++; } }
    int GetVolume() const { return _volume; }
    void SetVolume(int volume) { if (_volume != volume) { _volume = volume; _changeCount++; } }
    virtual bool ControlsTiming() const { return false; }
    virtual size_t GetPositionMS() const { return 0; }
    virtual size_t GetFrameMS() const { return 50; }
    size_t GetPriority() const { return _priority; }
    void SetPriority(size_t priority) { if (_priority != priority) { _priority = priority; _changeCount++; } }
    virtual bool Done() const { return false; }
    virtual void Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe) = 0;
    virtual std::string GetSyncItemFSEQ() const { return ""; }
    virtual std::string GetSyncItemMedia() { return ""; }
    virtual std::string GetTitle() const = 0;
    virtual std::list<std::string> GetMissingFiles() { return std::list<std::string>(); }
    virtual long GetFSEQChannels() const { return 0; }
    #pragma endregion Getters and Setters

    virtual wxXmlNode* Save() = 0;
    virtual void Load(wxXmlNode* node);

    #pragma region Playing
    virtual void Start(long stepLengthMS) { _stepLengthMS = stepLengthMS; }
    virtual void Stop() {}
    virtual void Restart() {}
    virtual void Pause(bool pause) {}
    virtual void Suspend(bool suspend) {}
    #pragma endregion Playing

    #pragma region UI
    // returns nullptr if cancelled
    virtual void Configure(wxNotebook* notebook) = 0;
    #pragma endregion UI
};
#endif