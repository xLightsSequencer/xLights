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
    bool _dirty;
    std::string _name;
    bool _excludeFromRandom;
    size_t _framecount;
    wxLongLong _startTime;
    wxLongLong _pause;
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

#pragma region Getters and Setters
    std::list<PlayListItem*> GetItems();
    bool IsDirty() const;
    void ClearDirty();
    std::string GetStatus(bool ms = false) const;
    bool GetExcludeFromRandom() const { return _excludeFromRandom; }
    void SetExcludeFromRandom(bool efr) { _excludeFromRandom = efr; _dirty = true; }
    std::string GetName() const;
    std::string GetNameNoTime() const;
    std::string GetRawName() const { return _name; }
    void SetName(const std::string& name) { _name = name; _dirty = true; }
    void Start();
    void Stop();
    void Restart();
    void Pause(bool pause);
    int GetPlayStepSize() const { return _items.size(); }
    void AddItem(PlayListItem* item) { _items.push_back(item); _items.sort(); _dirty = true; }
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
