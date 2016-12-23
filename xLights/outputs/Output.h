#ifndef OUTPUT_H
#define OUTPUT_H

#include "Controller.h"
#include <wx/window.h>

//#define USECHANGEDETECTION

class OutputManager;

#define OUTPUT_E131 "E131"
#define OUTPUT_ARTNET "ArtNet"
#define OUTPUT_DMX "DMX"
#define OUTPUT_NULL "NULL"
#define OUTPUT_PIXELNET "Pixelnet"
#define OUTPUT_OPENPIXELNET "Pixelnet-Open"
#define OUTPUT_LOR "LOR"
#define OUTPUT_DLIGHT "D-Light"
#define OUTPUT_RENARD "Renard"
#define OUTPUT_OPENDMX "OpenDMX"

class Output
{
protected:
    bool _dirty;
    std::string _description;
    std::string _ip;
    std::string _commPort;
    int _channels;
    int _baudRate;
    int _universe;
    bool _enabled;
    Controller* _controller;
    int _outputNumber;
    int _nullNumber;
    int _startChannel;
    long _timer_msec;
    bool _ok;

// This suppresses duplicate packets
#ifdef USECHANGEDETECTION
    #define MAXDUPLICATESTOSUPPRESS 10
    bool _changed;
    int _skipCount;
#endif

    virtual void Save(wxXmlNode* node);

public:
    Output(wxXmlNode* node);
    Output(Output* output);
    Output();
    virtual ~Output();

    virtual std::list<Output*> GetOutputs() const { std::list<Output*> res; return res; }
    static std::string XmlSafe(const std::string& s);
    static std::string UnXmlSafe(const std::string& s);
    static Output* Create(wxXmlNode* node);
    bool operator==(const Output& output) const;
    bool IsDirty() const { return _dirty; }
    void ClearDirty() { _dirty = false; }
    int GetStartChannel() const { return _startChannel; }
    int GetActualEndChannel() const { return _startChannel + _channels - 1; }
    virtual int GetEndChannel() const { return _startChannel + _channels - 1; }
    std::string GetDescription() const { return _description; }
    void SetDescription(const std::string& description) { _description = description; _dirty = true; }
    std::string GetIP() const { return _ip; }
    void SetIP(const std::string& ip);
    std::string GetCommPort() const { return _commPort; }
    void SetCommPort(const std::string& commPort) { _commPort = commPort; _dirty = true; }
    int GetChannels() const { return _channels; }
    void SetChannels(int channels) { _channels = channels; _dirty = true; }
    int GetUniverse() const { return _universe; }
    void SetUniverse(int universe) { _universe = universe; _dirty = true; }
    virtual int GetUniverses() const { return 1; }
    int GetBaudRate() const { return _baudRate; }
    void SetBaudRate(int baudRate) { _baudRate = baudRate; _dirty = true; }
    bool IsEnabled() const { return _enabled; }
    void Enable(bool enable) { _enabled = enable; _dirty = true; }
    void SetController(const std::string& id);
    Controller* GetController() const { return _controller; }
    int GetOutputNumber() const { return _outputNumber; }
    virtual void SetTransientData(int on, int startChannel, int nullnumber);
    long GetTimer() const { return _timer_msec; }
    virtual wxXmlNode* Save() = 0;
    bool IsOk() { return _ok; }
    virtual bool Open();
    virtual void Close() = 0;
    virtual void StartFrame(long msec) { _timer_msec = msec; }
    virtual void EndFrame() = 0;
    virtual void ResetFrame() {}
    virtual void SetOneChannel(int channel, unsigned char data) = 0;
    virtual void SetManyChannels(int channel, unsigned char data[], size_t size);
    virtual void AllOff() = 0;
    virtual std::string GetType() const = 0;
    virtual void SendHeartbeat() const {}
    virtual std::string GetLongDescription() const = 0;
    virtual bool IsIpOutput() const = 0; 
    virtual bool IsOutputable() const { return true; }
    virtual Output* GetActualOutput(int startChannel) { return this; }
    virtual bool IsOutputCollection() const { return false; }
    virtual std::string GetChannelMapping(int ch) const = 0;
    virtual size_t TxNonEmptyCount() const = 0;
    virtual bool TxEmpty() const = 0;
    virtual int GetMaxChannels() const = 0;
    virtual bool IsValidChannelCount(int channelCount) const = 0;

#ifndef EXCLUDENETWORKUI
    virtual int Configure(wxWindow* parent, OutputManager& outputManager) = 0;
#endif
};

 #endif