#ifndef PLAYLISTSTEP_H
#define PLAYLISTSTEP_H
#include <list>
#include <string>

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
#pragma endregion Member Variables

public:

#pragma region Constructors and Destructors
    PlayListStep(wxXmlNode* node);
    PlayListStep();
    virtual ~PlayListStep();
#pragma endregion Constructors and Destructors

#pragma region Getters and Setters
    std::list<PlayListItem*> GetItems() const { return _items; }
    bool IsDirty() const;
    void ClearDirty() { _dirty = false; }
    std::string GetName() const;
    void SetName(const std::string& name) { _name = name; _dirty = true; }
    void Play();
    int GetPlayStepSize() const { return _items.size(); }
    void AddItem(PlayListItem* item) { _items.push_back(item); _dirty = true; }
    void RemoveItem(PlayListItem* item);
    #pragma endregion Getters and Setters

    wxXmlNode* Save();
    void Load(wxXmlNode * node);
};
#endif
