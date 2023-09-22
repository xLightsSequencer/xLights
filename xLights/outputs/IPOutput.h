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

#include <map>

#include "Output.h"

class IPOutput : public Output
{
protected:

    #pragma region Private Functions
    virtual void Save(wxXmlNode* node) override;
    #pragma endregion

public:

    #pragma region Constructors and Destructors
    IPOutput(wxXmlNode* node, bool isActive);
    IPOutput();
    IPOutput(const IPOutput& from);
    virtual ~IPOutput() override {};
    virtual wxXmlNode* Save() override;
    #pragma endregion 

    #pragma region Static Functions
    static Output::PINGSTATE Ping(const std::string& ip, const std::string& proxy);
    #pragma endregion 

    #pragma region Getters and Setters
    virtual void SetIP(const std::string& ip, bool isActive) override;

    virtual bool IsIpOutput() const override { return true; }
    virtual bool IsSerialOutput() const override { return false; }

    virtual std::string GetSortName() const override { return GetIP(); }
    #pragma endregion 

    #pragma region Operators
    bool operator==(const IPOutput& output) const;
    #pragma endregion 
    
    #pragma region Start and Stop
    virtual bool Open() override { return Output::Open(); }
    #pragma endregion 
};
