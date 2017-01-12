#ifndef PLAYLISTITEM_H
#define PLAYLISTITEM_H

#include <string>
#include <wx/wx.h>

class wxXmlNode;
class wxNotebook;

class PlayListItem
{
protected:

    #pragma region Member Variables
    bool _dirty;
    std::string _name;
    long _frames;
    int _msPerFrame;
    long _delay;
    size_t _priority;
    int _volume;
    #pragma endregion Member Variables

    void Save(wxXmlNode* node);
    void Copy(PlayListItem* to) const;

public:

    #pragma region Constructors and Destructors
    PlayListItem(wxXmlNode* node);
    PlayListItem();
    virtual ~PlayListItem() {};
    virtual PlayListItem* Copy() const = 0;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    virtual size_t GetDurationMS() const { return _delay; }
    virtual size_t GetDurationMS(size_t frameMS) const { return GetDurationMS(); }
    bool IsDirty() const { return _dirty; }
    void ClearDirty() { _dirty = false; }
    std::string GetName() const;
    virtual std::string GetNameNoTime() const;
    void SetName(const std::string& name) { _name = name; _dirty = true; }
    virtual long GetLength() { return _frames; }
    long GetDelay() const { return _delay; }
    void SetDelay(long delay) { _delay = delay; _dirty = true; }
    int GetVolume() const { return _volume; }
    void SetVolume(int volume) { _volume = volume; _dirty = true; }
    virtual bool ControlsTiming() const { return false; }
    virtual size_t GetPositionMS() const { return 0; }
    virtual size_t GetFrameMS() const { return 0; }
    size_t GetPriority() const { return _priority; }
    void SetPriority(size_t priority) { _priority = priority; _dirty = true; }
    virtual bool Done() const { return false; }
    virtual void Frame(wxByte* buffer, size_t size, size_t ms, size_t framems) = 0;
    #pragma endregion Getters and Setters

    virtual wxXmlNode* Save() = 0;
    virtual void Load(wxXmlNode* node);

    #pragma region Playing
    virtual void Start() {}
    virtual void Stop() {}
    virtual void Restart() {}
    virtual void Pause(bool pause) {}
    #pragma endregion Playing

    #pragma region UI
    // returns nullptr if cancelled
    virtual void Configure(wxNotebook* notebook) = 0;
    #pragma endregion UI
};
#endif