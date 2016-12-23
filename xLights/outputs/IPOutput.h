#ifndef IPOUTPUT_H
#define IPOUTPUT_H

#include "Output.h"

class IPOutput : public Output
{
protected:
    virtual void Save(wxXmlNode* node) override;

public:
    IPOutput(wxXmlNode* node);
    IPOutput();
    virtual ~IPOutput() override;

    static bool IsIPValid(std::string ip);
    static std::string CleanupIP(std::string ip);
    bool operator==(const IPOutput& output) const;
    virtual bool IsIpOutput() const override { return true; } 
    virtual size_t TxNonEmptyCount() const override { return 0; }
    virtual bool TxEmpty() const override { return true; }
    virtual bool Open() override { return Output::Open(); }
};

 #endif