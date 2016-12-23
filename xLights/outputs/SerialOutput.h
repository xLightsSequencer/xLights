#ifndef SERIALOUTPUT_H
#define SERIALOUTPUT_H

#include "Output.h"
#include "../serial.h"

class SerialOutput : public Output
{
protected:
    SerialPort* _serial;
    char _serialConfig[4];
    std::string _setType;

    void Save(wxXmlNode* node);

public:
    static std::string GetAvailableSerialPorts();
    static SerialOutput* Mutate(SerialOutput* output);
    static int ConfigureWithMutate(wxWindow* parent, OutputManager& outputManager, SerialOutput** output);
    SerialOutput(wxXmlNode* node);
    SerialOutput(SerialOutput* output);
    SerialOutput();
    virtual ~SerialOutput() override;

    bool operator==(const SerialOutput& output) const;
    virtual bool IsIpOutput() const override { return false; }
    virtual bool Open() override;
    virtual std::string GetChannelMapping(int ch) const override;
    virtual std::string GetLongDescription() const override;
    virtual size_t TxNonEmptyCount() const override;
    virtual bool TxEmpty() const override;
    virtual void Close() override;
    virtual int GetMaxChannels() const override { return 0; }

#ifndef EXCLUDENETWORKUI
    virtual int Configure(wxWindow* parent, OutputManager& outputManager) override;
#endif
};

 #endif