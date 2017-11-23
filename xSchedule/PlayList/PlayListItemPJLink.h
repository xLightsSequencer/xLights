#ifndef PLAYLISTITEMPJLINK_H
#define PLAYLISTITEMPJLINK_H

#include "PlayListItem.h"
#include <string>
#include <list>

class wxXmlNode;
class wxWindow;
class AudioManager;
class wxSocketClient;
class Projector;

class PlayListItemPJLink : public PlayListItem
{
protected:

    #pragma region Member Variables
    std::string _command;
    std::string _parameter;
    std::string _projector;
    bool _started;
    wxSocketClient* _socket;
    std::string _hash;
    #pragma endregion Member Variables

    void Logout();
    bool Login();
    bool SendCommand(const std::string& command);
    void ExecutePJLinkCommand();

public:

    #pragma region Constructors and Destructors
    PlayListItemPJLink(wxXmlNode* node);
    PlayListItemPJLink();
    virtual ~PlayListItemPJLink() { };
    virtual PlayListItem* Copy() const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    std::string GetCommand() const { return _command; }
    void SetCommand(std::string command) { if (_command != command) { _command = command; _changeCount++; } }
    virtual std::string GetNameNoTime() const override;
    std::string GetParameter() const { return _parameter; }
    std::string GetProjector() const { return _projector; }
    void SetParameter(const std::string& parameter) { if (_parameter != parameter) { _parameter = parameter; _changeCount++; } }
    void SetProjector(const std::string& projector) { if (_projector != projector) { _projector = projector; _changeCount++; } }
    std::list<Projector*> GetProjectors();
    virtual std::string GetTitle() const override;
    #pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;
    void Load(wxXmlNode* node) override;

    #pragma region Playing
    virtual void Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe) override;
    virtual void Start(long stepLengthMS) override;
    #pragma endregion Playing

#pragma region UI
    virtual void Configure(wxNotebook* notebook) override;
#pragma endregion UI
};
#endif