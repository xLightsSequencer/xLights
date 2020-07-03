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
class wxWindow;

class PlayListItemARTNetTrigger : public PlayListItem
{
protected:

    #pragma region Member Variables
    int _oem;
    int _key;
    int _subkey;
    std::string _data;
    std::string _ip;
    bool _started;
    #pragma endregion Member Variables

    unsigned char* PrepareData(const std::string s, int& size);

public:

    #pragma region Constructors and Destructors
    PlayListItemARTNetTrigger(wxXmlNode* node);
    PlayListItemARTNetTrigger();
    virtual ~PlayListItemARTNetTrigger() {};
    virtual PlayListItem* Copy() const override;
    #pragma endregion Constructors and Destructors

    #pragma region Getters and Setters
    static std::string GetTooltip();
    std::string GetNameNoTime() const override;
    void SetOEM(const int oem) { if (_oem != oem) { _oem = oem; _changeCount++; } }
    void SetKey(const int key) { if (_key != key) { _key = key; _changeCount++; } }
    void SetSubKey(const int subkey) { if (_subkey != subkey) { _subkey = subkey; _changeCount++; } }
    void SetIP(const std::string ip) { if (_ip != ip) { _ip = ip; _changeCount++; } }
    void SetData(const std::string data) { if (_data != data) { _data = data; _changeCount++; } }
    int GetOEM() const { return _oem; }
    int GetKey() const { return _key; }
    int GetSubKey() const { return _subkey; }
    std::string GetIP() const { return _ip; }
    std::string GetData() const { return _data; }
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

