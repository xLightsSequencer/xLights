#ifndef PLAYLIST_H
#define PLAYLIST_H
#include <list>
#include <map>

class wxXmlNode;
class PlayListStep;
class wxWindow;

class PlayList
{
protected:

    #pragma region Member Variables
    std::list<PlayListStep*> _steps;
    bool _dirty;
    std::string _name;
    bool _firstOnlyOnce;
    bool _lastOnlyOnce;
    PlayListStep* _currentStep;
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    PlayList(wxXmlNode* node);
    PlayList();
    virtual ~PlayList() {};
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    std::list<PlayListStep*> GetSteps() const { return _steps;  }
    bool IsDirty() const;
    void ClearDirty() { _dirty = false; }
    std::string GetName() const { return _name; }
    void SetName(const std::string& name) { _name = name; _dirty = true; }
    bool GetFirstOnce() const
    { return _firstOnlyOnce; }
    void SetFirstOnce(bool foo) { _firstOnlyOnce = foo; _dirty = true; }
    bool GetLastOnce() const
    { return _lastOnlyOnce; }
    void SetLastOnce(bool foo) { _lastOnlyOnce = foo; _dirty = true; }
    PlayListStep* PlayNextStep();
    PlayListStep* PlayPriorStep();
    PlayListStep* PlayStep(const std::string& stepName);
    PlayListStep* GetStep(int n) const;
    int GetPlayListSize() const { return _steps.size(); }
    void AddStep(PlayListStep* item, int pos);
    #pragma endregion Getters and Setters

    wxXmlNode* Save();
    void Load(wxXmlNode * node);
    
    #pragma region UI
    // returns nullptr if cancelled
    PlayList* Configure(wxWindow* parent);
    #pragma endregion UI

};
#endif