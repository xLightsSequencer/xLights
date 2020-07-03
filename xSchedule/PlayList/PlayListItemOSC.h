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
#include "../OSCPacket.h"

class wxXmlNode;

#define MAXOSCPARMS 5

class PlayListItemOSC : public PlayListItem
{
protected:

#pragma region Member Variables
    std::string _path;
    std::string _ip;
    int _port;
    bool _started;
    OSCTYPE _types[MAXOSCPARMS];
    std::string _values[MAXOSCPARMS];
#pragma endregion Member Variables

    OSCTYPE EncodeType(const std::string type) const;
    std::string DecodeType(OSCTYPE type) const;
    std::string SubstituteVariables(const std::string value);

public:

    static int GetMaxParams() { return MAXOSCPARMS; }

#pragma region Constructors and Destructors
    PlayListItemOSC(wxXmlNode* node);
    PlayListItemOSC();
    virtual ~PlayListItemOSC() {};
    virtual PlayListItem* Copy() const override;
#pragma endregion Constructors and Destructors

#pragma region Getters and Setters
    virtual bool HasIP() const override { return true; }
    std::string GetIP() const { return _ip; }
    static std::string GetTooltip();
    std::string GetNameNoTime() const override;
    void SetIPAddress(const std::string& ip) { if (_ip != ip) { _ip = ip; _changeCount++; } }
    void SetPath(const std::string& path) { if (_path != path) { _path = path; _changeCount++; } }
    void SetPort(int port) { if (_port != port) { _port = port; _changeCount++; } }
    void SetOSCType(int num, OSCTYPE type) { if (_types[num] != type) { _types[num] = type; _changeCount++; } }
    void SetOSCType(int num, const std::string& type) { if (_types[num] != EncodeType(type)) { _types[num] = EncodeType(type); _changeCount++; } }
    void SetValue(int num, const std::string value) { if (_values[num] != value) { _values[num] = value; _changeCount++; } }
    std::string GetIPAddress() const { return _ip; }
    std::string GetPath() const { return _path; }
    int GetPort() const { return _port; }
    OSCTYPE GetOSCType(int num) const { return _types[num]; }
    std::string GetTypeString(int num) const { return DecodeType(_types[num]); }
    std::string GetValue(int num) const { return _values[num]; }
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
