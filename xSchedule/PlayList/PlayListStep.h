#ifndef PLAYLISTSTEP_H
#define PLAYLISTSTEP_H
#include <list>
#include <string>
#include <wx/wx.h>

class wxXmlNode;
class PlayListItem;
class wxWindow;

class PlayListStep
{
protected:

#pragma region Member Variables
    std::list<PlayListItem*> _items;
    int _lastSavedChangeCount;
    int _changeCount;
    std::string _name;
    bool _excludeFromRandom;
    size_t _framecount;
    wxLongLong _startTime;
    wxLongLong _pause;
    wxLongLong _id;
    wxLongLong _suspend;
    int _loops;
#pragma endregion Member Variables

    PlayListItem* GetTimeSource(int& ms) const;
    std::string FormatTime(size_t timems, bool ms = false) const;

public:

#pragma region Constructors and Destructors
    PlayListStep(wxXmlNode* node);
    PlayListStep();
    virtual ~PlayListStep();
    PlayListStep(const PlayListStep& step);
#pragma endregion Constructors and Destructors

    bool operator==(const PlayListStep& rhs) const { return _id == rhs._id; }

#pragma region Getters and Setters
    std::list<PlayListItem*> GetItems();
    bool IsDirty() const;
    void ClearDirty();
    std::string GetStatus(bool ms = false) const;
    bool GetExcludeFromRandom() const { return _excludeFromRandom; }
    void SetExcludeFromRandom(bool efr) { _excludeFromRandom = efr; _changeCount++; }
    std::string GetName() const;
    std::string GetNameNoTime() const;
    std::string GetRawName() const { return _name; }
    void SetName(const std::string& name) { _name = name; _changeCount++; }
    void Start(int _loops);
    int GetLoopsLeft() const { return _loops; }
    void DoLoop() { _loops--; }
    bool IsMoreLoops() { return _loops > 0; }
    void SetLoops(int loops) { _loops = loops; }
    bool IsPaused() const { return _pause != 0; }
    void Stop();
    void Suspend(bool suspend);
    void StartSuspended();
    void Restart();
    void Pause(bool pause);
    int GetPlayStepSize() const { return _items.size(); }
    void AddItem(PlayListItem* item) { _items.push_back(item); _items.sort(); _changeCount++; }
    void RemoveItem(PlayListItem* item);
    bool Frame(wxByte* buffer, size_t size);
    size_t GetPosition() const;
    size_t GetLengthMS() const;
    size_t GetFrameMS() const;
    void AdjustTime(wxTimeSpan by);
    #pragma endregion Getters and Setters

    wxXmlNode* Save();
    void Load(wxXmlNode * node);
};
#endif
