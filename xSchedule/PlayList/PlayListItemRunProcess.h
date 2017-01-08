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
    std::string GetNameNoTime() const override;
    std::string GetRawName() const { return _name; }
    void SetCommand(const std::string& command) { _command = command; }
    std::string GetCommand() const { return _command; }
    bool GetWaitForCompletion() const { return _waitForCompletion; }
    void SetWaitForCompletion(bool waitForCompletion) { _waitForCompletion = waitForCompletion; }
    #pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;
    void Load(wxXmlNode* node) override;

    #pragma region Playing
    virtual void Frame(wxByte* buffer, size_t size, size_t ms, size_t framems) override;
    virtual void Start() override;
    #pragma endregion Playing

    #pragma region UI
    // returns nullptr if cancelled
    virtual void Configure(wxNotebook* notebook) override;
    #pragma endregion UI
};
#endif