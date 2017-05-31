#ifndef PLAYLISTITEMRUNPROCESS_H
#define PLAYLISTITEMRUNPROCESS_H

#include "PlayListItem.h"
#include <string>

class wxXmlNode;
class wxWindow;

class PlayListItemRunProcess : public PlayListItem
{
protected:

    #pragma region Member Variables
    std::string _command;
    bool _waitForCompletion;
    bool _started;
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    PlayListItemRunProcess(wxXmlNode* node);
    PlayListItemRunProcess();
    virtual ~PlayListItemRunProcess() {};
    virtual PlayListItem* Copy() const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    static std::string GetTooltip();
    std::string GetNameNoTime() const override;
    std::string GetRawName() const { return _name; }
    void SetCommand(const std::string& command) { if (_command != command) { _command = command; _changeCount++; } }
    std::string GetCommand() const { return _command; }
    bool GetWaitForCompletion() const { return _waitForCompletion; }
    void SetWaitForCompletion(bool waitForCompletion) {
        if (_waitForCompletion != waitForCompletion) { _waitForCompletion = waitForCompletion; _changeCount++; }
    }
    virtual std::string GetTitle() const override;
    #pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;
    void Load(wxXmlNode* node) override;

    #pragma region Playing
    virtual void Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe) override;
    virtual void Start() override;
    #pragma endregion Playing

    #pragma region UI
    virtual void Configure(wxNotebook* notebook) override;
#pragma endregion UI
};
#endif