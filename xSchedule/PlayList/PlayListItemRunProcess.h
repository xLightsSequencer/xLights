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
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    PlayListItemRunProcess(wxXmlNode* node);
    PlayListItemRunProcess();
    virtual ~PlayListItemRunProcess() {};
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    void SetCommand(const std::string& command) { _command = command; }
    std::string GetCommand() const { return _command; }
    bool GetWaitForCompletion() const { return _waitForCompletion; }
    void SetWaitForCompletion(bool waitForCompletion) { _waitForCompletion = waitForCompletion; }
    #pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;
    void Load(wxXmlNode* node) override;

    #pragma region Playing
    virtual void Play() override {};
    virtual void Stop() override {};
    virtual void PlayFrame(long frame) override {};
    virtual wxByte* GetFrameData(long frame) override { return nullptr; }
    #pragma endregion Playing

#pragma region UI
    // returns nullptr if cancelled
    virtual void Configure(wxNotebook* notebook) override;
#pragma endregion UI
};
#endif