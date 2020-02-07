#pragma once

#include <map>

#include "Output.h"

class IPOutput : public Output
{
protected:

    #pragma region Private Functions
    virtual void Save(wxXmlNode* node) override;
    #pragma endregion

public:

    #pragma region Static Members
    static std::string __localIP;
    #pragma endregion

    #pragma region Constructors and Destructors
    IPOutput(wxXmlNode* node);
    IPOutput();
    IPOutput(IPOutput* output) : Output(output) {};
    virtual ~IPOutput() override {};
    virtual wxXmlNode* Save() override;
    #pragma endregion 

    #pragma region Static Functions
    static void SetLocalIP(const std::string& localIP) { __localIP = localIP; }
    static std::string GetLocalIP() { return __localIP; }
    static Output::PINGSTATE Ping(const std::string& ip, const std::string& proxy);
    #pragma endregion 

    #pragma region Getters and Setters
    virtual void SetIP(const std::string& ip) override;

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
