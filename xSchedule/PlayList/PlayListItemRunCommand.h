#ifndef PLAYLISTITEMRUNCOMMAND_H
#define PLAYLISTITEMRUNCOMMAND_H

#include "PlayListItem.h"
#include <string>
#include <list>

class wxXmlNode;
class wxWindow;

class PlayListItemRunCommand : public PlayListItem
{
protected:

    #pragma region Member Variables
    std::string _command;
    std::string _parm1;
    std::string _parm2;
    std::string _parm3;
    bool _started;
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    PlayListItemRunCommand(wxXmlNode* node);
    PlayListItemRunCommand();
    virtual ~PlayListItemRunCommand() {};
    virtual PlayListItem* Copy() const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    std::string GetNameNoTime() const override;
    std::string GetRawName() const { return _name; }
    void SetCommand(const std::string& command) {
        if (_command != command) { _command = command; _changeCount++; }
    }
    std::string GetCommand() const { return _command; }
    void SetParm1(const std::string& parm) {
        if (_parm1 != parm) { _parm1 = parm; _changeCount++; }
    }
    std::string GetParm1() const { return _parm1; }
    void SetParm2(const std::string& parm) {
        if (_parm2 != parm) { _parm2 = parm; _changeCount++; }
    }
    std::string GetParm2() const { return _parm2; }
    void SetParm3(const std::string& parm) {
        if (_parm3 != parm) { _parm3 = parm; _changeCount++; }
    }
    std::string GetParm3() const { return _parm3; }
    std::list<std::string> GetCommands() const;
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