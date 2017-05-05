#ifndef IPOUTPUT_H
#define IPOUTPUT_H

#include "Output.h"

class IPOutput : public Output
{
protected:
    static std::string __localIP;

    virtual void Save(wxXmlNode* node) override;

public:

    #pragma region Constructors and Destructors
    IPOutput(wxXmlNode* node);
    IPOutput();
    virtual ~IPOutput() override {};
    #pragma endregion Constructors and Destructors

    #pragma region Static Functions
    static bool IsIPValid(const std::string &ip);
    static bool IsIPValidOrHostname(const std::string &ip, bool iponly = false);
    static std::string CleanupIP(const std::string &ip);
    static void SetLocalIP(const std::string& localIP) { __localIP = localIP; }
    static std::string GetLocalIP() { return __localIP; }
    #pragma endregion Static Functions

    #pragma region Getters and Setters
    virtual bool IsIpOutput() const override { return true; }
    virtual bool IsSerialOutput() const override { return false; }
    #pragma endregion Getters and Setters

    #pragma region Operators
    bool operator==(const IPOutput& output) const;
    #pragma endregion Operators
    
    virtual wxXmlNode* Save() override;

    #pragma region Start and Stop
    virtual bool Open() override { return Output::Open(); }
    #pragma endregion Start and Stop
};

 #endif
