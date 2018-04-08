#ifndef PLAYLISTSTEP_H
#define PLAYLISTSTEP_H
#include <list>
#include <string>
#include <wx/wx.h>
#include <mutex>

class OutputManager;
class PlayListItemText;
class wxXmlNode;
class PlayListItem;
class wxWindow;
class AudioManager;

class PlayListStep
{
protected:

#pragma region Member Variables
    std::recursive_mutex _itemLock;
    std::list<PlayListItem*> _items;
    int _lastSavedChangeCount;
    int _changeCount;
    std::string _name;
    bool _excludeFromRandom;
    size_t _framecount;
    wxUint32 _startTime;
    wxUint32 _pause;
    wxUint32 _id;
    wxUint32 _suspend;
    int _loops;
#pragma endregion Member Variables

    std::string FormatTime(size_t timems, bool ms = false) const;
    AudioManager* GetAudioManager(PlayListItem* pli) const;

public:

#pragma region Constructors and Destructors
    PlayListStep(OutputManager* outputManager, wxXmlNode* node);
    PlayListStep();
    virtual ~PlayListStep();
    PlayListStep(const PlayListStep& step);
#pragma endregion Constructors and Destructors

    bool operator==(const PlayListStep& rhs) const { return _id == rhs._id; }

#pragma region Getters and Setters
    PlayListItemText* GetTextItem(const std::string& name);
    wxUint32 GetId() const { return _id; }
    PlayListItem* GetTimeSource(size_t& ms);
    std::list<PlayListItem*> GetItems();
    bool IsDirty();
    void ClearDirty();
    std::string GetStatus(bool ms = false);
    bool GetExcludeFromRandom() const { return _excludeFromRandom; }
    void SetExcludeFromRandom(bool efr) { if (_excludeFromRandom != efr) { _excludeFromRandom = efr; _changeCount++; } }
    std::string GetName();
    std::string GetNameNoTime();
    std::string GetRawName() const { return _name; }
    void SetName(const std::string& name) { if (_name != name) { _name = name; _changeCount++; } }
    void Start(int _loops);
    bool IsSimple();
    int GetLoopsLeft() const { return _loops; }
    void DoLoop() { _loops--; }
    bool IsMoreLoops() const { return _loops > 0; }
    void SetLoops(int loops) { _loops = loops; }
    bool IsPaused() const { return _pause != 0; }
    void Stop();
    void Suspend(bool suspend);
    void Restart();
    void Pause(bool pause);
    std::string GetActiveSyncItemFSEQ();
    std::string GetActiveSyncItemMedia();
    int GetPlayStepSize() const { return _items.size(); }
    void AddItem(PlayListItem* item) { _items.push_back(item); _items.sort(); _changeCount++; }
    void RemoveItem(PlayListItem* item);
    bool Frame(wxByte* buffer, size_t size, bool outputframe);
    size_t GetPosition();
    PlayListItem* GetItem(const std::string item);
    PlayListItem* GetItem(const wxUint32 id);
    size_t GetLengthMS();
    size_t GetFrameMS();
    void AdjustTime(wxTimeSpan by);
    bool IsRunningFSEQ(const std::string& fseqFile);
    void SetSyncPosition(size_t ms, bool force = false);
    PlayListItem* FindRunProcessNamed(const std::string& item);
    AudioManager* GetAudioManager();
    #pragma endregion Getters and Setters

    wxXmlNode* Save();
    void Load(OutputManager* outputManager, wxXmlNode * node);
};
#endif
