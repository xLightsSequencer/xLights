#ifndef IPOUTPUT_H
#define IPOUTPUT_H

#include "Output.h"

class IPOutput : public Output
{
protected:

    virtual void Save(wxXmlNode* node) override;

public:

    static std::string __localIP;

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
    #pragma endregion Static Functions

    #pragma region Getters and Setters
    virtual bool IsIpOutput() const override { return true; }
    virtual bool IsSerialOutput() const override { return false; }
    virtual std::string GetPingDescription() const override;
    virtual void SetIP(const std::string& ip) override;

    #pragma endregion Getters and Setters

    #pragma region Operators
    bool operator==(const IPOutput& output) const;
    #pragma endregion Operators
    
    virtual wxXmlNode* Save() override;
    PINGSTATE Ping() const override;
    static PINGSTATE Ping(const std::string ip);
    bool CanPing() const override { return (GetIP() != "MULTICAST"); }

    #pragma region Start and Stop
    virtual bool Open() override { return Output::Open(); }
    #pragma endregion Start and Stop
};

 #endif
