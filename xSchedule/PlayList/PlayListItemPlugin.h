#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "PlayListItem.h"
#include <string>

class wxXmlNode;

class PlayListItemPlugin : public PlayListItem
{
protected:

    #pragma region Member Variables
    std::string _plugin;
    std::string _action;
	std::string _eventParm;
    bool _started;
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    PlayListItemPlugin(wxXmlNode* node);
    PlayListItemPlugin();
    virtual ~PlayListItemPlugin() {};
    virtual PlayListItem* Copy() const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    std::string GetNameNoTime() const override;
    void SetPlugin(const std::string& plugin) { if (_plugin != plugin) { _plugin = plugin; _changeCount++; } }
    void SetAction(const std::string& action) { if (_action != action) { _action = action; _changeCount++; } }
    void SetEventParm(const std::string& eventParm) { if (_eventParm != eventParm) { _eventParm = eventParm; _changeCount++; } }
    std::string GetPlugin() const { return _plugin; }
    std::string GetAction() const { return _action; }
    std::string GetEventParm() const { return _eventParm; }
    virtual std::string GetTitle() const override;
    #pragma endregion Getters and Setters

    virtual wxXmlNode* Save() override;
    void Load(wxXmlNode* node) override;

    #pragma region Playing
    virtual void Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe) override;
    virtual void Start(long stepLengthMS) override;
    #pragma endregion Playing

    #pragma region UI
    virtual void Configure(wxNotebook* notebook) override;
#pragma endregion UI
};
