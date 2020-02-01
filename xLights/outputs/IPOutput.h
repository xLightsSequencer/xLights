#pragma once

#include <map>

#include "Output.h"

class IPOutput : public Output
{
protected:

    virtual void Save(wxXmlNode* node) override;

public:

    static std::string __localIP;
    static std::map<std::string, std::string> __resolvedIPMap;

    #pragma region Constructors and Destructors
    IPOutput(wxXmlNode* node);
    IPOutput();
    IPOutput(IPOutput* output) : Output(output) {};
    virtual ~IPOutput() override {};
    #pragma endregion Constructors and Destructors

    #pragma region Static Functions
    // These have been moved to util functions
    //static bool IsIPValid(const std::string &ip);
    //static bool IsIPValidOrHostname(const std::string &ip, bool iponly = false);
    static std::string CleanupIP(const std::string &ip);
    static void SetLocalIP(const std::string& localIP) { __localIP = localIP; }
    static std::string GetLocalIP() { return __localIP; }
    static std::string ResolveIP(const std::string& ip);
    #pragma endregion Static Functions

    #pragma region Getters and Setters
    virtual bool IsIpOutput() const override { return true; }
    virtual bool IsSerialOutput() const override { return false; }
    virtual std::string GetPingDescription() const override;
    virtual void SetIP(const std::string& ip) override;
    virtual std::string GetSortName() const override { return GetIP(); }
    #pragma endregion Getters and Setters

    #pragma region Operators
    bool operator==(const IPOutput& output) const;
    #pragma endregion Operators
    
    virtual wxXmlNode* Save() override;
    static PINGSTATE Ping(const std::string& ip, const std::string& proxy);
    //bool CanPing() const override { return (GetIP() != "MULTICAST"); }

    #pragma region Start and Stop
    virtual bool Open() override { return Output::Open(); }
    #pragma endregion Start and Stop
};
