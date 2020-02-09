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

#include "Output.h"
#include "serial.h"

class SerialOutput : public Output
{
protected:

    #pragma region Member Variables
    SerialPort* _serial = nullptr;
    char _serialConfig[4];
    wxLongLong _dieTime = 0;
    #pragma endregion

    #pragma region Private Functions
    virtual void Save(wxXmlNode* node) override;
    void SetDontDieUntil(wxLongLong dieTime) { _dieTime = dieTime; }
    #pragma endregion

public:

    #pragma region Constructors and Destructors
    SerialOutput(wxXmlNode* node);
    SerialOutput(SerialOutput* output);
    SerialOutput();
    virtual ~SerialOutput() override;
    virtual wxXmlNode* Save() override;
    #pragma endregion 

    #pragma region Static Functions
    static std::list<std::string> GetAvailableSerialPorts();
    static std::list<std::string> GetPossibleSerialPorts();
    static std::list<std::string> GetPossibleBaudRates();
    #pragma endregion

    #pragma region Getters and Setters
    virtual bool IsIpOutput() const override { return false; }
    virtual bool IsSerialOutput() const override { return true; }

    virtual std::string GetLongDescription() const override;

    virtual int GetMaxChannels() const override { return 0; }

    virtual bool AllowsBaudRateSetting() const { return true; }

    virtual size_t TxNonEmptyCount() const override;
    virtual bool TxEmpty() const override;

    int GetId() const { return _universe; }
    void SetId(int id) { if (_universe != id) { _universe = id; _dirty = true; } }

    virtual std::string GetSortName() const override { return GetCommPort(); }

    PINGSTATE Ping() const;
    #pragma endregion

    #pragma region Operators
    bool operator==(const SerialOutput& output) const;
    #pragma endregion 

    #pragma region Start and Stop
    virtual bool Open() override;
    virtual void Close() override;
    #pragma endregion 

    #pragma region Frame Handling
    virtual void StartFrame(long msec) override;
    #pragma endregion 
};
